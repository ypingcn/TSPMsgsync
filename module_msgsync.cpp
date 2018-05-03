#include "module_msgsync.h"

pthread_mutex_t RedisPool::lock = PTHREAD_MUTEX_INITIALIZER;
RedisPool *RedisPool::redis = nullptr;

string RedisPool::command(const string &s) {
  string result;
  redisReply *reply = (redisReply *)redisCommand(rContext, s.c_str());
  if (reply != NULL && reply->type == REDIS_REPLY_STATUS ||
      reply->type == REDIS_REPLY_STRING) {
    result = reply->str;
  }
  freeReplyObject(reply);
  return result;
}

int TSPMsgsyncResponse::handle(const TSPRequest &req) {
  int sockfd = req.get_sockfd();
  string url = req.get_url();
  string val;

  if (get_opt_val(url, val) == 0) {
    string command;
    if (req.get_method() == "GET") {
      command = "get " + val;
    } else if (req.get_method() == "POST") {
      command = "set " + val + " " + req.get_body();
    }
    string result = RedisPool::instance()->command(command);
    tsp_response_ok(sockfd);
    tsp_response_headers_end(sockfd);
    write(sockfd, result.c_str(), result.length());
  } else {
    tsp_response_not_found(sockfd);
    tsp_response_headers_end(sockfd);
    return -1;
  }
  return 0;
}

int get_opt_val(const string &s, string &val) {

  int cnt = 0, pos = -1, pre = 0;
  while ((pos = s.find("/", pos + 1)) != string::npos) {
    cnt++;
  }
  if (cnt < 2)
    return -1;

  cnt = 0, pos = -1, pre = 0;
  while ((pos = s.find("/", pos + 1)) != string::npos) {
    cnt++;
    pre = pos;
  }
  val = s.substr(pre + 1, pos - pre - 1);
  return 0;
}

void install_msgsync_modules(ResponseVectorType &t) {
  t.push_back(new TSPMsgsyncResponse(
      DEFAULT_RESPONSE_PRIORITY,
      {regex("^/msg/[0-9a-zA-z]{8}$"), "GET", "HTTP/1.1"}));
  t.push_back(new TSPMsgsyncResponse(
      DEFAULT_RESPONSE_PRIORITY,
      {regex("^/msg/[0-9a-zA-z]{8}$"), "POST", "HTTP/1.1"}));
}
