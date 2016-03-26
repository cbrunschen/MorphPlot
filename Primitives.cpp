/*
 *  Primitives.cpp
 *  Morph
 *
 *  Created by Christian Brunschen on 06/11/2010.
 *  Copyright 2010 Christian Brunschen. All rights reserved.
 *
 */

#include "Primitives.h"

using namespace std;

namespace Primitives {
#if 0
}
#endif

const char Hex::hexchars[] = "0123456789abcdef";

namespace Neighbourhood {
#if 0
}
#endif

int xOffsets[DIRECTIONS+1] = {
  -1, 0, 1, 1, 1, 0, -1, -1, 0,
};
int yOffsets[DIRECTIONS+1] = {
  -1, -1, -1, 0, 1, 1, 1, 0, 0,
};

bool singleConnected[] = {
  /*   0 */ false, //
  /*   1 */ true,  // 0
  /*   2 */ true,  // 1
  /*   3 */ true,  // 01
  /*   4 */ true,  // 2
  /*   5 */ false, // 0 2
  /*   6 */ true,  // 12
  /*   7 */ true,  // 012
  /*   8 */ true,  // 3
  /*   9 */ false, // 0 3
  /*  10 */ true,  // 13
  /*  11 */ true,  // 013
  /*  12 */ true,  // 23
  /*  13 */ false, // 0 23
  /*  14 */ true,  // 123
  /*  15 */ true,  // 0123
  /*  16 */ true,  // 4
  /*  17 */ false, // 0 4
  /*  18 */ false, // 1 4
  /*  19 */ false, // 01 4
  /*  20 */ false, // 2 4
  /*  21 */ false, // 0 2 4
  /*  22 */ false, // 12 4
  /*  23 */ false, // 012 4
  /*  24 */ true,  // 34
  /*  25 */ false, // 0 34
  /*  26 */ true,  // 134
  /*  27 */ true,  // 0134
  /*  28 */ true,  // 234
  /*  29 */ false, // 0 234
  /*  30 */ true,  // 1234
  /*  31 */ true,  // 01234
  /*  32 */ true,  // 5
  /*  33 */ false, // 0 5
  /*  34 */ false, // 1 5
  /*  35 */ false, // 01 5
  /*  36 */ false, // 2 5
  /*  37 */ false, // 0 2 5
  /*  38 */ false, // 12 5
  /*  39 */ false, // 012 5
  /*  40 */ true,  // 35
  /*  41 */ false, // 0 35
  /*  42 */ true,  // 135
  /*  43 */ true,  // 0135
  /*  44 */ true,  // 235
  /*  45 */ false, // 0 235
  /*  46 */ true,  // 1235
  /*  47 */ true,  // 01235
  /*  48 */ true,  // 45
  /*  49 */ false, // 0 45
  /*  50 */ false, // 1 45
  /*  51 */ false, // 01 45
  /*  52 */ false, // 2 45
  /*  53 */ false, // 0 2 45
  /*  54 */ false, // 12 45
  /*  55 */ false, // 012 45
  /*  56 */ true,  // 345
  /*  57 */ false, // 0 345
  /*  58 */ true,  // 1345
  /*  59 */ true,  // 01345
  /*  60 */ true,  // 2345
  /*  61 */ false, // 0 2345
  /*  62 */ true,  // 12345
  /*  63 */ true,  // 012345
  /*  64 */ true,  // 6
  /*  65 */ false, // 0 6
  /*  66 */ false, // 1 6
  /*  67 */ false, // 01 6
  /*  68 */ false, // 2 6
  /*  69 */ false, // 0 2 6
  /*  70 */ false, // 12 6
  /*  71 */ false, // 012 6
  /*  72 */ false, // 3 6
  /*  73 */ false, // 0 3 6
  /*  74 */ false, // 13 6
  /*  75 */ false, // 013 6
  /*  76 */ false, // 23 6
  /*  77 */ false, // 0 23 6
  /*  78 */ false, // 123 6
  /*  79 */ false, // 0123 6
  /*  80 */ false, // 4 6
  /*  81 */ false, // 0 4 6
  /*  82 */ false, // 1 4 6
  /*  83 */ false, // 01 4 6
  /*  84 */ false, // 2 4 6
  /*  85 */ false, // 0 2 4 6
  /*  86 */ false, // 12 4 6
  /*  87 */ false, // 012 4 6
  /*  88 */ false, // 34 6
  /*  89 */ false, // 0 34 6
  /*  90 */ false, // 134 6
  /*  91 */ false, // 0134 6
  /*  92 */ false, // 234 6
  /*  93 */ false, // 0 234 6
  /*  94 */ false, // 1234 6
  /*  95 */ false, // 01234 6
  /*  96 */ true,  // 56
  /*  97 */ false, // 0 56
  /*  98 */ false, // 1 56
  /*  99 */ false, // 01 56
  /* 100 */ false, // 2 56
  /* 101 */ false, // 0 2 56
  /* 102 */ false, // 12 56
  /* 103 */ false, // 012 56
  /* 104 */ true,  // 356
  /* 105 */ false, // 0 356
  /* 106 */ true,  // 1356
  /* 107 */ true,  // 01356
  /* 108 */ true,  // 2356
  /* 109 */ false, // 0 2356
  /* 110 */ true,  // 12356
  /* 111 */ true,  // 012356
  /* 112 */ true,  // 456
  /* 113 */ false, // 0 456
  /* 114 */ false, // 1 456
  /* 115 */ false, // 01 456
  /* 116 */ false, // 2 456
  /* 117 */ false, // 0 2 456
  /* 118 */ false, // 12 456
  /* 119 */ false, // 012 456
  /* 120 */ true,  // 3456
  /* 121 */ false, // 0 3456
  /* 122 */ true,  // 13456
  /* 123 */ true,  // 013456
  /* 124 */ true,  // 23456
  /* 125 */ false, // 0 23456
  /* 126 */ true,  // 123456
  /* 127 */ true,  // 0123456
  /* 128 */ true,  // 7
  /* 129 */ true,  // 07
  /* 130 */ true,  // 17
  /* 131 */ true,  // 017
  /* 132 */ false, // 2 7
  /* 133 */ false, // 07 2
  /* 134 */ true,  // 127
  /* 135 */ true,  // 0127
  /* 136 */ false, // 3 7
  /* 137 */ false, // 07 3
  /* 138 */ true,  // 137
  /* 139 */ true,  // 0137
  /* 140 */ false, // 23 7
  /* 141 */ false, // 07 23
  /* 142 */ true,  // 1237
  /* 143 */ true,  // 01237
  /* 144 */ false, // 4 7
  /* 145 */ false, // 07 4
  /* 146 */ false, // 17 4
  /* 147 */ false, // 017 4
  /* 148 */ false, // 2 4 7
  /* 149 */ false, // 07 2 4
  /* 150 */ false, // 127 4
  /* 151 */ false, // 0127 4
  /* 152 */ false, // 34 7
  /* 153 */ false, // 07 34
  /* 154 */ true,  // 1347
  /* 155 */ true,  // 01347
  /* 156 */ false, // 234 7
  /* 157 */ false, // 07 234
  /* 158 */ true,  // 12347
  /* 159 */ true,  // 012347
  /* 160 */ true,  // 57
  /* 161 */ true,  // 057
  /* 162 */ true,  // 157
  /* 163 */ true,  // 0157
  /* 164 */ false, // 2 57
  /* 165 */ false, // 057 2
  /* 166 */ true,  // 1257
  /* 167 */ true,  // 01257
  /* 168 */ true,  // 357
  /* 169 */ true,  // 0357
  /* 170 */ true,  // 1357
  /* 171 */ true,  // 01357
  /* 172 */ true,  // 2357
  /* 173 */ true,  // 02357
  /* 174 */ true,  // 12357
  /* 175 */ true,  // 012357
  /* 176 */ true,  // 457
  /* 177 */ true,  // 0457
  /* 178 */ true,  // 1457
  /* 179 */ true,  // 01457
  /* 180 */ false, // 2 457
  /* 181 */ false, // 0457 2
  /* 182 */ true,  // 12457
  /* 183 */ true,  // 012457
  /* 184 */ true,  // 3457
  /* 185 */ true,  // 03457
  /* 186 */ true,  // 13457
  /* 187 */ true,  // 013457
  /* 188 */ true,  // 23457
  /* 189 */ true,  // 023457
  /* 190 */ true,  // 123457
  /* 191 */ true,  // 0123457
  /* 192 */ true,  // 67
  /* 193 */ true,  // 067
  /* 194 */ true,  // 167
  /* 195 */ true,  // 0167
  /* 196 */ false, // 2 67
  /* 197 */ false, // 067 2
  /* 198 */ true,  // 1267
  /* 199 */ true,  // 01267
  /* 200 */ false, // 3 67
  /* 201 */ false, // 067 3
  /* 202 */ true,  // 1367
  /* 203 */ true,  // 01367
  /* 204 */ false, // 23 67
  /* 205 */ false, // 067 23
  /* 206 */ true,  // 12367
  /* 207 */ true,  // 012367
  /* 208 */ false, // 4 67
  /* 209 */ false, // 067 4
  /* 210 */ false, // 167 4
  /* 211 */ false, // 0167 4
  /* 212 */ false, // 2 4 67
  /* 213 */ false, // 067 2 4
  /* 214 */ false, // 1267 4
  /* 215 */ false, // 01267 4
  /* 216 */ false, // 34 67
  /* 217 */ false, // 067 34
  /* 218 */ true,  // 13467
  /* 219 */ true,  // 013467
  /* 220 */ false, // 234 67
  /* 221 */ false, // 067 234
  /* 222 */ true,  // 123467
  /* 223 */ true,  // 0123467
  /* 224 */ true,  // 567
  /* 225 */ true,  // 0567
  /* 226 */ true,  // 1567
  /* 227 */ true,  // 01567
  /* 228 */ false, // 2 567
  /* 229 */ false, // 0567 2
  /* 230 */ true,  // 12567
  /* 231 */ true,  // 012567
  /* 232 */ true,  // 3567
  /* 233 */ true,  // 03567
  /* 234 */ true,  // 13567
  /* 235 */ true,  // 013567
  /* 236 */ true,  // 23567
  /* 237 */ true,  // 023567
  /* 238 */ true,  // 123567
  /* 239 */ true,  // 0123567
  /* 240 */ true,  // 4567
  /* 241 */ true,  // 04567
  /* 242 */ true,  // 14567
  /* 243 */ true,  // 014567
  /* 244 */ false, // 2 4567
  /* 245 */ false, // 04567 2
  /* 246 */ true,  // 124567
  /* 247 */ true,  // 0124567
  /* 248 */ true,  // 34567
  /* 249 */ true,  // 034567
  /* 250 */ true,  // 134567
  /* 251 */ true,  // 0134567
  /* 252 */ true,  // 234567
  /* 253 */ true,  // 0234567
  /* 254 */ true,  // 1234567
  /* 255 */ true,  // 01234567
};

bool thinConnected[] = {
  /*   0 */ true,  //
  /*   1 */ true,  // 0
  /*   2 */ true,  //  1
  /*   3 */ false, // 01
  /*   4 */ true,  //   2
  /*   5 */ true,  // 0 2
  /*   6 */ false, //  12
  /*   7 */ false, // 012
  /*   8 */ true,  //    3
  /*   9 */ true,  // 0  3
  /*  10 */ true,  //  1 3
  /*  11 */ false, // 01 3
  /*  12 */ false, //   23
  /*  13 */ false, // 0 23
  /*  14 */ false, //  123
  /*  15 */ false, // 0123
  /*  16 */ true,  //     4
  /*  17 */ true,  // 0   4
  /*  18 */ true,  //  1  4
  /*  19 */ false, // 01  4
  /*  20 */ true,  //   2 4
  /*  21 */ false, // 0 2 4
  /*  22 */ false, //  12 4
  /*  23 */ false, // 012 4
  /*  24 */ false, //    34
  /*  25 */ false, // 0  34
  /*  26 */ false, //  1 34
  /*  27 */ false, // 01 34
  /*  28 */ false, //   234
  /*  29 */ false, // 0 234
  /*  30 */ false, //  1234
  /*  31 */ false, // 01234
  /*  32 */ true,  //      5
  /*  33 */ true,  // 0    5
  /*  34 */ true,  //  1   5
  /*  35 */ false, // 01   5
  /*  36 */ true,  //   2  5
  /*  37 */ false, // 0 2  5
  /*  38 */ false, //  12  5
  /*  39 */ false, // 012  5
  /*  40 */ true,  //    3 5
  /*  41 */ false, // 0  3 5
  /*  42 */ false, //  1 3 5
  /*  43 */ false, // 01 3 5
  /*  44 */ false, //   23 5
  /*  45 */ false, // 0 23 5
  /*  46 */ false, //  123 5
  /*  47 */ false, // 0123 5
  /*  48 */ false, //     45
  /*  49 */ false, // 0   45
  /*  50 */ false, //  1  45
  /*  51 */ false, // 01  45
  /*  52 */ false, //   2 45
  /*  53 */ false, // 0 2 45
  /*  54 */ false, //  12 45
  /*  55 */ false, // 012 45
  /*  56 */ false, //    345
  /*  57 */ false, // 0  345
  /*  58 */ false, //  1 345
  /*  59 */ false, // 01 345
  /*  60 */ false, //   2345
  /*  61 */ false, // 0 2345
  /*  62 */ false, //  12345
  /*  63 */ false, // 012345
  /*  64 */ true,  //       6
  /*  65 */ true,  // 0     6
  /*  66 */ true,  //  1    6
  /*  67 */ false, // 01    6
  /*  68 */ true,  //   2   6
  /*  69 */ false, // 0 2   6
  /*  70 */ false, //  12   6
  /*  71 */ false, // 012   6
  /*  72 */ true,  //    3  6
  /*  73 */ false, // 0  3  6
  /*  74 */ false, //  1 3  6
  /*  75 */ false, // 01 3  6
  /*  76 */ false, //   23  6
  /*  77 */ false, // 0 23  6
  /*  78 */ false, //  123  6
  /*  79 */ false, // 0123  6
  /*  80 */ true,  //     4 6
  /*  81 */ false, // 0   4 6
  /*  82 */ false, //  1  4 6
  /*  83 */ false, // 01  4 6
  /*  84 */ false, //   2 4 6
  /*  85 */ false, // 0 2 4 6
  /*  86 */ false, //  12 4 6
  /*  87 */ false, // 012 4 6
  /*  88 */ false, //    34 6
  /*  89 */ false, // 0  34 6
  /*  90 */ false, //  1 34 6
  /*  91 */ false, // 01 34 6
  /*  92 */ false, //   234 6
  /*  93 */ false, // 0 234 6
  /*  94 */ false, //  1234 6
  /*  95 */ false, // 01234 6
  /*  96 */ false, //      56
  /*  97 */ false, // 0    56
  /*  98 */ false, //  1   56
  /*  99 */ false, // 01   56
  /* 100 */ false, //   2  56
  /* 101 */ false, // 0 2  56
  /* 102 */ false, //  12  56
  /* 103 */ false, // 012  56
  /* 104 */ false, //    3 56
  /* 105 */ false, // 0  3 56
  /* 106 */ false, //  1 3 56
  /* 107 */ false, // 01 3 56
  /* 108 */ false, //   23 56
  /* 109 */ false, // 0 23 56
  /* 110 */ false, //  123 56
  /* 111 */ false, // 0123 56
  /* 112 */ false, //     456
  /* 113 */ false, // 0   456
  /* 114 */ false, //  1  456
  /* 115 */ false, // 01  456
  /* 116 */ false, //   2 456
  /* 117 */ false, // 0 2 456
  /* 118 */ false, //  12 456
  /* 119 */ false, // 012 456
  /* 120 */ false, //    3456
  /* 121 */ false, // 0  3456
  /* 122 */ false, //  1 3456
  /* 123 */ false, // 01 3456
  /* 124 */ false, //   23456
  /* 125 */ false, // 0 23456
  /* 126 */ false, //  123456
  /* 127 */ false, // 0123456
  /* 128 */ true,  //        7
  /* 129 */ false, // 0      7
  /* 130 */ true,  //  1     7
  /* 131 */ false, // 01     7
  /* 132 */ true,  //   2    7
  /* 133 */ false, // 0 2    7
  /* 134 */ false, //  12    7
  /* 135 */ false, // 012    7
  /* 136 */ true,  //    3   7
  /* 137 */ false, // 0  3   7
  /* 138 */ false, //  1 3   7
  /* 139 */ false, // 01 3   7
  /* 140 */ false, //   23   7
  /* 141 */ false, // 0 23   7
  /* 142 */ false, //  123   7
  /* 143 */ false, // 0123   7
  /* 144 */ true,  //     4  7
  /* 145 */ false, // 0   4  7
  /* 146 */ false, //  1  4  7
  /* 147 */ false, // 01  4  7
  /* 148 */ false, //   2 4  7
  /* 149 */ false, // 0 2 4  7
  /* 150 */ false, //  12 4  7
  /* 151 */ false, // 012 4  7
  /* 152 */ false, //    34  7
  /* 153 */ false, // 0  34  7
  /* 154 */ false, //  1 34  7
  /* 155 */ false, // 01 34  7
  /* 156 */ false, //   234  7
  /* 157 */ false, // 0 234  7
  /* 158 */ false, //  1234  7
  /* 159 */ false, // 01234  7
  /* 160 */ true,  //      5 7
  /* 161 */ false, // 0    5 7
  /* 162 */ false, //  1   5 7
  /* 163 */ false, // 01   5 7
  /* 164 */ false, //   2  5 7
  /* 165 */ false, // 0 2  5 7
  /* 166 */ false, //  12  5 7
  /* 167 */ false, // 012  5 7
  /* 168 */ false, //    3 5 7
  /* 169 */ false, // 0  3 5 7
  /* 170 */ false, //  1 3 5 7
  /* 171 */ false, // 01 3 5 7
  /* 172 */ false, //   23 5 7
  /* 173 */ false, // 0 23 5 7
  /* 174 */ false, //  123 5 7
  /* 175 */ false, // 0123 5 7
  /* 176 */ false, //     45 7
  /* 177 */ false, // 0   45 7
  /* 178 */ false, //  1  45 7
  /* 179 */ false, // 01  45 7
  /* 180 */ false, //   2 45 7
  /* 181 */ false, // 0 2 45 7
  /* 182 */ false, //  12 45 7
  /* 183 */ false, // 012 45 7
  /* 184 */ false, //    345 7
  /* 185 */ false, // 0  345 7
  /* 186 */ false, //  1 345 7
  /* 187 */ false, // 01 345 7
  /* 188 */ false, //   2345 7
  /* 189 */ false, // 0 2345 7
  /* 190 */ false, //  12345 7
  /* 191 */ false, // 012345 7
  /* 192 */ false, //       67
  /* 193 */ false, // 0     67
  /* 194 */ false, //  1    67
  /* 195 */ false, // 01    67
  /* 196 */ false, //   2   67
  /* 197 */ false, // 0 2   67
  /* 198 */ false, //  12   67
  /* 199 */ false, // 012   67
  /* 200 */ false, //    3  67
  /* 201 */ false, // 0  3  67
  /* 202 */ false, //  1 3  67
  /* 203 */ false, // 01 3  67
  /* 204 */ false, //   23  67
  /* 205 */ false, // 0 23  67
  /* 206 */ false, //  123  67
  /* 207 */ false, // 0123  67
  /* 208 */ false, //     4 67
  /* 209 */ false, // 0   4 67
  /* 210 */ false, //  1  4 67
  /* 211 */ false, // 01  4 67
  /* 212 */ false, //   2 4 67
  /* 213 */ false, // 0 2 4 67
  /* 214 */ false, //  12 4 67
  /* 215 */ false, // 012 4 67
  /* 216 */ false, //    34 67
  /* 217 */ false, // 0  34 67
  /* 218 */ false, //  1 34 67
  /* 219 */ false, // 01 34 67
  /* 220 */ false, //   234 67
  /* 221 */ false, // 0 234 67
  /* 222 */ false, //  1234 67
  /* 223 */ false, // 01234 67
  /* 224 */ false, //      567
  /* 225 */ false, // 0    567
  /* 226 */ false, //  1   567
  /* 227 */ false, // 01   567
  /* 228 */ false, //   2  567
  /* 229 */ false, // 0 2  567
  /* 230 */ false, //  12  567
  /* 231 */ false, // 012  567
  /* 232 */ false, //    3 567
  /* 233 */ false, // 0  3 567
  /* 234 */ false, //  1 3 567
  /* 235 */ false, // 01 3 567
  /* 236 */ false, //   23 567
  /* 237 */ false, // 0 23 567
  /* 238 */ false, //  123 567
  /* 239 */ false, // 0123 567
  /* 240 */ false, //     4567
  /* 241 */ false, // 0   4567
  /* 242 */ false, //  1  4567
  /* 243 */ false, // 01  4567
  /* 244 */ false, //   2 4567
  /* 245 */ false, // 0 2 4567
  /* 246 */ false, //  12 4567
  /* 247 */ false, // 012 4567
  /* 248 */ false, //    34567
  /* 249 */ false, // 0  34567
  /* 250 */ false, //  1 34567
  /* 251 */ false, // 01 34567
  /* 252 */ false, //   234567
  /* 253 */ false, // 0 234567
  /* 254 */ false, //  1234567
  /* 255 */ false, // 01234567
};

#if 0
{
#endif
}


#if 0
{
#endif
}