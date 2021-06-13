//
//  Copyright (C) 2011
//  Bill Xia
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
//  of the Software, and to permit persons to whom the Software is furnished to do so,
//  subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all copies
//  or substantial portions of the Software.
//

//
//  This is the use management module, includes user login, authentication and quit.
//
//

#include "module_user.hpp"
#include "module_mail.hpp"

using namespace std;

// check username and passwd from file
bool check_name_pass(const char *const name, const char *const pass);

bool check_user(const char *const from_user)
{
    const auto strlen_from_user = strlen(from_user);
    ifstream file(userinfo_file);
    if (!file.is_open())
    {
        perror("File opening failed");
        exit(EXIT_FAILURE);
    }
    for (string line; getline(file, line);)
        if (strncmp(from_user, line.c_str(), strlen_from_user) == 0) // valid user
            return true;
    return false;
}

void auth(const int sockfd, int &mail_stat)
{
    send_data(sockfd, reply_code[25]); // require username
    sleep(3);
    vector<char> name;
    {
        array<char, 50> encrypted_name;
        if (recv(sockfd, encrypted_name.data(), sizeof(encrypted_name), 0) <= 0)
            send_data(sockfd, reply_code[16]);

        cout << "Request stream: " << encrypted_name.data() << "\n";
        name = base64_decode(encrypted_name.data());
        cout << "Decoded username: " << name.data() << "\n";
        send_data(sockfd, reply_code[26]); // require passwd
        sleep(3);
    }
    vector<char> passwd;
    {
        array<char, 50> encrypted_passwd;
        if (recv(sockfd, encrypted_passwd.data(), sizeof(encrypted_passwd), 0) <= 0)
            send_data(sockfd, reply_code[16]);

        cout << "Request stream: " << encrypted_passwd.data() << "\n";
        passwd = base64_decode(encrypted_passwd.data());
        cout << "Decoded password: " << passwd.data() << "\n";
    }
    if (check_name_pass(name.data(), passwd.data())) // check username and passwd
    {
        mail_stat = 13; // changing mail status
        send_data(sockfd, reply_code[27]);
    }
    else
        send_data(sockfd, reply_code[16]);
}

void user_quit(const char *const from_user)
{
    const auto strlen_from_user = strlen(from_user);
    fstream file(userstat_file, ios::in | ios::out | ios::trunc); // same as fopen(userstat_file, "w+")
    if (!file.is_open())
    {
        perror("File opening failed");
        exit(EXIT_FAILURE);
    }
    for (string line; getline(file, line);)
    {
        if (strncmp(line.c_str(), from_user, strlen_from_user) == 0)
        {
            long len = strlen(line.c_str());
            if (getline(file, line))
            {
                len = -len;

                file.seekp(len, ios::cur);
                // fseek(fp, len, SEEK_CUR);

                file << line; // file.write(line.data(), line.size());
                // fputs(data, fp);

                // len = strlen(data);
                file.seekg(line.size(), ios::cur);
                // fseek(fp, len, SEEK_CUR);
            }
        }
    }
}

bool check_name_pass(const char *const name, const char *const pass)
{
    fstream file;
    file.open(userinfo_file, ios::in); // fopen(userinfo_file, "r")
    if (!file.is_open())
    {
        perror("File opening failed");
        exit(EXIT_FAILURE);
    }
    for (string line; getline(file, line);)
    {
        if (strncmp(line.c_str(), name, strlen(name)) != 0) // find username
            continue;

        if (strncmp(strchr(line.c_str(), ' ') + 1, pass, strlen(pass)) != 0)
            return false; // invalid passwd

        // valid passwd
        file.close();
        file.open(userstat_file, ios::in | ios::out | ios::trunc); // same as fopen(userstat_file, "w+")
        if (!file.is_open())
        {
            perror("File opening failed");
            exit(EXIT_FAILURE);
        }
        const string new_status = name + " on"s; // change the status of the user to ON
        file << new_status;
        // fwrite(new_status.c_str(), 1, strlen(new_status.c_str()), fp);
        return true; // success
    }
    return false; // invalid username
}
