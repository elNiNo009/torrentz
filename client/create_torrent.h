/*
    Name : Anurag Maithani
    Name : Dheeraj K. Pant
*/

#include <common.h>
#include <client_database.h>
#include <openssl/sha.h>
#ifndef CREATE_TORRENT_H
#define CREATE_TORRENT_H

struct mtorrent
{
    string tracker_1_url;
    string tracker_2_url;
    string filename;
    lo filesize;
    string SHA_hash;
    //lo generate_torrent(string Tracker_1_url, string Tracker_2_url, string filename, string mtorrent);
};

istream &operator>>(istream &in, mtorrent &entry);
ostream &operator<<(ostream &out, mtorrent &entry);

#endif