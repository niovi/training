#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <poll.h>
#include <cstring>

#include "server.h"

#define READ 0
#define WRITE 1
#define PERMS   0666
#define BUFSIZE 10

using namespace std;

int main(int argc, char** argv) {
    int wNum;
    char* fileName;
    if (argc != 5) {
        std::cout << "Wrong number of arguments!" << std::endl;
        return 1;
    } else {
        if (strcmp(argv[1], "-d") == 0) {
            fileName = new char[strlen(argv[2]) + 1];
            fileName[strlen(argv[2])] = '\0';
            strncpy(fileName, argv[2], strlen(argv[2]) + 1);
            if (strcmp(argv[3], "-w") == 0) {
                wNum = Util::toInteger(argv[4]);
                if (wNum == -1)
                    return 1;
            } else
                return 1;
        } else if (strcmp(argv[1], "-w") == 0) {
            wNum = Util::toInteger(argv[2]);
            if (wNum == -1)
                return 1;
            if (strcmp(argv[3], "-d") == 0) {
                fileName = new char[strlen(argv[4]) + 1];
                fileName[strlen(argv[4])] = '\0';
                strncpy(fileName, argv[4], strlen(argv[4]) + 1);
            } else return 1;
        }
    }

    Server* server = createServer(wNum);
    initializeServer(server);
    initializeWorkers(server, fileName);
    waitWorkers(server);
    startServer(server);
    destroyServer(server);
    exit(0);
}

