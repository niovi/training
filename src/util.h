#ifndef UTIL_H
#define UTIL_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/errno.h>
#include <signal.h>

#define FIFO1 "/tmp/fifo1"
#define FIFO2 "/tmp/fifo2"

#define EXIT        12
#define WC          11
#define MINCOUNT    10
#define MAXCOUNT    9
#define SEARCH      8
#define MSG         6
#define MSGSTART    4
#define MSGEND      5
#define CONTINUE    3
#define STOP        7
#define READY       2
#define START       1
#define END         0
#define ERROR      -1

class Util {
public:
    Util();
    Util(const Util& orig);
    virtual ~Util();
    static int toInteger(char*);
    static int sendMessage(int, char*, int);
    static char* getMessage(int);
    static int sendCode(int, int);
    static int getCode(int);
    static int sendWC(int, int, int, int);
    static int getInteger(int);
    static int sendInteger(int, int);
    static int sendExitCmd(int);
    static int sendMaxcountCmd(int, char*);
    static int sendMincountCmd(int, char*);
    static int sendSearchCmd(int, char*);
    static int sendWcCmd(int);

private:

};

#endif /* UTIL_H */

