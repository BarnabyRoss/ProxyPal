
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
#include <map>

//辅助类
class Connection{

private:
  int client_fd_;
  struct sockaddr_in client_addr_;
  std::string read_buffer_;
  std::string write_buffer_;

public:
  Connection(int fd, const struct sockaddr_in& addr) : client_fd_(fd), client_addr_(addr) {}

  //核心功能
  bool readData(){

    char temp_buffer[4096];
    memset(temp_buffer, 0, sizeof(temp_buffer));
    int len = recv(client_fd_, temp_buffer, sizeof(temp_buffer) - 1, 0);
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
  int getFd() const { return client_fd_; }
  std::string getReadBuffer() const { return read_buffer_; }
  void appendToWriteBuffer(const std::string& data) { write_buffer_ += data; }
  bool hasDataToWrite() const { return !write_buffer_.empty(); }

  ~Connection(){
    if( client_fd_ != -1 ){
      close(client_fd_);
      client_fd_ = -1;
    }
  }

};

class TcpServer{

private:
  int epoll_fd_;
  int server_fd_;
  std::vector<struct epoll_event> events_;
  std::map< int, std::shard_ptr<Connection> > connections_;   //管理所有客户端连接

public:
  TcpServer(int port);

  void start();

  ~TcpServer();

private:
  void setNonBlocking(int fd);
  void closeConnection();

  //连接管理
  void addConnection(int fd, struct sockaddr_in client_addr);
  void removeConnection(int fd);

  //事件管理
  void handleEvent();
  void handleRead(int fd);
  void handleWrite(int fd);
  void acceptNewConnection();

};



#endif
