#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
//包含socket库
#include <arpa/inet.h>
#include "threadpool.h"

//信息结构体
struct SockInfo
{
    struct sockaddr_in addr;
    int fd;
};

typedef struct PoolInfo
{
    ThreadPool* p;
    int fd;
}PoolInfo;

void working(void* arg);
void acceptConn(void* arg);
void bzero(void *__s, size_t __n);

int main(){
    //1.创建监听的套接字    ipv4，流式
    int fd =socket(AF_INET, SOCK_STREAM,0);
    //如果创建失败
    if(fd == -1)
    {
        perror("socket");
        return -1;
    }

    //2.绑定本地的IP port
    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(9999);
    saddr.sin_addr.s_addr = INADDR_ANY; //0=0.0.0.0 会自动寻找本地的ip地址
    int ret = bind(fd,(struct sockaddr*)&saddr,sizeof(saddr));
    //如果绑定失败
    if(ret == -1)
    {
        perror("bind");
        return -1;
    }

    //3.设置监听
    ret = listen(fd,128); //#用于监听的文件描述符 #同时能够检测的最多连接数（<=128）
    //如果监听失败
    if(ret == -1)
    {
        perror("listen");
        return -1;
    }
    
    //创建线程池
    ThreadPool *pool = threadPoolCreate(3,8,100);
    PoolInfo* info = (PoolInfo*)malloc(sizeof(PoolInfo));
    info->p = pool;
    info->fd = fd;
    threadPoolAdd(pool,acceptConn,info);

    pthread_exit(NULL);

    return 0;
}   

void acceptConn(void* arg)
{
    PoolInfo* poolInfo = (PoolInfo*)arg;
    //4.阻塞并等待客户端的连接
    int addrlen = sizeof(struct sockaddr_in);
    while (1)
    {
        struct SockInfo* pinfo;
        pinfo = (struct SockInfo*)malloc(sizeof(struct SockInfo));
        pinfo->fd = accept(poolInfo->fd,(struct sockaddr*)&pinfo->addr, &addrlen);//# #输出参数 #输出参数 return用于通信的文件描述符
        if(pinfo->fd == -1)
        {
            perror("accept");
            break;
        }
        //添加通信的任务
        threadPoolAdd(poolInfo->p,working,pinfo);
    }
    close(poolInfo->fd);
}

void working(void* arg)
{
    struct SockInfo* pinfo = (struct SockInfo*)arg;
    //建立连接成功，打印客户端的IP和端口信息
    char ip[32];
    printf("客户端的IP:%s,端口:%d\n",
            inet_ntop(AF_INET,&pinfo->addr.sin_addr.s_addr, ip,sizeof(ip)),   //#协议 #要转换的大端的ip地址 #转换好的ip储存的位置 # 3指针指向内存的大小   返回3指针指向的内存
            ntohs(pinfo->addr.sin_port)); //#端口信息大端到小段
    
    //5.建立通信
    while(1)
    {
        //接收数据
        char buff[1024];
        int len =recv(pinfo->fd, buff,sizeof(buff),0); //#文件描述符 #接收数据的内存 # #flag return数据的长度
        if(len >0)
        {
            printf("client say: %s\n",buff);
            send(pinfo->fd,buff,len,0);
        }
        else if(len == 0)
        {
            printf("客户端已经断开连接...\n");
            break;
        }
        else
        {
            perror("recv");
            break;
        }
    }

    //关闭文件描述符
    close(pinfo->fd);
}