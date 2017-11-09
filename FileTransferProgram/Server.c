#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

char *EXIT_STRING = "exit";
int recv_and_print(int sd);
int input_and_send(int sd);

#define MAXLINE 511
struct sockaddr_in cliaddr, servaddr;
char cli_ip[20];

int main(int argc, char *argv[]) {
    int listen_sock, accp_sock, addrlen = sizeof(cliaddr);
    pid_t pid;
    if (argc != 2) {
        printf("사용법 : %s [포트번호]\n", argv[0]);
        exit(0);
    }

    if((listen_sock=socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket fail");
        exit(0);
    }

    bzero((char *)&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(atoi(argv[1]));

    if(bind(listen_sock, (struct sockaddr *)&servaddr,
            sizeof(servaddr)) < 0) {
        perror("bind fail");
        exit(0);
    }
    puts("[* 서버가 클라이언트를 기다리고 있습니다... *]");
    listen(listen_sock, 1);

    if((accp_sock = accept(listen_sock,
          (struct sockaddr *)&cliaddr, &addrlen)) < 0) {
        perror("accept fail");
        exit(0);
    }
    puts("[* 클라이언트가 연결되었습니다 !! *]");
    puts("[* 채팅을 시작합니다. *]");

    if( (pid = fork()) > 0)
        input_and_send(accp_sock);
    else if (pid == 0)
        recv_and_print(accp_sock); 
    close(listen_sock);
    close(accp_sock);
    return 0;
}


int input_and_send(int sd) {
    char buf[MAXLINE+1];
    int nbyte;
    while(fgets(buf, sizeof(buf), stdin) != NULL) {
        nbyte = strlen(buf);
        write(sd, buf, strlen(buf));

        if (strstr(buf, "put") != NULL ) {
	    char filename[50];
	    int filesize=0, total=0, sread=0, fp=0;

            inet_ntop(AF_INET, &cliaddr.sin_addr.s_addr, cli_ip, sizeof(cli_ip));

	    bzero( filename, 50 );
	    recv( sd, filename, sizeof(filename), 0 );
	    if(strstr(filename, "error") != NULL)
		continue;
	    printf( "파일 이름 : %s\n", filename );

	    read( sd, &filesize, sizeof(filesize) );

	    fp = open( filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	    while( total != filesize ) {
		sread = recv( sd, buf, 100, 0 );
		total += sread;
		buf[sread] = 0;
		write( fp, buf, sread );
		bzero( buf, sizeof(buf) );
	    }
	    printf( "파일 받기 완료 !! \n" );
	    printf( "파일 크기 : %d \n\n", filesize);
	    close(fp);
	    continue;   
        }

	if (strstr(buf, "get") != NULL ) {
            char filename[50];
	    char clientpath[50];
	    int filesize=0, fp=0, total=0, sread=0;

            inet_ntop(AF_INET, &cliaddr.sin_addr.s_addr, cli_ip, sizeof(cli_ip));

	    bzero( filename, 50 );
	    bzero( clientpath, 50 );
	    recv( sd, filename, sizeof(filename), 0 );
	    if(strstr(filename, "error") != NULL) {
		printf("\n[에러 : 파일이 존재하지 않습니다 !!]\n\n");
		continue;
	    }

	    recv( sd, clientpath, sizeof(clientpath), 0);

	    read( sd, &filesize, sizeof(filesize) );

	    fp = open( clientpath, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	    while( total != filesize ) {
		sread = recv( sd, buf, 100, 0 );
		total += sread;
		buf[sread] = 0;
		write( fp, buf, sread );
		bzero( buf, sizeof(buf) );
	    }
	    printf( "파일 전송 완료 !! \n" );
	    printf( "파일 크기 : %d \n\n", filesize);
	    continue;
	}
    }
    return 0;
}


int recv_and_print(int sd) {
    char buf[MAXLINE+1];
    int nbyte;
    while(1) {
        if( (nbyte=read(sd, buf, MAXLINE))<0){
            perror("read fail");
            close(sd);
            exit(0);
        }
        buf[nbyte] = 0;

        if (strstr(buf, "put") != NULL ) {
	    char filename[50];
	    int filesize=0, total=0, sread=0, fp=0;

            inet_ntop(AF_INET, &cliaddr.sin_addr.s_addr, cli_ip, sizeof(cli_ip));

	    bzero( filename, 50 );
	    recv( sd, filename, sizeof(filename), 0 );
	    if(strstr(filename, "error") != NULL)
		continue;
	    printf( "파일 이름 : %s\n", filename );

	    read( sd, &filesize, sizeof(filesize) );

	    fp = open( filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	    while( total != filesize ) {
		sread = recv( sd, buf, 100, 0 );
		total += sread;
		buf[sread] = 0;
		write( fp, buf, sread );
		bzero( buf, sizeof(buf) );
	    }
	    printf( "파일 받기 완료 !! \n" );
	    printf( "파일 크기 : %d \n\n", filesize);
	    close(fp);
	    continue;   
        }

	if (strstr(buf, "get") != NULL ) {
	    char filename[50];
	    char clientpath[50];
	    int filesize=0, fp=0, total=0, sread=0;

            inet_ntop(AF_INET, &cliaddr.sin_addr.s_addr, cli_ip, sizeof(cli_ip));

	    bzero( filename, 50 );
	    bzero( clientpath, 50 );
	    recv( sd, filename, sizeof(filename), 0 );
	    if(strstr(filename, "error") != NULL) {
		printf("\n[에러 : 파일이 존재하지 않습니다 !!]\n\n");
		continue;
	    }

	    recv( sd, clientpath, sizeof(clientpath), 0);

	    read( sd, &filesize, sizeof(filesize) );

	    fp = open( clientpath, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	    while( total != filesize ) {
		sread = recv( sd, buf, 100, 0 );
		total += sread;
		buf[sread] = 0;
		write( fp, buf, sread );
		bzero( buf, sizeof(buf) );
	    }
	    printf( "파일 전송 완료 !! \n" );
	    printf( "파일 크기 : %d \n\n", filesize);
	    continue;
	}

	printf("%s", buf);
    }
    return 0;
}
