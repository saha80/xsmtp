# https://makefiletutorial.com/
CC = g++
CFLAGS = -g -O2

XSMTP_CFLAGS_EXTRA = -lpthread

XSMTP_BIN_FILE = xsmtp
XSMTP_OBJ_FILES = module_mail.o module_user.o xsmtp.o
XSMTP_SRC_FILES = module_mail.cpp module_user.cpp xsmtp.cpp

SSHA_CFLAGS_EXTRA = -lssl -lcrypto

SSHA_BIN_FILE = ssha
SSHA_OBJ_FILES = ssha.o
SSHA_SRC_FILES = ssha.cpp

all: xsmtp ssha

xsmtp: $(XSMTP_OBJ_FILES)
	$(CC) $(CFLAGS) $(XSMTP_OBJ_FILES) -o $(XSMTP_BIN_FILE) $(XSMTP_CFLAGS_EXTRA)

$(XSMTP_OBJ_FILES):
	$(CC) $(CFLAGS) -c $(XSMTP_SRC_FILES)

ssha: $(SSHA_OBJ_FILES)
	$(CC) $(CFLAGS) $(SSHA_OBJ_FILES) -o $(SSHA_BIN_FILE) $(SSHA_CFLAGS_EXTRA)

$(SSHA_OBJ_FILES):
	$(CC) $(CFLAGS) -c $(SSHA_SRC_FILES)

clean:
	rm $(XSMTP_OBJ_FILES) $(XSMTP_BIN_FILE) $(SSHA_OBJ_FILES) $(SSHA_BIN_FILE)
