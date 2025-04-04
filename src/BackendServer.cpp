
#include "BackendServer.h"

BackendServer::BackendServer(const std::string& ip, int port) : ip_(ip), port_(port){ 

}

std::string BackendServer::sendRequest(const std::string& request){


}

std::string BackendServer::getHost() const{

  return this->ip_;
}

int BackendServer::getPort() const{

  return this->port_;
}

BackendServer::~BackendServer(){


}






