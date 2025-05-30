
#include "HttpServer.h"

HttpServer::HttpServer(int port, const std::string& config_file, size_t threadCount) : tcpServer_(port), 
           backend_manager_(config_file), thread_pool_(threadCount){

  tcpServer_.setMessageCallBack([this](std::shared_ptr<Connection> conn, const std::string& data){
    this->onMessage(conn, data);
  });
}

void HttpServer::start(){

  //启动配置检查线程
  std::thread config_checker(&HttpServer::checkConfigPeriodically, this);
  config_checker.detach();

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
    request.client_ip_ = task.conn_->getClientIP();

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
  //return backend_server_.sendRequest(newRequest);
  return backend_manager_.sendRequest(newRequest);
}

std::string HttpServer::buildForwardRequest(const HttpRequest& request){

  std::string forwardRequest = request.method_ + " " + request.uri_ + " " + request.version_ + "\r\n";

  for(const auto& header : request.headers_){
    if( header.first != "Host" ){
      forwardRequest += header.first + ":" + header.second + "\r\n";
    }else{
      forwardRequest += "Host: " + backend_server_.getHost() + ":" + std::to_string(backend_server_.getPort()) + "\r\n";
    }
  }

  forwardRequest += "X-Forward-For:" + request.client_ip_ + "\r\n";

  forwardRequest += "\r\n";

  return forwardRequest;

}

//定期检查配置更新
void HttpServer::checkConfigPeriodically(){

  while( true ){

    std::this_thread::sleep_for(std::chrono::seconds(60));
    backend_manager_.checkConfigUpdate();
  }
}


