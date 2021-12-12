/*
    Name : Anurag Maithani
    Name : Dheeraj K. Pant
*/

#include <message_client.h>
#include <client_database.h>
#ifndef RESPOND_TO_REQUEST_CLIENT_H
#define RESPOND_TO_REQUEST_CLIENT_H

string send_details(map<string, torrent_for_map> &details_of_file, string &request);
string send_file(map<string, torrent_for_map> &details_of_file, vector<string> &request);

#endif