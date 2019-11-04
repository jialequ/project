/*****************************************************************
*   Copyright (C) 2019 * Ltd. All rights reserved.
*   
*   File name   : tcpsocket.hpp
*   Author      : qujiale
*   Created date: 2019-05-22
*   Description : 在这里属于用c++封装tcpsocket类
*     创建套接字
*       bool Socket()
*     绑定地址信息
*       bool Bind(std::string &ip, uint16_t port)
*     服务端开始监听
*       bool Listen(int baklog = 5)
*     客户端发起链接请求
*       bool Connect(std::string &ip, uint16_t port)
*     服务端获取已经链接完成的客户端新建的socket
*       bool Accept(TcpSocket clisock, struct sockaddr_in *addr)
*
*     接收数据
*       bool 非常危险
*       bool Recv(std::string &buf)
*     发送数据
*       bool Send(std::string &buf)
*     关闭套接字
*       bool Close()
*****************************************************************/
#pragma once
#include <iostream>
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h> 
#include <fcntl.h>
using namespace std;

#define CHECK_RET(q) if((q) == false){return -1;}

class TcpSocket
{
    public:
        TcpSocket()
            :_sockfd(-1)
        {}
        void SetFd(int fd)
        {
            _sockfd = fd;
        }
        int GetFd()
        {
            return _sockfd;
        }
        void SetNonblock()
        {
            int flag = fcntl(_sockfd, F_GETFL, 0);
            fcntl(_sockfd, F_SETFL, O_NONBLOCK | flag);
        }
        bool SocketInit(int port)
        {
            //1.create socket
            _sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if(_sockfd < 0)
            {
                cerr << "socket error" << endl;
                return false;
            }
            //2.bind socket
            //addr reuse
            int opt = 1;
            setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));

            sockaddr_in addr;
            addr.sin_addr.s_addr = INADDR_ANY;
            addr.sin_port = htons(port);
            addr.sin_family = AF_INET;
            socklen_t len = sizeof(addr);
            int ret = bind(_sockfd, (struct sockaddr*)&addr, len);
            if(ret < 0)
            {
                cerr << "bind error" << endl;
                close(_sockfd);
                return false;
            }
            //3.start to listen
            ret = listen(_sockfd, 10);
            if(ret < 0)
            {
                cerr << "listen error" << endl;
                close(_sockfd);
                return false;
            }

            return true;
        }
        bool SocketConnect(string &ip, uint16_t port)
        {
            sockaddr_in srv_addr;
            srv_addr.sin_family = AF_INET;
            srv_addr.sin_addr.s_addr = inet_addr(&ip[0]);
            srv_addr.sin_port = htons(port);
            socklen_t len = sizeof(srv_addr);
            int ret = connect(_sockfd, (sockaddr*)&srv_addr, len);
            if(ret < 0)
            {
                cerr << "connect error" << endl;
                return false;
            }
            return true;
        }
        bool SocketAccept(TcpSocket &newsock)
        {
            sockaddr_in addr;
            socklen_t len = sizeof(addr);
            int fd = accept(_sockfd, (sockaddr*)&addr, &len);
            if(fd < 0)
            {
                cerr << "accept error" << endl;
                return false;
            }
            newsock.SetFd(fd);
            newsock.SetNonblock();
            return true;
        }
        bool SocketSend(const string &buf)
        {
            size_t slen = 0;
            while(slen < buf.size())
            {
                int ret = send(_sockfd, &buf[slen], buf.size() - slen, 0);
                if(ret < 0)
                {
                    if(errno == EAGAIN)
                    {
                        usleep(1000);
                        continue;
                    }
                    cerr << "send error" << endl;
                    return false;
                }
                slen += ret;
            }
            return true;
        }
        bool SocketRecvPeek(string& buf)
        {
            buf.clear();
            char tmp[8192] = {0}; 
            int ret = recv(_sockfd, tmp, 8192, MSG_PEEK);
            if(ret <= 0)
            {
                if(errno == EAGAIN)
                {
                    return true;
                }
                cerr << "recv error" << endl;
                return false;
            }
            buf.assign(tmp, ret);
            return true;
        }
        bool SocketRecv(string &buf, int len)
        {
            buf.resize(len);
            int rlen = 0, ret;
            while(rlen < len)
            {
                ret = recv(_sockfd, &buf[0] + rlen, len - rlen, 0);
                if(ret <= 0)
                {
                    if(errno == EAGAIN)
                    {
                        usleep(1000);
                        continue;
                    }
                    return false;
                }
                rlen += ret;
            }
            return true;
        }
        bool SocketClose()
        {
            if(_sockfd > 0)
            {
                close(_sockfd);
                _sockfd = -1;
            }
            return true;
        }
    private:
        int _sockfd;
};
