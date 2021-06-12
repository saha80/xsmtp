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

#include "module_mail.h"
#include "module_user.h"

using namespace std;

// respond to request from the client
void respond(const int client_sockfd, int &mail_stat, int &rcpt_user_num, const char *const request);

// recieve mail contents
void mail_data(const int sockfd, const char *const from_user, const char rcpt_user[MAX_RCPT_USR][30], const int rcpt_user_num);

void *mail_proc(void *param)
{
    const int client_sockfd = *(int *)param;
    send_data(client_sockfd, reply_code[4]); // send 220
    int mail_stat = 1;
    int rcpt_user_num = 0;
    while (true)
    {
        array<char, BUFSIZ> buf{};
        const auto len = recv(client_sockfd, buf.data(), sizeof(buf), 0);
        if (len > 0)
        {
            cout << "Request stream: " << buf.data() << flush;
            respond(client_sockfd, mail_stat, rcpt_user_num, buf.data());
        }
        else
        {
            // cout << "S: no data received from client. The server exit permanently.\n";
            // break;
            sleep(1);
            continue;
        }
    }
    cout << "S:[" << client_sockfd << "] socket closed by client." << endl;
    cout << "============================================================\n\n";
    return (void *)EXIT_SUCCESS;
}

void send_data(const int sockfd, const char *const data)
{
    if (!data || sockfd == -1)
        return;
    send(sockfd, data, strlen(data), 0);
    cout << "Reply stream: " << data;
}

vector<char> base64_decode(const char *s)
{
    constexpr char B64[64] = {
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
            for (i = 0; unit[j] != B64[i] && i < sizeof(B64); ++i)
                ;
            unit[j] = (i == 64 ? 0 : i);
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
    char from_user[64];
    char rcpt_user[MAX_RCPT_USR][30];
    memset(from_user, 0, sizeof(from_user));
    memset(rcpt_user, 0, sizeof(rcpt_user));

    //smtp
    if (strncmp(request, "HELP", 4) == 0)
        send_data(client_sockfd, help_reply);
    else if (strncmp(request, "HELO", 4) == 0)
    {
        if (mail_stat == 1)
        {
            send_data(client_sockfd, reply_code[6]);
            rcpt_user_num = 0;
            memset(rcpt_user, 0, sizeof(rcpt_user));
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
            memset(from_user, 0, sizeof(from_user));
            strncpy(from_user, mail_addr_beg, len);
            if (check_user(from_user))
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
            char *pa, *pb;
            pa = strchr(request, '<');
            pb = strchr(request, '>');
            strncpy(rcpt_user[rcpt_user_num++], pa + 1, pb - pa - 1);
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
            mail_data(client_sockfd, from_user, rcpt_user, rcpt_user_num);
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
        user_quit(from_user);
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

void mail_data(const int sockfd, const char *const from_user, const char rcpt_user[MAX_RCPT_USR][30], const int rcpt_user_num)
{
    sleep(1);
    char buf[BUFSIZ];
    memset(buf, 0, sizeof(buf));
    recv(sockfd, buf, sizeof(buf), 0); // recieve mail contents
    cout << "Mail Contents:\n"
         << buf << endl;

    //mail content and format check

    //mail content store
    auto tm = time(nullptr);
    char file[80], tp[20], strtime[20];

    sprintf(strtime, "%ld", tm);

    for (int i = 0; i < rcpt_user_num; i++)
    {
        strcpy(file, data_dir);
        strcat(file, rcpt_user[i]);
        if (access(file, 0) == -1)
            mkdir(file, 0777);
        sprintf(tp, "/%s", from_user);
        strcat(file, tp);
        sprintf(tp, ".txt");
        strcat(file, tp);

        FILE *fp = fopen(file, "a");
        fseek(fp, 0, SEEK_END);
        if (fp)
        {
            fwrite(strtime, 1, strlen(strtime), fp);
            fwrite(" : ", 1, 3, fp);
            fwrite(buf, 1, strlen(buf), fp);
            fclose(fp);
        }
        else
            cout << "File open error!" << endl;
    }
    send_data(sockfd, reply_code[6]);
}
