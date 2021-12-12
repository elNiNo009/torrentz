/*
    Name : Anurag Maithani
    Name : Dheeraj K. Pant
*/

#include <respond_to_request_server.h>

void update_database(map<string, set<string>> &hash_map, string &file_name)
{
    ofstream out;
    out.open(file_name);
    TRV(hash_map)
    {
        out << it.X << endl;
        out << it.Y.size() << endl;
        for (auto it_in : it.Y)
        {
            out << it_in << endl;
        }
    }
    out.close();
}

pair<string, string> add_seeder(map<string, set<string>> &hash_vs_seeder_ip_port, vector<string> &request)
{
    string file_name = request[0];
    string SHA_hash = request[1];
    string client_ip_and_port = request[2];
    if (present(hash_vs_seeder_ip_port, SHA_hash))
    {
        hash_vs_seeder_ip_port[SHA_hash].insert(client_ip_and_port);
    }
    else
    {
        set<string> for_insertion;
        for_insertion.insert(client_ip_and_port);
        hash_vs_seeder_ip_port[SHA_hash] = for_insertion;
    }
    //update seeder_list_file
    //write to log
    cerr << "Successfully added to tracker" << endl;
    Message message({"SUCCESS", "Your Entry has been recorded"});
    Message message_for_tracker({"SHAREADD", file_name, SHA_hash, client_ip_and_port});
    return make_pair(message.encode_message(), message_for_tracker.encode_message());
}

void add_share_seeder(map<string, set<string>> &hash_vs_seeder_ip_port, vector<string> &request)
{
    string file_name = request[0];
    string SHA_hash = request[1];
    string client_ip_and_port = request[2];
    if (present(hash_vs_seeder_ip_port, SHA_hash))
    {
        hash_vs_seeder_ip_port[SHA_hash].insert(client_ip_and_port);
    }
    else
    {
        set<string> for_insertion;
        for_insertion.insert(client_ip_and_port);
        hash_vs_seeder_ip_port[SHA_hash] = for_insertion;
    }
    //update seeder_list_file
    //write to log
    return;
}

pair<string, string> remove_seeder(map<string, set<string>> &hash_vs_seeder_ip_port, vector<string> &request)
{
    string SHA_hash = request[0];
    string client_ip_and_port = request[1];
    Message message;
    debug(client_ip_and_port);
    if (present(hash_vs_seeder_ip_port, SHA_hash))
    {
        hash_vs_seeder_ip_port[SHA_hash].erase(client_ip_and_port);
        if (hash_vs_seeder_ip_port[SHA_hash].empty())
        {
            hash_vs_seeder_ip_port.erase(SHA_hash);
        }
        cerr << "Successfully removed from tracker" << endl;
        message.reload({"SUCCESS", "Your have been removed from the Seeder List"});
    }
    else
    {
        cerr << "Entry was not on tracker" << endl;
        message.reload({"ERROR", "You were not a seeder"});
    }
    //Update seeder_list_file
    Message message_for_tracker({"SHAREREMOVE", SHA_hash, client_ip_and_port});
    return make_pair(message.encode_message(), message_for_tracker.encode_message());
}

void remove_share_seeder(map<string, set<string>> &hash_vs_seeder_ip_port, vector<string> &request)
{
    string SHA_hash = request[0];
    string client_ip_and_port = request[1];
    Message message;
    debug(client_ip_and_port);
    if (present(hash_vs_seeder_ip_port, SHA_hash))
    {
        hash_vs_seeder_ip_port[SHA_hash].erase(client_ip_and_port);
        if (hash_vs_seeder_ip_port[SHA_hash].empty())
        {
            hash_vs_seeder_ip_port.erase(SHA_hash);
        }
        cerr << "Successfully removed from tracker" << endl;
        message.reload({"SUCCESS", "Your have been removed from the Seeder List"});
    }
    else
    {
        cerr << "Entry was not on tracker" << endl;
        message.reload({"ERROR", "You were not a seeder"});
    }
    //Update seeder_list_file
}

string provide_seeder_list(map<string, set<string>> &hash_vs_seeder_ip_port, vector<string> &request)
{
    string SHA_hash = request[0];
    Message message;
    vector<string> result;
    if (!present(hash_vs_seeder_ip_port, SHA_hash))
    {
        message.reload({"ERROR", "Requested file not found"});
        return message.encode_message();
    }
    result.push_back("SUCCESS");
    TRV(hash_vs_seeder_ip_port[SHA_hash])
    {
        result.push_back(it);
    }
    cerr << "Successfully given list" << endl;
    message.reload(result);
    return message.encode_message();
}

string dump(map<string, set<string>> &hash_map)
{
    Message message;
    vector<string> to_send;
    TRV(hash_map)
    {
        to_send.push_back(it.X);
        to_send.push_back(to_string(it.Y.size()));
        for (auto it_in : it.Y)
        {
            to_send.push_back(it_in);
        }
    }
    message.reload(to_send);
    return message.encode_message();
}
void load_from_seeder_file(map<string, set<string>> &hash_map, string &file_name)
{
    ifstream fin;
    fin.open(file_name);
    string hash;
    while (fin >> hash)
    {
        lo number_of_clients;
        fin >> number_of_clients;
        set<string> for_inserting;
        while (number_of_clients--)
        {
            string client;
            fin >> client;
            for_inserting.insert(client);
        }
        hash_map[hash] = for_inserting;
    }
}

void load_from_message(map<string, set<string>> &hash_map, vector<string> details)
{
    lo i = 0;
    while (i < details.size())
    {
        string hash = details[i];
        i++;
        lo number_of_clients = stoll(details[i]);
        i++;
        set<string> for_inserting;
        while (number_of_clients--)
        {
            for_inserting.insert(details[i]);
            i++;
        }
        hash_map[hash] = for_inserting;
    }
}
