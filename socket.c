#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include<errno.h>

// 当发送端发完数据到sockfd的发送缓存中完成后就关闭通信（send是当sockfd发送缓存不满就向其写入，通信有系统协议完成，即不确保接收端收到数据）。接受端recv在收完数据会接收到0，知道客户端关闭连接。
// 当发送端发完数据到sockfd的发送缓存中完成后不关闭通信。接受端recv在收完数据后阻塞，如果接收端sockfd非阻塞则返回-1并发送信号errno=EAGAIN || errno = EWOULDBLOCK 。


//创建套接字
int CreateSocket()
{
    int fd=socket(AF_INET,SOCK_STREAM,0);
    if(fd==-1)
    {
        perror("socket\n");
        return -1;
    }
    printf("socket create success, fd=%d\n",fd);
    return fd;
}

//服务器端绑定IP和PORT，设置监听
int Set_Bind_Listen(int lfd,unsigned short port)
{
    struct sockaddr_in saddr;//服务器端地址信息
    saddr.sin_family=AF_INET;
    saddr.sin_port=htons(port);
    saddr.sin_addr.s_addr=INADDR_ANY;//0.0.0.0(即绑定本地任意一个ip地址)
    int ret=bind(lfd,(struct sockaddr*)&saddr,sizeof(saddr));
    if(ret==-1)
    {
        perror("bind\n");
        return -1;
    }
    printf("bind success\n");

    ret=listen(lfd,128);//服务器可以在请求连接队列中等待的最大连接数128
    if(ret==-1)
    {
        perror("listen\n");
        return -1;
    }
    printf("lieten success\n");
    return ret;
}

//服务器端接受客户端连接
int AcceptCoon(int lfd,struct sockaddr_in*caddr)
{
    int cfd=-1;
    if(caddr==NULL)
    {
        cfd=accept(lfd, NULL, NULL);//不需要客户端信息
    }
    else 
    {
         socklen_t addrlen=sizeof(struct sockaddr_in);
        cfd=accept(lfd, (struct sockaddr *)caddr, &addrlen);
    }
    if(cfd==-1)
    {
        perror("accept\n");
        return -1;
    }
    printf("和客户端建立连接\n");
    return cfd;
}

//客户端连接服务器
int ConnectServer(int coonfd,const char*ip,unsigned short port)
{
    struct sockaddr_in saddr;
    saddr.sin_family=AF_INET;
    saddr.sin_port=htons(port);
    inet_pton(AF_INET, ip, &saddr.sin_addr.s_addr);
    int ret=connect(coonfd, (const struct sockaddr *)&saddr, sizeof(struct sockaddr_in));
    if(ret==-1)
    {
        perror("connect");
        return -1;
    }
    printf("成功和服务器建立连接\n");
    return ret;
}


//关闭套接字
int CloseSocket(int fd)
{
    int ret=close(fd);
    if(ret==-1)
    {
        perror("close\n");
    }
    return ret;
}

//接收指定字节个数
int readn(int fd,char*buf,int size)
{
    char* pt=buf;
    int count=size;
    while(count>0)
    {
        int len=recv(fd,pt,count,0);
        if(len==-1)
        {
            if(errno == EAGAIN || errno == EWOULDBLOCK )
            {
                printf("读完了\n");//非阻塞IO当对方保持连接且无数据接收时
                printf("\n");
                break;
            }
            return -1;
        }
        else if(len==0)
        {
            //对方断开连接，返回收到的字节数
            printf("对方断开连接\n");
            printf("\n");
            return size-count;
        }
        pt+=len;
        count-=len;
    }
    return size-count;
}

//接受数据
int RecvMessage(int cfd,char**msg)
{
    int len=0;
    readn(cfd, (char*)&len, 4);
    len=ntohl(len);
    printf("要接收的数据块大小：%d\n",len);
    char*data=(char*)malloc(len+1);//末尾加0；
    int length=readn(cfd, data, len);
    if(length!=len)
    {
        printf("接受数据失败\n");
        free(data);
        close(cfd);
    }
    data[len]='\0';
    *msg=data;
    return length;
}


//发送指定长度的字符串
int writen(int fd,const char* msg,int size)
{
    const char* buf=msg;
    int count=size;
    while(count>0)
    {
        int len=send(fd,buf,count,0);
        if(len==-1)
        {
            return -1;
        }
        buf+=len;
        count-=len;
    }
    return size-count;
}

//发送数据
int SendMessage(int cfd,const char* msg,int len)
{
    if(cfd<0||msg==NULL||len<0)
    {
        return -1;
    }
    char*data=(char*)malloc(len+4);
    int biglen=htonl(len);
    memcpy(data, &biglen,4);
    memcpy(data+4, msg, len);
   // int ret=send(cfd,msg,len,0);
   int ret=writen(cfd, data, len+4);
    if(ret==-1)
    {
        perror("send error\n");
        close(cfd);
    }
    return ret;
}


void setnonblock(int fd)
{
    int old_flag=fcntl(fd,F_GETFL);
    int new_flag=old_flag|O_NONBLOCK;
    fcntl(fd,F_SETFL,new_flag);
}


