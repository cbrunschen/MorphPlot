"\
constant short2 MARKER = (short2)(-32768, -32768);\n\
\n\
kernel void zerosToSites(global uchar *src, global short2 *sites) {\n\
  short x = get_global_id(0);\n\
  short y = get_global_id(1);\n\
  short w = get_global_size(1);\n\
  short offset = y*w + x;\n\
\n\
  if (src[offset] == 0) {\n\
    sites[offset] = (short2)( x, y );\n\
  } else {\n\
    sites[offset] = MARKER;\n\
  }\n\
}\n\
\n\
kernel void nonZerosToSites(global uchar *src, global short2 *sites) {\n\
  short x = get_global_id(0);\n\
  short y = get_global_id(1);\n\
  short w = get_global_size(1);\n\
  short offset = y*w + x;\n\
\n\
  if (src[offset] != 0) {\n\
    sites[offset] = (short2)( x, y );\n\
  } else {\n\
    sites[offset] = MARKER;\n\
  }\n\
}\n\
\n\
kernel void distanceTransformPass1(global short2 *sites, global short2 *transform, int h) {\n\
  int w = get_global_size(0);\n\
  int infinity = w + h + 1;\n\
  infinity *= infinity;\n\
  int stride = w;\n\
    \n\
  int x = get_global_id(0);\n\
  \n\
  // scan 1:\n\
  global short2 *src = sites + x;\n\
  global short2 *p = transform + x;\n\
\n\
  // - start with the primitive value\n\
  short2 last = MARKER;\n\
    \n\
  if (src->x >= 0) {\n\
    last = *p = *src;\n\
  } else {\n\
    last = *p = MARKER;\n\
  }\n\
\n\
  int y = 1;\n\
  // - calculate the next one from the previous one(s)\n\
  for (p += stride, src += stride; y < h; p += stride, src += stride, ++y) {\n\
    if (src->x >= 0) {\n\
      last = *p = *src;\n\
      // scan backwards\n\
      int rDistance = 1;\n\
      int ry = y - 1;\n\
      for (global short2 *q = p - stride; ry >= 0; q -= stride, --ry) {\n\
        int rd = ry - q->y;\n\
        if (rd <= rDistance) {\n\
          break;\n\
        } else {\n\
          *q = last;                    \n\
        }\n\
        rDistance++;\n\
      }\n\
    } else {\n\
      *p = last;\n\
    }\n\
  }\n\
}\n\
\n\
kernel void distanceTransformPass1_edges(global short2 *sites, global short2 *transform, short h) {\n\
  short w = get_global_size(0);\n\
  short infinity = w + h + 1;\n\
  short stride = w;\n\
    \n\
  short x = get_global_id(0);\n\
  \n\
  // scan 1:\n\
  global short2 *src = sites + x;\n\
  global short2 *p = transform + x;\n\
\n\
  // - start with the primitive value\n\
  short2 last;\n\
    \n\
  if (src->x >= 0) {\n\
    last = *p = *src;\n\
  } else {\n\
    last = *p = (short2)( x, -1 ); \n\
  }\n\
\n\
  int y = 0;\n\
  // - calculate the next one from the previous one(s)\n\
  for (p += stride, src += stride, y = 1; y < h; p += stride, src += stride, ++y) {\n\
    if (src->x >= 0) {\n\
      last = *p = *src;\n\
      // scan backwards\n\
      int rDistance = 1;\n\
      int ry = y - 1;\n\
      for (global short2 *q = p - stride; ry >= 0; q -= stride, --ry) {\n\
        int rd = ry - q->y;\n\
        if (rd <= rDistance) {\n\
          break;\n\
        } else {\n\
          *q = last;                    \n\
        }\n\
        rDistance++;\n\
      }\n\
    } else {\n\
      *p = last;\n\
    }\n\
  }\n\
    \n\
  // scan backwards from the max-y edge\n\
  int rDistance = 1;\n\
  int ry = h - 1;\n\
  for (global short2 *q = p - stride; y >= 0; q -= stride, --y) {\n\
    int rd = ry - q->y;\n\
    if (rd <= rDistance) {\n\
      break;\n\
    } else {\n\
      *q = last;                    \n\
    }\n\
    rDistance++;\n\
  }\n\
}\n\
\n\
inline short meijsterSeparation(global short2 *gr, short y, short ix, short ux) {\n\
  short2 i = gr[ix] - (short2)( ix, y );\n\
  short2 u = gr[ux] - (short2)( ux, y );\n\
  return (u.x*u.x - i.x*i.x + u.y*u.y - i.y*i.y) / (2 * (u.x - i.x));\n\
}\n\
\n\
inline int distanceFromColumn(global short2 *gr, short y, short column, short x) {\n\
  short dy = y - gr[column].y;\n\
  short dx = x - column;\n\
  return dx*dx + dy*dy;\n\
}\n\
\n\
kernel void distanceTransformPass2(global short2 *g, global short2 *stacks, global short2 *transform, int w) {\n\
  short y = get_global_id(0);\n\
\n\
  // In the stack, .x indicates the start of that parabola\'s extent; .y indicates the column where the parabola originates.\n\
  global short2 *stack = stacks + (y * w);\n\
  global short2 *dst = transform + (y * w);\n\
  global short2 *gr = g + (y * w);\n\
\n\
  short q = 0;\n\
  stack[0] = (short2)( 0, 0 );\n\
  \n\
  // scan 3\n\
  for (int u = 1; u < w; u++) {\n\
    while (q >= 0 && distanceFromColumn(gr, y, stack[q].y, stack[q].x) > distanceFromColumn(gr, y, u, stack[q].x)) {\n\
      q--;\n\
    }\n\
    \n\
    if (q < 0) {\n\
      q = 0;\n\
      stack[0] = (short2)( 0, u );\n\
    } else {\n\
      // calculate the \'sep\' function:\n\
      int start = 1 + meijsterSeparation(gr, y, stack[q].y, u);\n\
      if (start < w) {\n\
        q++;\n\
        stack[q] = (short2)( w, u );\n\
      }\n\
    }\n\
  }\n\
  \n\
  // scan 4\n\
  short2 nearest = stack[q];\n\
  for (int u = w - 1; u >= 0; u--) {\n\
    dst[u] = nearest;\n\
    if (u == nearest.x) {\n\
      nearest = stack[--q];\n\
    }\n\
  }\n\
}\n\
"
