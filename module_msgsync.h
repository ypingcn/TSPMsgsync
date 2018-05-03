#ifndef TSP_MODULE_MSGSYNC_H
#define TSP_MODULE_MSGSYNC_H

#include "../core/response.h"
#include "load-module.h"

#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>

extern "C" {
#include "../include/hiredis/hiredis.h"
}

class RedisPool {
public:
  RedisPool(const RedisPool &) = delete;
  RedisPool &operator=(const RedisPool &) = delete;
  static RedisPool *instance() {
    if (redis == nullptr) {
      pthread_mutex_lock(&lock);
      if (redis == nullptr)
        redis = new RedisPool();
      pthread_mutex_unlock(&lock);
    }
    return redis;
  }
  string command(const string &s);

private:
  RedisPool() {
    rContext = redisConnect("127.0.0.1", 6379);
    if (!rContext || rContext->err) {
      TSPLogger::instance()->error("redisConnect error");
    }
  }
  ~RedisPool() { redisFree(rContext); }

  static pthread_mutex_t lock;
  static RedisPool *redis;
  redisContext *rContext;
};

class TSPMsgsyncResponse : public TSPBasicResponse {
public:
  TSPMsgsyncResponse(int _priority, TSPResponsePatternType _type)
      : TSPBasicResponse(_priority, _type) {}
  ~TSPMsgsyncResponse() {}

  int handle(const TSPRequest &req) final;
};

int get_opt_val(const string &s, string &val);

void install_msgsync_modules(ResponseVectorType &t);

#endif
