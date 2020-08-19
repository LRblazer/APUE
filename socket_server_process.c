/*********************************************************************************
 *      Copyright:  (C) 2020 lingyun
 *                  All rights reserved.
 *
 *       Filename:  jcserve.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(2020年02月28日)
 *         Author:  xuxinhua <[4~[D[4~xxu>
 *      ChangeLog:  1, Release initial version on "2020年02月28日 09时28分01秒"
 *                 
 ********************************************************************************/

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <getopt.h>

#define MSG_STR "hello  myself client xxh"

void print_usage(char *progname)
{
    printf("%s usage :\n", progname);
    printf("-p(--port) : sepcify server listen port .\n");
    printf("-h(--help) : print this help information\n");
    return ;
}

int main(int argc, char *argv[])
{
    int                     sockfd = -1;
    int                     rv = -1;
    struct   sockaddr_in    servaddr;
    struct   sockaddr_in    cliaddr;
    socklen_t               len;
    int                     port = 0;
    int                     clifd;
    int                     ch;
    int                     on = 1;
    pid_t                   pid;

    struct  option      opts[] = {
        {"port", required_argument, NULL, 'p'},
        {"help", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0},
    };

    while ((ch=getopt_long(argc, argv, "p:h", opts, NULL)) != -1) 
    {
            switch (ch) 
            {
                case 'p':
                    port = atoi(optarg);
                    break;
                case 'h':
                    print_usage(argv[0]);
                    break;
            
                default:
                    return 0;
            }   
    }


    if( !port ) 
    {
            print_usage(argv[0]);   
            return 0;
    }


    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) 
    {
            printf("creat socket failure : %s\n", strerror(errno));   
            return -1;
    }    
    printf("creat socket [%d] success!\n", sockfd);

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 


    rv = bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if( rv < 0) 
    {
            printf("socket [%d] bind on port [%d] failure : %s\n", sockfd, port, strerror(errno));   
            return -2;
    }

    listen(sockfd, 13);
    printf("start to listen on port [%d]\n",port);

    while (1) 
    {
            printf("start accept new client incoming......\n");           

            clifd = accept(sockfd, (struct sockaddr *)&cliaddr, &len);

            if(clifd < 0) 
            {
                    printf("accept new client failure :%s\n", strerror(errno));         
                    continue;
            }

            printf("accept new client[%s:%d]  successfully\n",inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));


            pid = fork();
            if( pid < 0) 
            {
                    printf("fork()creat child failure :%s\n", strerror(errno));   
                    close(clifd);
                    continue;
            }
            else if ( pid > 0 ) 
            {
                    close(clifd);   
                    continue;
            }
            else if ( pid == 0 ) 
            {
                    char        buf[1024];   
                    close(sockfd);

                    printf("child start communicate with socket client......\n");
                    memset(buf, 0, sizeof(buf));


                    rv = read(clifd, buf, sizeof(buf));

                    if( rv < 0 ) 
                    {
                            printf("read data from client socket[%d] failure:%s\n", clifd, strerror(errno));   
                            close(clifd);
                            exit(0);
                    }
                    else if ( rv == 0 ) 
                    {
                            printf("socket [%d] get disconnect\n",clifd);   
                            exit(0);
                    }
                    else if ( rv > 0 ) 
                    {
                            printf("read  %d date from server :%s\n",rv, buf);   
                    }


                    rv = write(clifd, MSG_STR, strlen(MSG_STR));
                    if( rv < 0 ) 
                    {
                            printf("write data by socket [%d] failure :%s\n", sockfd, strerror(errno) );   
                            close(clifd);
                            exit(0);
                    }

                    sleep(5);

                    printf("close client socket[%d] and child process exit\n",clifd);
                    close(clifd);
                    exit(0);
                    
            }

            close(sockfd);
            return 0;
    }


    return 0;
}














