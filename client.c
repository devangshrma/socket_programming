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

int connect_to_server(char*,char*);
void read_and_write(int);

int main(int argc,char *argv[]){

	if(argc != 3)
	{
		printf("\nCorrect usage is ./%s <server-ip address> <port number>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

    /* connect to server */
    int csfd = connect_to_server(argv[1], argv[2]);

    /* write to the client and read back the reversed string from server, and print the same on standard output.*/
    read_and_write(csfd);
    
    /* close the socket descriptors for client. */
    close(csfd);
}

int connect_to_server(char *ip_addr, char *port_no){

    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sfd, ret;

    memset(&hints, 0, sizeof(struct addrinfo));

    hints.ai_family = AF_INET; /* Only want IPv4 (use AF_INET6 for IPv6) */
    hints.ai_socktype = SOCK_STREAM; /* Only want stream-based connection */

    ret = getaddrinfo(ip_addr, port_no, &hints, &result);
    if (ret != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ret));
        exit(EXIT_FAILURE);
    }

    /* getaddrinfo() returns a list of address structures.
       Try each address until we successfully connect(2).
       If socket(2) (or connect(2)) fails, we (close the socket
       and) try the next address. */

    for (rp = result; rp != NULL; rp = rp->ai_next){
        sfd = socket(rp->ai_family, rp->ai_socktype,rp->ai_protocol);
        if (sfd == -1){
            continue;
        }

        if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1){
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
    int size=0;
	int i = 0, nbytes;
	char *buff = malloc(0);
    char *temp_buff = buff;
    
    do{ 
       printf("\nEnter the string you want to send to the server!!\n");
       i++;
       buff = realloc(buff,READ_BUFFER_SIZE*i);
       temp_buff = buff + READ_BUFFER_SIZE*(i-1);
       nbytes = read(STDIN_FILENO,temp_buff,READ_BUFFER_SIZE); //STDIN_FILENO:- <unistd.h> symbolic constant for standard-input fd.
       size += nbytes;
    }while(nbytes == READ_BUFFER_SIZE && temp_buff[nbytes-1]!='\n');

    temp_buff[nbytes-1] = '\0';     

    fd = fdopen(csfd,"r+");
    if(fd == NULL){
        perror("fdopen failed!!");
        exit(EXIT_FAILURE);
    }
     
    nbytes = fwrite(buff,1,size,fd);
    fflush(fd);
 
    fcntl(csfd,F_SETFL,O_NONBLOCK);
    while((nbytes = fread(buff,1,size,fd))<=0);
 
    buff[size] = '\0'; 
 
    printf("\nReceived Message is:-\n%s\n",buff);	
}
