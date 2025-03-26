
#ifndef __TCP_SERVER_H__
#define __TCP_SERVER_H__

#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <errno.h>
#include <vector>
#include <unistd.h>
#include <fcntl.h>

class TcpServer{

private:
  int epoll_fd_;
  int server_fd_;
  std::vector<struct epoll_event> events_;
  std::vector<int> clients_;

public:
  TcpServer(int port);



  ~TcpServer();

private:
  void setNonBlocking(int fd);
  void acceptNewConnection();
  void closeConnection();

};



#endif
