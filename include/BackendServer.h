
#ifndef BACKEND_SERVER_H__
#define BACKEND_SERVER_H__

#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>

class BackendServer{

private:
  std::string ip_;
  int port_;

public:
  BackendServer(const std::string& ip, int port);

  std::string sendRequest(const std::string& request);

  std::string getHost() const;
  int getPort() const;

  ~BackendServer();
};



#endif

