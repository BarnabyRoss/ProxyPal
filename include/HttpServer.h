
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
#include "BackendServer.h"
#include "BackendManger.h"

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
  //BackendServer backend_server_; //增加连接后端服务器对象
  BackendManger backend_manager_;

public:
  HttpServer(int port, const std::string& config_file, size_t threadCount = 4); //添加后端服务器构造参数

  void start();
  void registerHandler(const std::string& path, std::function<HttpResponse(const HttpRequest&)> handler);

  ~HttpServer() = default;

private:
  void onMessage(std::shared_ptr<Connection> conn, const std::string& raw_data);
  void processRequest(HttpTask task);
  //新增：转发请求到后端服务器
  std::string forwardRequest(const HttpRequest& request);
  std::string buildForwardRequest(const HttpRequest& request);

  //定期检查配置更新
  void checkConfigPeriodically();

};



#endif
