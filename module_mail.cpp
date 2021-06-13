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
//  This is a header file, mainly about mailing events processing.
//
//

#include "module_mail.hpp"
#include "module_user.hpp"

using namespace std;

// respond to request from the client
void respond(const int client_sockfd, int &mail_stat, int &rcpt_user_num, const char *const request);

// recieve mail contents
void mail_data(const int sockfd, const char *const from_user, const array<array<char, 30>, MAX_RCPT_USR> &rcpt_user, const int rcpt_user_num);

void *mail_proc(void *param)
{
    const int client_sockfd = *static_cast<int *>(param);
    send_data(client_sockfd, reply_code[4]); // send 220
    int mail_stat = 1;
    int rcpt_user_num = 0;
    while (true)
    {
        array<char, BUFSIZ> buf{};
        const auto len = recv(client_sockfd, buf.data(), sizeof(buf), 0);
        if (len > 0)
        {
            cout << "Request stream: " << buf.data();
            respond(client_sockfd, mail_stat, rcpt_user_num, buf.data());
        }
        else
        {
            // cout << "S: no data received from client. The server exit permanently.\n";
            // break;
            sleep(1); // todo: add timeout. client will be disconnected due to inactivity.
            continue;
        }
    }
    cout << "S:[" << client_sockfd << "] socket closed by client.\n"
         << "============================================================\n\n";
    return (void *)EXIT_SUCCESS;
}

void send_data(const int sockfd, const char *const data)
{
    if (data == nullptr || sockfd == -1)
        return;
    send(sockfd, data, strlen(data), 0);
    cout << "Reply stream: " << data;
}

vector<char> base64_decode(const char *s)
{
    constexpr array<char, 64> B64{
        'A', 'B', 'C', 'D', 'E', 'F', 'G',
        'H', 'I', 'J', 'K', 'L', 'M', 'N',
        'O', 'P', 'Q', 'R', 'S', 'T',
        'U', 'V', 'W', 'X', 'Y', 'Z',

        'a', 'b', 'c', 'd', 'e', 'f', 'g',
        'h', 'i', 'j', 'k', 'l', 'm', 'n',
        'o', 'p', 'q', 'r', 's', 't',
        'u', 'v', 'w', 'x', 'y', 'z',

        '0', '1', '2', '3', '4', '5', '6',
        '7', '8', '9', '+', '/'};
    const auto len = strlen(s);
    vector<char> result(len / 4 * 3 + 1);
    char *r = result.data();
    for (const auto s_end = s + len; s < s_end; s += 4)
    {
        array<uint8_t, 4> unit;
        memcpy(unit.data(), s, sizeof(unit));
        unit[2] = (unit[2] == '=' ? 0 : unit[2]);
        unit[3] = (unit[3] == '=' ? 0 : unit[3]);
        for (uint8_t j = 0, i; j < 4; ++j)
        {
            for (i = 0; unit[j] != B64[i] && i < B64.size(); ++i)
                ;
            unit[j] = (i == B64.size() ? 0 : i);
        }
        *r++ = (unit[0] << 2) | (unit[1] >> 4);
        *r++ = (unit[1] << 4) | (unit[2] >> 2);
        *r++ = (unit[2] << 6) | (unit[3]);
    }
    *r = 0;
    return result;
}

void respond(const int client_sockfd, int &mail_stat, int &rcpt_user_num, const char *const request)
{
    array<char, 64> from_user{};
    array<array<char, 30>, MAX_RCPT_USR> rcpt_user{};

    //smtp
    if (strncmp(request, "HELP", 4) == 0)
        send_data(client_sockfd, help_reply);
    else if (strncmp(request, "HELO", 4) == 0)
    {
        if (mail_stat == 1)
        {
            send_data(client_sockfd, reply_code[6]);
            rcpt_user_num = 0;
            rcpt_user.fill({});
            mail_stat = 2;
        }
        else
            send_data(client_sockfd, reply_code[15]);
    }
    else if (strncmp(request, "MAIL FROM", 9) == 0)
    {
        if (mail_stat == 2 || mail_stat == 13)
        {
            auto mail_addr_beg = strchr(request, '<');
            const auto mail_addr_end = strchr(request, '>');
            if (!mail_addr_beg || !mail_addr_end)
                send_data(client_sockfd, reply_code[15]);
            mail_addr_beg = std::next(mail_addr_beg);
            const size_t len = mail_addr_end - mail_addr_beg - 1;
            from_user.fill(0);
            strncpy(from_user.data(), mail_addr_beg, len);
            if (check_user(from_user.data()))
            {
                send_data(client_sockfd, reply_code[6]);
                mail_stat = 3;
            }
            else
                send_data(client_sockfd, reply_code[15]);
        }
        else if (mail_stat == 12)
            send_data(client_sockfd, reply_code[23]);
        else
            send_data(client_sockfd, "503 Error: send HELO/EHLO first\r\n");
    }
    else if (strncmp(request, "RCPT TO", 7) == 0)
    {
        if ((mail_stat == 3 || mail_stat == 4) && rcpt_user_num < MAX_RCPT_USR)
        {
            const auto pa = strchr(request, '<'), pb = strchr(request, '>');
            strncpy((rcpt_user[rcpt_user_num++]).data(), pa + 1, pb - pa - 1);
            send_data(client_sockfd, reply_code[6]);
            mail_stat = 4;
        }
        else
            send_data(client_sockfd, reply_code[16]);
    }
    else if (strncmp(request, "DATA", 4) == 0)
    {
        if (mail_stat == 4)
        {
            send_data(client_sockfd, reply_code[8]);
            mail_data(client_sockfd, from_user.data(), rcpt_user, rcpt_user_num);
            mail_stat = 5;
        }
        else
            send_data(client_sockfd, reply_code[16]);
    }
    else if (strncmp(request, "RSET", 4) == 0)
    {
        mail_stat = 1;
        send_data(client_sockfd, reply_code[6]);
    }
    else if (strncmp(request, "QUIT", 4) == 0)
    {
        mail_stat = 0;
        user_quit(from_user.data());
        send_data(client_sockfd, reply_code[5]);
        pthread_exit((void *)EXIT_FAILURE);
    }
    //esmpt
    else if (strncmp(request, "EHLO", 4) == 0)
        if (mail_stat == 1)
        {
            send_data(client_sockfd, reply_code[24]);
            mail_stat = 12;
        }
        else
            send_data(client_sockfd, reply_code[15]);
    else if (strncmp(request, "AUTH LOGIN", 10) == 0)
        auth(client_sockfd, mail_stat);
    else
        send_data(client_sockfd, reply_code[15]);
}

void mail_data(const int sockfd, const char *const from_user, const array<array<char, 30>, MAX_RCPT_USR> &rcpt_user, const int rcpt_user_num)
{
    sleep(1);
    array<char, BUFSIZ> buf{};
    recv(sockfd, buf.data(), sizeof(buf), 0); // recieve mail contents
    cout << "Mail Contents:\n"
         << buf.data() << "\n";

    //mail content and format check

    //mail content store
    array<char, 80> file_name{};
    array<char, 20> tp{}, strtime{};

    sprintf(strtime.data(), "%ld", time(nullptr));

    for (int i = 0; i < rcpt_user_num; i++)
    {
        strcpy(file_name.data(), data_dir);
        strcat(file_name.data(), (rcpt_user[i]).data());
        if (access(file_name.data(), 0) == -1)
            mkdir(file_name.data(), 0777);
        sprintf(tp.data(), "/%s", from_user);
        strcat(file_name.data(), tp.data());
        sprintf(tp.data(), ".txt");
        strcat(file_name.data(), tp.data());

        ofstream file(file_name.data(), ios::app); // fopen(file.data(), "a")
        // fseek(fp, 0, SEEK_END);
        if (file.is_open())
        {
            file << strtime.data()
                 // fwrite(strtime.data(), 1, strlen(strtime.data()), fp);
                 << " : "
                 // fwrite(" : ", 1, 3, fp);
                 << buf.data();
            // fwrite(buf.data(), 1, strlen(buf.data()), fp);
        }
        else
            perror("File opening failed");
    }
    send_data(sockfd, reply_code[6]);
}
