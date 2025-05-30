
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
#include <functional>
#include <memory>

#define MAX_EVENTS 128

//辅助类
//这个类封装了与单个客户端的连接相关的所有信息和操作
class Connection{

private:
  int client_fd_;
  struct sockaddr_in client_addr_;
  std::string read_buffer_;
  std::string write_buffer_;

public:
  Connection(int fd, const struct sockaddr_in& addr) : client_fd_(fd), client_addr_(addr){}

  //核心功能
  //读取数据到一块临时缓冲区，然后将这部分数据追加到 read_buffer_ 中
  bool readData(){

    char temp_buffer[4096];
    memset(temp_buffer, 0, sizeof(temp_buffer));
    int len = recv(client_fd_, temp_buffer, sizeof(temp_buffer) - 1, 0);
    if( len > 0 ){
      temp_buffer[len] = '\0';
      read_buffer_ += temp_buffer;
      return true;

    }else if( len == 0 ){ //客户端关闭了连接或发生了其他错误

      return false;

    }else if( errno == EAGAIN || errno == EWOULDBLOCK ){//这表示当前没有数据可读。在非阻塞模式下，这是正常情况

      return true;

    }else{

      return false;
    }
  }

  //尝试将 write_buffer_ 中的数据发送给客户端
  //如果数据成功发送了一部分，就从 write_buffer_ 中移除已发送的部分
  bool writeData(){

    int len = send(client_fd_, write_buffer_.c_str(), write_buffer_.length(), 0);
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

  // 尝试从 read_buffer_ 中提取一条以 '\n' 结尾的消息
  // 如果成功，消息被放入 out_message，并从 read_buffer_ 中移除，返回 true
  // 否则返回 false
  bool popMessage(std::string& out_message) {
    size_t newline_pos = read_buffer_.find('\n');
    if (newline_pos != std::string::npos) {
      // 提取消息 (包括换行符，如果需要可以 newline_pos + 1)
      out_message = read_buffer_.substr(0, newline_pos + 1);
      // 从缓冲区移除已提取的消息
      read_buffer_.erase(0, newline_pos + 1);
      return true;
    }
    return false; // 没有找到完整的消息
  }

  //基本访问器
  int getFd() const { return client_fd_; }
  std::string getReadBuffer() const { return read_buffer_; }
  void appendToWriteBuffer(const std::string& data) { write_buffer_ += data; }
  bool hasDataToWrite() const { return !write_buffer_.empty(); }

  //Connection类中添加getClientIP方法用于获取连接过来的客户端IP信息
  std::string getClientIP() const{
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    if( getpeername(client_fd_, (struct sockaddr*)&addr, &addr_len ) == 0 ){

      char ip_str[INET_ADDRSTRLEN];//INET_ADDRSTRLEN的大小是16字节。定义在#include<netinet/in.h>或#include<arpa/inet.h>
      inet_ntop(AF_INET, &(addr.sin_addr), ip_str, INET_ADDRSTRLEN);

      return std::string(ip_str);
    }
    return "Unknown";
  }

  ~Connection(){
    if( client_fd_ != -1 ){
      close(client_fd_);
      client_fd_ = -1;
    }
  }

};

class TcpServer{

public:
  using CallBack = std::function<void(std::shared_ptr<Connection>, const std::string&)>;

private:
  int epoll_fd_;  //epoll 实例的文件描述符，epoll 是 Linux 内核提供的一种高效的 I/O 事件通知机制
  int server_fd_; //服务器监听套接字的文件描述符。服务器通过这个套接字等待新的客户端连接请求
  std::vector<struct epoll_event> events_; //存储 epoll_wait 函数返回的、发生了事件的文件描述符的信息
  std::map< int, std::shared_ptr<Connection> > connections_;   //管理所有客户端连接
  bool running;
  CallBack message_callback_;

public:
  TcpServer(int port);

  void start();
  void setMessageCallBack(CallBack cb){ message_callback_ = std::move(cb); }//用户通过这个方法告诉服务器，当收到客户端数据时应该调用哪个函数来处理

  ~TcpServer();

private:
  void setNonBlocking(int fd);
  void closeConnection();

  //连接管理
  void addConnection(int fd, struct sockaddr_in client_addr);
  void removeConnection(int fd);

  //事件管理
  void handleEvents();
  void handleRead(int fd);
  void handleWrite(int fd);
  void handleError(int fd);
  void acceptNewConnection();

};



#endif
