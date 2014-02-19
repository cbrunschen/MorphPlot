
/*
 *  Count.h
 *  Morph
 *
 *  Created by Christian Brunschen on 19/06/2011.
 *  Copyright 2011 Christian Brunschen. All rights reserved.
 *
 */

inline const char *count(const char *prefix, int value) {
  ostringstream os;
  os << prefix << value;
  return os.str().c_str();
}
