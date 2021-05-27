#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
const int MAXLEN = 1e6;
char send_msg[MAXLEN];
char recv_msg[MAXLEN];

int main(int argc, const char * argv[]) {
    if (argc != 3) {
        printf("argument error.\nusage: ./client <ip address> <port>\n");
        exit(1);
    }
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("create socket error!");
    }
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    char *server_ip = (char*)malloc(strlen(argv[1]) * sizeof(char));
    strcpy(server_ip, argv[1]);
    if (strcmp(server_ip, "localhost") == 0) {
        strcpy(server_ip, "127.0.0.1");
    }
    if ((server_addr.sin_addr.s_addr = inet_addr(server_ip)) == INADDR_NONE) {
        perror("invalid ip address");
    }
    memset(server_addr.sin_zero, 0, sizeof(server_addr.sin_zero));
    //连接服务器
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect error");
        exit(0);
    }
    //发送消息
    recv(sockfd, recv_msg, sizeof(recv_msg), 0);
    printf("\nServer: \n%s\n\n", recv_msg);
    printf("Me    : \n");
    while (std::cin.getline(send_msg, MAXLEN)) {
        send(sockfd, send_msg, strlen(send_msg), 0);
        if (!strcmp(send_msg, "bye()") || send_msg[0] == '3') {
            break;
        }
        //接收并显示消息
        memset(recv_msg, 0, MAXLEN * sizeof(char)); //接收数组置零
        if (recv(sockfd, recv_msg, sizeof(recv_msg), 0) == -1) {
            perror("receive error!");
        }
        printf("\nServer: \n%s\n\n", recv_msg);
        printf("Me    : \n");
    }
    //关闭socket
    close(sockfd);
    return 0;
}
