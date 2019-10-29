/**********************************************************************
 *    > File Name    : upload.cpp
 *    > Author       : qujaile
 *    > Created Time : 2019年10月29日 星期二 14时06分45秒
 *    > Description  : 子进程CGI处理上传请求
 **********************************************************************/

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sstream>
using namespace std;


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
    } 
    return 0;
}
