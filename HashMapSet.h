/*
 *  HashMapSet.h
 *
 *  Created by Christian Brunschen on 21/03/2010.
 *  Copyright 2010 Christian Brunschen. All rights reserved.
 *
 */

#ifndef __HashMapSet_h__
#define __HashMapSet_h__

//
// Hashed / Unordered set & map
//
#define HASH_NAMESPACE std
#define BEGIN_HASH_NAMESPACE namespace std { namespace
#define END_HASH_NAMESPACE }
#define HASH_MAP unordered_map
#define HASH_SET unordered_set
#include <unordered_map>
#include <unordered_set>

#endif // __HashMapSet_h__
