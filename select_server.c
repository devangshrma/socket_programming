#include <linux/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>



int main(int argc,char *argv[])
{
	//===========================================socket

	struct sockaddr_in saddr;
	struct sockaddr_in caddr;
	int ssd,ret;
	ssd = socket(AF_INET,SOCK_STREAM,0);
	if(ssd<0)
	{
		perror("Socket init failed\n");
		exit(EXIT_FAILURE);
	}
	//==========================================bind

	char *ptr;
	//strtoul(argv[1],&ptr,10);
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(strtoul(argv[1],&ptr,10));
	saddr.sin_addr.s_addr = INADDR_ANY;
	bzero(&saddr.sin_zero,sizeof(saddr.sin_zero));

	ret = bind(ssd,(struct sockaddr *)&saddr,sizeof(saddr));
	if(ret<0)
        {
                perror("Bind failed\n");
                exit(EXIT_FAILURE);
        }
	//==========================================listen
	int backlog = 5;
	ret = listen(ssd,backlog);
	if(ret<0)
        {
                perror("listen API failed\n");
                exit(EXIT_FAILURE);
        }
	
	//===========================================select
	socklen_t clen = sizeof(caddr);
	fd_set read_set,active_set;
	FD_ZERO(&active_set);
	FD_SET(ssd,&active_set);
	int max_fds=ssd;	//3
	int fd,j,nbytes,new_csd;
	char buf[256];
	while(1)
	{
		read_set = active_set;
		printf("read set before select %llu\n",read_set);
		printf("active set before select %llu\n\n",active_set);
		ret = select(max_fds+1, &read_set, NULL, NULL, NULL);
		printf("active set after select %llu\n",active_set);
		printf("read set after select %llu\n\n",read_set);
		if(ret<0)
		{
			perror("Select failed");
			exit(EXIT_FAILURE);
		}

		for(fd=3;fd<max_fds+1;fd++)
		{
			if(FD_ISSET(fd,&read_set))
			{
				if(fd == ssd)
				{
					//===========server processing(accept)
					new_csd=accept(ssd,(struct sockaddr *)&caddr,&clen);
					if(new_csd<0)
					{
						perror("accept");
						exit(EXIT_FAILURE);
					}

					FD_SET(new_csd, &active_set); // add to master set
					if (new_csd > max_fds) 
					{
						max_fds = new_csd;
					}
					
				}//server processing (if)ends
				
				else	//===========client processing(close,recv)
				{
					nbytes = recv(fd, buf, sizeof(buf), 0);
					if(nbytes < 0)
				        {
				                perror("Recv failed");
				                exit(EXIT_FAILURE);
				        }
						
					if(nbytes == 0){
						
						close(fd);
						FD_CLR(fd, &active_set);
						max_fds = (max_fds-1);
					}	
					
					if(nbytes > 0)
					{							
						// we got some data from a client
						//===================BROADCAST
						for(j = 0; j < max_fds+1; j++)
						{
						// send to everyone!
							if (FD_ISSET(j, &active_set))
							{
							// except the listener and ourselves
								if (j != ssd && j != fd)
								{
									if (send(j, buf, nbytes, 0) == -1)
									{
										perror("send");
									}
								}
							}
						}
						write(1,buf,nbytes);		//======================STDOUT
						/*if(strncmp("exit",buf,4)==0)
						{
						printf("Exiting client recv\n");
						close()							
						//break;
						}*/
					}
				}//client processing (else)ends
				//server-client (if-else)processing ends
			}//FD_SET ends
		}//for ends
		
	}//while1 ends

}//main ends