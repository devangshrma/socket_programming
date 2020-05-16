#include<arpa/inet.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netdb.h>
#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>

#define READ_BUFFER_SIZE 10
#define BACKLOG 5

int bind_socket(char*);
void read_and_write(int);

int main(int argc,char *argv[]){

    if(argc != 2)
	{
		printf("\nCorrect usage is ./%s <port number>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

    /* bind_socket to specific IP addr and PORT */
    int sfd = bind_socket(argv[1]);

	/* start listening for incoming connections */
	int ret = listen(sfd, BACKLOG);
	if(ret<0){
                perror("\nlisten failed\n");
                exit(EXIT_FAILURE);
    }
	printf("\nListening for incoming connections......\n");

	/* accept a connection from client */
	struct sockaddr_in caddr;
	socklen_t clen = sizeof(caddr);

    int csfd = accept(sfd,(struct sockaddr*)&caddr, &clen);
    if(csfd<0)
    {
        perror("\naccept failed\n");
        exit(EXIT_FAILURE);
    }
    printf("A Client got connected from %s:%d",inet_ntoa(caddr.sin_addr),ntohs(caddr.sin_port));

    /* read from the client and write back the reversed string to server */
    read_and_write(csfd);
        
    /*close the socket descriptors for client.*/
    close(csfd);

    /* close the socket descriptors for server. */
	close(sfd);
}

int bind_socket(char *port_no){

    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sfd, ret;

    memset(&hints, 0, sizeof(struct addrinfo));

    hints.ai_family = AF_INET; /* Allow IPv4 only */
    hints.ai_socktype = SOCK_STREAM; /* Provides sequenced, reliable, two-way, connection-based byte streams */
    hints.ai_flags = AI_PASSIVE; /* use my IP address */
    hints.ai_protocol = 0;          /* Any protocol */

    ret = getaddrinfo(NULL, port_no, &hints, &result);
    if (ret != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ret));
        exit(EXIT_FAILURE);
    }
    
    /* getaddrinfo() returns a list of address structures.
       Try each address until we successfully bind(2).
       If socket(2) (or bind(2)) fails, we (close the socket
       and) try the next address. */

    for (rp = result; rp != NULL; rp = rp->ai_next){

        sfd = socket(rp->ai_family, rp->ai_socktype,rp->ai_protocol);
        if (sfd == -1){
            continue;
        }

        if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0){
             break; /* Success */
        } 

        close(sfd);
    }

    if (rp == NULL){               /* No address succeeded */
       fprintf(stderr, "Could not connect\n");
       exit(EXIT_FAILURE);
    }

    freeaddrinfo(result);

    return sfd;
}

void read_and_write(int csfd){

	FILE *fd;
	int i = 0, nbytes;
	char *buff = malloc(0);
    char *temp_buff = buff;
    int size=0;

    fd = fdopen(csfd,"r+");
    if(fd == NULL){
        perror("fdopen failed!!");
        exit(EXIT_FAILURE);
    }

    fcntl(csfd, F_SETFL, O_NONBLOCK);
    do{
        i++;
        buff = realloc(buff,READ_BUFFER_SIZE*i);
        temp_buff = buff + READ_BUFFER_SIZE*(i-1);
        while((nbytes=fread(temp_buff,1,READ_BUFFER_SIZE,fd))<=0);
        size += nbytes;
    }while(nbytes == READ_BUFFER_SIZE);
    
    temp_buff[size] = '\0';    
    printf("\nReceived String is:\n%s\n",temp_buff);

    size = size-1;
    char send_buff[size];
    i = 0;
    while(size--){
        send_buff[i++] = buff[size];
    }
    send_buff[i] = '\0';
    printf("\nReversed String is:\n%s\n",send_buff);
    clearerr(fd);
    
    nbytes = fwrite(send_buff,1,sizeof(send_buff),fd);
    fflush(fd);
}
//
//int bind_socket(char *port_no){
///*
// * int bind(int sockfd, const struct sockaddr *addr,socklen_t addrlen);
// * After creation of socket, bind function associates socket to address and port number specified in struct addr.
// */
//
//	char *ptr;
//	struct sockaddr_in saddr;
//
//	unsigned long int s_port = strtoul(port_no, &ptr, 10); //Converts a string to an unsigned long integer
//	saddr.sin_family = AF_INET; //IPv4 protocol
//	saddr.sin_port = htons(s_port);
//	saddr.sin_addr.s_addr = INADDR_ANY; //It binds the socket to all the available interfaces.
//	memset(saddr.sin_zero,'\0',sizeof(saddr.sin_zero)); //set sin_zero to ZERO
//
//	int ret = bind(sfd,(struct sockaddr *)&saddr,sizeof(saddr));
//	if(ret<0){
//                perror("Bind failed\n");
//                exit(EXIT_FAILURE);
//    }
//}
