/**********************************************************************
 *    > File Name    : upload.cpp
 *    > Author       : qujaile
 *    > Created Time : 2019年10月29日 星期二 14时06分45秒
 *    > Description  : 子进程CGI处理上传请求
 **********************************************************************/

#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sstream>
#include <vector>
using namespace std;

#define WWW_ROOT "./www/"

class Boundary
{
    public:
        int64_t _start_addr;
        int64_t _data_len;
        string _name;
        string _filename;
};

bool GetHeader(const string& key, string& val)
{
    string body;
    char* ptr = getenv(key.c_str());
    if(ptr == NULL)
    {
        return false;
    }
    val = ptr;
    return true;
}

bool HeaderParse(string& header, Boundary& file)
{
    vector<string> list;
    boost::split(list, header, boost::is_any_of("\r\n"), boost::token_compress_on);
    for(size_t i = 0; i < list.size(); i++)
    {
        string sep = ": ";
        size_t pos = list[i].find(sep);
        if(pos == string::npos)
        {
            return false;
        }
        string key = list[i].substr(0, pos);
        string val = list[i].substr(pos + sep.size());
        if(key != "Content-Dispostion")
        {
            continue;
        }
        string name_field = "fileupload";
        string filename_sep = "filename=\"";

        pos = val.find(name_field);
        if(pos == string::npos)
        {
            continue; 
        }
        pos = val.find(filename_sep);
        if(pos == string::npos)
        {
            return false;
        }
        pos += filename_sep.size();
        size_t next_pos = val.find("\"", pos);
        if(next_pos == string::npos)
        {
            return false;
        }
        file._filename = val.substr(pos, next_pos - pos);
        file._name = "fileupload";
    }
    return true;
}

bool BoundaryParse(string& body, vector<Boundary>& list)
{
    string cont_b = "boundary=";
    string tmp; 
    if(GetHeader("Content-Type", tmp) == false)
    {
        return false;
    }
    size_t pos = tmp.find(cont_b);
    if(pos == string::npos)
    {
        return false;
    }

    string boundary = tmp.substr(pos + cont_b.size());
    string dash = "--";
    string craf = "\r\n";
    string tail = "\r\n\r\n";
    string f_boundary = dash + boundary + craf;
    string m_boundary = craf + dash + boundary;
    size_t nex_pos;
    //找第一个boundary
    pos = body.find(f_boundary);
    if(pos != 0)
    {
        //如果f_boundary的位置不是起始位置, 则错误
        cerr << "first boundary error" << endl;
        return false;
    }
    //找第一块头部起始位置
    nex_pos = pos + f_boundary.size();
    while(pos <= body.size())
    {
        //找寻头部结尾
        pos = body.find(tail, nex_pos);
        if(pos == string::npos)
        {
            return false;
        }
        //获取头部
        //nex_pos只想数据的起始地址
        nex_pos = pos + tail.size();
        //找\r\n--boundary, 如果没有则格式错误
        pos = body.find(m_boundary, nex_pos);
        if(pos == string::npos)
        {
            return false;
        }
        int64_t offset = nex_pos;
        //下一个boundary的起始地址 - 数据的起始地址, 数据长度
        int64_t len = pos - nex_pos;
        nex_pos = pos + m_boundary.size();
        pos = body.find(craf, nex_pos);
        if(pos == string::npos)
        {
            return false;
        } 
        //pos指向下一个m_boundary的头部起始地址
        //若没有m_boundary了, 则指向数据结尾, pos = body.size()
        pos += craf.size(); 
        Boundary node;
        node._data_len = len;
        node._start_addr = offset;
        //解析头部
        string header = body.substr(pos, nex_pos - pos);
         
    }
    return true;
}

bool StorageFile(string& body,vector<Boundary>& list)
{
    for(size_t i = 0; i < list.size(); i++)
    {
        if(list[i]._name != "fileupload")
        {
            continue;
        }
        string realpath = WWW_ROOT + list[i]._filename;
    }
    return true;
}

int main(int argc, char* argv[], char* env[])
{
    for(int i = 0; env[i] != NULL; i++)
    {
        cerr << "env[i]=====[" << env[i] << "]" << endl;
    }
    string body;
    char* con_len = getenv("Content-Length");
    if(con_len != NULL)
    {
        stringstream tmp;
        tmp << con_len;
        int64_t filesize;
        tmp >> filesize;

        body.resize(filesize);
        int rlen = 0;
        int ret;
        while(rlen < filesize)
        {
            ret = read(0, &body[0] + rlen, filesize - rlen);
            if(ret <= 0)
            {
                exit(-1);
            }
            rlen += ret;
        }
        cerr << "body:[" << body << "]" << endl;
        vector<Boundary> list;
        BoundaryParse(body, list);
        StorageFile(body, list);
    } 
    return 0;
}
