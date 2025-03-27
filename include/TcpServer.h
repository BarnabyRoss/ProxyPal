
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
#include <cstring>

//辅助类
class Connection{

private:
  int socket_fd_;
  struct sockaddr_in client_addr_;
  std::string read_buffer_;
  std::string write_buffer_;

public:
  Connection(int fd, const struct sockaddr_in& addr) : socket_fd_(fd), client_addr_(addr) {}

  //核心功能
  bool readData(){

    char temp_buffer[4096];
    memset(temp_buffer, 0, sizeof(temp_buffer));
    int len = recv(socket_fd_, temp_buffer, sizeof(temp_buffer) - 1, 0);
    if( len > 0 ){
      temp_buffer[len] = '\0';
      read_buffer_ += temp_buffer;
      return true;

    }else if( len == 0 ){

      return false;

    }else if( errno == EAGAIN || errno == EWOULDBLOCK ){

      return true;

    }else{

      return false;
    }
  }

  bool writeData(){

    int len = send(socket_fd_, write_buffer_.c_str(), write_buffer_.length(), 0);
    if( len > 0 ){

      write_buffer_.erase(0, len);
      return true;

    }else if( len == 0 ){

      return false;

    }else if( errno == EAGAIN || errno == EWOULDBLOCK ){

      return true;

    }else if( errno == EINTR ){
  
      return true;

    }else{

      return false;
    }
  }

  //基本访问器
  int getFd() const { return socket_fd_; }
  std::string getReadBuffer() const { return read_buffer_; }
  void appendToWriteBuffer(const std::string& data) { write_buffer_ += data; }
  bool hasDataToWrite() const { return !write_buffer_.empty(); }

  ~Connection() { close(socket_fd_); }

};

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
