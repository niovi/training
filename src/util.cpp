#include "util.h"

Util::Util() {
}

Util::Util(const Util& orig) {
}

Util::~Util() {
}

int Util::toInteger(char* num) {
    errno = 0;
    char* endptr;
    int id = strtol(num, &endptr, 10);
    if (endptr == num) {
        errno = 0;
        return -1;
    }
    return id;
}

int Util::sendCode(int fd, int code) {
    return write(fd, &code, sizeof (int));
}

int Util::getCode(int fd) {
    int code;
    ssize_t rSize = read(fd, &code, sizeof (int));
    if (rSize <= 0)
        return ERROR;
    return code;
}

int Util::sendMessage(int fd, char* data, int len) {
    if (write(fd, &len, sizeof (int)) == -1)
        return -1;
    if (write(fd, data, len * sizeof (char)) == -1)
        return -1;
    return 0;
}

char* Util::getMessage(int fd) {
    int size;
    ssize_t rSize = read(fd, &size, sizeof (size));
    if (rSize <= 0)
        return NULL;
    char *data = new char[size];
    int offset = 0;
    rSize = read(fd, data + offset, size);
    while (offset + rSize < size) {
        rSize = read(fd, data + offset, size - offset);
        offset += rSize;
    }
    //    if (rSize != size)
    //        return NULL;
    return data;
}

int Util::sendWC(int fd, int wCount, int cCount, int lCount) {
    if (write(fd, &wCount, sizeof (int)) == -1)
        return -1;
    if (write(fd, &cCount, sizeof (int)) == -1)
        return -1;
    if (write(fd, &lCount, sizeof (int)) == -1)
        return -1;
    return 0;
}

int Util::getInteger(int fd) {
    int integer;
    ssize_t rSize = read(fd, &integer, sizeof (int));
    if (rSize <= 0)
        return 0;
    return integer;
}

int Util::sendInteger(int fd, int integer) {
    return write(fd, &integer, sizeof (int));
}

int Util::sendWcCmd(int fd) {
    return Util::sendCode(fd, WC);
}

int Util::sendExitCmd(int fd) {
    return Util::sendCode(fd, EXIT);
}

int Util::sendMaxcountCmd(int fd, char* arg) {
    if (Util::sendCode(fd, MAXCOUNT) == -1)
        return -1;
    if (Util::sendMessage(fd, arg, strlen(arg) + 1) == -1)
        return -1;
    return 0;
}

int Util::sendMincountCmd(int fd, char* arg) {
    if (Util::sendCode(fd, MINCOUNT) == -1)
        return -1;
    if (Util::sendMessage(fd, arg, strlen(arg) + 1) == -1)
        return -1;
    return 0;
}

int Util::sendSearchCmd(int fd, char* args) {
    if (Util::sendCode(fd, SEARCH) == -1)
        return -1;
    if (Util::sendMessage(fd, args, strlen(args) + 1) == -1)
        return -1;
    return 0;
}