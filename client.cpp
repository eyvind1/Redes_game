#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <random>
#include <iostream>
#include <thread>
#include <ncurses.h>
#include <chrono>
#include "protocol.h"
#include "game_utils.h"
#include <map>
#include <time.h>
#include <stdlib.h>
#include "bullet_thread.h"

using namespace std;

WIN win;
WIN win_second;

map<int,WIN> bingo_players;
ThreadBullet bullets;

void print_vector(vector<char> vec){
    for(int i=0; i<vec.size(); i++)
        cout << vec[i];
    cout << endl;
}


void read_from_client(int SocketFD){
    //Modify this it's necessary only read 4 bytes
    //we use 256 to allocate all type of messages
    //this function shows all the coded message
    char *message_buffer;
    char buffer[4];
    bzero(buffer,4);
    int n = read(SocketFD,buffer,4);
    bool first_game = true;
    do{
        if (n>0){
            int size_message = atoi(buffer);
            char buffer_op[1];
            //now read operation
            n = read(SocketFD, buffer_op, 1);
            if(buffer_op[0] == 'R'){
                message_buffer = new char[size_message];
                n = read(SocketFD, message_buffer, size_message);
                cout << "Server Message Received:  " << message_buffer << endl;
            }
            else if(buffer_op[0] == 'D'){
                file_utils::process_received_file(SocketFD, size_message);
            }
            else if(buffer_op[0] == 'I'){
                message_buffer = new char[size_message];
                n = read(SocketFD, message_buffer, size_message);

                int playerID = atoi(message_buffer);
        //        mvprintw(25, 0, "%d %d", bingo_players[playerID].startx, bingo_players[playerID].starty);
                n = read(SocketFD, message_buffer,4);
                int x = atoi(message_buffer);
                n = read(SocketFD, message_buffer,4);
                int y = atoi(message_buffer);

                create_box(&(bingo_players[playerID]),FALSE);
                bingo_players[playerID].startx = x;
                create_box(&(bingo_players[playerID]),FALSE);
                bingo_players[playerID].starty = y;
                create_box(&(bingo_players[playerID]),FALSE);
                create_box(&(bingo_players[playerID]),TRUE);
            }
            else if(buffer_op[0] == 'W'){
                message_buffer = new char[size_message];
                n = read(SocketFD, message_buffer, size_message);

                // mvprintw(25, 0, "wwww");
                // refresh();

                //cout<< "Juego Iniciado: " << message_buffer << endl;
                int playerID = atoi(message_buffer);
                n = read(SocketFD, message_buffer,4);
                int x = atoi(message_buffer);
                n = read(SocketFD, message_buffer,4);
                int y = atoi(message_buffer);

                WIN new_player;
                bingo_players[playerID] = new_player;

                bingo_players[playerID].startx = x;
                bingo_players[playerID].starty = y;
                init_win_params(&(bingo_players[playerID]));
                map<int,WIN>::iterator it;
                for(it = bingo_players.begin(); it != bingo_players.end(); ++it){
                    create_box(&(it->second),FALSE);
                    create_box(&(it->second),TRUE);
                    refresh();
                }
            }
            else if(buffer_op[0] == 'B'){
                message_buffer = new char[size_message];
                n = read(SocketFD, message_buffer, size_message);
                int bullet_id = atoi(message_buffer);
                n = read(SocketFD, message_buffer, 4);
                int bullet_x = atoi(message_buffer);
                n = read(SocketFD, message_buffer, 4);
                int bullet_y = atoi(message_buffer);
                //mvprintw(25, 0, "%d %d",bullet_x,bullet_y);
                bullets.start_thread(bullet_id,bullet_x,bullet_y);
            }

            else if(buffer_op[0] == 'X'){
                message_buffer = new char[size_message];
                n = read(SocketFD, message_buffer, size_message);
                int disconnected_id = atoi(message_buffer);
                create_box(&(bingo_players[disconnected_id]),FALSE);
                bingo_players.erase(disconnected_id);
            }
        }
        bzero(buffer,4);
        n = read(SocketFD,buffer,4);
    }while(true);
}





int main(int argc, char *argv[])
{


    struct sockaddr_in stSockAddr;
    int Res;
    int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP );
    int n;

    if (-1 == SocketFD)
    {
        perror("cannot create socket");
        exit(EXIT_FAILURE);
    }

    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));

    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(1100);
    Res = inet_pton(AF_INET, "192.168.0.10", &stSockAddr.sin_addr);

    if (0 > Res)
    {
        perror("error: first parameter is not a valid address family");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }
    else if (0 == Res)
    {
        perror("char string (second parameter does not contain valid ipaddress");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }

    if (-1 == connect(SocketFD, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in)))
    {
        perror("connect failed");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }

    std::thread(read_from_client, SocketFD).detach();
    cout << "Connecting..." << endl;

    string input_message;
    string to_send;
    //Please never use cin alone, use cin with get line
    while(true){
        cout<<"Ingrese una de las opciones"<<endl;
        cout<<"P : Print list of users"<<endl;
        cout<<"L : Login chat"<<endl;
        cout<<"C : Send message "<<endl;
        cout<<"F : Send a file "<<endl;
        cout<<"E : logout"<<endl;
        std::getline(std::cin, input_message);
        //We have 3 cases to send
        //Sending P we will receive the users list
        if(input_message == "P"){
            to_send = "";
            to_send = encode_simple_message(string("P"));
        }
        //Writing L, we can login in the chat server
        else if(input_message == "L"){
            to_send = "";
            cout << "Please enter your nickname: ";
            std::getline(std::cin, to_send);
            to_send = encode_simple_message(string("L")+to_send);
        }
        //Writing C we can send a message to other user
        else if(input_message == "C"){
            string to_user;
            to_send = "";
            cout << "Enter the username to send: ";
            std::getline(std::cin, to_user);
            cout << "Enter the message: ";
            std::getline(std::cin, to_send);
            to_send = encode_to_user_message(to_send, to_user);
        }
        else if(input_message == "F"){
            string to_user;
            string filename;
            string binary_file;
            int size_file;
            to_send = "";
            cout << "Enter the username to send: ";
            std::getline(std::cin, to_user);
            cout << "Enter the filename: ";
            std::getline(std::cin, filename);
            file_utils::get_size_string_of_file(filename, binary_file, size_file);
            to_send = file_utils::prepare_file_message(binary_file, size_file, filename, to_user);
            std::vector<char> v_data(to_send.begin(), to_send.end());
            //cout << filename << endl;
            n = write(SocketFD, &v_data[0], v_data.size());
            input_message = "";
            continue;
        }
        else if(input_message == "E"){
            to_send = "";
            to_send = encode_simple_message(string("E"));
        }

        else if(input_message == "G"){

            initscr();			/* Start curses mode 		*/
            std::random_device rd;
            std::mt19937 mt(rd());
            std::uniform_int_distribution<> dist(4,10);

            int starty = (LINES - 3)/ dist(mt);
            int startx = (COLS - 10)/ dist(mt);
            string msg = encode_welcome_game_message(startx,starty);

            n = write(SocketFD, msg.c_str(), msg.length());



            int ch;

            start_color();			/* Start the color functionality */
            cbreak();			/* Line buffering disabled, Pass on
                                 * everty thing to me 		*/
            keypad(stdscr, TRUE);		/* I need that nifty F1 	*/
            noecho();
            init_pair(1, COLOR_CYAN, COLOR_BLACK);

            /* Initialize the window parameters */
            //init_win_params(&win);
            //print_win_params(&win);

            attron(COLOR_PAIR(1));
            printw("Press F1 to exit");
            refresh();
            attroff(COLOR_PAIR(1));

            //create_box(&win, TRUE);
            while((ch = getch()) != KEY_F(1)){
                //
                to_send = encode_movement_message(ch);
                n = write(SocketFD, to_send.c_str(), to_send.length());


                // input_message = "";
            }
            endwin();
        }
        else{
            cout << "Command not recognized :(" << endl;
            continue;
        }
        n = write(SocketFD, to_send.c_str(), to_send.length());
        input_message = "";
    }

    shutdown(SocketFD, SHUT_RDWR);
    close(SocketFD);

    return 0;
}
