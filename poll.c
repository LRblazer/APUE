#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <poll.h>
#include <getopt.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <libgen.h>
#include <sys/types.h>        
#include <sys/socket.h>
#include <unistd.h>





#define ARRAY_SIZE(x)  (sizeof(x)/sizeof(x[0]))




static inline void print_usage(char *progname);
int socket_server_init(char * listen_ip,int listen_port);




int main(int argc,char **argv)
{
	int                     max;
	int                     i,j;
	int                     rv;
	char                    *progname = NULL;
	int                     daemon_run = 0;
	char                    buf[1024];
	int                     opt;
	int                     serv_port = 0;
	int                     listenfd,connfd;
	int                     found;




    struct pollfd           fds_array[1024];
	struct option           long_options[] =
	{
		{"daemon",no_argument,NULL,'b'},
		{"port",required_argument,NULL,'p'},
		{"help",no_argument,NULL,'h'},
		{NULL,0,NULL,0}
	};
	progname = basename(argv[0]);


	while((opt = getopt_long(argc,argv,"bp:h",long_options,NULL))!= -1)
	{
		switch(opt)
		{
			case 'b':
				daemon_run = 1;
				break;

			case 'p':
				serv_port = atoi(optarg);
				break;

			case 'h':
				print_usage(progname);
                return EXIT_SUCCESS;
			default:
				break;
		}
	}


	if( !serv_port )
	{
		print_usage(progname);
		return -1;
	}


	if((listenfd = socket_server_init(NULL,serv_port)) < 0)
	{
		printf("ERROR:%s server listen on port[%d] failure.\n",progname,serv_port);
		return -2;
	}
	printf("%s server listen on port[%d] ok.\n",progname,serv_port);

    if(daemon_run)
	{
		daemon(0,0);
	}



	for(i = 0;i < ARRAY_SIZE(fds_array);i++)
	{
		fds_array[i].fd = -1;
	}
	fds_array[0].fd = listenfd;
	fds_array[0].events = POLLIN;

	max = 0;
	printf("in the front for;;;;------\n");


	for( ; ; )
	{       
		printf("for back--------------poll front\n");
		rv = poll(fds_array,max+1,-1);
		printf("poll back \n");
		if(rv < 0)
		{
			printf("poll failure:%s\n",strerror(errno));
			break;
		}
		else if(rv == 0)
		{
			printf("timeout\n");
			continue;
		}
		
		if(fds_array[0].revents & POLLIN)
		{
			printf("\n");
			if((connfd = accept(listenfd,(struct sockaddr *)NULL,NULL)) < 0)
			{
				printf("accept new client failure:%s\n",strerror(errno));
				continue;
			}
			found = 0;
			for(i = 1;i < ARRAY_SIZE(fds_array);i++)
			{
				if(fds_array[i].fd < 0)
				{
					printf("accept new client[%d] into array\n",connfd);
					fds_array[i].fd = connfd;
					fds_array[i].events = POLLIN;
					found = 1;
					break;
				}
			}

			if(!found)
			{
				printf("Put connfd into array failure,array full.\n");
			    close(connfd);
                continue;

			}
			max = i>max ? i : max;
			if(--rv <= 0)
				continue;
		}

		else
		{
			for(i = 1;i < ARRAY_SIZE(fds_array);i++)
			{
				if(fds_array[i].fd < 0 || fds_array[i].events != POLLIN)
					continue;
			
				if((rv = read(fds_array[i].fd,buf,sizeof(buf))) <= 0)
				{
					printf("socket[%d] read failure or get disconnect.\n",fds_array[i].fd);
					close(fds_array[i].fd);
					fds_array[i].fd = -1;
				}
			    else
				{
					printf("Read %d bytes data from socket[%d].\n",rv,fds_array[i].fd);
					for(j = 0;j < rv;j++)
					{
						buf[i] = toupper(buf[i]);
					}
					if(write(fds_array[i].fd,buf,sizeof(buf)) < 0)
					{
						printf("sockte[%d] write failure:%s\n",fds_array[i].fd,strerror(errno));
						close(fds_array[i].fd);
						fds_array[i].fd = -1;
					}
				}
			}
		}
	}


cleanup:
        close(listenfd);
	return 0;
}










static inline void print_usage(char *progname)
{
	printf("%s usage:\n",progname);
	printf("-p(--port) input the port you will listen.\n");
	printf("-h(--help) print how to use it.\n");
    printf("-b(--daemon)means the %s will run at the background.\n",progname);
}








int socket_server_init(char *listen_ip,int listen_port)
{
	struct sockaddr_in             servaddr;
	int                            rv = 0;
    int                            listenfd;
    int                            on = 1;
	if((listenfd = socket(AF_INET,SOCK_STREAM,0)) < 0)
	{
		printf("Create TCP socket failure:%s.\n",strerror(errno));
		return -1;
	}
	printf("Create socket[%d] ok!\n",listenfd);
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_port = htons(listen_port);
	servaddr.sin_family = AF_INET;
	
	if(listen_ip)
	{
		if(inet_pton(AF_INET,listen_ip,&servaddr.sin_addr) < 0)
		{
			printf("inet_pton() set ip failure\n");
			rv = -2;
			goto cleanup;
	    }
	}
	else
	{
		 servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	
	if(bind(listenfd,(struct sockaddr *)&servaddr,sizeof(servaddr)) < 0)
	{
		printf("Bind listenfd[%d] on port[%d] failure:%s.\n",listenfd,listen_port,strerror(errno));
		rv = -3;
		goto cleanup;
	}
	printf("Bind listen[%d] on port[%d] ok!\n",listenfd,listen_port);

	listen(listenfd,13);
	printf("listen start\n");

cleanup:
    if(rv < 0)
    close(listenfd);
    else 
    {
        rv = listenfd;
    }

	return rv;

}
	
	





























