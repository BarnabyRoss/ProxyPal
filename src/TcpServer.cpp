
#include "TcpServer.h"

TcpServer::TcpServer(int port) : events_(MAX_EVENTS){

  server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
  if( server_fd_ < 0 ){
    std::cerr << "socket server create failed!" << std::endl;
    exit(EXIT_FAILURE);
  }

  //设置地址重用
  int opt = 1;
  setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  //设置非阻塞
  setNonBlocking(server_fd_);

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  if( bind(server_fd_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0 ){
    std::cerr << "bind server address failed!" << std::endl;
    close(server_fd_);
    exit(EXIT_FAILURE);
  }

  if( listen(server_fd_, 128) < 0 ){
    std::cerr << "listen failed!" << std::endl;
    close(server_fd_);
    exit(EXIT_FAILURE);
  }

  epoll_fd_ = epoll_create1(0);
  if( epoll_fd_ < 0 ){
    std::cerr << "epoll create failed!" << std::endl;
    close(server_fd_);
    exit(EXIT_FAILURE);
  }

  struct epoll_event ev;
  ev.events = EPOLLIN;
  ev.data.fd = server_fd_;
  if( epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, server_fd_, &ev) < 0 ){
    std::cerr << "epoll add server fd failed!" << std::endl;
    close(server_fd_);
    close(epoll_fd_);
    exit(EXIT_FAILURE);
  }
  events_.push_back(ev);
}

void TcpServer::start(){

  running = true;

  while( running ){

    handleEvents();
  }
}

TcpServer::~TcpServer(){

  closeConnection();
}

void TcpServer::setNonBlocking(int fd){

  int flag = fcntl(fd, F_GETFL, 0);
  fcntl(fd, F_SETFL, flag | O_NONBLOCK);
}

void TcpServer::acceptNewConnection(){

  struct sockaddr_in client_addr;
  socklen_t client_addr_len = sizeof(client_addr);
  int client_fd = accept(server_fd_, (struct sockaddr*)&client_addr, &client_addr_len);
  if( client_fd < 0 ){
    std::cerr << "accept new connection failed!" << std::endl;
    return;
  }

  setNonBlocking(client_fd);

  struct epoll_event ev;
  ev.events = EPOLLIN;
  ev.data.fd = client_fd;
  if( epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, client_fd, &ev) < 0 ){
    std::cerr << "epoll add client fd failed!" << std::endl;
    close(client_fd);
    return;
  }
  events_.push_back(ev);
  addConnection(client_fd, client_addr);
}

void TcpServer::closeConnection(){

  close(server_fd_);
  close(epoll_fd_);
}

void TcpServer::addConnection(int fd, struct sockaddr_in client_addr){

  std::shard_ptr<Connection> conn = std::make_shared<Connection>(fd, client_addr);

  connections_[fd] = conn;
}

void TcpServer::removeConnection(int fd){

  epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr);

  connections_.erase(fd);

  std::cout << "Connection close fd : " << fd << std::endl;
}

void TcpServer::handleEvents(){

  int active = epoll_wait(epoll_fd_, events_.data(), MAX_EVENTS, -1);
  if( active < 0 ){
    if( errno == EINTR ){
      continue;
    }else{
      std::cerr << "epoll wait failed!" << std::endl;
      break;
    }
  }

  for(int i = 0; i < active; ++i){
    size_t fd = events_[i].data.fd;
    if( fd == server_fd_ ){ //服务端server接收到的请求事件
      acceptNewConnection();
    }else if( events_[i].events & EPOLLIN ){
      handleRead(events_[i].data.fd);
    }else if( events_[i].events & EPOLLOUT ){
      handleWrite(events_[i].data.fd);
    }else if( events_[i].events & (EPOLLHUP | EPOLLERR | EPOLLRDHUP) ){
      handleError(events_[i].data.fd);
    }
  }

}

void TcpServer::handleRead(int fd){

  auto it = connections_.find(fd);
  if( it == connections_.end() ) return;

  std::shared_ptr<Connection> conn = it->second();

  if( !conn->readData() ){
    removeConnection(fd);
    return;
  }

  if( message_callback_ && !conn->getReadBuffer().empty() ){
    message_callback_(conn, conn->getReadBuffer());
  }
}

void TcpServer::handleWrite(int fd){

  auto it = connections_.find(fd);
  if( it == connections_.end() ) return;

  std::shared_ptr<Connection> conn = it->second();

  if( !conn->writeData() ){
    removeConnection(fd);
    return;
  }
}

void TcpServer::handleError(int fd){

  removeConnection(fd);
}
