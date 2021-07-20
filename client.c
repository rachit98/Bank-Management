#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main()
{
	int sock_fd;
	int status;
	struct sockaddr_in address;
	ssize_t rb, wb;
	char data_from_server[1000];
	char data_to_server[1000];
	char type[3];
	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	printf("Client side socket created\n");
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_family = AF_INET;
	address.sin_port = htons(5676);
	status = connect(sock_fd, (struct sockaddr *)&address, sizeof(address));
	printf("Connection Established!\n");
	rb = read(sock_fd, data_from_server, sizeof(data_from_server));
	printf("%s\n", data_from_server);
	scanf("%s",data_to_server);
	wb = write(sock_fd,data_to_server,sizeof(data_to_server));
	strcpy(data_to_server,"");
	scanf("%s",data_to_server);
	wb = write(sock_fd,data_to_server,sizeof(data_to_server));
	strcpy(data_to_server,"");
	strcpy(data_from_server,"");
	scanf("%s",data_to_server);
	wb = write(sock_fd,data_to_server,sizeof(data_to_server));
	strcpy(data_to_server,"");
	strcpy(data_from_server,"");
	rb = read(sock_fd, data_from_server, sizeof(data_from_server));
	printf("%s\n", data_from_server);
	if(strcmp(data_from_server,"Login Failed")==0)
		return 0;
	while(1)
	{
		rb = read(sock_fd, type, sizeof(type));
		//printf("%s\n",type );
		if(strcmp(type,"RW")==0)
		{
			rb = read(sock_fd, data_from_server, sizeof(data_from_server));
			printf("%s\n", data_from_server);
			scanf("%s",data_to_server);
			wb = write(sock_fd,data_to_server,sizeof(data_to_server));
			strcpy(data_to_server,"");
			strcpy(data_from_server,"");
			strcpy(type,"");  
		}
		if(strcmp(type,"RD")==0)
		{
			wb = write(sock_fd, "ACK",sizeof("ACK"));
			rb = read(sock_fd, data_from_server, sizeof(data_from_server));
			printf("%s\n", data_from_server);
			if(strcmp(data_from_server,"Connection closed")==0)
			{
				break;
			}
			strcpy(data_to_server,"");
			strcpy(data_from_server,"");
			strcpy(type,""); 
		}
	}
	close(sock_fd);
	return 0;
}