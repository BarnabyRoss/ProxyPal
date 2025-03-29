
#include "HttpServer.h"

HttpServer::HttpServer(int port) : tcpServer_(port){

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

  //使用HttpParser解析HTTP请求
  if( parser_.parseRequest(raw_data) ){

    HttpRequest request = parser_.getHttpRequest();

    auto it = url_handles_.find(request.uri_);
    if( it != url_handles_.end() ){
      HttpResponse response = it->second(request);

      conn->appendToWriteBuffer(response.toString());
      if( conn->hasDataToWrite() ){
        conn->writeData();
      }
    }else{
      
      HttpResponse response;
      response.setStatusCode(404);
      response.setBody("Not Found");
      conn->appendToWriteBuffer(response.toString());
      if( conn->hasDataToWrite() ) conn->writeData();
    }

  }
}

