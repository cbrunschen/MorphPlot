/*
 *  Bitmap.cpp
 *  Morph
 *
 *  Created by Christian Brunschen on 13/11/2010.
 *  Copyright 2010 Christian Brunschen. All rights reserved.
 *
 */

#include "Bitmap.h"
#include "Bitmap_Impl.h"

#include "GreyImage.h"
#include "GreyImage_Impl.h"


namespace Images {
#if 0
}
#endif

bool Bitmap::oldThinningPatterns[256] = {
  /* 00 */  false, false, false, false, false, true,  false, false,
  /* 08 */  false, false, false, false, false, true,  false, false,
  /* 10 */  false, false, false, false, true,  false, true,  false,
  /* 18 */  false, false, false, false, false, false, false, false,
  /* 20 */  false, false, false, false, false, false, false, false,
  /* 28 */  false, false, false, false, false, false, false, false,
  /* 30 */  false, false, false, false, true,  false, false, false,
  /* 38 */  false, false, false, false, false, false, false, false,
  /* 40 */  false, true,  false, true,  false, false, false, false,
  /* 48 */  false, false, false, false, false, false, false, false,
  /* 50 */  true,  false, false, false, false, false, false, false,
  /* 58 */  true,  false, false, false, false, false, false, false,
  /* 60 */  false, true,  false, false, false, false, false, false,
  /* 68 */  false, false, false, false, false, false, false, false,
  /* 70 */  false, false, false, false, false, false, false, false,
  /* 78 */  false, false, false, false, false, false, false, false,
  /* 80 */  false, false, false, false, false, true,  false, false,
  /* 88 */  false, false, false, false, false, false, false, false,
  /* 90 */  false, false, false, false, false, false, false, false,
  /* 98 */  false, false, false, false, false, false, false, false,
  /* a0 */  false, false, false, false, false, false, false, false,
  /* a8 */  false, false, false, false, false, false, false, false,
  /* b0 */  false, false, false, false, false, false, false, false,
  /* b8 */  false, false, false, false, false, false, false, false,
  /* c0 */  false, false, false, false, false, false, false, false,
  /* c8 */  false, false, false, false, false, false, false, false,
  /* d0 */  true,  false, false, false, false, false, false, false,
  /* d8 */  false, false, false, false, false, false, false, false,
  /* e0 */  false, false, false, false, false, false, false, false,
  /* e8 */  false, false, false, false, false, false, false, false,
  /* f0 */  false, false, false, false, false, false, false, false,
  /* f8 */  false, false, false, false, false, false, false, false,
};

bool Bitmap::smoothingPatterns[256] = {
  /* 0 */ false,
  /* 1 */ false,
  /* 2 */ false,
  /* 3 */ false,
  /* 4 */ false,
  /* 5 */ false,
  /* 6 */ false,
  /* 7 */ false,
  /* 8 */ false,
  /* 9 */ false,
  /* 10 */  true, // .1.3....
  /* 11 */  true, // 01.3....
  /* 12 */ false,
  /* 13 */ false,
  /* 14 */ false,
  /* 15 */ false,
  /* 16 */ false,
  /* 17 */ false,
  /* 18 */ false,
  /* 19 */ false,
  /* 20 */ false,
  /* 21 */ false,
  /* 22 */ false,
  /* 23 */ false,
  /* 24 */ false,
  /* 25 */ false,
  /* 26 */  true, // .1.34...
  /* 27 */ false,
  /* 28 */ false,
  /* 29 */ false,
  /* 30 */ false,
  /* 31 */ false,
  /* 32 */ false,
  /* 33 */ false,
  /* 34 */ false,
  /* 35 */ false,
  /* 36 */ false,
  /* 37 */ false,
  /* 38 */ false,
  /* 39 */ false,
  /* 40 */  true, // ...3.5..
  /* 41 */ false,
  /* 42 */ false,
  /* 43 */ false,
  /* 44 */  true, // ..23.5..
  /* 45 */ false,
  /* 46 */ false,
  /* 47 */ false,
  /* 48 */ false,
  /* 49 */ false,
  /* 50 */ false,
  /* 51 */ false,
  /* 52 */ false,
  /* 53 */ false,
  /* 54 */ false,
  /* 55 */ false,
  /* 56 */ false,
  /* 57 */ false,
  /* 58 */ false,
  /* 59 */ false,
  /* 60 */ false,
  /* 61 */ false,
  /* 62 */ false,
  /* 63 */ false,
  /* 64 */ false,
  /* 65 */ false,
  /* 66 */ false,
  /* 67 */ false,
  /* 68 */ false,
  /* 69 */ false,
  /* 70 */ false,
  /* 71 */ false,
  /* 72 */ false,
  /* 73 */ false,
  /* 74 */ false,
  /* 75 */ false,
  /* 76 */ false,
  /* 77 */ false,
  /* 78 */ false,
  /* 79 */ false,
  /* 80 */ false,
  /* 81 */ false,
  /* 82 */ false,
  /* 83 */ false,
  /* 84 */ false,
  /* 85 */ false,
  /* 86 */ false,
  /* 87 */ false,
  /* 88 */ false,
  /* 89 */ false,
  /* 90 */ false,
  /* 91 */ false,
  /* 92 */ false,
  /* 93 */ false,
  /* 94 */ false,
  /* 95 */ false,
  /* 96 */ false,
  /* 97 */ false,
  /* 98 */ false,
  /* 99 */ false,
  /* 100 */ false,
  /* 101 */ false,
  /* 102 */ false,
  /* 103 */ false,
  /* 104 */  true, // ...3.56.
  /* 105 */ false,
  /* 106 */ false,
  /* 107 */ false,
  /* 108 */ false,
  /* 109 */ false,
  /* 110 */ false,
  /* 111 */ false,
  /* 112 */ false,
  /* 113 */ false,
  /* 114 */ false,
  /* 115 */ false,
  /* 116 */ false,
  /* 117 */ false,
  /* 118 */ false,
  /* 119 */ false,
  /* 120 */ false,
  /* 121 */ false,
  /* 122 */ false,
  /* 123 */ false,
  /* 124 */ false,
  /* 125 */ false,
  /* 126 */ false,
  /* 127 */ false,
  /* 128 */ false,
  /* 129 */ false,
  /* 130 */  true, // .1.....7
  /* 131 */ false,
  /* 132 */ false,
  /* 133 */ false,
  /* 134 */  true, // .12....7
  /* 135 */ false,
  /* 136 */ false,
  /* 137 */ false,
  /* 138 */ false,
  /* 139 */ false,
  /* 140 */ false,
  /* 141 */ false,
  /* 142 */ false,
  /* 143 */ false,
  /* 144 */ false,
  /* 145 */ false,
  /* 146 */ false,
  /* 147 */ false,
  /* 148 */ false,
  /* 149 */ false,
  /* 150 */ false,
  /* 151 */ false,
  /* 152 */ false,
  /* 153 */ false,
  /* 154 */ false,
  /* 155 */ false,
  /* 156 */ false,
  /* 157 */ false,
  /* 158 */ false,
  /* 159 */ false,
  /* 160 */  true, // .....5.7
  /* 161 */  true, // 0....5.7
  /* 162 */ false,
  /* 163 */ false,
  /* 164 */ false,
  /* 165 */ false,
  /* 166 */ false,
  /* 167 */ false,
  /* 168 */ false,
  /* 169 */ false,
  /* 170 */ false,
  /* 171 */ false,
  /* 172 */ false,
  /* 173 */ false,
  /* 174 */ false,
  /* 175 */ false,
  /* 176 */  true, // ....45.7
  /* 177 */ false,
  /* 178 */ false,
  /* 179 */ false,
  /* 180 */ false,
  /* 181 */ false,
  /* 182 */ false,
  /* 183 */ false,
  /* 184 */ false,
  /* 185 */ false,
  /* 186 */ false,
  /* 187 */ false,
  /* 188 */ false,
  /* 189 */ false,
  /* 190 */ false,
  /* 191 */ false,
  /* 192 */ false,
  /* 193 */ false,
  /* 194 */  true, // .1....67
  /* 195 */ false,
  /* 196 */ false,
  /* 197 */ false,
  /* 198 */ false,
  /* 199 */ false,
  /* 200 */ false,
  /* 201 */ false,
  /* 202 */ false,
  /* 203 */ false,
  /* 204 */ false,
  /* 205 */ false,
  /* 206 */ false,
  /* 207 */ false,
  /* 208 */ false,
  /* 209 */ false,
  /* 210 */ false,
  /* 211 */ false,
  /* 212 */ false,
  /* 213 */ false,
  /* 214 */ false,
  /* 215 */ false,
  /* 216 */ false,
  /* 217 */ false,
  /* 218 */ false,
  /* 219 */ false,
  /* 220 */ false,
  /* 221 */ false,
  /* 222 */ false,
  /* 223 */ false,
  /* 224 */ false,
  /* 225 */ false,
  /* 226 */ false,
  /* 227 */ false,
  /* 228 */ false,
  /* 229 */ false,
  /* 230 */ false,
  /* 231 */ false,
  /* 232 */ false,
  /* 233 */ false,
  /* 234 */ false,
  /* 235 */ false,
  /* 236 */ false,
  /* 237 */ false,
  /* 238 */ false,
  /* 239 */ false,
  /* 240 */ false,
  /* 241 */ false,
  /* 242 */ false,
  /* 243 */ false,
  /* 244 */ false,
  /* 245 */ false,
  /* 246 */ false,
  /* 247 */ false,
  /* 248 */ false,
  /* 249 */ false,
  /* 250 */ false,
  /* 251 */ false,
  /* 252 */ false,
  /* 253 */ false,
  /* 254 */ false,
  /* 255 */ false,
};

int Bitmap::neighbourCounts[256] = {
  /* 0 */  0, // ........
  /* 1 */  1, // 0.......
  /* 2 */  1, // .1......
  /* 3 */  2, // 01......
  /* 4 */  1, // ..2.....
  /* 5 */  2, // 0.2.....
  /* 6 */  2, // .12.....
  /* 7 */  3, // 012.....
  /* 8 */  1, // ...3....
  /* 9 */  2, // 0..3....
  /* 10 */  2, // .1.3....
  /* 11 */  3, // 01.3....
  /* 12 */  2, // ..23....
  /* 13 */  3, // 0.23....
  /* 14 */  3, // .123....
  /* 15 */  4, // 0123....
  /* 16 */  1, // ....4...
  /* 17 */  2, // 0...4...
  /* 18 */  2, // .1..4...
  /* 19 */  3, // 01..4...
  /* 20 */  2, // ..2.4...
  /* 21 */  3, // 0.2.4...
  /* 22 */  3, // .12.4...
  /* 23 */  4, // 012.4...
  /* 24 */  2, // ...34...
  /* 25 */  3, // 0..34...
  /* 26 */  3, // .1.34...
  /* 27 */  4, // 01.34...
  /* 28 */  3, // ..234...
  /* 29 */  4, // 0.234...
  /* 30 */  4, // .1234...
  /* 31 */  5, // 01234...
  /* 32 */  1, // .....5..
  /* 33 */  2, // 0....5..
  /* 34 */  2, // .1...5..
  /* 35 */  3, // 01...5..
  /* 36 */  2, // ..2..5..
  /* 37 */  3, // 0.2..5..
  /* 38 */  3, // .12..5..
  /* 39 */  4, // 012..5..
  /* 40 */  2, // ...3.5..
  /* 41 */  3, // 0..3.5..
  /* 42 */  3, // .1.3.5..
  /* 43 */  4, // 01.3.5..
  /* 44 */  3, // ..23.5..
  /* 45 */  4, // 0.23.5..
  /* 46 */  4, // .123.5..
  /* 47 */  5, // 0123.5..
  /* 48 */  2, // ....45..
  /* 49 */  3, // 0...45..
  /* 50 */  3, // .1..45..
  /* 51 */  4, // 01..45..
  /* 52 */  3, // ..2.45..
  /* 53 */  4, // 0.2.45..
  /* 54 */  4, // .12.45..
  /* 55 */  5, // 012.45..
  /* 56 */  3, // ...345..
  /* 57 */  4, // 0..345..
  /* 58 */  4, // .1.345..
  /* 59 */  5, // 01.345..
  /* 60 */  4, // ..2345..
  /* 61 */  5, // 0.2345..
  /* 62 */  5, // .12345..
  /* 63 */  6, // 012345..
  /* 64 */  1, // ......6.
  /* 65 */  2, // 0.....6.
  /* 66 */  2, // .1....6.
  /* 67 */  3, // 01....6.
  /* 68 */  2, // ..2...6.
  /* 69 */  3, // 0.2...6.
  /* 70 */  3, // .12...6.
  /* 71 */  4, // 012...6.
  /* 72 */  2, // ...3..6.
  /* 73 */  3, // 0..3..6.
  /* 74 */  3, // .1.3..6.
  /* 75 */  4, // 01.3..6.
  /* 76 */  3, // ..23..6.
  /* 77 */  4, // 0.23..6.
  /* 78 */  4, // .123..6.
  /* 79 */  5, // 0123..6.
  /* 80 */  2, // ....4.6.
  /* 81 */  3, // 0...4.6.
  /* 82 */  3, // .1..4.6.
  /* 83 */  4, // 01..4.6.
  /* 84 */  3, // ..2.4.6.
  /* 85 */  4, // 0.2.4.6.
  /* 86 */  4, // .12.4.6.
  /* 87 */  5, // 012.4.6.
  /* 88 */  3, // ...34.6.
  /* 89 */  4, // 0..34.6.
  /* 90 */  4, // .1.34.6.
  /* 91 */  5, // 01.34.6.
  /* 92 */  4, // ..234.6.
  /* 93 */  5, // 0.234.6.
  /* 94 */  5, // .1234.6.
  /* 95 */  6, // 01234.6.
  /* 96 */  2, // .....56.
  /* 97 */  3, // 0....56.
  /* 98 */  3, // .1...56.
  /* 99 */  4, // 01...56.
  /* 100 */  3, // ..2..56.
  /* 101 */  4, // 0.2..56.
  /* 102 */  4, // .12..56.
  /* 103 */  5, // 012..56.
  /* 104 */  3, // ...3.56.
  /* 105 */  4, // 0..3.56.
  /* 106 */  4, // .1.3.56.
  /* 107 */  5, // 01.3.56.
  /* 108 */  4, // ..23.56.
  /* 109 */  5, // 0.23.56.
  /* 110 */  5, // .123.56.
  /* 111 */  6, // 0123.56.
  /* 112 */  3, // ....456.
  /* 113 */  4, // 0...456.
  /* 114 */  4, // .1..456.
  /* 115 */  5, // 01..456.
  /* 116 */  4, // ..2.456.
  /* 117 */  5, // 0.2.456.
  /* 118 */  5, // .12.456.
  /* 119 */  6, // 012.456.
  /* 120 */  4, // ...3456.
  /* 121 */  5, // 0..3456.
  /* 122 */  5, // .1.3456.
  /* 123 */  6, // 01.3456.
  /* 124 */  5, // ..23456.
  /* 125 */  6, // 0.23456.
  /* 126 */  6, // .123456.
  /* 127 */  7, // 0123456.
  /* 128 */  1, // .......7
  /* 129 */  2, // 0......7
  /* 130 */  2, // .1.....7
  /* 131 */  3, // 01.....7
  /* 132 */  2, // ..2....7
  /* 133 */  3, // 0.2....7
  /* 134 */  3, // .12....7
  /* 135 */  4, // 012....7
  /* 136 */  2, // ...3...7
  /* 137 */  3, // 0..3...7
  /* 138 */  3, // .1.3...7
  /* 139 */  4, // 01.3...7
  /* 140 */  3, // ..23...7
  /* 141 */  4, // 0.23...7
  /* 142 */  4, // .123...7
  /* 143 */  5, // 0123...7
  /* 144 */  2, // ....4..7
  /* 145 */  3, // 0...4..7
  /* 146 */  3, // .1..4..7
  /* 147 */  4, // 01..4..7
  /* 148 */  3, // ..2.4..7
  /* 149 */  4, // 0.2.4..7
  /* 150 */  4, // .12.4..7
  /* 151 */  5, // 012.4..7
  /* 152 */  3, // ...34..7
  /* 153 */  4, // 0..34..7
  /* 154 */  4, // .1.34..7
  /* 155 */  5, // 01.34..7
  /* 156 */  4, // ..234..7
  /* 157 */  5, // 0.234..7
  /* 158 */  5, // .1234..7
  /* 159 */  6, // 01234..7
  /* 160 */  2, // .....5.7
  /* 161 */  3, // 0....5.7
  /* 162 */  3, // .1...5.7
  /* 163 */  4, // 01...5.7
  /* 164 */  3, // ..2..5.7
  /* 165 */  4, // 0.2..5.7
  /* 166 */  4, // .12..5.7
  /* 167 */  5, // 012..5.7
  /* 168 */  3, // ...3.5.7
  /* 169 */  4, // 0..3.5.7
  /* 170 */  4, // .1.3.5.7
  /* 171 */  5, // 01.3.5.7
  /* 172 */  4, // ..23.5.7
  /* 173 */  5, // 0.23.5.7
  /* 174 */  5, // .123.5.7
  /* 175 */  6, // 0123.5.7
  /* 176 */  3, // ....45.7
  /* 177 */  4, // 0...45.7
  /* 178 */  4, // .1..45.7
  /* 179 */  5, // 01..45.7
  /* 180 */  4, // ..2.45.7
  /* 181 */  5, // 0.2.45.7
  /* 182 */  5, // .12.45.7
  /* 183 */  6, // 012.45.7
  /* 184 */  4, // ...345.7
  /* 185 */  5, // 0..345.7
  /* 186 */  5, // .1.345.7
  /* 187 */  6, // 01.345.7
  /* 188 */  5, // ..2345.7
  /* 189 */  6, // 0.2345.7
  /* 190 */  6, // .12345.7
  /* 191 */  7, // 012345.7
  /* 192 */  2, // ......67
  /* 193 */  3, // 0.....67
  /* 194 */  3, // .1....67
  /* 195 */  4, // 01....67
  /* 196 */  3, // ..2...67
  /* 197 */  4, // 0.2...67
  /* 198 */  4, // .12...67
  /* 199 */  5, // 012...67
  /* 200 */  3, // ...3..67
  /* 201 */  4, // 0..3..67
  /* 202 */  4, // .1.3..67
  /* 203 */  5, // 01.3..67
  /* 204 */  4, // ..23..67
  /* 205 */  5, // 0.23..67
  /* 206 */  5, // .123..67
  /* 207 */  6, // 0123..67
  /* 208 */  3, // ....4.67
  /* 209 */  4, // 0...4.67
  /* 210 */  4, // .1..4.67
  /* 211 */  5, // 01..4.67
  /* 212 */  4, // ..2.4.67
  /* 213 */  5, // 0.2.4.67
  /* 214 */  5, // .12.4.67
  /* 215 */  6, // 012.4.67
  /* 216 */  4, // ...34.67
  /* 217 */  5, // 0..34.67
  /* 218 */  5, // .1.34.67
  /* 219 */  6, // 01.34.67
  /* 220 */  5, // ..234.67
  /* 221 */  6, // 0.234.67
  /* 222 */  6, // .1234.67
  /* 223 */  7, // 01234.67
  /* 224 */  3, // .....567
  /* 225 */  4, // 0....567
  /* 226 */  4, // .1...567
  /* 227 */  5, // 01...567
  /* 228 */  4, // ..2..567
  /* 229 */  5, // 0.2..567
  /* 230 */  5, // .12..567
  /* 231 */  6, // 012..567
  /* 232 */  4, // ...3.567
  /* 233 */  5, // 0..3.567
  /* 234 */  5, // .1.3.567
  /* 235 */  6, // 01.3.567
  /* 236 */  5, // ..23.567
  /* 237 */  6, // 0.23.567
  /* 238 */  6, // .123.567
  /* 239 */  7, // 0123.567
  /* 240 */  4, // ....4567
  /* 241 */  5, // 0...4567
  /* 242 */  5, // .1..4567
  /* 243 */  6, // 01..4567
  /* 244 */  5, // ..2.4567
  /* 245 */  6, // 0.2.4567
  /* 246 */  6, // .12.4567
  /* 247 */  7, // 012.4567
  /* 248 */  5, // ...34567
  /* 249 */  6, // 0..34567
  /* 250 */  6, // .1.34567
  /* 251 */  7, // 01.34567
  /* 252 */  6, // ..234567
  /* 253 */  7, // 0.234567
  /* 254 */  7, // .1234567
  /* 255 */  8, // 01234567
};

int Bitmap::transitions[256] = {
  /* 0 */  0, // ........
  /* 1 */  1, // 0.......
  /* 2 */  1, // .1......
  /* 3 */  1, // 01......
  /* 4 */  1, // ..2.....
  /* 5 */  2, // 0.2.....
  /* 6 */  1, // .12.....
  /* 7 */  1, // 012.....
  /* 8 */  1, // ...3....
  /* 9 */  2, // 0..3....
  /* 10 */  2, // .1.3....
  /* 11 */  2, // 01.3....
  /* 12 */  1, // ..23....
  /* 13 */  2, // 0.23....
  /* 14 */  1, // .123....
  /* 15 */  1, // 0123....
  /* 16 */  1, // ....4...
  /* 17 */  2, // 0...4...
  /* 18 */  2, // .1..4...
  /* 19 */  2, // 01..4...
  /* 20 */  2, // ..2.4...
  /* 21 */  3, // 0.2.4...
  /* 22 */  2, // .12.4...
  /* 23 */  2, // 012.4...
  /* 24 */  1, // ...34...
  /* 25 */  2, // 0..34...
  /* 26 */  2, // .1.34...
  /* 27 */  2, // 01.34...
  /* 28 */  1, // ..234...
  /* 29 */  2, // 0.234...
  /* 30 */  1, // .1234...
  /* 31 */  1, // 01234...
  /* 32 */  1, // .....5..
  /* 33 */  2, // 0....5..
  /* 34 */  2, // .1...5..
  /* 35 */  2, // 01...5..
  /* 36 */  2, // ..2..5..
  /* 37 */  3, // 0.2..5..
  /* 38 */  2, // .12..5..
  /* 39 */  2, // 012..5..
  /* 40 */  2, // ...3.5..
  /* 41 */  3, // 0..3.5..
  /* 42 */  3, // .1.3.5..
  /* 43 */  3, // 01.3.5..
  /* 44 */  2, // ..23.5..
  /* 45 */  3, // 0.23.5..
  /* 46 */  2, // .123.5..
  /* 47 */  2, // 0123.5..
  /* 48 */  1, // ....45..
  /* 49 */  2, // 0...45..
  /* 50 */  2, // .1..45..
  /* 51 */  2, // 01..45..
  /* 52 */  2, // ..2.45..
  /* 53 */  3, // 0.2.45..
  /* 54 */  2, // .12.45..
  /* 55 */  2, // 012.45..
  /* 56 */  1, // ...345..
  /* 57 */  2, // 0..345..
  /* 58 */  2, // .1.345..
  /* 59 */  2, // 01.345..
  /* 60 */  1, // ..2345..
  /* 61 */  2, // 0.2345..
  /* 62 */  1, // .12345..
  /* 63 */  1, // 012345..
  /* 64 */  1, // ......6.
  /* 65 */  2, // 0.....6.
  /* 66 */  2, // .1....6.
  /* 67 */  2, // 01....6.
  /* 68 */  2, // ..2...6.
  /* 69 */  3, // 0.2...6.
  /* 70 */  2, // .12...6.
  /* 71 */  2, // 012...6.
  /* 72 */  2, // ...3..6.
  /* 73 */  3, // 0..3..6.
  /* 74 */  3, // .1.3..6.
  /* 75 */  3, // 01.3..6.
  /* 76 */  2, // ..23..6.
  /* 77 */  3, // 0.23..6.
  /* 78 */  2, // .123..6.
  /* 79 */  2, // 0123..6.
  /* 80 */  2, // ....4.6.
  /* 81 */  3, // 0...4.6.
  /* 82 */  3, // .1..4.6.
  /* 83 */  3, // 01..4.6.
  /* 84 */  3, // ..2.4.6.
  /* 85 */  4, // 0.2.4.6.
  /* 86 */  3, // .12.4.6.
  /* 87 */  3, // 012.4.6.
  /* 88 */  2, // ...34.6.
  /* 89 */  3, // 0..34.6.
  /* 90 */  3, // .1.34.6.
  /* 91 */  3, // 01.34.6.
  /* 92 */  2, // ..234.6.
  /* 93 */  3, // 0.234.6.
  /* 94 */  2, // .1234.6.
  /* 95 */  2, // 01234.6.
  /* 96 */  1, // .....56.
  /* 97 */  2, // 0....56.
  /* 98 */  2, // .1...56.
  /* 99 */  2, // 01...56.
  /* 100 */  2, // ..2..56.
  /* 101 */  3, // 0.2..56.
  /* 102 */  2, // .12..56.
  /* 103 */  2, // 012..56.
  /* 104 */  2, // ...3.56.
  /* 105 */  3, // 0..3.56.
  /* 106 */  3, // .1.3.56.
  /* 107 */  3, // 01.3.56.
  /* 108 */  2, // ..23.56.
  /* 109 */  3, // 0.23.56.
  /* 110 */  2, // .123.56.
  /* 111 */  2, // 0123.56.
  /* 112 */  1, // ....456.
  /* 113 */  2, // 0...456.
  /* 114 */  2, // .1..456.
  /* 115 */  2, // 01..456.
  /* 116 */  2, // ..2.456.
  /* 117 */  3, // 0.2.456.
  /* 118 */  2, // .12.456.
  /* 119 */  2, // 012.456.
  /* 120 */  1, // ...3456.
  /* 121 */  2, // 0..3456.
  /* 122 */  2, // .1.3456.
  /* 123 */  2, // 01.3456.
  /* 124 */  1, // ..23456.
  /* 125 */  2, // 0.23456.
  /* 126 */  1, // .123456.
  /* 127 */  1, // 0123456.
  /* 128 */  1, // .......7
  /* 129 */  1, // 0......7
  /* 130 */  2, // .1.....7
  /* 131 */  1, // 01.....7
  /* 132 */  2, // ..2....7
  /* 133 */  2, // 0.2....7
  /* 134 */  2, // .12....7
  /* 135 */  1, // 012....7
  /* 136 */  2, // ...3...7
  /* 137 */  2, // 0..3...7
  /* 138 */  3, // .1.3...7
  /* 139 */  2, // 01.3...7
  /* 140 */  2, // ..23...7
  /* 141 */  2, // 0.23...7
  /* 142 */  2, // .123...7
  /* 143 */  1, // 0123...7
  /* 144 */  2, // ....4..7
  /* 145 */  2, // 0...4..7
  /* 146 */  3, // .1..4..7
  /* 147 */  2, // 01..4..7
  /* 148 */  3, // ..2.4..7
  /* 149 */  3, // 0.2.4..7
  /* 150 */  3, // .12.4..7
  /* 151 */  2, // 012.4..7
  /* 152 */  2, // ...34..7
  /* 153 */  2, // 0..34..7
  /* 154 */  3, // .1.34..7
  /* 155 */  2, // 01.34..7
  /* 156 */  2, // ..234..7
  /* 157 */  2, // 0.234..7
  /* 158 */  2, // .1234..7
  /* 159 */  1, // 01234..7
  /* 160 */  2, // .....5.7
  /* 161 */  2, // 0....5.7
  /* 162 */  3, // .1...5.7
  /* 163 */  2, // 01...5.7
  /* 164 */  3, // ..2..5.7
  /* 165 */  3, // 0.2..5.7
  /* 166 */  3, // .12..5.7
  /* 167 */  2, // 012..5.7
  /* 168 */  3, // ...3.5.7
  /* 169 */  3, // 0..3.5.7
  /* 170 */  4, // .1.3.5.7
  /* 171 */  3, // 01.3.5.7
  /* 172 */  3, // ..23.5.7
  /* 173 */  3, // 0.23.5.7
  /* 174 */  3, // .123.5.7
  /* 175 */  2, // 0123.5.7
  /* 176 */  2, // ....45.7
  /* 177 */  2, // 0...45.7
  /* 178 */  3, // .1..45.7
  /* 179 */  2, // 01..45.7
  /* 180 */  3, // ..2.45.7
  /* 181 */  3, // 0.2.45.7
  /* 182 */  3, // .12.45.7
  /* 183 */  2, // 012.45.7
  /* 184 */  2, // ...345.7
  /* 185 */  2, // 0..345.7
  /* 186 */  3, // .1.345.7
  /* 187 */  2, // 01.345.7
  /* 188 */  2, // ..2345.7
  /* 189 */  2, // 0.2345.7
  /* 190 */  2, // .12345.7
  /* 191 */  1, // 012345.7
  /* 192 */  1, // ......67
  /* 193 */  1, // 0.....67
  /* 194 */  2, // .1....67
  /* 195 */  1, // 01....67
  /* 196 */  2, // ..2...67
  /* 197 */  2, // 0.2...67
  /* 198 */  2, // .12...67
  /* 199 */  1, // 012...67
  /* 200 */  2, // ...3..67
  /* 201 */  2, // 0..3..67
  /* 202 */  3, // .1.3..67
  /* 203 */  2, // 01.3..67
  /* 204 */  2, // ..23..67
  /* 205 */  2, // 0.23..67
  /* 206 */  2, // .123..67
  /* 207 */  1, // 0123..67
  /* 208 */  2, // ....4.67
  /* 209 */  2, // 0...4.67
  /* 210 */  3, // .1..4.67
  /* 211 */  2, // 01..4.67
  /* 212 */  3, // ..2.4.67
  /* 213 */  3, // 0.2.4.67
  /* 214 */  3, // .12.4.67
  /* 215 */  2, // 012.4.67
  /* 216 */  2, // ...34.67
  /* 217 */  2, // 0..34.67
  /* 218 */  3, // .1.34.67
  /* 219 */  2, // 01.34.67
  /* 220 */  2, // ..234.67
  /* 221 */  2, // 0.234.67
  /* 222 */  2, // .1234.67
  /* 223 */  1, // 01234.67
  /* 224 */  1, // .....567
  /* 225 */  1, // 0....567
  /* 226 */  2, // .1...567
  /* 227 */  1, // 01...567
  /* 228 */  2, // ..2..567
  /* 229 */  2, // 0.2..567
  /* 230 */  2, // .12..567
  /* 231 */  1, // 012..567
  /* 232 */  2, // ...3.567
  /* 233 */  2, // 0..3.567
  /* 234 */  3, // .1.3.567
  /* 235 */  2, // 01.3.567
  /* 236 */  2, // ..23.567
  /* 237 */  2, // 0.23.567
  /* 238 */  2, // .123.567
  /* 239 */  1, // 0123.567
  /* 240 */  1, // ....4567
  /* 241 */  1, // 0...4567
  /* 242 */  2, // .1..4567
  /* 243 */  1, // 01..4567
  /* 244 */  2, // ..2.4567
  /* 245 */  2, // 0.2.4567
  /* 246 */  2, // .12.4567
  /* 247 */  1, // 012.4567
  /* 248 */  1, // ...34567
  /* 249 */  1, // 0..34567
  /* 250 */  2, // .1.34567
  /* 251 */  1, // 01.34567
  /* 252 */  1, // ..234567
  /* 253 */  1, // 0.234567
  /* 254 */  1, // .1234567
  /* 255 */  0, // 01234567
};

int Bitmap::corners[] = {
  /* ...345.. */ 0x38,
  /* .....567 */ 0xe0,
  /* .123.... */ 0x0e,
  /* 01.....7 */ 0x83
};

namespace Bitmaps {
#if 0
}
#endif

#define PNG_BYTES_TO_CHECK 8
shared_ptr<Bitmap> readPng(FILE *fp) {
  shared_ptr< GreyImage<uint8_t> > greyImage = GreyImage<uint8_t>::readPng(fp);
  shared_ptr<Bitmap> result(NULL);
  if (!greyImage) {
    return result;
  }

  result = Bitmap::make(greyImage->width(), greyImage->height());
  result->copyRes(*greyImage);

  for (int y = 0; y < greyImage->height(); y++) {
    for (int x = 0; x < greyImage->width(); x++) {
      result->at(x, y) = greyImage->get(x, y) > 0x7f;
    }
  }

  return result;
}

bool writePng(const Bitmap &image, FILE *fp) {
  png_structp png_ptr;
  png_infop info_ptr;

  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (png_ptr == NULL) {
    fclose(fp);
    return false;
  }

  /* Allocate/initialize the image information data.  REQUIRED */
  info_ptr = png_create_info_struct(png_ptr);
  if (info_ptr == NULL) {
    fclose(fp);
    png_destroy_write_struct(&png_ptr,  NULL);
    return false;
  }

  /* Set error handling.  REQUIRED if you aren't supplying your own
   * error handling functions in the png_create_write_struct() call.
   */
  if (setjmp(png_jmpbuf(png_ptr)))
  {
    /* If we get here, we had a problem writing the file */
    fclose(fp);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    return false;
  }

  /* Set up the output control if you are using standard C streams */
  png_init_io(png_ptr, fp);

  png_set_IHDR(png_ptr, info_ptr, image.width(), image.height(),
               1, PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

  /* Optional gamma chunk is strongly suggested if you have any guess
   * as to the correct gamma of the image.
   */
  png_set_gAMA(png_ptr, info_ptr, 1.0);

  png_set_pHYs(png_ptr, info_ptr, image.xRes(), image.yRes(), image.resUnit());

  /* Write the file header information.  REQUIRED */
  png_write_info(png_ptr, info_ptr);

  int width_bytes = (image.width() + 7) >> 3;
  png_bytep row = new png_byte[width_bytes];

  for (int y = 0; y < image.height(); y++) {
    for (int x = 0; x < image.width(); x += 8) {
      uint8_t b = 0;
      uint8_t mask = 0x80;
      for (int xx = x; xx < x+8 && xx < image.width(); xx++) {
        if (image.get(xx, y)) {
          b |= mask;
        }
        mask >>= 1;
      }
      row[x >> 3] = ~b;
    }

    png_write_rows(png_ptr, &row, 1);
  }

  delete [] row;

  /* It is REQUIRED to call this to finish writing the rest of the file */
  png_write_end(png_ptr, info_ptr);

  /* Clean up after the write, and free any memory allocated */
  png_destroy_write_struct(&png_ptr, &info_ptr);

  /* Close the file */
  fclose(fp);

  return true;
}

#if 0
{
#endif
}  // namespace Bitmaps

void distance_transform_thread_background(void *params) {
  distance_transform_params *dt_params = static_cast<distance_transform_params *>(params);
  dt_params->bitmap->distanceTransformPass1<true>(dt_params->x0, dt_params->x1, dt_params->g);
  Workers::waitBarrier(dt_params->barrier);
  dt_params->bitmap->distanceTransformPass2<true>(dt_params->g, dt_params->y0, dt_params->y1, dt_params->result);
}

void distance_transform_thread_foreground(void *params) {
  distance_transform_params *dt_params = static_cast<distance_transform_params *>(params);
  dt_params->bitmap->distanceTransformPass1<false>(dt_params->x0, dt_params->x1, dt_params->g);
  Workers::waitBarrier(dt_params->barrier);
  dt_params->bitmap->distanceTransformPass2<false>(dt_params->g, dt_params->y0, dt_params->y1, dt_params->result);
}

void feature_transform_thread_background(void *params) {
  feature_transform_params *ft_params = static_cast<feature_transform_params *>(params);
  ft_params->bitmap->featureTransformPass1<true>(ft_params->x0, ft_params->x1, ft_params->g, ft_params->ys);
  Workers::waitBarrier(ft_params->barrier);
  ft_params->bitmap->featureTransformPass2<true>(ft_params->g, ft_params->ys, ft_params->y0, ft_params->y1, ft_params->result);
}

void feature_transform_thread_foreground(void *params) {
  feature_transform_params *ft_params = static_cast<feature_transform_params *>(params);
  ft_params->bitmap->featureTransformPass1<false>(ft_params->x0, ft_params->x1, ft_params->g, ft_params->ys);
  Workers::waitBarrier(ft_params->barrier);
  ft_params->bitmap->featureTransformPass2<false>(ft_params->g, ft_params->ys, ft_params->y0, ft_params->y1, ft_params->result);
}

#if 0
{
#endif
}  // namespace Images
