//
//  OpenCLWorkers.cpp
//  MorphPlot
//
//  Created by Christian Brunschen on 16/04/2014.
//
//

#include "OpenCLWorkers.h"


string OpenCLWorkers::kernels =
#include "kernels.inc"
;

string OpenCLWorkers::toSitesFormat =
#include "to_sites.inc"
;

