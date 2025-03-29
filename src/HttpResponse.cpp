
#include "HttpResponse.h"
#include <sstream>

HttpResponse::HttpResponse() : status_code_(200), status_message_("OK"){

  headers_["Server"] = "SimpleHttp/1.0";
  headers_["Connection"] = "close";
}
  
void HttpResponse::setStatusCode(int code){

  status_code_ = code;

  switch(code){
    case 200 : status_message_ = "OK"; break;
    case 201 : status_message_ = "Created"; break;
    case 204 : status_message_ = "No Content"; break;
    case 400 : status_message_ = "Bad Request"; break;
    case 404 : status_message_ = "Not Found"; break;
    case 500 : status_message_ = "Internal Server Erron"; break;
    default : status_message_ = "Unknown"; break;
  }
}

void HttpResponse::setBody(const std::string& body){

  body_ = body;
  headers_["Content-Length"] = std::to_string(body.length());
}

std::string HttpResponse::toString() const{

  std::stringstream ss;

  ss << "Http/1.1 " << status_code_ << " " << status_message_ << "\r";  //响应行
  
  //响应头
  for(auto& header : headers_){
    ss << header.first << ": " << header.second << "\r";
  }

  //空行
  ss << "\r";

  //响应体
  ss << body_;

  return ss.str();

}

