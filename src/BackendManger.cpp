
#include "BackendManger.h"
#include "nlohmann/json.hpp"
#include <fstream>
#include <sys/stat.h>

using json = nlohmann::json;

BackendManger::BackendManger(const std::string& config_file){


}

  //加载配置文件
bool BackendManger::loadConfig(){

  std::lock_guard<std::mutex> lock(mutex_);

  std::ifstream file(config_file_);
  if( !file.is_open() ){
    std::cerr << "open config file failed!" << std::endl;
    return false;
  }

  try{

    //清空后端服务器列表
    backends_.clear();

    json config;
    file >> config;
    for(const auto& backend : config["backend"]){
      std::string ip = backend["ip"];
      int port = backend["port"];
      int weight = backend.value("weight", 1);

      //将后端服务器添加weight次
      for(int i = 0; i < weight; ++i){
        backends_.push_back(std::make_shared<BackendServer>(ip, port));
      }
    }

    //解析负载均衡策略
    load_balance_strategy_ = config.value("load_balance_strategy", "round_robin");

    //重置计数器
    current_index_ = 0;

    //更新文件最近一次被修改时间
    struct stat file_stat;
    if( stat(config_file_.c_str(), &file_stat) == 0 ){
      last_modified_ = file_stat.st_mtime;
    }

    return true;

  }catch(const std::exception& e){

    std::cerr << "Error parsing config : " << e.what() << std::endl;
    return false;
  }
  
}

//检查配置文件是否更新
bool BackendManger::checkConfigUpdate(){

  struct stat file_stat;
  if( stat(config_file_.c_str(), &file_stat) != 0 ){
    return false;
  }
  if( file_stat.st_mtime > last_modified_ ){
    
    std::cout << "config file update..., reload.." << std::endl;
    return loadConfig();
  }
  return false;
}

//获取下一个后端服务器（负载均衡）
std::shared_ptr<BackendServer> BackendManger::getNextBackend(){


}

//发送请求到后端
std::string BackendManger::sendRequest(const std::string& request){


}


