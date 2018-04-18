#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <ncurses.h>
#include <chrono> 
#include "protocol_server.h"

using namespace std;

std::mutex mutx;
queue<Messsage> queue_messages;
Protocol protocol_chat;


void read_from_client(int SocketFD){
    char *message_buffer;
    char buffer[4];
    bzero(buffer,4);
    int n = read(SocketFD,buffer,4);
    do{
        if (n>0){
            int size_message = atoi(buffer);
            char buffer_op[1];
            //now read operation
            n = read(SocketFD, buffer_op, 1);
            Messsage msg = protocol_chat.read_s(buffer_op[0],size_message,SocketFD);
            mutx.lock();
            queue_messages.push(msg);
            mutx.unlock();
            if(buffer_op[0]=='E') return;
        }
        bzero(buffer,4);
        n = read(SocketFD,buffer,4);
    }while(true);
}

void send_messages(){
    Messsage msg;
    while (true) {
        if(!queue_messages.empty()){
            mutx.lock();
            msg = queue_messages.front();
            queue_messages.pop();
            mutx.unlock();
            msg.send_message();
        }
    }
}

int main()
{
    struct sockaddr_in stSockAddr;
    int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(-1 == SocketFD)
    {
        perror("can not create socket");
        exit(EXIT_FAILURE);
    }
    int n;
    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));
    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(1100);
    stSockAddr.sin_addr.s_addr = INADDR_ANY;

    if(-1 == bind(SocketFD,(const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in)))
    {
        perror("error bind failed");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }

    if(-1 == listen(SocketFD, 10))
    {
        perror("error listen failed");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }
    // the thread to send messages of queue
    std::thread(send_messages).detach();

    for(;;)
    {
        int ConnectFD = accept(SocketFD, NULL, NULL);

        if(0 > ConnectFD)
        {
            perror("error accept failed");
            close(SocketFD);
            exit(EXIT_FAILURE);
        }
        // one thread por client to check
        std::thread(read_from_client, ConnectFD).detach();

    }
//    shutdown(ConnectFD, SHUT_RDWR);
//    close(ConnectFD);
    cout << "Disconnected..." << endl;
    return 0;
}
