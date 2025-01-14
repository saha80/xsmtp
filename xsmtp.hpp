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
//  This is a header file, include the configurations and interfaces of the whole project.
//
//

#pragma once

#include "common.hpp"
#include "module_mail.hpp"
#include "module_user.hpp"

class xsmtp_server
{
private:
    int server_sockfd_;

public:
    xsmtp_server();
    ~xsmtp_server();
    void server_loop() const;
};
