#ifndef PROTOCOL_H
#define PROTOCOL_H

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
#include <string>
#include <sstream>
#include <fstream>
#include <iterator>
#include <vector>

using namespace std;


string FirstNumberToString(int n)
{
     ostringstream ss;
     ss << n;
     return ss.str();
}

string number_to_string_with_padding(int n, int n_bytes){
    string ret = FirstNumberToString(n);
    ret = string(n_bytes-ret.length(), '0').append(ret);
    return ret;
}

//The next two methods are a bad practice,
//but works it's necessary optimize this methods

string encode_simple_message(string message){
    char tmp = message[0];
    message.erase(0,1);
    int size_message = message.length();
    string size_str = FirstNumberToString(size_message);
    size_str = string(4-size_str.length(), '0').append(size_str);
    size_str += tmp;
    size_str += message;
    return size_str;
}

string encode_game_message(string message)
{
    char tmp = message[0];
    message.erase(0,2);
    int size_message = message.length();
    string size_str = FirstNumberToString(size_message);
    size_str = string(4-size_str.length(), '0').append(size_str);
    size_str += tmp;
    size_str += message;
    return size_str;
}

string encode_to_user_message(string message, string to_user, char custom_op='C'){
    int size_message = message.length();
    string size_str = FirstNumberToString(size_message);
    string size_str_user = FirstNumberToString(to_user.length());
    size_str = string(4-size_str.length(), '0').append(size_str);
    size_str += custom_op;
    size_str += string(2-size_str_user.length(), '0').append(size_str_user);
    size_str += to_user;
    size_str += message;
    return size_str;
}



namespace file_utils {
    static string read_all_bytes(char const* filename)
    {
        ifstream ifs(filename, ios::binary|ios::ate);
        ifstream::pos_type pos = ifs.tellg();
        std::vector<char>  vec_buffer(pos);
        ifs.seekg(0, ios::beg);
        ifs.read(&vec_buffer[0], pos);
        std::string ret = string(vec_buffer.begin(), vec_buffer.end());
        return ret;
    }

    void get_size_string_of_file(string &filename, string &binary_file, int &size_file){
        binary_file = read_all_bytes(filename.c_str());
        size_file = binary_file.length();
    }
    string prepare_file_message(string binary_file, int size_file,
                                string filename, string user_dest){
        string ret = number_to_string_with_padding(filename.length(), 4);
        ret += 'F';
        ret += number_to_string_with_padding(user_dest.length(), 2);
        ret += user_dest;
        ret += filename;
        ret += number_to_string_with_padding(size_file, 4);
        ret += binary_file;
        return ret;
    }
    void process_received_file(int socket, int size_filename){
        string nickname_source, filename, file_binary;
        int file_size, /*a n to read bytes*/ n;
        char *buffer = new char[2];
        n = read(socket, buffer, 2);
        int nickname_source_size = atoi(buffer);
        buffer = new char[nickname_source_size];
        n = read(socket, buffer, nickname_source_size);
        nickname_source = string(buffer);

        buffer = new char[size_filename];
        n = read(socket, buffer, size_filename);
        filename = string(buffer);

        buffer = new char[4];
        n = read(socket, buffer, 4);
        file_size = atoi(buffer);
        cout << file_size << endl;

        //readed the size_file now read the file
        std::vector<char> buf_file(file_size);
        //message_buffer = new char[file_size];
        n = read(socket, &buf_file[0], file_size);
        //file_binary = string(buf_file.begin(), buf_file.end());

        cout << "File Received from: " << nickname_source << " file_name is: " << filename << endl;
        ofstream fout(filename.c_str(), ios::out | ios::binary);
        fout.write((char*)&buf_file[0], buf_file.size());
        fout.close();

    }
}

#endif // PROTOCOL_H
