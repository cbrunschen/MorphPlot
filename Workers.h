//
//  Workers.h
//  MorphPlot
//
//  Created by Christian Brunschen on 12/02/2014.
//
//

#ifndef __Workers_h__
#define __Workers_h__

#include <pthread.h>
#include <errno.h>

#if __APPLE__

typedef int pthread_barrierattr_t;
typedef struct
{
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  int count;
  int tripCount;
} pthread_barrier_t;

int pthread_barrier_init(pthread_barrier_t *barrier, const pthread_barrierattr_t *attr, unsigned int count);
int pthread_barrier_destroy(pthread_barrier_t *barrier);
int pthread_barrier_wait(pthread_barrier_t *barrier);

#endif  // __APPLE__

class Workers {
  
public:
  enum Status {
    waiting = 0,
    working = 1,
    quit = 2
  };
  
  struct Worker {
    pthread_mutex_t lock;
    pthread_cond_t status_changed;
    Status status;
    void (*func)(void *);
    void *func_params;
    
    pthread_t thread_id;
  };
  
  template<typename V>
  struct Job {
    Workers &workers;
    const V &value;
    Job(Workers &workers_, const V &value_) : workers(workers_), value(value_) { }
  };
  
  typedef pthread_barrier_t Barrier;

private:
  int n_;
  Worker *workers_;
  
  static void *worker_thread(void *params);
  
public:
  Workers(int n) : n_(n), workers_(new Worker[n-1]) {
    for (int i = 0; i < n-1; i++) {
      pthread_mutex_init(&workers_[i].lock, NULL);
      pthread_cond_init(&workers_[i].status_changed, NULL);
      workers_[i].status = waiting;
      workers_[i].func = NULL;
      workers_[i].func_params = NULL;
      
      pthread_create(&workers_[i].thread_id, NULL, worker_thread, &workers_[i]);
    }
  }
  
  int n() { return n_; }
  
  template<typename P>
  void perform(void (*func)(void *), P *params) {
    for (int i = 0; i < n_-1; i++) {
      pthread_mutex_lock(&workers_[i].lock);
      workers_[i].status = working;
      workers_[i].func = func;
      workers_[i].func_params = static_cast<void *>(&params[i]);
      pthread_cond_broadcast(&workers_[i].status_changed);
      pthread_mutex_unlock(&workers_[i].lock);
    }
    
    func(static_cast<void *>(&params[n_-1]));
    
    for (int i = 0; i < n_-1; i++) {
      pthread_mutex_lock(&workers_[i].lock);
      while (workers_[i].status != waiting) {
        pthread_cond_wait(&workers_[i].status_changed, &workers_[i].lock);
      }
      workers_[i].func = NULL;
      workers_[i].func_params = NULL;
      pthread_mutex_unlock(&workers_[i].lock);
    }
  }

  void initBarrier(Barrier *barrier) {
    pthread_barrier_init(barrier, NULL, n_);
  }
  
  static void waitBarrier(Barrier *barrier) {
    pthread_barrier_wait(barrier);
  }
  
  void destroyBarrier(Barrier *barrier) {
    pthread_barrier_destroy(barrier);
  }
  
  ~Workers() {
    for (int i = 0; i < n_-1; i++) {
      pthread_mutex_lock(&workers_[i].lock);
      workers_[i].status = quit;
      pthread_cond_broadcast(&workers_[i].status_changed);
      pthread_mutex_unlock(&workers_[i].lock);
      
      pthread_join(workers_[i].thread_id, NULL);
      pthread_cond_destroy(&workers_[i].status_changed);
      pthread_mutex_destroy(&workers_[i].lock);
    }
    
    delete [] workers_;
  }
};

#endif  // __Workers_h__
