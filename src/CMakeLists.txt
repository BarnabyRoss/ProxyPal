
add_executable(ProxyPal main.cpp HttpServer.cpp TcpServer.cpp HttpParser.cpp HttpResponse.cpp ThreadPool.cpp BackendServer.cpp)

target_link_libraries(ProxyPal PRIVATE nlohmann_json::nlohmann_json)


