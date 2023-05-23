//
// Created by Rambo.Liu on 2023/5/22.
//

#ifndef M3U8_TCPSERVER_HPP
#define M3U8_TCPSERVER_HPP


#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "../utils/Log.hpp"

#define BACKLOG 5

/**
 * socket Server 端
 */
class TcpServer {
private:
    int _port;
    int _listen_sock;       // 监听套接字
    static TcpServer *_svr; // 指向单例对象的static指针

private:
    TcpServer(int port) : _port(port), _listen_sock(-1) {

    }

    /**
     * 禁止拷贝构造
     */
    TcpServer(const TcpServer &) = delete;

    /**
     * 禁止通过重载= 构造对象，其实也是通过拷贝构造 实例对象
     * @return
     */
    TcpServer *operator=(const TcpServer &) = delete;

public:
    static TcpServer *getInstance(int port) {
        static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
        if (_svr == nullptr) {
            pthread_mutex_lock(&mtx);
            // 为什么要两个if？ 原因:当首个拿锁者完成了对象创建，之后的线程都不会通过第一个if了，而这期间阻塞的线程开始唤醒，它们则需要靠第二个if语句来避免再次创建对象。
            if (_svr == nullptr) {
                _svr = new TcpServer(port);
                _svr->InitServer();
            }
            pthread_mutex_unlock(&mtx);
        }
        return _svr;
    }

    void InitServer() {
        Socket(); // 创建
        Bind();   // 绑定
        Listen(); // 监听
        LOG("TcpServer Init Success");
    }

    /**
     * 创建监听套接字
     */
    void Socket() {
        _listen_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (_listen_sock < 0) {
            LOG("socket error!");
            exit(1);
        }
        int opt = 1;// 将 SO_REUSEADDR 设置为 1 将允许在端口上快速重启套接字
        setsockopt(_listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        LOG("creat listen_sock success");
    }

    void Bind() // 绑定端口
    {
        struct sockaddr_in local{};
        memset(&local, 0, sizeof(local));
        local.sin_family = AF_INET;
        local.sin_port = htons(_port);
        local.sin_addr.s_addr = INADDR_ANY;

        if (bind(_listen_sock, (struct sockaddr *) &local, sizeof(local)) < 0) {
            LOG("bind error");
            exit(2);
        }
        LOG("port bind listen_sock success");
    }

    void Listen() const // 监听
    {
        if (listen(_listen_sock, BACKLOG) < 0) // 声明_listen_sock处于监听状态,并且最多允许有backlog个客户端处于连接等待状态,如果接收到更多的连接请求就忽略
        {
            LOG("listen error");
            exit(3);
        }
        LOG("listen listen_sock success");
    }

    int Sock() const // 获取监听套接字fd
    {
        return _listen_sock;
    }

    ~TcpServer() {
        if (_listen_sock >= 0) {
            close(_listen_sock);
        }
    }
};

// 单例对象指针初始化
TcpServer *TcpServer::_svr = nullptr;


#endif //M3U8_TCPSERVER_HPP
