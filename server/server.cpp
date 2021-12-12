/*
    Name : Anurag Maithani
    Name : Dheeraj  K. Pant
*/
#include <message_server.h>
#include <respond_to_request_server.h>
#include <thread>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sstream>
#include <netdb.h>
#include <mutex>

map<string, set<string>> hash_vs_seeder_ip_port;
mutex hash_vs_seeder_ip_port_mutex;
mutex log_file_descriptor;
string my_tracker_url, other_tracker_url;
string seeder_file;
mutex seeder_file_mutex;

void inform_other_tracker(string message)
{
    log_file_descriptor.lock();
    cerr << "I am trying to contact other tracker" << endl;
    log_file_descriptor.unlock();
    string tracker_url = other_tracker_url;
    int port_no;
    auto client_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    //cerr<<"5";
    if (client_socket_fd < 0)
    {
        log_file_descriptor.lock();
        cerr << "ERROR in creating socket" << endl;
        log_file_descriptor.unlock();
    }
    //cerr<<"2";
    port_no = stoi(tracker_url.substr(tracker_url.find_last_of(':') + 1));
    string tracker_name = tracker_url.substr(0, tracker_url.find_last_of(':'));
    //cerr<<"3";
    sockaddr_in server_address;
    struct hostent *server = gethostbyname(tracker_name.c_str());
    server_address.sin_family = AF_INET;      // host byte order
    server_address.sin_port = htons(port_no); // short, network byte order
    server_address.sin_addr = *((struct in_addr *)server->h_addr);
    memset(&(server_address.sin_zero), 0, 8); // zero the rest of the
    //cerr<<"4";
    if (connect(client_socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        log_file_descriptor.lock();
        cerr << "I have sent message to the other tracker" << endl;
        log_file_descriptor.unlock();
        return;
    }
    log_file_descriptor.lock();
    cerr << "I will be sending to other tracker" << endl;
    cerr << message << endl;
    log_file_descriptor.unlock();
    if (send(client_socket_fd, message.c_str(), message.length(), 0) < 0)
    {
        log_file_descriptor.lock();
        cerr << "I was not able to send message" << endl;
        log_file_descriptor.unlock();
    }
    close(client_socket_fd);
}

bool load_from_other_tracker()
{
    Message message({"DUMP"});
    string encoded_message = message.encode_message();
    string tracker_url = other_tracker_url;
    int port_no;
    auto client_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket_fd < 0)
    {
        log_file_descriptor.lock();
        cerr << "ERROR in creating socket" << endl;
        log_file_descriptor.unlock();
        return false;
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
    send(client_socket_fd, encoded_message.c_str(), encoded_message.length(), 0);
    auto response = message.decode_message(client_socket_fd);

    hash_vs_seeder_ip_port_mutex.lock();
    load_from_message(hash_vs_seeder_ip_port, response);
    seeder_file_mutex.lock();
    update_database(hash_vs_seeder_ip_port, seeder_file);
    seeder_file_mutex.unlock();
    hash_vs_seeder_ip_port_mutex.unlock();
    close(client_socket_fd);
    log_file_descriptor.lock();
    cerr << "EVerything was successfull" << endl;
    log_file_descriptor.unlock();
    return true;
}

void handle_connection(int new_socket_fd, sockaddr_in *client_addrress)
{
    Message message;
    string encoded_message;
    while (true)
    {
        auto request = message.decode_message(new_socket_fd);
        log_file_descriptor.lock();
        cerr << "request decoded" << endl;
        cerr << request << endl;
        log_file_descriptor.unlock();
        if (request.empty())
        {
            log_file_descriptor.lock();
            cerr << "Didnot recieved the message properly from client : " << inet_ntoa(client_addrress->sin_addr) << ":" << ntohs(client_addrress->sin_port) << endl;
            log_file_descriptor.unlock();
            message.reload({"ERROR", "didn't recieve the message properly"});
            encoded_message = message.encode_message();
            close(new_socket_fd);
            return;
        }
        if (request[0] == "CLOSE")
        {
            log_file_descriptor.lock();
            cerr << "Connection gracefully closed by client : " << inet_ntoa(client_addrress->sin_addr) << ":" << ntohs(client_addrress->sin_port) << endl;
            log_file_descriptor.unlock();
            close(new_socket_fd);
            //message.reload({"ERROR","didn't recieve the message properly"});
            return;
        }
        if (request[0] == "SHARE")
        {
            log_file_descriptor.lock();
            cerr << "SHAREING on request of " << inet_ntoa(client_addrress->sin_addr) << ":" << ntohs(client_addrress->sin_port) << endl;
            request.erase(request.begin());
            hash_vs_seeder_ip_port_mutex.lock();

            auto response = add_seeder(hash_vs_seeder_ip_port, request);
            encoded_message = response.first;
            string message_for_other_tracker = response.second;
            update_database(hash_vs_seeder_ip_port, seeder_file);
            std ::thread T(inform_other_tracker, message_for_other_tracker);
            T.detach();

            hash_vs_seeder_ip_port_mutex.unlock();
            log_file_descriptor.unlock();
        }
        else if (request[0] == "REMOVE")
        {
            log_file_descriptor.lock();
            cerr << "REMOVING on request of " << inet_ntoa(client_addrress->sin_addr) << ":" << ntohs(client_addrress->sin_port) << endl;
            request.erase(request.begin());
            hash_vs_seeder_ip_port_mutex.lock();

            auto response = remove_seeder(hash_vs_seeder_ip_port, request);
            encoded_message = response.first;
            string message_for_other_tracker = response.second;
            update_database(hash_vs_seeder_ip_port, seeder_file);
            std ::thread T(inform_other_tracker, message_for_other_tracker);
            T.detach();

            hash_vs_seeder_ip_port_mutex.unlock();
            log_file_descriptor.unlock();
        }
        else if (request[0] == "SEEDERLIST")
        {
            log_file_descriptor.lock();
            cerr << "PROVIDING SEEDERLIST on request of " << inet_ntoa(client_addrress->sin_addr) << ":" << ntohs(client_addrress->sin_port) << endl;
            request.erase(request.begin());
            hash_vs_seeder_ip_port_mutex.lock();

            encoded_message = provide_seeder_list(hash_vs_seeder_ip_port, request);

            hash_vs_seeder_ip_port_mutex.unlock();
            log_file_descriptor.unlock();
        }
        else if (request[0] == "SHAREREMOVE")
        {
            log_file_descriptor.lock();
            cerr << "REMOVING on request of " << inet_ntoa(client_addrress->sin_addr) << ":" << ntohs(client_addrress->sin_port) << endl;
            request.erase(request.begin());
            hash_vs_seeder_ip_port_mutex.lock();

            remove_share_seeder(hash_vs_seeder_ip_port, request);
            update_database(hash_vs_seeder_ip_port, seeder_file);

            hash_vs_seeder_ip_port_mutex.unlock();
            log_file_descriptor.unlock();

            close(new_socket_fd);
            return;
        }
        else if (request[0] == "SHAREADD")
        {
            log_file_descriptor.lock();
            cerr << "SHAREING on request of " << inet_ntoa(client_addrress->sin_addr) << ":" << ntohs(client_addrress->sin_port) << endl;
            request.erase(request.begin());
            hash_vs_seeder_ip_port_mutex.lock();

            add_share_seeder(hash_vs_seeder_ip_port, request);
            update_database(hash_vs_seeder_ip_port, seeder_file);

            hash_vs_seeder_ip_port_mutex.unlock();
            log_file_descriptor.unlock();
            close(new_socket_fd);
            return;
        }
        else if (request[0] == "DUMP")
        {
            log_file_descriptor.lock();
            cerr << "SHAREING full content " << inet_ntoa(client_addrress->sin_addr) << ":" << ntohs(client_addrress->sin_port) << endl;
            log_file_descriptor.unlock();
            hash_vs_seeder_ip_port_mutex.lock();
            encoded_message = dump(hash_vs_seeder_ip_port);
            hash_vs_seeder_ip_port_mutex.unlock();
        }
        else if (request[0] == "CLOSEAPPLICATION")
        {
            vector<pair<string, string>> to_remove;
            hash_vs_seeder_ip_port_mutex.lock();
            TRV(hash_vs_seeder_ip_port)
            {
                if (it.Y.find(request[1]) != it.Y.end())
                {
                    it.Y.erase(request[1]);
                    to_remove.push_back(mp(it.X, request[1]));
                }
            }
            hash_vs_seeder_ip_port_mutex.unlock();
            TRV(to_remove)
            {
                Message message({"SHAREREMOVE", it.X, it.Y});
                inform_other_tracker(message.encode_message());
            }
        }
        else
        {
            log_file_descriptor.lock();
            cerr << "I was herer" << endl;
            cerr << request[0] << endl;
            log_file_descriptor.unlock();
            close(new_socket_fd);
            return;
        }
        //debug(encoded_message);

        hash_vs_seeder_ip_port_mutex.lock();
        /*TRV(hash_vs_seeder_ip_port)
        {
            cerr << it.X << ":::::";
            for (auto &it2 : it.Y)
            {
                cerr << it2 << "    &    ";
            }
            cerr << endl;
        }*/
        hash_vs_seeder_ip_port_mutex.unlock();
        // log_file_descriptor.lock();
        // debug(encoded_message);
        // log_file_descriptor.unlock();
        log_file_descriptor.lock();
        derr(encoded_message);
        log_file_descriptor.unlock();
        send(new_socket_fd, encoded_message.c_str(), encoded_message.length(), 0);
        close(new_socket_fd);
        return;
    }
}

int main(int argc, char *argv[])
{

    cout<<" welcome to torrentz"
    my_tracker_url = string(argv[1]);
    other_tracker_url = string(argv[2]);
    seeder_file = string(argv[3]);
    string log_file = string(argv[4]);
    //////

    freopen(log_file.c_str(), "w", stderr);

    if (!load_from_other_tracker())
    {
        log_file_descriptor.lock();
        cerr << "loading from the seeder file" << endl
             << endl;
        load_from_seeder_file(hash_vs_seeder_ip_port, seeder_file);
        log_file_descriptor.unlock();
    }

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
    port_no = stoi(my_tracker_url.substr(my_tracker_url.find_last_of(':') + 1)); //set port no.

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

    unsigned int back_log_size = 25;
    listen(socket_fd, back_log_size); //Server Listening
    log_file_descriptor.lock();
    cerr << "Server opened on port: " << port_no << endl;
    log_file_descriptor.unlock();

    //cerr << hash_vs_seeder_ip_port.size() << endl;

    while (true)
    {
        log_file_descriptor.lock();
        cerr << "waiting for client" << endl;
        log_file_descriptor.unlock();
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
            cout << "Connected to " << inet_ntoa(client_address.sin_addr) << ":" << ntohs(client_address.sin_port) << endl;
            log_file_descriptor.unlock();
        }

        std ::thread T(handle_connection, new_socket_fd, &client_address);
        T.detach();
        log_file_descriptor.lock();
        cerr << "waiting for client" << endl;
        log_file_descriptor.unlock();
    }

    close(socket_fd);

    return 0;
}
