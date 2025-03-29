
#include "HttpParser.h"


bool HttpParser::parseRequest(const std::string& raw_request){

  if( raw_request.empty() ) return false;

  std::vector<std::string> lines = splitLines(raw_request);
  if( lines.empty() ) return false;
  //parse line
  if( !parseRequestLine(lines[0]) ) return false;
  //parse header
  for(int i = 1; i < lines.size(); ++i){
    if( lines[i].empty() ) break;
    if( !parseRequestHeader(lines[i]) ) return false;
  }

  if( request_.headers_.find("HOST") == request_.headers_.end() ) return false;

  return true;
}

std::vector<std::string> HttpParser::splitLines(const std::string& raw_request){

  std::vector<std::string> lines;
  size_t begin = 0;
  size_t end;
  while( (end = raw_request.find("\r\n", begin)) != std::string::npos ){
    
    lines.push_back(raw_request.substr(begin, end - begin));
    begin = end + 2;
  }
  if( begin < raw_request.length() ){

    lines.push_back(raw_request.substr(begin));
  }
  return lines;
}

bool HttpParser::parseRequestLine(const std::string& line){

  if( line.empty() ) return false;

  size_t method_end = line.find(' ');
  if( method_end == std::string::npos ) return false;

  size_t uri_end = line.find(' ', method_end + 1);
  if( uri_end == std::string::npos ) return false;

  request_.method_ = line.substr(0, method_end);
  request_.uri_ = line.substr(method_end + 1, uri_end - (method_end + 1));
  request_.version_ = line.substr(uri_end + 1);

  if( request_.method_ != "GET" ) return false;

  return true;
}

bool HttpParser::parseRequestHeader(const std::string& line){

  if( line.empty() ) return false;

  size_t colon = line.find(':');
  if( colon == std::string::npos ) return false;

  std::string key = line.substr(0, colon);

  size_t valueIndex = line.find_first_not_of(' ', colon + 1);
  if( valueIndex == std::string::npos ) valueIndex = colon + 1;

  std::string value = line.substr(valueIndex);
  request_.headers_[key] = value;

  return true;
}


