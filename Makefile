# https://makefiletutorial.com/
CC = g++
CFLAGS = -g -O2
CFLAGS_EXTRA = -lpthread
BIN_FILE = xsmtp
OBJ_FILES = module_mail.o module_user.o xsmtp.o
SRC_FILES = module_mail.cpp module_user.cpp xsmtp.cpp

all: xsmtp ssha

xsmtp: $(OBJ_FILES)
	$(CC) $(CFLAGS) $(OBJ_FILES) -o xsmtp $(CFLAGS_EXTRA)

$(OBJ_FILES):
	$(CC) $(CFLAGS) -c $(SRC_FILES)

ssha:
	$(CC) $(CFLAGS) -c ssha.cpp
	$(CC) $(CFLAGS) ssha.o -o ssha -lssl -lcrypto

clean:
	rm $(OBJ_FILES) xsmtp ssha.o ssha
