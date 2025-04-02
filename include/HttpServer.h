
#ifndef __HTTP_SERVER_H__
#define __HTTP_SERVER_H__

#include <iostream>
#include <memory>
#include <cstring>
#include <unordered_map>
#include "TcpServer.h"
#include "HttpParser.h"
#include "HttpResponse.h"
#include "ThreadPool.h"

struct HttpTask{

  std::shared_ptr<Connection> conn_;
  std::string request_data_;
};

class HttpServer{

public:
  using UrlResponse = std::unordered_map< std::string, std::function<HttpResponse(const HttpRequest&)> >;

private:
  TcpServer tcpServer_;
  HttpParser parser_;
  UrlResponse url_handles_;
  ThreadPool thread_pool_;

public:
  HttpServer(int port);

  void start();
  void registerHandler(const std::string& path, std::function<HttpResponse(const HttpRequest&)> handler);

  ~HttpServer() = default;

private:
  void onMessage(std::shared_ptr<Connection> conn, const std::string& raw_data);
  void processRequest(HttpTask task);

};



#endif
