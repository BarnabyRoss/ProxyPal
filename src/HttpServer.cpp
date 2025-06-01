
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

  //1.从 BackendManger 获取下一个目标后端服务器
  std::shared_ptr<BackendServer> target_backend = backend_manager_.getNextBackend();
  // 2. 检查是否成功获取到后端服务器
    if (!target_backend) {
        // 如果没有可用的后端服务器，返回 503 Service Unavailable
        std::string error_response = "HTTP/1.1 503 Service Unavailable\r\n";
        error_response += "Content-Type: text/plain\r\n";
        error_response += "Content-Length: 29\r\n"; // "No backend service available" 的长度
        error_response += "Connection: close\r\n\r\n";
        error_response += "No backend service available";
        return error_response;
    }

  //3.使用目标后端服务器的 IP (host) 和端口构建新的 HTTP 请求字符串
  std::string newRequest = buildForwardRequest(request, target_backend->getHost(), target_backend->getPort());

  //4.直接调用目标后端服务器实例的 sendRequest 方法发送请求并获取响应
  return backend_manager_.sendRequest(newRequest);
}

std::string HttpServer::buildForwardRequest(const HttpRequest& request, const std::string& backend_host, int backend_port){

  std::string forwardRequest = request.method_ + " " + request.uri_ + " " + request.version_ + "\r\n";

  //复制原始请求的大部分头部
  for(const auto& header : request.headers_){
    if( header.first != "Host" ){
      forwardRequest += header.first + ":" + header.second + "\r\n";
    }else{
      //设置指向目标后端服务器的 Host 头部
      forwardRequest += "Host: " + backend_host + ":" + std::to_string(backend_port) + "\r\n";
    }
  }

  //添加 X-Forwarded-For 头部，传递原始客户端 IP
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


