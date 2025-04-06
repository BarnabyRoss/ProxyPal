
#include "HttpServer.h"


int main(int argc, char* argv[]){

  int port = 5000;
  std::string config_file = "etc/backend_servers.json";
  size_t thread_count = 4;
  //HttpServer server(5000, "127.0.0.1", 5050);

  // 用户注册URL处理函数
  /*server.registerHandler("/", [](const HttpRequest& req) {
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
  });*/

  try{
    HttpServer server(port, config_file, thread_count);
    // 启动服务器
    server.start();
  }catch(const std::exception& e){
    std::cerr << "Error: " << e.what() << cout << endl;
    return 1;
  }

  

  return 0;
}
