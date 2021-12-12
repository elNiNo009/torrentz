/*
    Name : Anurag Maithani
    Name : Dheeraj K. Pant
*/

#include <create_torrent.h>

istream &operator>>(istream &in, mtorrent &entry)
{
    getline(in, entry.tracker_1_url);
    getline(in, entry.tracker_2_url);
    getline(in, entry.filename);
    getline(in, entry.SHA_hash);
    entry.filesize = stoll(entry.SHA_hash);
    getline(in, entry.SHA_hash);
    return in;
}
ostream &operator<<(ostream &out, mtorrent &entry)
{
    out << entry.tracker_1_url << endl;
    out << entry.tracker_2_url << endl;
    out << entry.filename << endl;
    out << entry.filesize << endl;
    out << entry.SHA_hash << endl;
    return out;
}