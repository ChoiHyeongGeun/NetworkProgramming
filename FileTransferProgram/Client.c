#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#define MAXLINE   511

char *EXIT_STRING = "exit";
int recv_and_print(int sd);
int input_and_send(int sd);

int main(int argc, char *argv[]) {
   pid_t pid;
   static int s;
   static struct sockaddr_in servaddr;


   if (argc != 3) {
      printf("사용법 : %s [서버IP] [포트번호] \n", argv[0]);
      exit(0);
   }

   if((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
      printf("Client : Can't open stream socket.\n");
      exit(0);
   }

   bzero((char *)&servaddr, sizeof(servaddr));

   servaddr.sin_family = AF_INET;
   inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
   servaddr.sin_port = htons(atoi(argv[2]));

   if(connect(s,(struct sockaddr *)&servaddr,sizeof(servaddr)) <
        0) {
      printf("Client : can't connect to server.\n");
      exit(0);
   }
   puts("[* 서버에 접속하였습니다. *]");
   puts("[* 채팅을 시작합니다. *]");
   if( (pid=fork())>0)
       input_and_send(s);
   else if(pid==0)
       recv_and_print(s);
    close(s);
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
	    int filesize=0, filenamesize=0, fp=0, total=0, sread=0;

	    printf("PUT(Upload mode)\n");
	    printf("파일 이름 : ");
    
	    if ( fgets(filename, sizeof(filename), stdin) == NULL )
		exit(0);

	    filenamesize = strlen(filename);
	    filename[filenamesize-1] = 0;

	    if( (fp = open( filename, O_RDONLY )) < 0 ) {
		printf( "\n[에러 : 파일이 존재하지 않습니다 !!]\n\n" );
		send( sd, "error", sizeof("error"), 0 );
		continue;
	    }

	    printf( "\n[파일이 존재합니다 !!]\n\n" );

	    send( sd, filename, sizeof(filename), 0 );
	    filesize = lseek( fp, 0, SEEK_END );
	    send( sd, &filesize, sizeof(filesize), 0 );
	    lseek(fp, 0, SEEK_SET );

	    while( total != filesize ) {
		sread = read( fp, buf, 100 );
		total += sread;
		buf[sread] = 0;
		send( sd, buf, sread, 0 );
	    }
	    printf( "파일 업로드 완료 !! \n" );
	    printf( "파일 크기 : %d \n\n", filesize);
	    close(fp);
	    continue;   
        }

	if (strstr(buf, "get") != NULL ) {
            char filepath[50];
	    char filename[50];
	    char clientpath[50];
	    int filesize=0, filepathsize=0, fp=0, filenamesize=0, total=0, sread=0;

            printf("GET(Download mode)\n");
	    printf("파일 경로 : ");

	    if ( fgets(filepath, sizeof(filepath), stdin) == NULL )
		exit(0);

	    filepathsize = strlen(filepath);
	    filepath[filepathsize-1] = 0;

	    printf("파일 이름 : ");

	    if ( fgets(filename, sizeof(filename), stdin) == NULL )
		exit(0);

	    filenamesize = strlen(filename);
	    filename[filenamesize-1] = 0;

	    strcat(filepath, filename);
	    getcwd(clientpath, 50);
	    strcat(clientpath, "/");
	    strcat(clientpath, filename);

	    if( (fp = open( filepath, O_RDONLY )) < 0 ) {
		printf( "\n[에러 : 파일이 존재하지 않습니다 !!]\n\n" );
		send( sd, "error", sizeof("error"), 0 );
		continue;
	    }

	    printf( "\n[파일이 존재합니다 !!]\n\n" );

	    send( sd, filepath, sizeof(filepath), 0 );
	    send( sd, clientpath, sizeof(clientpath), 0 );

	    filesize = lseek( fp, 0, SEEK_END );
	    send( sd, &filesize, sizeof(filesize), 0 );
	    lseek(fp, 0, SEEK_SET );

	    while( total != filesize ) {
		sread = read( fp, buf, 100 );
		total += sread;
		buf[sread] = 0;
		send( sd, buf, sread, 0 );
	    }
	    printf( "파일 다운로드 완료 !! \n" );
	    printf( "파일 크기 : %d \n\n", filesize);
	    close(fp);
	    continue;
	}
    }
    return 0;
}


int recv_and_print(int sd) {
    char buf[MAXLINE+1];
    int nbyte;
    while(1) {
        if( (nbyte=read(sd, buf, MAXLINE))<0) {
            perror("read fail");
            close(sd);
            exit(0);
        }
        buf[nbyte] = 0;

        if (strstr(buf, "put") != NULL ) {
	    char filename[50];
	    int filesize=0, filenamesize=0, fp=0, total=0, sread=0;

	    printf("PUT(Upload mode)\n");
	    printf("파일 이름 : ");
    
	    if ( fgets(filename, sizeof(filename), stdin) == NULL )
		exit(0);

	    filenamesize = strlen(filename);
	    filename[filenamesize-1] = 0;

	    if( (fp = open( filename, O_RDONLY )) < 0 ) {
		printf( "\n[에러 : 파일이 존재하지 않습니다 !!]\n\n" );
		send( sd, "error", sizeof("error"), 0 );
		continue;
	    }

	    printf( "\n[파일이 존재합니다 !!]\n\n" );

	    send( sd, filename, sizeof(filename), 0 );
	    filesize = lseek( fp, 0, SEEK_END );
	    send( sd, &filesize, sizeof(filesize), 0 );
	    lseek(fp, 0, SEEK_SET );

	    while( total != filesize ) {
		sread = read( fp, buf, 100 );
		total += sread;
		buf[sread] = 0;
		send( sd, buf, sread, 0 );
	    }
	    printf( "파일 업로드 완료 !! \n" );
	    printf( "파일 크기 : %d \n\n", filesize);
	    close(fp);
	    continue;   
        }

	if (strstr(buf, "get") != NULL ) {
	    char filepath[50];
	    char filename[50];
	    char clientpath[50];
	    int filesize=0, filepathsize=0, fp=0, filenamesize=0, total=0, sread=0;

            printf("GET(Download mode)\n");
	    printf("파일 경로 : ");

	    if ( fgets(filepath, sizeof(filepath), stdin) == NULL )
		exit(0);

	    filepathsize = strlen(filepath);
	    filepath[filepathsize-1] = 0;

	    printf("파일 이름 : ");

	    if ( fgets(filename, sizeof(filename), stdin) == NULL )
		exit(0);

	    filenamesize = strlen(filename);
	    filename[filenamesize-1] = 0;

	    strcat(filepath, filename);
	    getcwd(clientpath, 50);
	    strcat(clientpath, "/");
	    strcat(clientpath, filename);

	    if( (fp = open( filepath, O_RDONLY )) < 0 ) {
		printf( "\n[에러 : 파일이 존재하지 않습니다 !!]\n\n" );
		send( sd, "error", sizeof("error"), 0 );
		continue;
	    }

	    printf( "\n[파일이 존재합니다 !!]\n\n" );

	    send( sd, filepath, sizeof(filepath), 0 );
	    send( sd, clientpath, sizeof(clientpath), 0 );

	    printf("%s\n", clientpath);

	    filesize = lseek( fp, 0, SEEK_END );
	    send( sd, &filesize, sizeof(filesize), 0 );
	    lseek(fp, 0, SEEK_SET );

	    while( total != filesize ) {
		sread = read( fp, buf, 100 );
		total += sread;
		buf[sread] = 0;
		send( sd, buf, sread, 0 );
	    }
	    printf( "파일 다운로드 완료 !! \n" );
	    printf( "파일 크기 : %d \n\n", filesize);
	    close(fp);
	    continue;
	}

        printf("%s", buf);
    }
    return 0;
}
