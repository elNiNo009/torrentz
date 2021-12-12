/*
    Name : Anurag Maithani
    Name : Dheeraj K. Pant
*/

#include <common.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#ifndef MESSAGE_SERVER_H
#define MESSAGE_SERVER_H

struct Message
{
    lo no_of_fields;
    vector<string> fields_content;
    vl fields_size;
    Message(initializer_list<string>);
    Message(vector<string>);
    Message();
    void clear();
    void reload(initializer_list<string>);
    void reload(vector<string>);
    string encode_message();
    vector<string> decode_message(int file);
};

#endif