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

  OpenCLWorkers() : device(findBestDevice()), context(device), program(context, makeKernels(), true) {
  }

  OpenCLWorkers(int i) : device(findDevice(i)), context(device), program(context, makeKernels(), true) {
  }
};

#endif // __OpenCLWorkers__
