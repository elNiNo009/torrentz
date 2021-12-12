/*
    Name : Anurag Maithani
    Name : Dheeraj K. Pant
*/

#include <common.h>
#include <openssl/sha.h>
#include <sys/stat.h>
#include <experimental/filesystem>
#define DATABASE_FILE "/home/mtorrent/database.txt"
#ifndef CLIENT_DATABASE_H
#define CLIENT_DATABASE_H

struct torrent_DB
{
    string SHA_hash;
    string location;
    lo size;
    vector<lo> part_of_file;
};
istream &operator>>(istream &in, torrent_DB &entry);
ostream &operator<<(ostream &out, torrent_DB &entry);

struct torrent_for_map
{
    string location;
    vector<lo> part_of_file;
};
lo make_entry(string &hash, string &loc, vl &part_of_file);
lo load_to_map(map<string, torrent_for_map> &map_hash_to_file);
torrent_for_map generate_torrent(string Tracker_1_url, string Tracker_2_url, string filename, string mtorrent);
#endif