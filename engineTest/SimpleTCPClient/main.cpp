#include <winsock2.h>
#include <stdio.h>
#include <windows.h>
#pragma comment(lib,"ws2_32.lib")

int main(int argc, char* argv[]){
	//把字符串的IP地址转化为u_long
	unsigned long ip = inet_addr("127.0.0.1");
	//把端口号转化成整数
	short port = 8000;
	printf("Connecting to %s:%d....../n",inet_ntoa(*(in_addr*)&ip),port);
	WSADATA wsa;
	//初始化套接字DLL
	if(WSAStartup(MAKEWORD(2,2),&wsa)!=0){
		printf("套接字初始化失败!");
		exit(-1);
	}
	//创建套接字
	SOCKET sock;
	if((sock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))==INVALID_SOCKET){
		printf("创建套接字失败！");
		exit(-1);
	}
	int size = 0;
	int len = 4;
	if( 0 != ::getsockopt( sock, SOL_SOCKET, SO_SNDBUF, (char*)&size, &len ) )
	{
		return 0;
	}
	struct sockaddr_in serverAddress;
	memset(&serverAddress,0,sizeof(sockaddr_in));
	serverAddress.sin_family=AF_INET;
	serverAddress.sin_addr.S_un.S_addr = ip;
	serverAddress.sin_port = htons(port);
	//建立和服务器的连接
	if(connect(sock,(sockaddr*)&serverAddress,sizeof(serverAddress))==SOCKET_ERROR){
		printf("建立连接失败！");
		exit(-1);
	}
	printf( "建立连接" );
	char buf[256] = { 0 };
	send(sock,buf,sizeof(buf),0);
	shutdown( sock, SD_SEND );
	Sleep( ~0 );
	//清理套接字占用的资源
	WSACleanup();
	return 0;
}