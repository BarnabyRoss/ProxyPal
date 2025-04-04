
#include "HttpServer.h"

HttpServer::HttpServer(int port, const std::string& backendHost, int backendPort, size_t threadCount) : tcpServer_(port), backend_server_(backendHost, backendPort), thread_pool_(threadCount){

  tcpServer_.setMessageCallBack([this](std::shared_ptr<Connection> conn, const std::string& data){
    this->onMessage(conn, data);
  });
}

void HttpServer::start(){

  tcpServer_.start();
}

void HttpServer::registerHandler(const std::string& path, std::function<HttpResponse(const HttpRequest&)> handler){

  url_handles_[path] = handler;
}

void HttpServer::onMessage(std::shared_ptr<Connection> conn, const std::string& raw_data){

  HttpTask task;
  task.conn_ = conn;
  task.request_data_ = raw_data;

  thread_pool_.enqueue(&HttpServer::processRequest, this, task);
}

void HttpServer::processRequest(HttpTask task){

  if( parser_.parseRequest(task.request_data_)){

    HttpRequest request = parser_.getHttpRequest();

    /*auto it = url_handles_.find(request.uri_);
    if( it != url_handles_.end() ){

      HttpResponse response = it->second(request);
      task.conn_->appendToWriteBuffer(response.toString());
      if( task.conn_->hasDataToWrite() )  task.conn_->writeData();

    }else{

      HttpResponse response;
      response.setStatusCode(404);
      response.setBody("Not Found");
      task.conn_->appendToWriteBuffer(response.toString());
      if( task.conn_->hasDataToWrite() ) task.conn_->writeData();
    }
  }else{

    HttpResponse response;
    response.setStatusCode(404);
    response.setBody("Bad Request");
    task.conn_->appendToWriteBuffer(response.toString());
    if( task.conn_->hasDataToWrite() ) task.conn_->writeData();
  }*/
  /*实现请求转发逻辑*/
  std::string responseStr = forwardRequest(request);
  task.conn_->appendToWriteBuffer(responseStr);
  if( task.conn_->hasDataToWrite() ){
    task.conn_->writeData();
  }else{
    HttpResponse response;
    response.setStatusCode(404);
    response.setBody("Bad Request");
    task.conn_->appendToWriteBuffer(response.toString());
    if( task.conn_->hasDataToWrite() ) task.conn_->writeData();
  }
}

std::string HttpServer::forwardRequest(const HttpRequest& request){

  //构建新的HTTP请求
  std::string newRequest = buildForwardRequest(request);

  //发送到后端服务器并获取响应
  return backend_server_.sendRequest(newRequest);
}


