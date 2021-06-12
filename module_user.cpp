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

#include "module_user.h"
#include "module_mail.h"

using namespace std;

// check username and passwd from file
bool check_name_pass(const char *const name, const char *const pass);

bool check_user(const char *const from_user)
{
    const auto file_name = string(data_dir) + userinfo;
    const auto strlen_from_user = strlen(from_user);
    FILE *fp = fopen(file_name.c_str(), "r"); // todo: change FILE* to std::ifstream
    char data[60];
    if (!fp)
    {
        perror("File opening failed");
        exit(EXIT_FAILURE);
    }
    while (fgets(data, sizeof(data), fp))
    {
        if (strncmp(from_user, data, strlen_from_user) == 0) // valid user
        {
            fclose(fp);
            return true;
        }
    }
    fclose(fp);
    return false;
}

void auth(const int sockfd, int &mail_stat)
{
    char ename[50], epass[50];
    std::vector<char> name, pass;

    send_data(sockfd, reply_code[25]); // require username
    sleep(3);

    if (recv(sockfd, ename, sizeof(ename), 0) <= 0)
        send_data(sockfd, reply_code[16]);

    cout << "Request stream: " << ename << endl;
    name = base64_decode(ename);
    cout << "Decoded username: " << name.data() << endl;
    send_data(sockfd, reply_code[26]); // require passwd
    sleep(3);

    if (recv(sockfd, epass, sizeof(epass), 0) <= 0)
        send_data(sockfd, reply_code[16]);

    cout << "Request stream: " << epass << endl;
    pass = base64_decode(epass);
    cout << "Decoded password: " << pass.data() << endl;

    if (check_name_pass(name.data(), pass.data())) // check username and passwd
    {
        mail_stat = 13; // changing mail status
        send_data(sockfd, reply_code[27]);
    }
    else
        send_data(sockfd, reply_code[16]);
}

void user_quit(const char *const from_user)
{
    const auto file_name = string(data_dir) + userstat;
    const auto strlen_from_user = strlen(from_user);
    FILE *fp = fopen(file_name.c_str(), "w+"); // todo: change FILE* to std::ifstream
    char data[60];
    if (!fp)
    {
        perror("File opening failed");
        exit(EXIT_FAILURE);
    }
    while (fgets(data, sizeof(data), fp) != nullptr)
    {
        if (strncmp(data, from_user, strlen_from_user) == 0)
        {
            long len = strlen(data);
            if (fgets(data, sizeof(data), fp) != nullptr)
            {
                len = -len;
                fseek(fp, len, SEEK_CUR);
                fputs(data, fp);
                len = strlen(data);
                fseek(fp, len, SEEK_CUR);
            }
        }
    }
    fclose(fp);
}

bool check_name_pass(const char *const name, const char *const pass)
{
    FILE *fp;
    char file[80], data[60];
    strcpy(file, data_dir);
    strcat(file, userinfo);

    fp = fopen(file, "r");
    if (!fp)
    {
        perror("File opening failed");
        exit(EXIT_FAILURE);
    }
    while (fgets(data, sizeof(data), fp) != nullptr)
    {
        if (strncmp(data, name, strlen(name)) != 0) // find username
            continue;

        if (strncmp(strchr(data, ' ') + 1, pass, strlen(pass)) != 0)
            return false; // invalid passwd

        // valid passwd
        fclose(fp);
        strcpy(file, data_dir);
        strcat(file, userstat);
        fp = fopen(file, "w+");
        if (!fp)
        {
            perror("File opening failed");
            exit(EXIT_FAILURE);
        }
        strcat(name, " on"); // change the status of the user to ON
        fwrite(name, 1, strlen(name), fp);
        fclose(fp);
        return true; // success
    }
    return false; // invalid username
}
