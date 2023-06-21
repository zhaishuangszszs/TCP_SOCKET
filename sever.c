#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <fcntl.h>
#include <pthread.h>
#include<errno.h>
#include "socket.h"

//信息结构体（传给working函数的）
struct SockInfo
{
    struct sockaddr_in addr;
    int fd;
};
struct SockInfo infos[512];

void *working(void *arg);

int main()
{
    //1.创建通信套接字
    int fd=CreateSocket();
    if(fd==-1)
    {
        exit(-1);
    }
    
    //2.服务器绑定本地的ip,port，并设置监听
    int ret=Set_Bind_Listen(fd, 9999);
    if(ret==-1)
    {
         exit(-1);
    }

    int opt = 1;
    ret=setsockopt(fd, SOL_SOCKET,SO_REUSEADDR,  &opt, sizeof(opt));
    if(ret==-1)
    {
        perror("setsockopt error");
        exit(-1);
    }


    //初始化信息结构体数组(用于表示连接套接字地址信息)
    int max=sizeof(infos)/sizeof(infos[0]);
    for(int i=0;i<max;i++)
    {
        bzero(&infos[i],sizeof(infos[i]));
        infos[i].fd=-1;//-1表明空闲
    }


    //阻塞并等待客户端连接
     while (1)
    {
        struct SockInfo* pinfo;//用于获取连接套接字地址信息

        for(int i=0;i<max;i++)
        {
            if(infos[i].fd==-1)//该结构体空闲
            {
                pinfo=&infos[i];//pinfo指向该空闲结构体
                break;
            }
        }

       int coonfd=AcceptCoon(fd,&pinfo->addr);

       
        if (coonfd== -1)
        {
            break;//直接退出主线程
            //连接失败仍然可以连接用continue,但要回收信息结构体
        }

        //设置文件描述符非阻塞
        setnonblock(coonfd);

        pinfo->fd=coonfd;//信息结构体储存连接套接字，用于传参

        pthread_t tid;

        //创建线程进行通信
        pthread_create(&tid,NULL,working,pinfo);//创建子线程传入信息结构体
        pthread_detach(tid);
    }
   //关闭监听描述符
    return 0;
}



void *working(void *arg)
{
    struct SockInfo* pinfo=(struct SockInfo*)arg;
    // 连接成功，打印客户端的IP和端口信息
    char ip[32];
    printf("客户端的IP:%s,端口：%d\n", inet_ntop(AF_INET, &pinfo->addr.sin_addr.s_addr, ip, sizeof(ip)), ntohs(pinfo->addr.sin_port));

    // 5. 和客户端通信
    while (1)
    {
        char* buf;//用来指向创建的堆内存地址
        int len = RecvMessage(pinfo->fd, &buf);//c语言要让外面地址指向函数内创建的堆地址，只能传地址的地址
        //c语言传参要修改外面值只能传外面参数地址值
        printf("接受数据,%d....\n",len);
        if (len > 0)
        {
            printf("client say:%s\n", buf);
            printf("\n\n");
            free(buf);
        }
        else
        {
            break;
        }
        sleep(1);
    }
    // 6. 关闭文件描述符
    close(pinfo->fd);
    pinfo->fd=-1;//置为空
    return NULL;
}