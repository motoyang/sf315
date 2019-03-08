#include <iostream>
#include <sstream>

#include <nanolog.hpp>

// --

class A {};

int main(int argc, char *argv[]) {
  nlog::Logger logger(nlog::NonGuaranteedLogger(2), "./", "nlog", 1, 5);

  // nlog::set_log_level(nlog::LogLevel::WARN);
  LOG_WARN << "test: " << 100 << ", test2: " << 1.1 << ", test3: " << 3
           << ", test4: " << (unsigned long long)5;
  LOG_CRIT << "test_bool: " << true << ". ";

  LOG_CRIT
      << "test_bool: " << false << ". "
      << "BIND请求通常用于那些要求客户端接受来自服务器的连接的协议上。FTP是一个"
         "典型的例子，它使用一个从客户端到服务器的主连接来进行执行命令和接受状"
         "态报告，但是也可以使用一个服务器到客户端连接来根据需要传输数据（例如L"
         "S、GET、PUT）。如果人们希望某个应用协议的客户端可以使用BIND请求，那么"
         "需要先使用CONNECT命令建立主连接后才可以使用BIND命令建立第二个连接。在"
         "这个操作中SOCKS服务器将使用DST.ADDR和DST."
         "PORT来评估这个BIND请求。在一个BIND请求的操作过程中，SOCKS服务器要发送"
         "两个应答给客户端。当服务器建立并绑定一个新的端口时发送第一个应答。BND"
         ".POR字段包含SOCKS服务器用来监听进入的连接的端口号，BND."
         "ADDR字段包含了对应的IP地址。客户端通常使用这些信息来告诉（通过主连接"
         "或控制连接）应用服务器连接的汇接点。第二个应答仅发生在所期望到来的连"
         "接成功或失败之后。在第二个应答中，BND.PORT和BND."
         "ADDR字段包含了连接主机的 IP地址和端口号。";
  LOG_CRIT << "test_bool: " << true << ". "
           << "t = " << 2.345 << ", f = " << (long double)1.32e13;

  std::stringstream ss;
  ss << (int)1 << " abcd " << (int)23456 << " " << 1.23f;
  char *p = ss.str().data();

  int i1, i2;
  std::string s;
  float f;
  ss >> i1 >> s >> i2 >> f;
  std::cout << "i1 = " << i1 << ", s = " << s << ", i2 = " << i2
            << ", f = " << f << std::endl;

  return 0;
}
