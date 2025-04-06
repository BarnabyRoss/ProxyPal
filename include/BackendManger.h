
#ifndef BACKEND_MANGER_H__
#define BACKEDN_MANGER_H__

#include <iostream>
#include <vector>
#include <memory>
#include <ctime>
#include <mutex>
#include "BackendServer.h"

class BackendManger{

private:
  std::vector< std::shared_ptr<BackendServer> > backends_;
  std::string config_file_;
  std::time_t last_modified_;
  std::string load_balance_strategy_; //负载均衡策略
  int current_index_;  //轮询
  std::mutex mutex_;

public:
  BackendManger(const std::string& config_file);

  //加载配置文件
  bool loadConfig();

  //检查配置文件是否更新
  bool checkConfigUpdate();

  //获取下一个后端服务器（负载均衡）
  std::shared_ptr<BackendServer> getNextBackend();

  //发送请求到后端
  std::string sendRequest(const std::string& request);

};


#endif

