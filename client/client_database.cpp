/*
    Name : Anurag Maithani
    Name : Dheeraj K. Pant
*/

#include <client_database.h>

istream &operator>>(istream &in, torrent_DB &entry)
{
    getline(in, entry.SHA_hash);
    getline(in, entry.location);
    in >> entry.size;
    entry.part_of_file.resize(entry.size);
    in >> entry.part_of_file;
    in.ignore();
    return in;
}
ostream &operator<<(ostream &out, torrent_DB &entry)
{
    out << entry.SHA_hash << endl;
    out << entry.location << endl;
    out << entry.size << endl;
    out << entry.part_of_file << endl;
    return out;
}

lo make_entry(string &hash, string &loc, vl &part_of_file)
{
    torrent_DB single_entry_for_file;
    single_entry_for_file.SHA_hash = hash;
    single_entry_for_file.location = loc;
    single_entry_for_file.part_of_file = part_of_file;
    single_entry_for_file.size = part_of_file.size();
    ofstream fout;
    fout.open(DATABASE_FILE, ios::app);
    if (!fout.is_open())
    {
        cerr << "Couldn't open the Database file" << endl;
        return 0;
    }
    fout << single_entry_for_file;
    fout.close();
    return 1;
}
lo load_to_map(map<string, torrent_for_map> &map_hash_to_file)
{
    ifstream fin;
    fin.open(DATABASE_FILE, ios::in);
    torrent_DB temp;
    torrent_for_map for_map;
    while (fin >> temp)
    {
        for_map.location = temp.location;
        for_map.part_of_file = temp.part_of_file;
        map_hash_to_file[temp.SHA_hash] = for_map;
    }
    fin.close();
    return 1;
}

std::ifstream::pos_type filesize(const char *filename)
{
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return in.tellg();
}

torrent_for_map generate_torrent(string Tracker_1_url, string Tracker_2_url, string file_name, string file_name_torrent)
{
    torrent_for_map generated_details;
    //debug(file_name);
    FILE *file;
    file = fopen(file_name.c_str(), "rb");
    char buffer[BUFFER_SIZE];
    string SHA_hash = "";
    vl temp;
    lo bytes_read;
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0)
    {
        unsigned char *conbuf = (unsigned char *)buffer;
        unsigned char hash[HASH_SIZE];
        //debug(buffer);
        SHA1(conbuf, bytes_read, hash);
        char mdString[SHA_DIGEST_LENGTH * 2 + 1];
        for (int i = 0; i < SHA_DIGEST_LENGTH; i++)
            sprintf(&mdString[i * 2], "%02x", (unsigned int)hash[i]);
        SHA_hash += string(mdString);
    }
    //debug(SHA_hash);
    fclose(file);

    lo file_size = filesize(file_name.c_str());
    unsigned char *conbuf = (unsigned char *)SHA_hash.c_str();
    unsigned char hash[HASH_SIZE];
    //debug(buffer);
    SHA1(conbuf, bytes_read, hash);
    char mdString[SHA_DIGEST_LENGTH * 2 + 1];
    for (int i = 0; i < SHA_DIGEST_LENGTH; i++)
        sprintf(&mdString[i * 2], "%02x", (unsigned int)hash[i]);
    SHA_hash.clear();
    SHA_hash += string(mdString);
    //if (x)
    temp.pb(1);
    ofstream fout;
    string full_file_name = file_name_torrent;
    fout.open(full_file_name, ios::out);
    fout << Tracker_1_url << endl;
    fout << Tracker_2_url << endl;
    fout << file_name << endl;
    fout << file_size << endl;
    fout << SHA_hash << endl;
    fout.close();
    generated_details.location = SHA_hash;
    lo num_of_parts = (file_size + BUFFER_SIZE - 1LL) / BUFFER_SIZE;
    //debug(file_size);
    //debug(num_of_parts);
    generated_details.part_of_file.resize(num_of_parts);
    fill(all(generated_details.part_of_file), 1);
    return generated_details;
}