
#include "HttpServer.h"


int main(int argc, char* argv[]){

  HttpServer server(9900);

  // 用户注册URL处理函数
  server.registerHandler("/", [](const HttpRequest& req) {
    HttpResponse resp;
    resp.setStatusCode(200);
    resp.setBody("<h1>Home Page</h1>");
    return resp;
  });

  server.registerHandler("/about", [](const HttpRequest& req) {
    HttpResponse resp;
    resp.setStatusCode(200);
    resp.setBody("<h1>About Us</h1>");
    return resp;
  });

  // 启动服务器
  server.start();

  return 0;
}
