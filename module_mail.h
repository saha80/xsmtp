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

#pragma once

#include "common.h"

// process mailing events
void *mail_proc(void *param);

// send data by socket
void send_data(const int sockfd, const char *const data);

// decode base64 streams
std::vector<char> base64_decode(const char *s);

//extern int check_user();
//extern void auth(int sockfd);
//extern void user_quit();

// Local Variables:
// mode: C++
// End:
