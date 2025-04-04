
#include "BackendServer.h"

BackendServer::BackendServer(const std::string& ip, int port) : host_(ip), port_(port){ 

}

std::string BackendServer::sendRequest(const std::string& request){

  int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if( socket_fd < 0 ) return "HTTP/1.1 502 Bad GateWay\r\n\r\n";

  if( !connectServer(socket_fd) ) return "HTTP/1.1 502 Bad GateWay\r\n\r\n";

  int len = send(socket_fd, request.c_str(), request.length(), 0);
  if( len < 0 ){
    close(socket_fd);
    return "HTTP/1.1 502 Bad GateWay\r\n\r\n";
  }

  return readBuffer(socket_fd);

}

std::string BackendServer::getHost() const{

  return this->host_;
}

int BackendServer::getPort() const{

  return this->port_;
}

BackendServer::~BackendServer(){


}

bool BackendServer::connectServer(const int& fd){

  struct sockaddr_in server_addr;

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port_);

  if( inet_pton(AF_INET, host_.c_str(), &server_addr.sin_addr) < 0 ){
    close(fd);
    return false;
  }

  if( connect(fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0 ){
    close(fd);
    return false;
  }

  return true;
}

std::string BackendServer::readBuffer(const int& fd){

  char buffer[4096];
  std::string response;

  int len  = 0;
  while( (len = recv(fd, buffer, sizeof(buffer) - 1, 0)) > 0 ){
    buffer[len] = '\0';
    response += buffer;
  }

  close(fd);
  return response;
}




