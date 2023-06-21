#ifndef _SOCKET_H_
#define _SOCKET_H_
#include<errno.h>

//创建套接字
int CreateSocket();

//服务器端绑定IP和PORT，设置监听
int Set_Bind_Listen(int lfd,unsigned short port);

//服务器端接受客户端连接
int AcceptCoon(int lfd,struct sockaddr_in*caddr);

//客户端连接服务器
int ConnectServer(int coonfd,const char*ip,unsigned short port);

//关闭套接字
int CloseSocket(int fd);

//接受数据
int RecvMessage(int cfd,char** msg);

//发送数据
int SendMessage(int cfd,const char* msg,int len);

//套接字设置为非阻塞
void setnonblock(int fd);


#endif