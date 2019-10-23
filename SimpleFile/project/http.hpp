#include "tcpsocket.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
using namespace std;
class HttpRequest
{
    public:
        string _method;
        string _path;
        unordered_map<string, string> _param;
        unordered_map<string, string> _headers;
        string _body;
    public:
        bool RecvHeader(TcpSocket& sock, std::string header)
        {
            //接收头部
            //1.探测性接收大量数据
            string tmp;
            if(sock.SocketRecvPeek(tmp) == false)
            {
                return false;
            }
            //2.判断是否包含整个头部\r\n
            size_t pos;
            pos = tmp.find_first_of("\r\n\r\n");
            if(pos == string::npos && tmp.size() == 8192)
            {
                return false;
            }
            //3.判断当前接收的数据长度
            //4.若包含正个头部, 则直接获取头部
        }
        bool FirstLineParse(std::string &line);//解析首行
        bool PathIsLegal();//请求资源是否合法
    public:
        int  RequestParse(TcpSocket &sock)
        {
            //1.接收HTTP头部
            string header;
            if(RecvHeader(sock, header) == false)
            {
                return 400;
            }
            //2.对头部进行分割(\r\n),得到list,使用boost库split
            
            //3.list[0]-首行,进行首行解析
            //4.头部解析
            //5.请求信息检验
            return 200;
        }
};
class HttpResponse
{

};
