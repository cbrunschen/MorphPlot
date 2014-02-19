/*
 *  FnvHash.h
 *  Geometry
 *
 *  Created by Christian Brunschen on 22/08/2009.
 *  Copyright 2009 Christian Brunschen. All rights reserved.
 *
 */

#ifndef FnvHash_
#define FnvHash_

#define DEBUG_HASH 0

#if DEBUG_HASH
#define DH(x) do { x; } while(0)
#else
#define DH(x)
#endif

#include <cstddef>

template<size_t = sizeof(size_t)>
struct fnv_hash {
  static const size_t init = static_cast<size_t>(0);
  static size_t hash(const char *first, size_t length, size_t result = init) {
    DH(ios_base::fmtflags flags = cerr.flags(); cerr << "fnv_hash(" << hex << static_cast<const void *>(first) << dec << ", " << length << ", result)" << endl << flush; cerr.flags(flags));
    for (; length > 0; --length)
      result = (result * 131) + *first++;
    return result;
  }
};

template<>
struct fnv_hash<4>
{
  static const size_t init = static_cast<size_t>(2166136261UL);
  static size_t hash(const char* first, size_t length, size_t result = init) {
    DH(ios_base::fmtflags flags = cerr.flags(); cerr << "fnv_hash<4>(" << hex << static_cast<const void *>(first) << dec << ", " << length << ", result)" << endl << flush; cerr.flags(flags));
    for (; length > 0; --length) {
      result ^= static_cast<size_t>(*first++);
      result *= static_cast<size_t>(16777619UL);
    }
    return result;
  }
};

template<>
struct fnv_hash<8>
{
  static const size_t init = static_cast<size_t>(14695981039346656037ULL);
  static size_t hash(const char* first, size_t length, size_t result = init) {
    DH(ios_base::fmtflags flags = cerr.flags(); cerr << "fnv_hash<8>(" << hex << static_cast<const void *>(first) << dec << ", " << length << ", result)" << endl << flush; cerr.flags(flags));
    for (; length > 0; --length)
    {
      result ^= static_cast<size_t>(*first++);
      result *= static_cast<size_t>(1099511628211ULL);
    }
    return result;
  }
};

template<typename T>
struct continue_hash {
  static size_t hash (const T &t, size_t hash = fnv_hash<sizeof(T)>::init) {
    return fnv_hash<>::hash(reinterpret_cast<const char *> (&t), sizeof(T), hash);
  }
};

#undef DH

#endif // FnvHash_

