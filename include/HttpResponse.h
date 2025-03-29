
#ifndef __HTTP_RESPONSE_H__
#define __HTTP_RESPONSE_H__

#include <iostream>
#include <cstring>
#include <map>

class HttpResponse{

  private:
    int status_code_;
    std::string status_message_;
    std::unordered_map<std::string, std::string> headers_;
    std::string body_;
  
  public:
    HttpResponse();
    
    void setStatusCode(int code);

    //自定义状态消息
    void setStatusMessage(const std::string& message){ status_message_ = message; }

    //设置响应头
    void addHeader(const std::string& key, const std::string& value){ headers_[key] = value; }

    //设置响应体
    void setBody(const std::string& body);

    //序列化HTTP为响应字符串
    std::string toString() const;


    ~HttpResponse() = default;
  
  };


#endif
