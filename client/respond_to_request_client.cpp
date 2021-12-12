/*
    Name : Anurag Maithani
    Name : Dheeraj K. Pant
*/

#include <respond_to_request_client.h>

string send_details(map<string, torrent_for_map> &details_of_file, string &file_name)
{
    vector<string> result;
    Message message;
    debug(details_of_file[file_name].part_of_file);
    if (!present(details_of_file, file_name))
    {
        cerr << "The requested file is not present" << endl;
        message.reload({"ERROR", "I don't have this file"});
    }
    else
    {
        result.push_back("SUCCESS");
        REP(0, details_of_file[file_name].part_of_file.size())
        {
            if (details_of_file[file_name].part_of_file[i] == 1)
            {
                string temp = to_string(i);
                temp = string(sizeof(lo) - temp.length(), '0').append(temp);
                result.push_back(temp);
            }
        }
        message.reload(result);
        cerr << "Successfully given details of file" << endl;
    }
    return message.encode_message();
}

std::ifstream::pos_type FILEsize(const char *filename)
{
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return in.tellg();
}

string send_file(map<string, torrent_for_map> &details_of_file, vector<string> &request)
{
    vector<string> result;
    string file_name = request[1];
    lo part_number = stoll(request[2]);
    Message message;
    if (!present(details_of_file, file_name))
    {
        cerr << "The requested file is not present" << endl;
        message.reload({"ERROR", "I don't have this file"});
        return message.encode_message();
    }
    string location = details_of_file[file_name].location;
    FILE *fp = fopen(location.c_str(), "rb");
    fseek(fp, part_number * BUFFER_SIZE, SEEK_SET);
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    lo bytes_read = fread(buffer, 1, BUFFER_SIZE, fp);
    if (bytes_read < 0)
    {
        cerr << "The requested file is not present" << endl;
        message.reload({"ERROR", "I don't have this file"});
        return message.encode_message();
    }
    string buffer_string = string(buffer);
    lo file_size = FILEsize(file_name.c_str());
    lo part = file_size / BUFFER_SIZE;
    // if(part == part_number){
    //     if(file_size%BUFFER_SIZE){
    //         length = file_size%BUFFER_SIZE;
    //     }
    // }
    derr3(part_number, buffer_string, buffer_string.substr(0, bytes_read));
    message.reload({"SUCCESS", buffer_string.substr(0, bytes_read)});
    return message.encode_message();
}
//.././///fdsfjadsifadshfila