/*
    Name : Anurag Maithani
    Name : Dheeraj K. Pant
*/

#include <message_server.h>

Message ::Message(initializer_list<string> list)
{
    this->no_of_fields = 0;
    this->fields_content.clear();
    this->fields_size.clear();
    for (auto &it : list)
    {
        this->fields_content.push_back(it);
        this->fields_size.pb(it.length());
        this->no_of_fields++;
    }
}

Message ::Message(vector<string> list)
{
    this->no_of_fields = 0;
    this->fields_content.clear();
    this->fields_size.clear();
    for (auto &it : list)
    {
        this->fields_content.push_back(it);
        this->fields_size.pb(it.length());
        this->no_of_fields++;
    }
}

void Message ::reload(initializer_list<string> list)
{
    this->no_of_fields = 0;
    this->fields_content.clear();
    this->fields_size.clear();
    for (auto &it : list)
    {
        this->fields_content.push_back(it);
        this->fields_size.pb(it.length());
        this->no_of_fields++;
    }
}
void Message ::reload(vector<string> list)
{
    this->no_of_fields = 0;
    this->fields_content.clear();
    this->fields_size.clear();
    for (auto &it : list)
    {
        this->fields_content.push_back(it);
        this->fields_size.pb(it.length());
        this->no_of_fields++;
    }
}

Message ::Message()
{
    this->no_of_fields = 0;
    this->fields_content.clear();
    this->fields_size.clear();
}

string Message ::encode_message()
{
    string ans = "";
    string length_of_feild = to_string(this->no_of_fields);
    length_of_feild = string(sizeof(lo) - length_of_feild.length(), '0').append(length_of_feild);
    ans += length_of_feild;
    REP(0, this->no_of_fields)
    {
        length_of_feild = to_string(this->fields_size[i]);
        length_of_feild = string(sizeof(lo) - length_of_feild.length(), '0').append(length_of_feild);
        ans += length_of_feild;
        ans += this->fields_content[i];
    }
    return ans;
}

void Message ::clear()
{
    this->no_of_fields = 0;
    this->fields_size.clear();
    this->fields_content.clear();
}

vector<string> Message ::decode_message(int file)
{
    this->clear();
    char long_buffer[sizeof(lo)];
    memset(long_buffer, 0, sizeof(long_buffer));
    auto result = read(file, long_buffer, sizeof(lo));
    //derr(long_buffer);
    if (result < 0)
    {
        this->clear();
        return this->fields_content;
    }
    else if (result == 0)
    {
        this->clear();
        this->fields_content.pb("CLOSE");
        return this->fields_content;
    }
    this->no_of_fields = stoll(string(long_buffer));
    cout << "I have recieved the message" << endl;
    REP(0, this->no_of_fields)
    {
        memset(long_buffer, 0, sizeof(long_buffer));
        result = read(file, long_buffer, sizeof(lo));
        if (result < 0)
        {
            this->clear();
            return this->fields_content;
        }
        //derr(long_buffer);
        this->fields_size.pb(stoll(string(long_buffer)));
        //derr(this->fields_size.back());
        char field_buffer[this->fields_size.back()];
        memset(field_buffer, 0, sizeof(field_buffer));
        result = read(file, field_buffer, this->fields_size.back());
        if (result < 0)
        {
            this->clear();
            return this->fields_content;
        }
        string temp_field_buffer = string(field_buffer);
        //derr2(temp_field_buffer,temp_field_buffer.length());
        //derr(temp_field_buffer.substr(0,this->fields_size.back()));
        this->fields_content.pb(temp_field_buffer.substr(0, this->fields_size.back()));
    }
    return this->fields_content;
}