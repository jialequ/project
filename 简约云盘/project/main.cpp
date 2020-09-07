/**********************************************************************
 *    > File Name    : main.cpp
 *    > Author       : qujaile
 *    > Created Time : 2019年10月24日 星期四 09时04分27秒
 *    > Description  :  
 **********************************************************************/

#include <iostream>
#include "http.hpp"
#include "sever.hpp"
using namespace std;

int main()
{
    Server srv;
    srv.Start(8888); 
    return 0;
}
