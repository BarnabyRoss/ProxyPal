
#include "TcpServer.h"

TcpServer::TcpServer(int port) : events_(MAX_EVENTS){

  server_fd_ = socket(AF_INET, SOCK_STREAM, 0);  //1. 创建套接字
  if( server_fd_ < 0 ){
    std::cerr << "socket server create failed!" << std::endl;
    exit(EXIT_FAILURE); //创建失败则退出程序
  }

  //设置地址重用(允许服务器快速重启，即使之前的连接还未完全释放)
  int opt = 1;
  setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); //2. 设置套接字选项

  //设置非阻塞
  setNonBlocking(server_fd_);  //3.将服务器套接字设为非阻塞

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port); //端口号 (htons: host to network short, 转换字节序)
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY); //IP 地址 (INADDR_ANY: 监听所有可用网络接口)
  if( bind(server_fd_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0 ){//4.绑定地址和端口
    std::cerr << "bind server address failed!" << std::endl;
    close(server_fd_); //绑定失败则关闭套接字并退出
    exit(EXIT_FAILURE);
  }

  if( listen(server_fd_, 128) < 0 ){ //5.开始监听连接 (128 是等待队列的最大长度)
    std::cerr << "listen failed!" << std::endl;
    close(server_fd_);  //监听失败则关闭套接字并退出
    exit(EXIT_FAILURE);
  }

  epoll_fd_ = epoll_create1(0); //6.创建 epoll 实例
  if( epoll_fd_ < 0 ){
    std::cerr << "epoll create failed!" << std::endl;
    close(server_fd_);
    exit(EXIT_FAILURE);
  }

  struct epoll_event ev; //epoll 事件结构体
  ev.events = EPOLLIN; //监控可读事件 (对于监听套接字，就是有新连接)
  ev.data.fd = server_fd_; //关联文件描述符为服务器套接字
  if( epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, server_fd_, &ev) < 0 ){ //7.将服务器套接字添加到 epoll 监控
    std::cerr << "epoll add server fd failed!" << std::endl;
    close(server_fd_);
    close(epoll_fd_);
    exit(EXIT_FAILURE);
  }
  //events_.push_back(ev);
}

void TcpServer::start(){

  running = true;  //设置服务器运行状态为 true

  while( running ){  //服务器主循环

    handleEvents();  //在循环中持续处理网络事件
  }
}

TcpServer::~TcpServer(){

  closeConnection();  //调用 closeConnection 清理资源
}

void TcpServer::setNonBlocking(int fd){

  int flag = fcntl(fd, F_GETFL, 0);  //获取文件描述符 fd 当前的标志
  fcntl(fd, F_SETFL, flag | O_NONBLOCK); //设置新的标志 (原有标志 或上 O_NONBLOCK)
}

void TcpServer::acceptNewConnection(){

  struct sockaddr_in client_addr;  //用于存储客户端地址信息
  socklen_t client_addr_len = sizeof(client_addr);
  int client_fd = accept(server_fd_, (struct sockaddr*)&client_addr, &client_addr_len); //1.接受新连接
  if( client_fd < 0 ){
    std::cerr << "accept new connection failed!" << std::endl; //接受失败则返回
    return;
  }

  setNonBlocking(client_fd); //2.将新的客户端套接字设为非阻塞

  struct epoll_event ev;
  ev.events = EPOLLIN;
  ev.data.fd = client_fd; //3.监控新客户端套接字的可读事件
  if( epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, client_fd, &ev) < 0 ){ //4.将新客户端套接字添加到 epoll
    std::cerr << "epoll add client fd failed!" << std::endl;
    close(client_fd);  //添加失败则关闭客户端套接字
    return;
  }
  //events_.push_back(ev);
  addConnection(client_fd, client_addr); //5.将新连接添加到服务器管理
}

void TcpServer::closeConnection(){

  close(server_fd_);
  close(epoll_fd_);
}

void TcpServer::addConnection(int fd, struct sockaddr_in client_addr){

  //使用 std::make_shared 创建一个 Connection 对象的智能指针
  std::shared_ptr<Connection> conn = std::make_shared<Connection>(fd, client_addr);

  connections_[fd] = conn; //将连接对象存入 map, 以 fd 为键
}

void TcpServer::removeConnection(int fd){

  epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr); //1.从 epoll 监控中移除 fd

  connections_.erase(fd);  //2.从 map 中移除连接对象

  std::cout << "Connection close fd : " << fd << std::endl; //打印日志
}

void TcpServer::handleEvents(){

  //1.等待 epoll 事件, -1 表示阻塞等待直到有事件发生
  int active = epoll_wait(epoll_fd_, events_.data(), MAX_EVENTS, -1);//active: 返回实际发生事件的文件描述符数量
  if( active < 0 ){ //epoll_wait 出错
    if( errno == EINTR ){ //如果是被信号中断
      //continue;  //可以选择继续循环 (在当前代码中，如果break/continue被注释，则会打印错误并可能退出循环)
      return;

    }else{ //发生其他严重错误
      std::cerr << "CRITICAL: epoll_wait failed: " << strerror(errno) << std::endl;
      running = false;  //设置标准，通知主循环停止
      return; //从handleEvents返回，主循环将终止
    }
  }

  //如果 active == 0 (epoll_wait 超时，但这里 timeout 是 -1，所以正常情况下不会是0)
  for(int i = 0; i < active; ++i){  //2.遍历所有活动的事件
    size_t fd = events_[i].data.fd; //获取发生事件的文件描述符
    if( fd == server_fd_ ){ //3.服务端server接收到的请求事件（如果是服务器监听套接字上的事件
      acceptNewConnection(); //处理新的连接请求
    }else if( events_[i].events & EPOLLIN ){ //4.如果是客户端套接字上的可读事件
      handleRead(events_[i].data.fd); //处理读事件
    }else if( events_[i].events & EPOLLOUT ){ //5.如果是客户端套接字上的可写事件
      handleWrite(events_[i].data.fd); //处理写事件 
    }else if( events_[i].events & (EPOLLHUP | EPOLLERR | EPOLLRDHUP) ){ //6.如果是错误或挂断事件
      handleError(events_[i].data.fd); //处理错误事件
    }
  }

}

void TcpServer::handleRead(int fd){

  auto it = connections_.find(fd); //在 map 中查找连接对象
  if( it == connections_.end() ) return; //如果没找到，直接返回

  std::shared_ptr<Connection> conn = it->second;  //获取连接对象的智能指针

  if( !conn->readData() ){ //调用 Connection 对象的 readData 方法读取数据
    removeConnection(fd); //如果 readData 返回 false (读取失败或连接关闭)
    return; //则移除连接并返回
  }

  // 2. 循环尝试从缓冲区中提取并处理完整的消息
  std::string message;
  while( conn->popMessage(message) ){ // popMessage 会修改 conn 内部的 read_buffer_
    if (message_callback_) {
      message_callback_(conn, message); // 将提取出的单个消息传递给回调
    }
    // 如果 popMessage 返回 true, message 变量会被填充新消息
    // 循环直到 popMessage 返回 false (缓冲区为空或只有不完整的消息)
  }
}

void TcpServer::handleWrite(int fd){

  auto it = connections_.find(fd);  //1.查找连接对象
  if( it == connections_.end() ) return;

  std::shared_ptr<Connection> conn = it->second;

  if( !conn->writeData() ){  //2.调用 Connection 对象的 writeData 方法发送数据
    removeConnection(fd);  //如果 writeData 返回 false (发送失败或连接关闭)
    return;   //则移除连接并返回
  }

  //3.如果数据发送完毕，应该修改 epoll 监听，移除 EPOLLOUT
  if( conn->write_buffer_.empty() ){

    struct epoll_event ev;
    ev.events = EPOLLIN;  //只关心读事件
    ev.data.fd = fd;
    epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev);
  }
}

void TcpServer::handleError(int fd){

  removeConnection(fd); //直接移除并清理发生错误的连接
}
