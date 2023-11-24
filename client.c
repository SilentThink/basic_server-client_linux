#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
//包含socket库
#include <arpa/inet.h>

int main()
{
    //1.创建通信的套接字    ipv4，流式
    int fd =socket(AF_INET, SOCK_STREAM,0);
    //如果创建失败
    if(fd == -1)
    {
        perror("socket");
        return -1;
    }

    //2.连接服务器的IP port
    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(9999);
    inet_pton(AF_INET,"10.0.16.11",&saddr.sin_addr.s_addr);
    //saddr.sin_addr.s_addr = 43.140.250.7; //0=0.0.0.0 会自动寻找本地的ip地址
    int ret = connect(fd,(struct sockaddr*)&saddr,sizeof(saddr));
    //如果连接失败
    if(ret == -1)
    {
        perror("connect");
        return -1;
    }

    //5.通信
    int number=0;
    while(1)
    {
        //发送数据
        char buff[1024];
        sprintf(buff,"你好，hello,world,%d...\n",number++);
        send(fd, buff, strlen(buff)+1,0);

        //接收数据
        memset(buff,0,sizeof(buff));
        int len = recv(fd, buff,sizeof(buff),0);    //阻塞函数，fd中有数据才解除阻塞
        if(len >0)
        {
            printf("server say: %s\n",buff);
        }
        else if(len == 0)
        {
            printf("服务器已经断开连接...\n");
            break;
        }
        else
        {
            perror("recv失败");
            break;
        }
        sleep(1);
    }

    //关闭文件描述符
    close(fd);

    return 0;
}