#include <bits/stdc++.h>
#include "initialize.cpp"
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
const int MAXLEN = 1e6;
char recv_msg[MAXLEN];

int main(int argc, char *argv[]) {
    if (argc > 2 || argc < 2 || atoi(argv[1]) == 0) {
        printf("usage: ./server <port>\n");
        exit(1);
    }
    
    initSeperationChar();
    int fileCnt = initFilePaths();
    int wordCnt = initWordsNum();
    initTf();
    initWeight();
    std::cout << "Total file count: " << fileCnt << "\n";
    std::cout << "different words: " <<  wordCnt << "\n\n";

/********************************** socket **********************************/
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("create socket error");
    }
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1]));
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); //INADDR_ANY表示本机所有IP地址
    memset(&server_addr.sin_zero, 0, sizeof(server_addr.sin_zero)); //零填充
    //绑定socket与地址
    bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)); //监听socket
    listen(sockfd, 0);
    int cfd;
    int num = 0;
    pid_t pid;
    char client_ip[128];
    printf("等待询问......\n");
    while (233) {
        socklen_t len = sizeof(client_addr);
        if ((cfd = accept(sockfd, (struct sockaddr *)&client_addr, &len)) == -1) {
            perror("accept error");
            exit(0);
        }
        // 清除僵尸进程
        signal(SIGCHLD, SIG_IGN);
        // 获取客户端ip地址、端口
        const char *ip = inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_ip, sizeof(client_ip));
        const int port = ntohs(client_addr.sin_port);
        printf("Accept %s:%d\n", ip, port);
        fflush(stdout);
        pid = fork();
        if (pid == 0) {
            close(sockfd);
            memset(recv_msg, 0, sizeof(recv_msg)); //接收数组置零
            ssize_t tag;
            send(cfd, "请输入查询: ", strlen("请输入查询: "), 0);
            while ((tag = recv(cfd, recv_msg, sizeof(recv_msg), 0)) != 0) {
                std::string msg(recv_msg);
                if (tag == -1) perror("receive error");
                if (msg == "bye()") {
                    break;
                }
                std::string sendstr("搜索结果:\n");
                std::string queryMsg = msg;
                std::vector<int> answer = query(msg);
                sendstr = getResultStr(answer);
                sendstr += "输入「1」进行相关反馈查询，\n";
                sendstr += "输入「2」进行新的词项查询，\n";
                sendstr += "输入「3」结束查询：\n";
                send(cfd, sendstr.c_str(), strlen(sendstr.c_str()), 0);
                memset(recv_msg, 0, sizeof(recv_msg));
                recv(cfd, recv_msg, sizeof(recv_msg), 0);
                msg = recv_msg;
                while (msg[0] != '1' && msg[0] != '2' && msg[0] != '3') {
                    // std::cout << "wtf? " << msg << fflush;
                    sendstr = "输入「1」进行相关反馈查询，\n";
                    sendstr += "输入「2」进行新的词项查询，\n";
                    sendstr += "输入「3」结束查询：\n";
                    send(cfd, sendstr.c_str(), strlen(sendstr.c_str()), 0);
                    memset(recv_msg, 0, sizeof(recv_msg));
                    recv(cfd, recv_msg, sizeof(recv_msg), 0);
                    msg = recv_msg;
                }
                if (msg[0] == '1') {
                    sendstr = "请输入相关文档以供相关反馈查询，用逗号(,)隔开。\n";
                    sendstr += "注：如果格式不符合，则此次查询取消相关反馈：\n";
                    send(cfd, sendstr.c_str(), strlen(sendstr.c_str()), 0);
                    memset(recv_msg, 0, sizeof(recv_msg));
                    recv(cfd, recv_msg, sizeof(recv_msg), 0);
                    msg = recv_msg;
                    std::set<char> u{','};
                    std::vector<std::string> tmp = split(msg, u);
                    std::vector<int> isRel(10);
                    std::vector<int> relevant;
                    std::vector<int> irrelevant;
                    int okFlag = 1;
                    for (size_t i = 0; i < tmp.size(); i++) {
                        if (!isnumber(tmp[i])) {
                            okFlag = 0;
                            break;
                        }
                        int relevantID = atoi(tmp[i].c_str()) - 1;
                        if (relevantID < 0 || relevantID > 9) {
                            okFlag = 0;
                            break;
                        }
                        isRel[relevantID] = 1;
                    }
                    for (size_t i = 0; i < isRel.size(); i++) {
                        if (isRel[i]) relevant.push_back(answer[i]);
                        else irrelevant.push_back(answer[i]);
                    }
                    if (!okFlag) {
                        sendstr = "非法的反馈序列！本次查询结束！\n";
                        sendstr += "请输入查询: ";
                        send(cfd, sendstr.c_str(), strlen(sendstr.c_str()), 0);
                    }
                    else {
                        std::vector<int> answer = feedback(queryMsg, relevant, irrelevant);
                        sendstr = getResultStr(answer);
                        sendstr = "相关反馈查询结果: \n" + sendstr;
                        sendstr += "请输入查询: ";
                        send(cfd, sendstr.c_str(), strlen(sendstr.c_str()), 0);
                    }
                }
                else if (msg[0] == '2') {
                    send(cfd, "请输入查询: ", strlen("请输入查询: "), 0);
                }
                else {
                    break;
                }
                memset(recv_msg, 0, sizeof(recv_msg));
            }
        }
        else if (pid > 0) {
            close(cfd);
        }
        else {
            perror("folk error");
        }
    }

    return 0;
}
