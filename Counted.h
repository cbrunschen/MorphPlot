/*
 *  Counted.h
 *  Morph
 *
 *  Created by Christian Brunschen on 04/07/2009.
 *  Copyright 2009 Christian Brunschen. All rights reserved.
 *
 */

#ifndef Counted_
#define Counted_

#undef DEBUG_COUNTING
#define DEBUG_COUNTING 0

#include <iostream>
#if DEBUG_COUNTING
#include <typeinfo>
#define D(x) do { x; } while(0)
#else
#define D(x)
#endif

using namespace std;

/* base class for reference-counted objects */
class Counted {
private:
  unsigned int count_;
public:
  Counted() : count_(0) { 
    D(cerr << "instantiating " << typeid(*this).name() << " " << this << " @ " << count_ << endl << flush);
  }
  virtual ~Counted() { }
  virtual Counted *retain() { 
    D(cerr << "retaining " << typeid(*this).name() << " " << this << " @ " << count_);
    count_++;
    D(cerr << "->" << count_ << endl << flush);
    return this; 
  }
  virtual void release() { 
    D(cerr << "releasing " << typeid(*this).name() << " " << this << " @ " << count_);
    if (count_ == 0 || count_ == 0xDEADF001) {
      D(cerr << "\nOverreleasing already-deleted object " << this << "!!!" << endl << flush);
      throw 4711;
    }
    count_--; 
    D(cerr << "->" << count_ << endl << flush);
    if (count_ == 0) { 
      D(cerr << "deleting " << typeid(*this).name() << " " << this << endl << flush);
      count_ = 0xDEADF001;
      delete this; 
    } 
  }
  
  /* return the current count for denugging purposes or similar */
  int count() const { return count_; }
};

/* counting reference to reference-counted objects */
template<typename T> class Ref {
public:
  typedef T Target;
  typedef Ref<T> Self;

  typedef struct {
    size_t operator()(const Self &ref) const { return reinterpret_cast<size_t> (ref.object_); }
  } hash;
  
  typedef struct {
    bool operator()(const Self &a, const Self &b) const { return a.object_ == b.object_; }
  } equals;
  
protected:
  T *object_;

public:
  explicit Ref(T *o = 0) : object_(0) { 
    D(cerr << "instantiating Ref " << this << " from pointer" << endl << flush);
    reset(o);
  }
  
  explicit Ref(const T &o) : object_(0) {
    D(cerr << "instantiating Ref " << this << " from reference" << endl << flush);
    reset(const_cast<T *>(&o));
  }
  
  Ref(const Ref &other) : object_(0) {
    D(cerr << "instantiating Ref " << this << " from Ref " << &other << endl << flush);
    reset(other.object_);
  }
  
  template<class Y>
  Ref(const Ref<Y> &other) : object_(0) {
    D(cerr << "instantiating Ref " << this << " from reference" << endl << flush);
    reset(other.object_);
  }
  
  
  ~Ref() { 
    D(cerr << "destroying Ref " << this << " with " <<
      (object_ ? typeid(*object_).name() : "NULL") << " " << object_ << endl << flush);
    if (object_) { 
      object_->release(); 
    } 
  }
  
  void reset(T *o) {
    D(cerr << "resetting Ref " << this << " from " <<
      (object_ ? typeid(*object_).name() : "NULL") << " " << object_ <<
      " to "  << (o ? typeid(*o).name() : "NULL") << " " << o << endl << flush);
    if (o) { o->retain(); }
    if (object_) { object_->release(); }
    object_ = o;
  }
  Ref& operator=(const Ref &other) {
    reset(other.object_);
    return *this;
  }
  template<class Y>
  Ref& operator=(const Ref<Y> &other) {
    reset(other.object_);
    return *this;
  }
  Ref& operator=(T* o) {
    reset(o);
    return *this;
  }
  template<class Y>
  Ref& operator=(Y* o) {
    reset(o);
    return *this;
  }
  
  T& operator*() const { return *object_; }
  T* operator->() const { return object_; }
  operator T*() const { return object_; }
  
  uintptr_t ptr() { return reinterpret_cast<uintptr_t> (object_); }
  const uintptr_t ptr() const { return reinterpret_cast<const uintptr_t> (object_); }
  
  bool operator==(const int x) const { return x == 0 ? object_ == 0 : false; }
  bool operator==(const Ref &other) const { 
    return object_ == other.object_ || *object_ == *(other.object_);
  }
  template<class Y>
  bool operator==(const Ref<Y> &other) const { 
    return object_ == other.object_ || *object_ == *(other.object_);
  }
  
  bool operator!=(const int x) const { return x == 0 ? object_ != 0 : true; }
  
  template<class Y>
  friend ostream& operator<<(ostream &out, Ref<Y>& ref);
  template<class Y>
  friend ostream& operator<<(ostream &out, const Ref<Y>& ref);
};

template <typename T>
ostream &operator<< (ostream &out, Ref<T> &ref) {
  ios_base::fmtflags flags = out.flags(); 
  out << "Ref(" << ref.object_->count() << "@" << hex << ref.ptr() << "):" << endl;
  out.flags(flags);
  if (ref.ptr() != 0) out << *ref; else out << "<NULL>";
  out << ")";
  return out;
}

template <typename T>
ostream &operator<< (ostream &out, const Ref<T> &ref) {
  ios_base::fmtflags flags = out.flags(); 
  out << "Ref(" << ref.object_->count() << "@" << hex << ref.ptr() << "):" << endl;
  out.flags(flags);
  if (ref.ptr() != 0) out << *ref; else out << "<NULL>";
  out << ")";
  return out;
}

#undef D
#undef DEBUG_COUNTING

#endif // Counted_
