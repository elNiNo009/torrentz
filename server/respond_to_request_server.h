/*
    Name : Anurag Maithani
    Name : Dheeraj K. Pant
*/

#include <message_server.h>
#ifndef RESPOND_TO_REQUEST_SERVER_H
#define RESPOND_TO_REQUEST_SERVER_H

pair<string, string> add_seeder(map<string, set<string>> &hash_vs_seeder_ip_port, vector<string> &request);
void add_share_seeder(map<string, set<string>> &hash_vs_seeder_ip_port, vector<string> &request);
pair<string, string> remove_seeder(map<string, set<string>> &hash_vs_seeder_ip_port, vector<string> &request);
void remove_share_seeder(map<string, set<string>> &hash_vs_seeder_ip_port, vector<string> &request);
string provide_seeder_list(map<string, set<string>> &hash_vs_seeder_ip_port, vector<string> &request);
void update_database(map<string, set<string>> &hash_map, string &file_name);
string dump(map<string, set<string>> &hash_map);
void load_from_seeder_file(map<string, set<string>> &hash_map, string &file_name);
void load_from_message(map<string, set<string>> &hash_map, vector<string> details);

#endif