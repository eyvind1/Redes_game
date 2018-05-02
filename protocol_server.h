#ifndef PROTOCOL_SERVER_H
#define PROTOCOL_SERVER_H

#include <iostream>
#include <map>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sstream>
#include <vector>
#include <ncurses.h>
#include <chrono>
#include <ncurses.h>
#include <thread>
using namespace std;
#define DELAY 120000



void print_vector(vector<char> vec){
    for(int i=0; i<vec.size(); i++)
        cout << vec[i];
    cout << endl;
}

string NumberToString(int n)
{
    ostringstream ss;
    ss << n;
    return ss.str();
}

string Numberstring_with_padding(int n, int n_bytes){
    string ret = NumberToString(n);
    ret = string(n_bytes-ret.length(), '0').append(ret);
    return ret;
}

typedef pair<int,int> coord;
map<int,coord> bullets_positions;
map<string,int> clients;
map<int,coord> movements_game;

void update_bullet_positions(){ // also colissions hehehehe
    while(true){
        if(!bullets_positions.empty()){
            map<int,coord>::iterator bullet;
            for(bullet = bullets_positions.begin(); bullet != bullets_positions.end(); ++bullet){
                if( (bullet->second).second != 0){
                    usleep(DELAY);
                    --(bullet->second).second;
                    map<int,coord>::iterator player;
                    for(player = movements_game.begin(); player != movements_game.end(); ++player){
                        if( (bullet->second.second == player->second.second) && (bullet->second.first >= player->second.first && bullet->second.first <= (player->second.first+10)) ){
                            cout<< "bullet on player "<< player->first<<endl;
                        }
                    }
                }
                else{
                    bullets_positions.erase(bullet);
                }
            }
        }
    }
}


class Messsage
{
public:
    int socket_dest;
    bool isFile;
    string message;
    Messsage(int socket_dest, string message, bool isfile=false) {
        this->socket_dest = socket_dest;
        this->message = message;
        this->isFile = isfile;
    }
    Messsage(){}
    Messsage& operator =(Messsage msg){
        this->socket_dest = msg.socket_dest;
        this->message = msg.message;
        return *this;
    }
    void send_message(){
        //cout << "To: " << socket_dest << "  " << message << endl;
        int n;
        if(this->isFile){
            std::vector<char> v_data(message.begin(), message.end());
            n = write(socket_dest, &v_data[0], v_data.size());
            return;
        }
        n = write(socket_dest, message.c_str(), message.length());
    }
};




class Protocol
{
public:
    int count_bullet;
    Protocol(){
        count_bullet = 0;
        thread(update_bullet_positions).detach();
    }
    Messsage read_s(char operation, int size_message, int source_socket, vector<Messsage> &multi){
        int n;
        char *message_buffer;
        if(size_message == 0){
            if(operation == 'P')
                return Messsage(source_socket, prepare_simple_response(get_clients()));
            else if(operation == 'E'){
                this->remove_client(source_socket);
                return Messsage(source_socket, prepare_simple_response("You left the chat"));
            }
            // In case that a new player is incoming
            else if(operation == 'W'){
                message_buffer = new char[4];
                n = read(source_socket, message_buffer, 4);
                int x = atoi(message_buffer);
                message_buffer = new char[4];
                n = read(source_socket, message_buffer, 4);
                int y = atoi(message_buffer);
                movements_game[source_socket] = make_pair(x,y);

                for (std::map<int, coord>::iterator it=movements_game.begin(); it!=movements_game.end(); ++it){
                    string s_x = Numberstring_with_padding(it->second.first, 4);
                    string s_y = Numberstring_with_padding(it->second.second, 4);
                    string message_simple = prepare_simple_response(NumberToString(it->first),'W');
                    message_simple += s_x;
                    message_simple += s_y;
                    for (std::map<int, coord>::iterator it2=movements_game.begin(); it2!=movements_game.end(); ++it2){
                        Messsage tmp_msg(it2->first, message_simple);
                        multi.push_back(tmp_msg);
                    }
                }
                return Messsage(source_socket, prepare_simple_response("Welcome to the game"));
            }
            
        }
        if(operation == 'L'){
            message_buffer = new char[size_message];
            n = read(source_socket, message_buffer, size_message);
            //Validate that n and data in future
            this->add_client(string(message_buffer), source_socket);
            return Messsage(source_socket, prepare_simple_response("You are logged now"));
        }
        else if(operation == 'C'){
            //Read Two First Bytes
            message_buffer = new char[2];
            n = read(source_socket, message_buffer, 2);
            int dest_name_size = atoi(message_buffer);
            //With that read for who is the message
            message_buffer = new char[dest_name_size];
            n = read(source_socket, message_buffer, dest_name_size);
            string dest_name = string(message_buffer);

            map<string,int>::iterator i = clients.find(dest_name);
            if (i == clients.end())
                return Messsage(source_socket, prepare_simple_response(dest_name + " is offline"));

            int dest_socket = clients[dest_name]; //get te socket by the name

            //now read the message
            message_buffer = new char[size_message];
            n = read(source_socket, message_buffer, size_message);
            string message = this->get_name_client(source_socket) + " says: " + string(message_buffer);
            return Messsage(dest_socket, prepare_simple_response(message));
        }
        else if (operation == 'F') {
            //get dest nickname size
            message_buffer = new char[2];
            n = read(source_socket, message_buffer, 2);
            int dest_name_size = atoi(message_buffer);
            // the file name size it's on the first 4 bytes
            int file_name_size = size_message;

            //Now read the nickname and filename
            char *nickname_dest = new char[dest_name_size];
            char *filename = new char[file_name_size];
            n = read(source_socket, nickname_dest, dest_name_size);

            map<string,int>::iterator i = clients.find(string(nickname_dest));
            if (i == clients.end())
                return Messsage(source_socket, prepare_simple_response(string(nickname_dest) + " is offline"));

            n = read(source_socket, filename, file_name_size);

            //read the size of the file
            message_buffer = new char[4];
            n = read(source_socket, message_buffer, 4);
            int file_size = atoi(message_buffer);

            //with the size its time to read the file
            std::vector<char> buf_file(file_size);
            //message_buffer = new char[file_size];
            n = read(source_socket, &buf_file[0], file_size);
            string file_str = string(buf_file.begin(), buf_file.end());

            int dest_socket = clients[string(nickname_dest)];
            string source_nickname = this->get_name_client(source_socket);


            string to_msg = prepare_file_response(source_nickname, file_name_size, string(filename), file_size, file_str);
            return Messsage(dest_socket, to_msg, true);
        }
        else if(operation == 'M'){
            message_buffer = new char[size_message];
            n = read(source_socket, message_buffer, size_message);
            int move = atoi(message_buffer);
            coord originals = movements_game[source_socket];

            if(move == 120){
                int x_bullet = originals.first + 5;
                int y_bullet = originals.second - 3;
                string s_bullet_x = Numberstring_with_padding(x_bullet, 4);
                string s_bullet_y = Numberstring_with_padding(y_bullet, 4);
                int id_bullet = ++count_bullet;
                bullets_positions[id_bullet] = make_pair(x_bullet, y_bullet);
                string message_bullet = prepare_simple_response(NumberToString(id_bullet),'B');
                message_bullet += s_bullet_x;
                message_bullet += s_bullet_y;

                for (std::map<int, coord>::iterator it=movements_game.begin(); it!=movements_game.end(); ++it){
                    Messsage tmp_msg_bullet(it->first, message_bullet);
                    multi.push_back(tmp_msg_bullet);
                }
                return Messsage(source_socket, prepare_simple_response("",'P'));
            }

            switch(move)
            {
            case KEY_LEFT:
                --originals.first;
                break;
            case KEY_RIGHT:
                ++originals.first;
                break;
            case KEY_UP:
                --originals.second;
                break;
            case KEY_DOWN:
                ++originals.second;
                break;
            }
            movements_game[source_socket] = originals;
            for (std::map<int, coord>::iterator it=movements_game.begin(); it!=movements_game.end(); ++it){
                string s_x = Numberstring_with_padding(it->second.first, 4);
                string s_y = Numberstring_with_padding(it->second.second, 4);
                string message_simple = prepare_simple_response(NumberToString(it->first),'I');
                message_simple += s_x;
                message_simple += s_y;
                for (std::map<int, coord>::iterator it2=movements_game.begin(); it2!=movements_game.end(); ++it2){
                    Messsage tmp_msg(it2->first, message_simple);
                    multi.push_back(tmp_msg);
                }
            }
            return Messsage(source_socket, prepare_simple_response("",'P'));
        }
    }

    string get_clients(){
        string ret = "\n";
        for (std::map<string,int>::iterator it=clients.begin(); it!=clients.end(); ++it)
            ret += it->first + "\n";
        return ret;
    }

    void remove_client(int socket){
        string name;
        name = this->get_name_client(socket);
        clients.erase(name);
    }

    string get_name_client(int socket){
        for (std::map<string,int>::iterator it=clients.begin(); it!=clients.end(); ++it)
            if(it->second == socket) return it->first;
        return "-";
    }

    string prepare_simple_response(string message, char protocol_ch = 'R'){
        int size_message = message.length();
        string size_str = NumberToString(size_message);
        size_str = string(4-size_str.length(), '0').append(size_str);
        size_str += protocol_ch;
        size_str += message;
        return size_str;
    }

    string prepare_game_response(string message){
        int size_message = message.length();
        string size_str = NumberToString(size_message);
        size_str = string(4-size_str.length(), '0').append(size_str);
        size_str += 'I';
        size_str += message;
        return size_str;
    }

    string prepare_file_response(string source_nick_name, int file_name_size, string file_name, int file_size, string file){
        string ret = "";

        string str_file_name_size = NumberToString(file_name_size);
        str_file_name_size = string(4-str_file_name_size.size(), '0').append(str_file_name_size);
        ret += str_file_name_size;

        char action = 'D';
        ret += action;

        string str_size_nickname = NumberToString(source_nick_name.size());
        str_size_nickname = string(2-str_size_nickname.size(), '0').append(str_size_nickname);
        ret += str_size_nickname;
        ret += source_nick_name;

        ret += file_name;
        string str_file_size = NumberToString(file_size);
        str_file_size = string(4-str_file_size.size(), '0').append(str_file_size);
        ret += str_file_size;
        ret += file;

        return ret;
    }

    void add_client(string name, int socket){
        clients[name] = socket;
    }
};



#endif // PROTOCOL_SERVER_H
