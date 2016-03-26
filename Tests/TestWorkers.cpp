//
//  TestWorkers.cpp
//  MorphPlot
//
//  Created by Christian Brunschen on 12/02/2014.
//
//

#define DEBUG 1
#include "catch.hpp"
#undef DEBUG

#include "TestWorkers.h"

#include "Workers.h"

struct Params {
  int src;
  int dst;
};

static void copyValue(void *v_params) {
  Params *params = static_cast<Params *>(v_params);
  params->dst = params->src;
}

TEST_CASE("workers/run", "Check that the workers actually do work")
{
#define N 100
  Params params[N];
  for (int i = 0; i < N; i++) {
    params[i].src = i;
    params[i].dst = -1;
  }

  Workers workers(N);

  workers.perform(copyValue, params);

  for (int i = 0; i < N; i++) {
    REQUIRE(params[i].src == i);
    REQUIRE(params[i].dst == i);
  }
}