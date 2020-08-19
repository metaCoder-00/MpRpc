# 一个基于 muduo 和 protobuf 的 RPC 分布式网络通信框架项目
本项目仅供学习参考使用，可以直接在项目根目录下运行 autobuild.sh 文件进行build。


## 每个文件模块的功能
lockqueue.h: 实现一个线程安全的队列（读者-写者模型）


logger.h: 日志记录模块


mprpcapplication.h: 用单例模式实现一个记录 rpc 配置信息的对象


mprpcchannel.h: 用 protobuf 实现序列化与反序列化，帮助调用方实现远程调用


mprpcconfig.h: 加载 rpc 服务端的配置文件


mprpccontroller.h: 记录 rpc 远程调用过程中的控制信息


mprpcheader.pb.h: 由 protobuf 生成的网络消息传递类及其方法的声明


mprpcprovider.h: rpc 服务提供方注册远程服务及运行监听端口，等待调用请求的实现（muduo 实现）


zookeeper.h: zookeeper 用作 rpc 远程服务注册管理中心的相关实现

## 测试程序
测试程序位于 /bin 目录下，需要实现运行 zookeeper。