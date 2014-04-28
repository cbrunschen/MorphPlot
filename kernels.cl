constant short2 MARKER = (short2)(-2, -2);

#define BLOCK_DIM 16
// This kernel is optimized to ensure all global reads and writes are coalesced,
// and to avoid bank conflicts in shared memory.  This kernel is up to 11x faster
// than the naive kernel below.  Note that the shared memory array is sized to
// (BLOCK_DIM+1)*BLOCK_DIM.  This pads each row of the 2D block in shared memory
// so that bank conflicts do not occur when threads address the array column-wise.
kernel __attribute__((reqd_work_group_size(BLOCK_DIM, BLOCK_DIM, 1)))
void transpose(global short2 *in, global short2 *out, int w, int h, local short2 *temp)
{
  // read the matrix tile into shared memory
  short x = get_global_id(0);
  short y = get_global_id(1);
  short lx = get_local_id(0);
  short ly = get_local_id(1);
  
  if ((x < w) && (y < h)) {
    unsigned int src = y * w + x;
    temp[ly * (BLOCK_DIM+1) + lx] = in[src];
  }
  
  barrier(CLK_LOCAL_MEM_FENCE);
  
  // write the transposed matrix tile to global memory, transposing X and Y as well!
  x = get_group_id(1) * BLOCK_DIM + lx;
  y = get_group_id(0) * BLOCK_DIM + ly;
  if ((x < h) && (y < w)) {
    unsigned int dst = y * h + x;
    out[dst] = temp[lx * (BLOCK_DIM+1) + ly].yx;
  }
}

kernel void featureTransformPass1(global short2 *sites, global short2 *transform, short w, short h) {
  short x = get_global_id(0);
  if (x >= w) {
    return;
  }
  
  short stride = w;
  
  // - start with the primitive value
  short2 last = MARKER;
  
  // scan 1:
  int y = 0;
  global short2 *src = sites + x;
  global short2 *p = transform + x;
  // - calculate the next one from the previous one(s)
  for (; y < h; p += stride, src += stride, ++y) {
    if (src->x >= 0) {
      last = *p = *src;
    } else {
      *p = last;
    }
  }
  
  y = h - 1;
  p = transform + x + y * stride;
  last = *p;
  for (p -= stride, --y; y >= 0; p -= stride, --y) {
    if (p->x >= 0 && abs(p->y - y) <= abs(last.y - y)) {
      last = *p;
    } else {
      *p = last;
    }
  }
}

kernel void featureTransformPass1_edges(global short2 *sites, global short2 *transform, short w, short h) {
  short x = get_global_id(0);
  if (x >= w) {
    return;
  }
  
  short stride = w;
  
  // - start with the primitive value
  short2 last = (short2)( x, -1 );
  
  // scan 1:
  int y = 0;
  global short2 *src = sites + x;
  global short2 *p = transform + x;
  // - calculate the next one from the previous one(s)
  for (; y < h; p += stride, src += stride, ++y) {
    if (src->x >= 0) {
      last = *p = *src;
    } else {
      *p = last;
    }
  }
  
  y = h - 1;
  p = transform + x + y * stride;
  last = (short2)( x, h );
  for (; y >= 0; p -= stride, --y) {
    if (abs(p->y - y) <= abs(last.y - y)) {
      last = *p;
    } else {
      *p = last;
    }
  }
}

inline short intersection(short ix, short iy, short ux, int uy) {
  return (ux*ux - ix*ix + uy*uy - iy*iy) / (2 * (ux - ix));
}

inline short meijsterSeparation(global short2 *g, short y, short i, short u) {
  return intersection(i, g[i].y - y, u, g[u].y - y);
}

inline int distanceFromColumn(global short2 *gr, short y, short column, short x) {
  short dy = y - gr[column].y;
  short dx = x - column;
  return dx*dx + dy*dy;
}

kernel void featureTransformPass2(global short2 *g, global short2 *stacks, short w, short h) {
  short y = get_global_id(0);
  if (y >= h) {
    return;
  }

  global short2 *stack = stacks + (y * w);
  global short2 *dst = g + (y * w);
  global short2 *gr = g + (y * w);
  
  // scan 3
  short u;
  for (u = 0; u < w && gr[u].x < 0; u++);
  
  short q = 0;
  short stackTopStart = 0;
  stack[0] = gr[u].y;

  for (int u = 1; u < w; u++) {
    if (gr[u].x < 0) {
      continue;
    }

    while (q >= 0 && distanceFromColumn(gr, y, stack[q].x, stackTopStart) > distanceFromColumn(gr, y, u, stackTopStart)) {
      q--;
      if (q > 0) {
        stackTopStart = 1 + intersection(stack[q-1].x, stack[q-1].y - y, stack[q].x, stack[q].y - y);
      } else {
        stackTopStart = 0;
      }
    }
    
    if (q < 0) {
      q = 0;
      stack[0] = gr[u];
    } else {
      // calculate the 'sep' function:
      short start = 1 + intersection(stack[q].x, stack[q].y - y, gr[u].x, gr[u].y - y);
      if (start < w) {
        ++q;
        stack[q] = gr[u];
        stackTopStart = start;
      }
    }
  }
  
  // scan 4
  for (int u = w - 1; u >= 0; u--) {
    if (u < stackTopStart) {
      --q;
      if (q > 0) {
        stackTopStart = 1 + intersection(stack[q-1].x, stack[q-1].y - y, stack[q].x, stack[q].y - y);
      } else {
        stackTopStart = -1;
      }
    }

    dst[u] = stack[q];
  }
}

kernel void featuresToDistance(global short2 *feature, global uint *distance, short w, short h) {
  short x = get_global_id(0);
  short y = get_global_id(1);
  if (x < w && y < h) {
    int offset = w*y + x;
    int2 d = convert_int2(((short2)(x, y)) - feature[offset]);
    d = d * d;
    distance[offset] = d.x + d.y;
  }
}

kernel void distanceToEdges(global uint *distance, short w, short h) {
  short x = get_global_id(0);
  short y = get_global_id(1);
  if (x < w && y < h) {
    int offset = w*y + x;
    distance[offset] = min(distance[offset], min(min((uint)(w - x), (uint)(x + 1)), min((uint)(h - y), (uint)(y + 1))));
  }
}

