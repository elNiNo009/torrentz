/*
    Name : Anurag Maithani
    Name : Dheeraj K. Pant
*/

#include <message_client.h>
#include <thread>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sstream>
#include <mutex>
#include <netdb.h>
#include <create_torrent.h>
#include <respond_to_request_client.h>

#include <sys/types.h>
#include <sys/stat.h>

bool check_wether_a_directory_or_not(string pathname)
{
    struct stat info;

    if (stat(pathname.c_str(), &info) != 0)
    {
        return false;
    }
    else if (info.st_mode & S_IFDIR) // S_ISDIR() doesn't exist on my windows
    {
        return true;
    }
    return false;
}
bool check_wether_a_file_or_not(string pathname)
{
    struct stat info;

    if (stat(pathname.c_str(), &info) != 0)
    {
        return false;
    }
    else if (info.st_mode & S_IFREG) // S_ISDIR() doesn't exist on my windows
    {
        return true;
    }
    return false;
}
string client_ip_2;
vector<string> split_string(string str)
{
    string temp;
    stringstream iss(str);
    vector<string> result;
    while (iss >> temp)
    {
        result.push_back(temp);
    }
    return result;
}

int client_socket_fd;
map<string, torrent_for_map> details_of_file;
mutex details_of_file_mutex;
mutex log_file_descriptor;

set<string> downloaded_files;
set<string> currently_downloading_files;

mutex downloaded_files_mutex;
mutex currently_downloading_files_mutex;

map<string, vector<string>> list_of_part_contained;
mutex list_of_part_contained_mutex;

vector<string> tracker_url_combined(2);
int tracker_is_on;
bool change_tracker()
{
    tracker_is_on = (tracker_is_on + 1) % 2;
    string tracker_url = tracker_url_combined[tracker_is_on];
    cerr << tracker_url << endl;
    int port_no;
    client_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket_fd < 0)
    {
        log_file_descriptor.lock();
        cerr << "ERROR in creating socket" << endl;
        log_file_descriptor.unlock();
    }

    port_no = stoi(tracker_url.substr(tracker_url.find_last_of(':') + 1));
    string tracker_name = tracker_url.substr(0, tracker_url.find_last_of(':'));

    sockaddr_in server_address;
    struct hostent *server = gethostbyname(tracker_name.c_str());
    server_address.sin_family = AF_INET;      // host byte order
    server_address.sin_port = htons(port_no); // short, network byte order
    server_address.sin_addr = *((struct in_addr *)server->h_addr);
    memset(&(server_address.sin_zero), 0, 8); // zero the rest of the

    if (connect(client_socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        return false;
    }
    else
    {
        log_file_descriptor.lock();
        cerr << "connected to " << tracker_url << endl;
        log_file_descriptor.unlock();
    }
    return true;
}

void get_files_by_part(string tracker_url, string hash_string, vector<string> part_no, string file_path, lo filesize)
{
    lo last_part = (filesize) / BUFFER_SIZE;
    lo part_to_download = filesize % BUFFER_SIZE;
    log_file_descriptor.lock();
    cerr << "Trying to connect to " << tracker_url << endl;
    log_file_descriptor.unlock();
    int port_no;
    int client_so_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_so_fd < 0)
    {
        log_file_descriptor.lock();
        cerr << "ERROR in creating socket" << endl;
        log_file_descriptor.unlock();
    }

    port_no = stoi(tracker_url.substr(tracker_url.find_last_of(':') + 1));
    string tracker_name = tracker_url.substr(0, tracker_url.find_last_of(':'));

    sockaddr_in server_address;
    struct hostent *server = gethostbyname(tracker_name.c_str());
    server_address.sin_family = AF_INET;      // host byte order
    server_address.sin_port = htons(port_no); // short, network byte order
    server_address.sin_addr = *((struct in_addr *)server->h_addr);
    memset(&(server_address.sin_zero), 0, 8); // zero the rest of the

    if (connect(client_so_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        log_file_descriptor.lock();
        cerr << "ERROR in connecting to server" << endl;
        log_file_descriptor.unlock();
    }
    lo count = 0;
    std::FILE *fp = std::fopen(file_path.c_str(), "wb");
    REP(0, part_no.size())
    {
        string part = part_no[i];
        Message message({"SEND_FILE", hash_string, part});
        string encoded_message = message.encode_message();

        //derr(encoded_message);
        send(client_so_fd, encoded_message.c_str(), encoded_message.length(), 0);
        auto response = message.decode_message(client_so_fd);
        //derr2(stoll(part), response[1]);
        if (response[0] == "SUCCESS")
        {
            fseek(fp, stoll(part) * BUFFER_SIZE, SEEK_SET);
            lo length_to_write = response[1].length();
            if (stoll(part) == last_part)
                length_to_write = part_to_download;
            std::fwrite(response[1].data(), sizeof('a'), length_to_write, fp);
            details_of_file_mutex.lock();
            details_of_file[hash_string].part_of_file[stoll(part)] = 1;
            details_of_file_mutex.unlock();
        }
        else
        {
            log_file_descriptor.lock();
            cerr << "The destination does not contain the file specified" << endl;
            log_file_descriptor.unlock();
            i--;
        }
    }
    std::fclose(fp);
    close(client_so_fd);
}

void get_file_details(string tracker_url, string hash_string)
{
    //cerr<<endl<<endl<<"********************"<<endl<<endl;
    log_file_descriptor.lock();
    cerr << "Trying to connect to " << tracker_url << endl;
    log_file_descriptor.unlock();
    int port_no;
    int client_so_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_so_fd < 0)
    {
        log_file_descriptor.lock();
        cerr << "ERROR in creating socket" << endl;
        log_file_descriptor.unlock();
    }

    port_no = stoi(tracker_url.substr(tracker_url.find_last_of(':') + 1));
    string tracker_name = tracker_url.substr(0, tracker_url.find_last_of(':'));

    sockaddr_in server_address;
    struct hostent *server = gethostbyname(tracker_name.c_str());
    server_address.sin_family = AF_INET;      // host byte order
    server_address.sin_port = htons(port_no); // short, network byte order
    server_address.sin_addr = *((struct in_addr *)server->h_addr);
    memset(&(server_address.sin_zero), 0, 8); // zero the rest of the

    if (connect(client_so_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        log_file_descriptor.lock();
        cerr << "ERROR in connecting to server" << endl;
        log_file_descriptor.unlock();
    }

    Message message({"SEND_FILE_DETAILS", hash_string});
    string encoded_message = message.encode_message();
    //derr(encoded_message);
    send(client_so_fd, encoded_message.c_str(), encoded_message.length(), 0);
    auto response = message.decode_message(client_so_fd);
    //cerr<<endl<<endl<<"##############################";derr2(response.size(),response);
    close(client_so_fd);
    if (response[0] == "SUCCESS")
    {
        vector<string> temp;
        REP(1, response.size())
        {
            temp.pb(response[i]);
        }
        log_file_descriptor.lock();
        cerr << "The destination contain these parts of the file specified" << endl;
        list_of_part_contained_mutex.lock();
        list_of_part_contained[tracker_url] = temp;
        list_of_part_contained_mutex.unlock();
        log_file_descriptor.unlock();
    }
    else
    {
        log_file_descriptor.lock();
        cerr << "The destination does not contain the file specified" << endl;
        log_file_descriptor.unlock();
    }
    return;
}

void do_join(std ::thread &t)
{
    t.join();
}

void schedule()
{
    map<string, vector<string>> scheduled_map;
    set<string> processed;
    while (!list_of_part_contained.empty())
    {
        vector<string> to_remove;
        TRV(list_of_part_contained)
        {
            if (present(processed, it.Y.back()))
            {
                while (!it.Y.empty() and present(processed, it.Y.back()))
                {
                    it.Y.pop_back();
                }
            }
            if (!it.Y.empty())
            {
                processed.insert(it.Y.back());
                if (present(scheduled_map, it.X))
                {
                    scheduled_map[it.X].push_back(it.Y.back());
                    it.Y.pop_back();
                }
                else
                {
                    vector<string> temp;
                    temp.push_back(it.Y.back());
                    scheduled_map[it.X] = temp;
                    it.Y.pop_back();
                }
            }
            if (it.Y.empty())
            {
                to_remove.push_back(it.X);
            }
        }
        TRV(to_remove)
        {
            list_of_part_contained.erase(it);
        }
    }
    swap(list_of_part_contained, scheduled_map);
    return;
}

void manage_download_file(vector<string> list_of_clients, int filesize)
{
    std ::vector<std::thread> all_threads;
    REP(1, list_of_clients.size() - 1)
    {
        //cerr << endl;
        //derr(list_of_clients[i]);
        all_threads.push_back(std ::thread(get_file_details, list_of_clients[i], list_of_clients.front()));
    }
    std ::for_each(all_threads.begin(), all_threads.end(), do_join);
    schedule();
    all_threads.clear();
    TRV(list_of_part_contained)
    {
        debug(it);
    }
    REP(1, list_of_clients.size() - 1)
    {
        list_of_part_contained_mutex.lock();
        all_threads.push_back(std ::thread(get_files_by_part, list_of_clients[i], list_of_clients.front(), list_of_part_contained[list_of_clients[i]], list_of_clients.back(), filesize));
        list_of_part_contained_mutex.unlock();
    }
    std ::for_each(all_threads.begin(), all_threads.end(), do_join);

    downloaded_files_mutex.lock();
    currently_downloading_files_mutex.lock();
    currently_downloading_files.erase(list_of_clients.back());
    downloaded_files.insert(list_of_clients.back());
    downloaded_files_mutex.unlock();
    currently_downloading_files_mutex.unlock();
    return;
}

void share(string client_ip, string filename, string mtorrent_name)
{
    if (!(check_wether_a_file_or_not(filename) and check_wether_a_file_or_not(mtorrent_name)))
    {
        log_file_descriptor.lock();
        cout << "File Doesnot Exists" << endl;
        cerr << "File Doesnot Exists" << endl;
        log_file_descriptor.unlock();
        return;
    }
    auto generated_torrent = generate_torrent(tracker_url_combined[0], tracker_url_combined[1], filename, mtorrent_name);
    string SHA_hash = generated_torrent.location;
    generated_torrent.location = filename;
    details_of_file_mutex.lock();
    details_of_file[SHA_hash] = generated_torrent;
    details_of_file_mutex.unlock();
    Message message({"SHARE", filename, SHA_hash, client_ip});
    string encoded_message = message.encode_message();
    //debug(encoded_message);
    send(client_socket_fd, encoded_message.c_str(), encoded_message.length(), 0);
    auto response = message.decode_message(client_socket_fd);
    if (response.empty())
    {
        log_file_descriptor.lock();
        cerr << "Server is not responding" << endl;
        log_file_descriptor.unlock();
        return;
    }
    log_file_descriptor.lock();
    cerr << response;
    cout << "Successfull" << endl;
    log_file_descriptor.unlock();
    return;
}

void share_without_creating_file(string client_ip, string filename, string mtorrent_name, mtorrent &entry_to_send)
{
    int x;
    close(client_socket_fd);
    cerr << "Inside Share function" << endl;
    if (!change_tracker())
    {
        if (!change_tracker())
        {
            log_file_descriptor.lock();
            cerr << "ERROR in connecting to both server" << endl;
            log_file_descriptor.unlock();
            close(client_socket_fd);
            //continue;
        }
    }
    torrent_for_map entry;
    entry.location = filename;
    entry.part_of_file.resize(((entry_to_send.filesize + BUFFER_SIZE - 1) / BUFFER_SIZE), 0);
    details_of_file_mutex.lock();
    details_of_file[entry_to_send.SHA_hash] = entry;
    details_of_file_mutex.unlock();
    Message message({"SHARE", filename, entry_to_send.SHA_hash, client_ip});
    string encoded_message = message.encode_message();
    //debug(encoded_message);
    send(client_socket_fd, encoded_message.c_str(), encoded_message.length(), 0);
    cerr << "I have sent";
    auto response = message.decode_message(client_socket_fd);
    if (response.empty())
    {
        log_file_descriptor.lock();
        cerr << "Server is not responding" << endl;
        log_file_descriptor.unlock();
        return;
    }
    log_file_descriptor.lock();
    cerr << response;
    cout << "the tracker is informed" << endl;
    log_file_descriptor.unlock();
    return;
}

void remove(string client_ip, string mtorrent_name)
{
    if (!check_wether_a_file_or_not(mtorrent_name))
    {
        log_file_descriptor.lock();
        cout << "File Doesnot Exists" << endl;
        cerr << "File Doesnot Exists" << endl;
        log_file_descriptor.unlock();
        return;
    }
    mtorrent entry_to_delete;
    ifstream fin;
    fin.open(mtorrent_name, ios::in);
    fin >> entry_to_delete;
    unlink(mtorrent_name.c_str());
    details_of_file_mutex.lock();
    details_of_file.erase(entry_to_delete.SHA_hash);
    details_of_file_mutex.unlock();
    Message message({"REMOVE", entry_to_delete.SHA_hash, client_ip});
    string encoded_message = message.encode_message();
    send(client_socket_fd, encoded_message.c_str(), encoded_message.length(), 0);
    auto response = message.decode_message(client_socket_fd);
    if (response.empty())
    {
        log_file_descriptor.lock();
        cerr << "Server is not responding" << endl;
        log_file_descriptor.unlock();
        return;
    }
    log_file_descriptor.lock();
    cerr << response;
    cout << "File Successfully Removed" << endl;
    log_file_descriptor.unlock();
    return;
}

void get_file(string path_to_mtorrent_file, string destination_path, string client_ip)
{
    if (!(check_wether_a_file_or_not(path_to_mtorrent_file) and check_wether_a_file_or_not(destination_path)))
    {
        log_file_descriptor.lock();
        cout << "File Doesnot Exists" << endl;
        cerr << "File Doesnot Exists" << endl;
        log_file_descriptor.unlock();
        return;
    }
    mtorrent entry_to_download;
    ifstream fin;
    fin.open(path_to_mtorrent_file, ios::in);
    fin >> entry_to_download;
    Message message({"SEEDERLIST", entry_to_download.SHA_hash});
    string encoded_message = message.encode_message();
    send(client_socket_fd, encoded_message.c_str(), encoded_message.length(), 0);
    auto response = message.decode_message(client_socket_fd);
    if (response.empty())
    {
        log_file_descriptor.lock();
        cerr << "Server is not responding" << endl;
        log_file_descriptor.unlock();
        return;
    }
    else if (response[0] == "ERROR")
    {
        log_file_descriptor.lock();
        cerr << response << endl;
        log_file_descriptor.unlock();
        return;
    }
    else if (response[0] == "CLOSE")
    {
        log_file_descriptor.lock();
        cerr << "Connection Successfully terminated" << endl;
        log_file_descriptor.unlock();
        return;
    }

    response[0] = entry_to_download.SHA_hash;
    response.push_back(destination_path);

    currently_downloading_files_mutex.lock();
    currently_downloading_files.insert(destination_path);
    currently_downloading_files_mutex.unlock();

    // std::ofstream file(destination_path);
    // file.seekp(entry_to_download.filesize);
    // file << '\0';
    // file.close();

    std ::thread T(manage_download_file, response, entry_to_download.filesize);
    T.detach();
    share_without_creating_file(client_ip, destination_path, path_to_mtorrent_file, entry_to_download);
    log_file_descriptor.lock();
    cout << "File is Downloading" << endl;
    log_file_descriptor.unlock();
    return;
}

void handle_incoming_request(int new_socket_fd, sockaddr_in *client_addrress)
{
    Message message;
    string encoded_message;
    while (true)
    {
        auto request = message.decode_message(new_socket_fd);
        cout << "request decoded" << endl;
        if (request.empty())
        {
            log_file_descriptor.lock();
            cerr << "Didnot recieved the message properly from client : " << inet_ntoa(client_addrress->sin_addr) << ":" << ntohs(client_addrress->sin_port) << endl;
            log_file_descriptor.unlock();
            message.reload({"ERROR", "didn't recieve the message properly"});
            encoded_message = message.encode_message();
            return;
        }
        else if (request[0] == "CLOSE")
        {
            log_file_descriptor.lock();
            cerr << "Connection gracefully closed by client : " << inet_ntoa(client_addrress->sin_addr) << ":" << ntohs(client_addrress->sin_port) << endl;
            log_file_descriptor.unlock();
            close(new_socket_fd);
            //message.reload({"ERROR","didn't recieve the message properly"});
            return;
        }
        else if (request[0] == "SEND_FILE_DETAILS")
        {
            log_file_descriptor.lock();
            details_of_file_mutex.lock();
            cerr << "SENDING details of file asked by : " << inet_ntoa(client_addrress->sin_addr) << ":" << ntohs(client_addrress->sin_port) << endl;
            encoded_message = send_details(details_of_file, request[1]);
            details_of_file_mutex.unlock();
            log_file_descriptor.unlock();
            send(new_socket_fd, encoded_message.c_str(), encoded_message.length(), 0);
            close(new_socket_fd);
            return;
        }
        else if (request[0] == "SEND_FILE")
        {
            log_file_descriptor.lock();
            details_of_file_mutex.lock();
            cerr << "SENDING file asked by : " << inet_ntoa(client_addrress->sin_addr) << ":" << ntohs(client_addrress->sin_port) << endl;
            encoded_message = send_file(details_of_file, request);
            details_of_file_mutex.unlock();
            log_file_descriptor.unlock();
        }
        //derr(encoded_message);
        send(new_socket_fd, encoded_message.c_str(), encoded_message.length(), 0);
    }
}

void create_server(string server_ip)
{
    int socket_fd;
    int port_no;
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0)
    {
        log_file_descriptor.lock();
        cerr << "ERROR in creating socket" << endl;
        log_file_descriptor.unlock();
    }

    int reuse_port = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT, &reuse_port, sizeof(reuse_port));

    sockaddr_in server_address; //Server Address
    bzero((char *)&server_address, sizeof(server_address));
    port_no = stoi(server_ip.substr(server_ip.find_last_of(':') + 1)); //set port no.

    /////Set the server details

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port_no);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (bind(socket_fd, (sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        log_file_descriptor.lock();
        cerr << "Could not bind the server to the given port" << endl;
        log_file_descriptor.unlock();
    }

    unsigned int back_log_size = 5;
    listen(socket_fd, back_log_size); //Server Listening
    log_file_descriptor.lock();
    cerr << "Server opened on port: " << port_no << endl;
    log_file_descriptor.unlock();

    while (true)
    {
        int new_socket_fd;
        unsigned int client_length;
        sockaddr_in client_address;
        client_length = sizeof(sockaddr_in);

        //Blocak until a client connects
        new_socket_fd = accept(socket_fd, (sockaddr *)&client_address, &client_length);

        if (new_socket_fd < 0)
        {
            log_file_descriptor.lock();
            cerr << "ERROR on accept: " << endl;
            log_file_descriptor.unlock();
        }
        else
        {
            log_file_descriptor.lock();
            cerr << "Connected to " << inet_ntoa(client_address.sin_addr) << ":" << ntohs(client_address.sin_port) << endl;
            log_file_descriptor.unlock();
        }

        std ::thread T(handle_incoming_request, new_socket_fd, &client_address);
        T.detach();
    }

    close(socket_fd);
}
void atexit_handler_1()
{
    cerr << "I reached here too" << endl;
    if (!change_tracker())
    {
        if (!change_tracker())
        {
            log_file_descriptor.lock();
            cerr << "ERROR in connecting to both server" << endl;
            log_file_descriptor.unlock();
            close(client_socket_fd);
            //continue;
        }
    }
    Message message({"CLOSEAPPLICATION", client_ip_2});
    string encoded_message = message.encode_message();
    send(client_socket_fd, encoded_message.c_str(), encoded_message.length(), 0);
    return;
}
int main(int argc, char *argv[])
{
    const int result_1 = std::atexit(atexit_handler_1);
    //take the command line arguments
    string client_ip = string(argv[1]);
    client_ip_2 = client_ip;
    tracker_url_combined[0] = string(argv[2]);
    tracker_url_combined[1] = string(argv[3]);
    string log_file = string(argv[4]);
    tracker_is_on = true;
    freopen(log_file.c_str(), "w", stderr);
    ///////Create a Server
    std ::thread T(create_server, client_ip);
    T.detach();

    ////////

    while (true)
    {
        cout << "Welcome" << endl;
        cout << "Enter share , get, remove, show downloads, close" << endl;
        string input;
        getline(cin, input);
        if (!change_tracker())
        {
            if (!change_tracker())
            {
                log_file_descriptor.lock();
                cerr << "ERROR in connecting to both server" << endl;
                log_file_descriptor.unlock();
                close(client_socket_fd);
                //continue;
            }
        }
        vector<string> input_commands = split_string(input);
        cerr << input_commands;
        if (input_commands.size() < 1)
        {
            log_file_descriptor.lock();
            cout << "You have entered less number of arguments " << endl;
            log_file_descriptor.unlock();
        }
        else if (input_commands[0] == "close")
        {
            Message message({"CLOSEAPPLICATION", client_ip_2});
            string encoded_message = message.encode_message();
            send(client_socket_fd, encoded_message.c_str(), encoded_message.length(), 0);
            close(client_socket_fd);
            break;
        }
        else if (input_commands.size() < 2)
        {
            log_file_descriptor.lock();
            cout << "You have entered less number of arguments " << endl;
            log_file_descriptor.unlock();
        }
        else if (input_commands[0] == "share")
        {
            share(client_ip, input_commands[1], input_commands[2]);
        }
        else if (input_commands[0] == "remove")
        {
            remove(client_ip, input_commands[1]);
        }
        else if (input_commands[0] == "get")
        {
            if (input_commands.size() < 3)
            {
                log_file_descriptor.lock();
                cout << "You have entered less number of arguments " << endl;
                log_file_descriptor.unlock();
            }
            else
            {
                get_file(input_commands[1], input_commands[2], client_ip);
            }
        }
        else if (input_commands[0] == "show" and input_commands[1] == "downloads")
        {
            currently_downloading_files_mutex.lock();
            cout << "List of Downloading Files" << endl;
            TRV(currently_downloading_files)
            {
                cout << "[D] " << it << endl;
            }
            currently_downloading_files_mutex.unlock();
            downloaded_files_mutex.lock();
            cout << "List of Downloaded Files" << endl;
            TRV(downloaded_files)
            {
                cout << "[S] " << it << endl;
            }
            downloaded_files_mutex.unlock();
        }
        else
        {
            log_file_descriptor.lock();
            cout << "You have entered wrong command " << endl;
            log_file_descriptor.unlock();
        }
        close(client_socket_fd);
    }
    return 0;
}