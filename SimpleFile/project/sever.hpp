/**********************************************************************
 *    > File Name    : sever.cpp
 *    > Author       : qujaile
 *    > Created Time : 2019年10月21日 星期一 14时45分01秒
 *    > Description  : 实现sever类, 完成服务端的整体结构流程 
 **********************************************************************/
#pragma once
#include <vector>
#include "tcpsocket.hpp"
#include "epollwait.hpp"
#include "threadpool.hpp"
#include "http.hpp"
using namespace std;
class Server
{
    public:
        bool Start(int port)
        {
            bool ret;
            //init中实现创建套接字, 绑定地址, 开始监听
            ret = _lst_sock.SocketInit(port);
            if(ret == false)
            {
                return false;
            }
            //创建并初始化epoll, 套接字未就绪的话就不用一直等待, 让操作系统来监听
            //如果客户端一直不发送消息就会占用线程池, 所以我们采用事件总线进行监控, 这样不会占用资源
            ret = _epoll.EpollInit();
            if(ret == false)
            {
                return false;
            }
            //pool init
            ret = _pool.PoolInit();
            if(ret == false)
            {
                return false;
            }
            //将监听套接字添加到epoll红黑树中
            _epoll.EpollAdd(_lst_sock);
            while(1)
            {
                //epoll开始监听, 如果有就绪的描述符则添加进链表中, 然后将就绪的描述符全部pushback进list中
                vector<TcpSocket> list;
                ret = _epoll.EpollWait(list);
                if(ret == false)
                {
                    continue;
                }
                for(size_t i = 0;i < list.size();i++)
                {
                    if(list[i].GetFd() == _lst_sock.GetFd())
                    {
                        //如果是监听套接字就获取新连接, 然后将新的描述符添加进红黑树中
                        TcpSocket cli_sock;
                        ret = _lst_sock.SocketAccept(cli_sock);
                        if(ret == false)
                        {
                            return false;
                        }
                        cli_sock.SetNonblock();
                        _epoll.EpollAdd(cli_sock);
                    }
                    else 
                    {
                        //不是监听套接字则组织一个任务抛进线程池, 设置任务(socket和对应的处理函数), 添加进任务队列
                        //但是这里注意, 一定要在_epoll中Del, epoll中包含每一个sockfd, 要是不进行删除, 会一直触发事件
                        ThreadTask tt;
                        tt.SetTask(list[i].GetFd(), ThreadHandler);
                        _pool.TaskPush(tt);
                        _epoll.EpollDel(list[i]);
                    }
                }
            }
            _lst_sock.SocketClose();
            return true;
        }
    public:
        static void ThreadHandler(int sockfd)
        {
            TcpSocket sock;
            sock.SetFd(sockfd);
            //1.请求类, 进行请求解析
            HttpRequest req;
            //回复类, 组织回复数据
            HttpResponse rsp;
            int status = req.RequestParse(sock);
            if(status != 200)
            {
                //则直接响应错误
                rsp.status = status;
                rsp.ErrorProcess();
                sock.SocketClose();
                return;
            }
            //2.根据req进行处理
            rsp.NormalProcess(sock);
            //当前采用短连接, 直接处理完毕后关闭套接字
            sock.SocketClose();
            return;
        }
        bool HttpProcess(HttpRequest &req, HttpResponse &rsp);
    private:
        TcpSocket _lst_sock;
        ThreadPool _pool;
        Epoll _epoll;
};
