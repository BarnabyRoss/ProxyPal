
#ifndef __HTTP_PARSER_H__
#define __HTTP_PARSER_H__

#include <iostream>
#include <cstring>
#include <map>
#include <vector>

struct HttpRequest{

std::string method_;
std::string uri_;
std::string version_;
std::map<std::string, std::string> headers_;

};

class HttpParser{

private:
  HttpRequest request_;

public:
  HttpParser() = default;

  bool parseRequest(const std::string& raw_request);
  HttpRequest getHttpRequest() const { return request_; }

  ~HttpParser() = default;

private:
  std::vector<std::string> splitLines(const std::string& raw_request);
  bool parseRequestLine(const std::string& line);
  bool parseRequestHeader(const std::string& line);

};



#endif


