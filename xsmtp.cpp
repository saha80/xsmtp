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
//  The main function entrance for the whole project.
//
//

#include "xsmtp.hpp"

using namespace std;

int mail_stat = 0;
int rcpt_user_num = 0;
char from_user[64] = "";
char rcpt_user[MAX_RCPT_USR][30] = {""};

void quit(int arg);

int main()
{
    signal(SIGINT, quit);     // go to MiniWebQuit when Ctrl+C key pressed.
    signal(SIGTERM, quit);    // terminal signal
    signal(SIGPIPE, SIG_IGN); // ignore pipe signal. For more see http://www.wlug.org.nz/SIGPIPE

    xsmtp_server server;
    server.server_loop();
}

void quit(int arg)
{
    if (arg)
    {
        printf("\nS:Caught signal (%d). Mail server shutting down...\n", arg);
        exit(EXIT_SUCCESS);
    }
}

xsmtp_server::xsmtp_server() : server_sockfd_(socket(AF_INET, SOCK_STREAM, 0)) // create socket
{
    if (server_sockfd_ == -1)
    {
        perror("S:socket create error!");
        exit(EXIT_FAILURE);
    }

    //set the socket's attributes
    sockaddr_in server_addr{AF_INET, htons(PORT), htonl(INADDR_ANY)};
    memset(server_addr.sin_zero, 0, sizeof(server_addr.sin_zero));

    // create a link
    if (bind(server_sockfd_, (sockaddr *)&server_addr, sizeof(sockaddr)) == -1)
    {
        perror("S:bind error!");
        exit(EXIT_FAILURE);
    }

    // set to non-blocking to avoid lockout issue
    fcntl(server_sockfd_, F_SETFL, fcntl(server_sockfd_, F_GETFL, 0) | O_NONBLOCK);

    //listening requests from clients
    if (listen(server_sockfd_, SOMAXCONN) == -1)
    {
        perror("S:listen error!");
        exit(EXIT_FAILURE);
    }
}

xsmtp_server::~xsmtp_server()
{
    close(server_sockfd_);
}

void xsmtp_server::server_loop()
{

    vector<pthread_t> thread_pool{};
    vector<int> clients_sockets{};
    thread_pool.reserve(SOMAXCONN);
    clients_sockets.reserve(SOMAXCONN);

    //accept requests from clients,loop and wait.
    cout << "SMTP mail server started..." << endl;
    while (true)
    {
        sockaddr_in client_addr;
        socklen_t sin_size = sizeof(client_addr);
        const int client_socket = accept(server_sockfd_, (sockaddr *)&client_addr, &sin_size);
        if (client_socket == -1)
        {
            // perror("S:accept error!");
            sleep(1);
            continue;
        }
        const auto t = time(nullptr);
        cout << "S:received a connection from " << inet_ntoa(client_addr.sin_addr)
             << " at " << put_time(localtime(&t), "%T")
             << endl;
        clients_sockets.emplace_back(client_socket);
        thread_pool.emplace_back();
        pthread_create(&thread_pool.back(), nullptr, mail_proc, &clients_sockets.back());
    }

    for (auto &thread : thread_pool)
        pthread_join(thread, nullptr);
    for (auto &socket : clients_sockets)
        close(socket);
}
