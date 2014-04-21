constant short2 MARKER = (short2)(-32768, -32768);

kernel void featureTransformPass1(global short2 *sites, global short2 *transform, short w, short h) {
  int w = get_global_size(0);
  int infinity = w + h + 1;
  infinity *= infinity;
  int stride = w;
    
  int x = get_global_id(0);
  
  // scan 1:
  global short2 *src = sites + x;
  global short2 *p = transform + x;

  // - start with the primitive value
  short2 last = MARKER;
    
  if (src->x >= 0) {
    last = *p = *src;
  } else {
    last = *p = MARKER;
  }

  int y = 1;
  // - calculate the next one from the previous one(s)
  for (p += stride, src += stride; y < h; p += stride, src += stride, ++y) {
    if (src->x >= 0) {
      last = *p = *src;
      // scan backwards
      int rDistance = 1;
      int ry = y - 1;
      for (global short2 *q = p - stride; ry >= 0; q -= stride, --ry) {
        int rd = ry - q->y;
        if (rd <= rDistance) {
          break;
        } else {
          *q = last;                    
        }
        rDistance++;
      }
    } else {
      *p = last;
    }
  }
}

kernel void featureTransformPass1_edges(global short2 *sites, global short2 *transform, short w, short h) {
  short w = get_global_size(0);
  short infinity = w + h + 1;
  short stride = w;
    
  short x = get_global_id(0);
  
  // scan 1:
  global short2 *src = sites + x;
  global short2 *p = transform + x;

  // - start with the primitive value
  short2 last;
    
  if (src->x >= 0) {
    last = *p = *src;
  } else {
    last = *p = (short2)( x, -1 ); 
  }

  int y = 0;
  // - calculate the next one from the previous one(s)
  for (p += stride, src += stride, y = 1; y < h; p += stride, src += stride, ++y) {
    if (src->x >= 0) {
      last = *p = *src;
      // scan backwards
      int rDistance = 1;
      int ry = y - 1;
      for (global short2 *q = p - stride; ry >= 0; q -= stride, --ry) {
        int rd = ry - q->y;
        if (rd <= rDistance) {
          break;
        } else {
          *q = last;                    
        }
        rDistance++;
      }
    } else {
      *p = last;
    }
  }
    
  // scan backwards from the max-y edge
  int rDistance = 1;
  int ry = h - 1;
  for (global short2 *q = p - stride; y >= 0; q -= stride, --y) {
    int rd = ry - q->y;
    if (rd <= rDistance) {
      break;
    } else {
      *q = last;                    
    }
    rDistance++;
  }
}

inline short meijsterSeparation(global short2 *gr, short y, short ix, short ux) {
  short2 i = gr[ix] - (short2)( ix, y );
  short2 u = gr[ux] - (short2)( ux, y );
  return (u.x*u.x - i.x*i.x + u.y*u.y - i.y*i.y) / (2 * (u.x - i.x));
}

inline int distanceFromColumn(global short2 *gr, short y, short column, short x) {
  short dy = y - gr[column].y;
  short dx = x - column;
  return dx*dx + dy*dy;
}

kernel void featureTransformPass2(global short2 *g, global short2 *stacks, global short2 *transform, short w, short h) {
  short y = get_global_id(0);

  // In the stack, .x indicates the start of that parabola's extent; .y indicates the column where the parabola originates.
  global short2 *stack = stacks + (y * w);
  global short2 *dst = transform + (y * w);
  global short2 *gr = g + (y * w);

  short q = 0;
  stack[0] = (short2)( 0, 0 );
  
  // scan 3
  for (int u = 1; u < w; u++) {
    while (q >= 0 && distanceFromColumn(gr, y, stack[q].y, stack[q].x) > distanceFromColumn(gr, y, u, stack[q].x)) {
      q--;
    }
    
    if (q < 0) {
      q = 0;
      stack[0] = (short2)( 0, u );
    } else {
      // calculate the 'sep' function:
      int start = 1 + meijsterSeparation(gr, y, stack[q].y, u);
      if (start < w) {
        q++;
        stack[q] = (short2)( w, u );
      }
    }
  }
}

kernel void featureTransformPass3(global short2 *stacks, global short2 *transform, short w, short h) {
  short y = get_global_id(0);

  // In the stack, .x indicates the start of that parabola's extent; .y indicates the column where the parabola originates.
  global short2 *stack = stacks + (y * w);
  global short2 *dst = transform + (y * w);

  // scan 4
  short2 nearest = stack[q];
  for (int u = w - 1; u >= 0; u--) {
    dst[u] = nearest;
    if (u == nearest.x) {
      nearest = stack[--q];
    }
  }
}

kernel void featuresToDistance(global short2 *feature, global uint *distance, short w, short h) {
  short x = get_global_id(0);
  short y = get_global_id(1);
  if (x < w && y < h) {
    int offset = w*y + x;
    int2 d = ((short2)(x, y)) - feature[offset];
    d = d * d;
    distance[offset] = d.x + d.y;
  }
}

