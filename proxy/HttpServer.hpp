//
// Created by Rambo.Liu on 2023/5/22.
//

#ifndef M3U8_HTTPSERVER_HPP
#define M3U8_HTTPSERVER_HPP


#include "TcpServer.hpp"
#include "Protocol.hpp"

/**
 * 本地http 代理 服务
 * 参考 https://linkylo.blog.csdn.net/article/details/129462699
 * 参考 git https://gitee.com/linkylo/HttpServer
 */
class HttpServer {

private:
    int _port;
    bool _startService;
    std::shared_ptr<ThreadPool> sp_thread;

public:
    explicit HttpServer(int port)
            : _port(port) {
        _startService = false;
    }

    ~HttpServer() {
        LOG("HttpServer 析构");
    }


    /**
     * 处理 http 请求
     * @param sock
     */
    static void HandlerRequest(void *sockPtr) {
        int *sock_n = (int *) sockPtr;
        printf("38-----------HandlerRequest begin sock = %d\n", *sock_n);
        int sock = *sock_n;
        EndPoint *ep = new EndPoint(sock);
        ep->RecvHttpRequest();          //读取请求
        if (!ep->IsStop()) {
            LOG("RecvHttpRequest Success");
            ep->HandlerHttpRequest();  //处理请求
            ep->BulidHttpResponse();   //构建响应
            ep->SendHttpResponse();    //发送响应
            if (ep->IsStop()) {
                LOG("SendHttpResponse Error, Stop Send HttpResponse");
            }
        } else {
            printf("RecvHttpRequest Error, Stop handler Response\n");
        }
        close(sock); //响应完毕，关闭与该客户端建立的套接字
        delete ep;
        ep = nullptr;
        LOG("http request task 执行完");
    }


    void InitServer() {
        _startService = true;
        sp_thread = std::make_shared<ThreadPool>(1, 3);
        //signal(SIGPIPE, SIG_IGN); // 直接粗暴处理cgi程序写入管道时崩溃的情况，忽略SIGPIPE信号,避免因为一个被关闭的socket连接而使整个进程终止
    }

    void Loop() {
        TcpServer *tsvr = TcpServer::getInstance(_port);
        int listen_sock = tsvr->Sock(); // 获取单例对象的监听套接字
        while (_startService) {
            LOG("监听客户端连接......");
            struct sockaddr_in peer{};
            memset(&peer, 0, sizeof(peer));
            socklen_t len = sizeof(peer);
            int sock = accept(listen_sock, (struct sockaddr *) &peer, &len);
            if (sock < 0) {
                continue;
            }
            std::string client_ip = inet_ntoa(peer.sin_addr);
            int client_port = ntohs(peer.sin_port);
            LOG("get a new link:[" + client_ip + ":" + std::to_string(client_port) + "]");

            // 构建请求任务并放入任务队列
            Task task;
            task.arg = &sock;
            task.function = HandlerRequest;
            sp_thread->addTask(task);
        }
        delete tsvr;
        tsvr = nullptr;
        LOG("服务器停止");
    }

    void stop() {
        _startService = false;
    }
};


#endif //M3U8_HTTPSERVER_HPP
