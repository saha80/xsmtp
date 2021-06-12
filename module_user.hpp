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

#pragma once

#include "common.hpp"

// find if user exists
bool check_user(const char *const from_user);

// user authentication
void auth(const int sockfd, int &mail_stat);

// user logout
void user_quit(const char *const from_user);

//extern void send_data(int sockfd, const char* data);
//extern char *base64_decode(char *s);

// Local Variables:
// mode: C++
// End: