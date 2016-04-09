//
//  Workers.cpp
//  MorphPlot
//
//  Created by Christian Brunschen on 12/02/2014.
//
//

#include "Workers.h"

#include <pthread.h>

#if __APPLE__

int pthread_barrier_init(pthread_barrier_t *barrier, const pthread_barrierattr_t *attr, unsigned int count) {
  if(count == 0)
  {
    errno = EINVAL;
    return -1;
  }
  if(pthread_mutex_init(&barrier->mutex, 0) < 0)
  {
    return -1;
  }
  if(pthread_cond_init(&barrier->cond, 0) < 0)
  {
    pthread_mutex_destroy(&barrier->mutex);
    return -1;
  }
  barrier->tripCount = count;
  barrier->count = 0;

  return 0;
}

int pthread_barrier_destroy(pthread_barrier_t *barrier) {
  pthread_cond_destroy(&barrier->cond);
  pthread_mutex_destroy(&barrier->mutex);
  return 0;
}

int pthread_barrier_wait(pthread_barrier_t *barrier) {
  pthread_mutex_lock(&barrier->mutex);
  ++(barrier->count);
  if(barrier->count >= barrier->tripCount)
  {
    barrier->count = 0;
    pthread_cond_broadcast(&barrier->cond);
    pthread_mutex_unlock(&barrier->mutex);
    return 1;
  }
  else
  {
    pthread_cond_wait(&barrier->cond, &(barrier->mutex));
    pthread_mutex_unlock(&barrier->mutex);
    return 0;
  }
}

#endif  // __APPLE__

void *Workers::worker_thread(void *params) {
  Workers::Worker *worker = static_cast<Workers::Worker *>(params);

  pthread_mutex_lock(&worker->lock);
  for (;;) {
    // wait for work
    while (worker->status == Workers::waiting) {
      pthread_cond_wait(&worker->status_changed, &worker->lock);
    }

    if (worker->status == Workers::quit) {
      // done!
      break;
    }

    void (*func)(void *) = worker->func;
    void *func_params = worker->func_params;

    pthread_mutex_unlock(&worker->lock);

    func(func_params);

    pthread_mutex_lock(&worker->lock);
    worker->status = Workers::waiting;
    pthread_cond_broadcast(&worker->status_changed);
  }
  pthread_mutex_unlock(&worker->lock);

  return NULL;
}
