//
//  OpenCLWorkers.h
//  MorphPlot
//
//  Created by Christian Brunschen on 16/04/2014.
//
//

#ifndef __OpenCLWorkers__
#define __OpenCLWorkers__

#include <iostream>
#include <vector>
#include <map>
#include <sstream>

#include "cl.hpp"

using namespace std;

template<typename T>
ostream &operator<<(ostream &out, vector<T> v) {
  out << "(";
  bool first = true;
  for (auto i = v.begin(); i != v.end(); ++i) {
    if (first) first = false; else out << ", ";
    out << *i;
  }
  return out << ")";
}

template <typename T>
struct PlatformInfo {
  const cl::Platform &platform;
  cl_platform_info info;
  PlatformInfo(const cl::Platform &p, cl_platform_info i) : platform(p), info(i) { }
  operator T () const {
    T value;
    platform.getInfo(info, &value);
    return value;
  }
};
template<typename T>
ostream &operator<<(ostream &out, const PlatformInfo<T> &info) {
  return out << static_cast<T>(info);
}

template <typename T>
struct DeviceInfo {
  const cl::Device &device;
  cl_device_info info;
  DeviceInfo(const cl::Device &d, cl_device_info i) : device(d), info(i) { }
  operator T () const {
    T value;
    device.getInfo(info, &value);
    return value;
  }
};
template<typename T>
ostream &operator<<(ostream &out, const DeviceInfo<T> &info) {
  return out << static_cast<T>(info);
}

class OpenCLWorkers {
  static string kernels;
  static string toSitesFormat;

  static cl::Device findBestDevice() {
#if defined(__APPLE__) || defined(__MACOSX)
    cl_device_id device_id;
    dispatch_queue_t queue =
        gcl_create_dispatch_queue(CL_DEVICE_TYPE_GPU, NULL);
    if (queue == NULL) {
      queue = gcl_create_dispatch_queue(CL_DEVICE_TYPE_CPU, NULL);
    }
    device_id = gcl_get_device_id_with_dispatch_queue(queue);
    dispatch_release(queue);
    return cl::Device(device_id);
#else
    cl::Platform platform = cl::Platform::getDefault();
    vector<cl::Device> devices;
    platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);
    if (devices.size() == 0) {
      platform.getDevices(CL_DEVICE_TYPE_CPU, &devices);
    }
    return devices.front();
#endif
  }
  
  static cl::Device findDevice(int i) {
    if (i < 0) {
      return findBestDevice();
    }
    vector<cl::Device> devices;
    cl::Platform::getDefault().getDevices(CL_DEVICE_TYPE_ALL, &devices);
    return devices[i];
  }
  
  static string substitute(const string &in, const map<string, string> &substitutions) {
    ostringstream out;
    size_t pos = 0;
    for (;;) {
      size_t subst_pos = in.find( "{{", pos );
      size_t end_pos = in.find( "}}", subst_pos );
      if (end_pos == string::npos) {
        break;
      }
      
      out.write(&* in.begin() + pos, subst_pos - pos);
      
      subst_pos += strlen( "{{" );
      auto subst_it = substitutions.find(in.substr(subst_pos, end_pos - subst_pos));
      if (subst_it == substitutions.end()) throw runtime_error( "undefined substitution" );
      
      out << subst_it->second;
      pos = end_pos + strlen( "}}" );
    }
    out << in.substr( pos, string::npos );
    return out.str();
  }
  
  static string makeToSites(const string &name, const string &type, const string &comparison) {
    map<string, string> substitutions;
    substitutions["name"] = name;
    substitutions["type"] = type;
    substitutions["comparison"] = comparison;
    return substitute(toSitesFormat, substitutions);
  }
  
  static string makeKernels() {
    ostringstream out;
    out << kernels << endl;
    out << makeToSites("zerosToSites", "uchar", "== 0");
    out << makeToSites("nonZerosToSites", "uchar", "!= 0");
    return out.str();
  }
  
public:
  
  cl::Device device;
  cl::Context context;
  cl::Program program;
  cl::CommandQueue queue;
  
  // kernels
  cl::make_kernel<cl::Buffer&, cl::Buffer&, cl_short, cl_short> nonZerosToSites;
  cl::make_kernel<cl::Buffer&, cl::Buffer&, cl_short, cl_short> zerosToSites;
  cl::make_kernel<cl::Buffer&, cl::Buffer&, cl_short, cl_short> featureTransformPass1;
  cl::make_kernel<cl::Buffer&, cl::Buffer&, cl_short, cl_short> featureTransformPass1_edges;
  cl::make_kernel<cl::Buffer&, cl::Buffer&, cl_short, cl_short> featureTransformPass2;

  OpenCLWorkers(int i = -1)
  : device(findDevice(i)),
    context(device),
    program(context, makeKernels(), true),
    queue(context, device),
    nonZerosToSites(program, "nonZerosToSites"),
    zerosToSites(program, "zerosToSites"),
    featureTransformPass1(program, "featureTransformPass1"),
    featureTransformPass1_edges(program, "featureTransformPass1_edges"),
    featureTransformPass2(program, "featureTransformPass2")
  {
  }
  
  cl::NDRange fitNDRange(size_t x, size_t y, size_t z) {
    size_t maxOverall = DeviceInfo<size_t>(device, CL_DEVICE_MAX_WORK_GROUP_SIZE);
    vector<size_t> maxSizes = DeviceInfo< vector<size_t> >(device, CL_DEVICE_MAX_WORK_ITEM_SIZES);
    while (x * y * z > maxOverall) {
      if (z > 1) z >>= 1;
      else if (y > 1) y >>= 1;
      else x >>= 1;
    }
    
    while (z > maxSizes[2]) {
      z >>= 1;
      y <<= 1;
    }

    while (y > maxSizes[1]) {
      y >>= 1;
      x <<= 1;
    }
    while (x > maxSizes[0]) {
      x >>= 1;
    }
    
    return cl::NDRange(x, y, z);
  }

  cl::NDRange fitNDRange(size_t x, size_t y) {
    size_t maxOverall = DeviceInfo<size_t>(device, CL_DEVICE_MAX_WORK_GROUP_SIZE);
    vector<size_t> maxSizes = DeviceInfo< vector<size_t> >(device, CL_DEVICE_MAX_WORK_ITEM_SIZES);
    while (x * y > maxOverall) {
      if (y > 1) y >>= 1;
      else x >>= 1;
    }
    
    while (y > maxSizes[1]) {
      y >>= 1;
      x <<= 1;
    }
    while (x > maxSizes[0]) {
      x >>= 1;
    }
    
    return cl::NDRange(x, y);
  }

  cl::NDRange fitNDRange(size_t x) {
    size_t maxOverall = DeviceInfo<size_t>(device, CL_DEVICE_MAX_WORK_GROUP_SIZE);
    vector<size_t> maxSizes = DeviceInfo< vector<size_t> >(device, CL_DEVICE_MAX_WORK_ITEM_SIZES);
    while (x > maxOverall) {
      x >>= 1;
    }
    
    while (x > maxSizes[0]) {
      x >>= 1;
    }
    
    return cl::NDRange(x);
  }
};


#endif // __OpenCLWorkers__
