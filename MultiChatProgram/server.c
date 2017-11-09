#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

#define MAXLINE  511
#define MAX_SOCK 1024
#define MAXCLIENT 10

char *EXIT_STRING = "exit";
char *HELLO_STRING = "hello";
char *START_STRING = "Connected to chat_server \n";

char join_time[10][50];
char client_ip[10][15];

// 클라이언트 환영 메시지
int maxfdp1;
int num_chat = 0;
int clisock_list[MAX_SOCK];
int listen_sock;

// 새로운 채팅 참가자 처리
void addClient(int s, struct sockaddr_in *newcliaddr);
int getmax();
void removeClient(int s);
int tcp_listen(int host, int port, int backlog);
void errquit(char *mesg) { perror(mesg); exit(1); }

int main(int argc, char *argv[]) {
    struct sockaddr_in cliaddr;
    char buf[MAXLINE+1];
    int i, j, nbyte, accp_sock, addrlen = sizeof(struct
        sockaddr_in);
    fd_set read_fds;

    if(argc != 2) {
        printf("사용법 :%s [포트번호]\n", argv[0]);
        exit(0);
    }

    // tcp_listen(host, port, backlog)
    listen_sock = tcp_listen(INADDR_ANY, atoi(argv[1]), MAXCLIENT);

    time_t seconds = time(NULL);
    char *str = ctime(&seconds);
    strcpy(join_time[num_chat-1], str);
    strcpy(client_ip[num_chat-1], buf);

    puts("[** 서버가 접속하였습니다. **]");
    printf("[서버의 최초 접속 시간] : %s", str);
    printf("[현재 접속자 수] : %d명\n\n", num_chat);

    while(1) {
        FD_ZERO(&read_fds);
        FD_SET(listen_sock, &read_fds);
        for(i=0; i<num_chat; i++)
            FD_SET(clisock_list[i], &read_fds);
        maxfdp1 = getmax() + 1;     // maxfdp1
        if(select(maxfdp1, &read_fds,NULL,NULL,NULL)<0)
            errquit("select fail");

        if(FD_ISSET(listen_sock, &read_fds)) {
           accp_sock=accept(listen_sock, (struct sockaddr
                *)&cliaddr, &addrlen);
            if(accp_sock == -1)
                errquit("accept fail");
	 
            addClient(accp_sock,&cliaddr);
            
            send(accp_sock, START_STRING, strlen(START_STRING), 0);
        }

	// 클라이언트가 보낸 메시지를 모든 클라이언트에게 방송
        for(i = 0; i < num_chat; i++) {
           if(FD_ISSET(clisock_list[i], &read_fds)) {
                nbyte = recv(clisock_list[i], buf, MAXLINE, 0);
                if(nbyte<= 0) {
                    removeClient(i);
                    continue;
                }
                buf[nbyte] = 0;

                if(strstr(buf, EXIT_STRING) != NULL) {
                    removeClient(i);
                    continue;
                }

		if(strstr(buf, HELLO_STRING) != NULL) {
                    
		    for(i = 0; i < num_chat; i++)
			send(clisock_list[i], join_time[i], 50, 0);

                    continue;
                }

                for (j = 0; j < num_chat; j++)
                    send(clisock_list[j], buf, nbyte, 0);
                printf("%s\n", buf);
            }
        }
    }  // end of while
    return 0;
}

// 새로운 채팅 참가자 처리
void addClient(int s, struct sockaddr_in *newcliaddr) {
    char buf[20];
    inet_ntop(AF_INET,&newcliaddr->sin_addr,buf,sizeof(buf));
    clisock_list[num_chat] = s;
    num_chat++;
    printf("[** 새로운 사용자가 들어왔습니다. **]\n"); 
    printf("[현재 접속자 수 = %d]\n", num_chat);
    printf("[접속자 IP] : %s\n", buf);

    time_t seconds = time(NULL);
    char *str = ctime(&seconds);
    strcpy(join_time[num_chat-1], str);
    strcpy(client_ip[num_chat-1], buf);
    printf("[접속 시간] : %s\n", join_time[num_chat-1]);
    int i;
    
}

// 채팅 탈퇴 처리
void removeClient(int s) {
    int i;
    close(clisock_list[s]);
    if(s != num_chat-1) {
        clisock_list[s] = clisock_list[num_chat-1];
	for(i = 0; i < num_chat; i++) {
	    strcpy(join_time[i], join_time[i+1]);
	    strcpy(client_ip[i], client_ip[i+1]);
	}
    }
    num_chat--;
    printf("[** 채팅 참가자 1명 탈퇴 **]\n");
    printf("[현재 접속자 수 = %d]\n\n", num_chat);
}

// 최대 소켓번호 찾기
int getmax() {
    int max = listen_sock;
    int i;
    for (i=0; i < num_chat; i++)
        if (clisock_list[i] > max )
            max = clisock_list[i];
    return max;
}

// listen 소켓 생성 및 listen
int  tcp_listen(int host, int port, int backlog) {
    int sd;
    struct sockaddr_in servaddr;

    sd = socket(AF_INET, SOCK_STREAM, 0);
    if(sd == -1) {
        perror("socket fail");
        exit(1);
    }

    bzero((char *)&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(host);
    servaddr.sin_port = htons(port);
    if (bind(sd , (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind fail");  exit(1);
    }

    listen(sd, backlog);
    return sd;
}
