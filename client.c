#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <fcntl.h>

#include "socket.h"

int main()
{
    //1.创建通信套接字
    int fd=CreateSocket();
    if(fd==-1)
    {
         exit(-1);
    }

    //2.连接服务器ip,port
    int ret=ConnectServer(fd, "192.168.160.131", 9999);
    if(ret==-1)
    {
         exit(-1);
    }

    //文本文件描述符
    int Txt_fd=open("test.txt",O_RDONLY);
    int length=0;
    char buf[200];

    //3.通信
    while ( (length=read(Txt_fd,buf,rand()%200) ) >0 ) //文本还有数据没读完
    {
        //发送数据
        SendMessage(fd, buf, length);

        memset(buf,0,sizeof(buf));

        usleep(100);
    }
    sleep(5);//进行对比分析原因
    //sleep(15);
    printf("客户端断开连接\n");
    close(fd);
    return 0;


}