#include "server.h"

volatile sig_atomic_t ChldKilled = 0;
volatile sig_atomic_t Interrupt = 0;
volatile sig_atomic_t Pipe = 0;

void SIGCHLDHandler(int sign) {
    ChldKilled = 1;
}

void SIGINTHandelerS(int sign) {
    Interrupt = 1;
}

void initializeSignHandlersS() {
    struct sigaction saCh, saI, saP;
    saCh.sa_handler = SIGCHLDHandler;
    saCh.sa_flags = 0;
    sigfillset(&saCh.sa_mask);
    sigaction(SIGCHLD, &saCh, NULL);

    saI.sa_handler = SIGINTHandelerS;
    saI.sa_flags = 0;
    sigfillset(&saI.sa_mask);
    sigaction(SIGINT, &saI, NULL);
    sigaction(SIGTERM, &saI, NULL);
    //set SIGPIPE handler to ignore
    saP.sa_handler = SIG_IGN;
    saP.sa_flags = 0;
    sigfillset(&saP.sa_mask);
    sigaction(SIGPIPE, &saP, NULL);
}

Server* createServer(int pNum) {
    Server* server = new Server();
    initializeSignHandlersS();
    server->wNum = pNum;
    server->paths = NULL;
    server->child_pid = new pid_t[pNum];
    server->readfd = new int[pNum];
    server->writefd = new int[pNum];
    for (int i = 0; i < pNum; i++) {
        server->readfd[i] = -1;
        server->writefd[i] = -1;
    }
    return server;
}

void destroyServer(Server* server) {
    closePipes(server);
    waitWorkersToExit(server);
    for (int i = 0; i < server->pathNum; i++) {
        if (server->paths[i] != NULL) {
            delete server->paths[i];
        }
    }
    delete[] server->paths;
    delete[] server->child_pid;
    delete[] server->readfd;
    delete[] server->writefd;
    delete server;
}

void initializeServer(Server* server) {
    createWorkers(server);
    initializePipes(server);
}

void initializeWorkers(Server* server, char* docfile) {
    FILE* file;
    char * line = NULL;
    size_t len = 0;
    ssize_t readSize;
    int x = 0, l = 0;
    file = fopen(docfile, "r");
    if (file == NULL) {
        printf("ERROR : %d\n", getpid());
        perror("ERROR fopen:");
        exit(1);
    }
    //send MSGSART to workers
    for (int i = 0; i < server->wNum; i++) {
        if (Util::sendCode(server->writefd[i], MSGSTART) == -1) {
            printf("Server %d: initializeWorkers write error 1\n", getpid());
        }
    }
    while ((readSize = getline(&line, &len, file)) != -1) {//find the number of
        l++; //lines/paths in the file
    }
    //go back to the start of the file
    if (fseek(file, 0L, SEEK_SET) != 0) {
        printf("ERROR : %d\n", getpid());
        perror("ERROR fseek:");
        exit(1);
    }
    //set number of paths in file equal to l and allocate memory for path array
    server->pathNum = l;
    server->paths = new char*[server->pathNum];
    for (int i = 0; i < server->pathNum; i++) {
        server->paths[i] = NULL;
    }
    l = 0;
    x = 0;
    while ((readSize = getline(&line, &len, file)) != -1) {//read lines from file
        if (x == server->wNum)
            x = 0;
        if (line[readSize - 1] == '\n')//set last char to \0 if it was \n
            line[readSize - 1] = '\0';
        server->paths[l] = new char[readSize];
        strncpy(server->paths[l], line, readSize); //copy paths to array
        if (Util::sendCode(server->writefd[x], MSG) == -1) {//send MSG code
            printf("Server %d: initializeWorkers error 1\n", getpid());
            break;
        }
        if (Util::sendMessage(server->writefd[x], line, readSize) == -1) {//send
            printf("Server %d: initializeWorkers error 2\n", getpid()); //data
            break;
        }
        x++;
        l++;
    }
    if (ChldKilled && !Interrupt) {//if a worker's status changed just exit
        printf("Server %d: Error in worker initialization: exiting\n", getpid());
        destroyWorkers(server);
        destroyServer(server);
        exit(3);
    }
    fclose(file);
    //if there are more workers than files terminate them and free resources
    if (l < server->wNum) {
        pid_t wpid;
        int status;
        for (int i = l; i < server->wNum; i++) {
            closePipe(server, i);
            destroyWorker(server, i);
            wpid = waitpid(server->child_pid[i], &status, 0);
            //            printf("Server %d: Exit status of %d: %d (%s)\n", getpid(), (int) wpid, status,
            //                    (status > 0) ? "reject" : "accept");
            //            if (WIFEXITED(status)) {
            //                printf("Server %d: Exit code was: %d\n", getpid(), WEXITSTATUS(status));
            //            }
        }
        //set new number of workers equal to l
        server->wNum = l;
        ChldKilled = 0;
    }
    //notify the remaining workers that the file distribution face ended
    for (int i = 0; i < server->wNum; i++) {
        if (Util::sendCode(server->writefd[i], MSGEND) == -1) {
            break;
        }
    }
    if (ChldKilled && !Interrupt) {
        printf("Server %d: Error in worker initialization: exiting\n", getpid());
        destroyWorkers(server);
        destroyServer(server);
        exit(3);
    }
}

void startServer(Server* server) {
    char* cmd = NULL;
    //main loop
    while (!Interrupt) {
        //call readCommand routine to read from stdin
        cmd = readCommand(server);
        if (cmd != NULL && !strncmp(cmd, "exit", 4)) {
            break;
        }

    }
    if (cmd != NULL) {
        delete cmd;
    }
}

void createWorkers(Server* server) {
    for (int i = 0; i < server->wNum; i++) {
        switch (server->child_pid[i] = fork()) {
            case -1:
                perror("ERROR fork:");
                exit(1);
            case 0:
                Worker* worker = new Worker();
                startWorker(worker);
                destroyWorker(worker);
                exit(0);
        }
    }
    if (Interrupt) {
        destroyWorkers(server);
        destroyServer(server);
    }

}

void destroyWorkers(Server* server) {
    for (int i = 0; i < server->wNum; i++) {
        destroyWorker(server, i);
    }
}

void destroyWorker(Server* server, int i) {
    //send an interrupt signal to worker i
    kill(server->child_pid[i], SIGINT);
}

void replaceWorkers(Server* server, int cmd, char* arg) {
    pid_t p;
    int status;
    for (int i = 0; i < server->wNum; i++) {
        //for every worker call wait with WNOHANG flag so that waitpid doesn't 
        //block for workers that are still alive
        p = waitpid(server->child_pid[i], &status, WNOHANG);
        if (p > 0) {
            replaceWorker(server, cmd, arg, p);
        } else if (p == -1) {
            printf("Server %d: replaceWorkers Error\n", getpid());
        }
    }
    ChldKilled = 0;
}

void replaceWorker(Server* server, int cmd, char* args, pid_t wpid) {
    int i = -1;
    //find which worker needs replacement
    for (int j = 0; j < server->wNum; j++) {
        if (server->child_pid[j] == wpid) {
            i = j;
        }
    }
    //unlink and close old pipe
    unlinkPipe(wpid);
    closePipe(server, i);
    //create new process
    Worker* worker = NULL;
    switch (server->child_pid[i] = fork()) {
        case -1:
            printf("ERROR : %d\n", getpid());
            perror("ERROR fork:");
            exit(1);
        case 0:
            worker = new Worker();
            startWorker(worker);
            destroyWorker(worker);
            exit(0);
    }
    //create and open new pipe
    char path[2][20];
    sprintf(path[0], "%s.%d", FIFO1, server->child_pid[i]);
    sprintf(path[1], "%s.%d", FIFO2, server->child_pid[i]);
    createPipe(server, path);
    openPipe(server, path, i);
    initializeWorker(server, i);
    waitWorker(server, i);
    //for WC,MAXCOUNT and MINCOUNT send the command to the new worker
    switch (cmd) {
        case WC:
            printf("server %d: sending wc command to new worker\n", getpid());
            if (Util::sendWcCmd(server->writefd[i]) <= 0)
                printf("Server %d: sendWcCmd error\n", getpid());
            break;
        case MAXCOUNT:
            if (Util::sendMaxcountCmd(server->writefd[i], args) == -1) {
                printf("server %d: replaceWorker error\n", getpid());
            }
            break;
        case MINCOUNT:
            if (Util::sendMincountCmd(server->writefd[i], args) == -1) {
                printf("server %d: replaceWorker error\n", getpid());
            }
            break;
        case EXIT:
            if (Util::sendExitCmd(server->writefd[i]) == -1) {
                printf("server %d: replaceWorker error\n", getpid());
            }
    }
}

void initializeWorker(Server* server, int i) {
    //send a new message to worker with all the paths he must process
    if (Util::sendCode(server->writefd[i], MSGSTART) == -1) {
        printf("Server %d: initializeWorker write error 1\n", getpid());
    }
    for (int j = i; j < server->pathNum; j += server->wNum) {
        if (Util::sendCode(server->writefd[i], MSG) == -1) {
            printf("Server %d: initializeWorker write error 2\n", getpid());
        }
        if (Util::sendMessage(server->writefd[i], server->paths[j]
                , strlen(server->paths[j]) + 1) == -1) {
            printf("Server %d: initializeWorker write error 3\n", getpid());
        }
    }
    if (Util::sendCode(server->writefd[i], MSGEND) == -1) {
        printf("Server %d: initializeWorker write error 4\n", getpid());
    }
}

void waitWorkers(Server * server) {
    struct pollfd fds[server->wNum];
    for (int i = 0; i < server->wNum; i++) {
        fds[i].fd = server->readfd[i];
        fds[i].events = POLLIN;
        fds[i].revents = 0;
    }
    int ret = 0;
    while (true) {
        //use poll to wait for all processes 
        //with -1 as timeout arg (block indefinitely)
        ret = poll(fds, server->wNum, -1);
        if (ret < 0) {
            printf("ERROR : poll\n");
            break;
        } else if (ret == server->wNum) {//if all processes have responded
            for (int i = 0; i < server->wNum; i++) {
                if (Util::getCode(server->readfd[i]) != READY) {//read to free the buffer
                    printf("Server %d: Unexpected code\n", getpid());
                }
            }
            break;
        }
    }
}

void waitWorker(Server* server, int i) {
    while (true) {
        if (Util::getCode(server->readfd[i]) != READY) {//block here until worker
            printf("Server %d: Unexpected code\n", getpid()); //sends READY code
        } else break;
    }

}

void sendCommand(Server* server, char* cmd, char* args) {
    if (!strcmp(cmd, "search")) {
        if (args == NULL) {
            printf("Not enough arguments!\n");
            return;
        }
        bool timeoutflag = false;
        int timeout = -1;
        int alength = 0;
        //check args string for -d
        for (int i = 0; i < strlen(args); i++) {
            if (args[i] == 'd' && args[i - 1] == '-') {
                timeout = atoi(args + i + 1);
                timeoutflag = true;
                alength = i;
            }
        }
        if (!timeoutflag) {
            printf("Missing timeout argument!\n");
            return;
        }
        if (timeout <= 0) {
            printf("Invalid timeout argument!\n");
            return;
        }
        //create new arguments without -d
        char* newArgs = new char[alength];
        strncpy(newArgs, args, alength - 1);
        newArgs[alength - 1] = '\0';
        //send command to workers
        for (int i = 0; i < server->wNum; i++) {
            if (Util::sendSearchCmd(server->writefd[i], newArgs) == -1) {
                printf("Server %d: sendCommand error 1\n", getpid());
            }
        }
        delete newArgs;
        //wait for results
        getSearchResults(server, timeout);
    } else if (!strcmp(cmd, "maxcount")) {
        char* keyword = strtok(args, " ");
        for (int i = 0; i < server->wNum; i++) {
            if (Util::sendMaxcountCmd(server->writefd[i], keyword) == -1) {
                printf("Server %d: sendCommand error 2\n", getpid());
            }
        }
        getMaxMinResults(server, MAXCOUNT, keyword);
    } else if (!strcmp(cmd, "mincount")) {
        char* keyword = strtok(args, " ");
        for (int i = 0; i < server->wNum; i++) {
            if (Util::sendMincountCmd(server->writefd[i], keyword) == -1) {
                printf("Server %d: sendCommand error 3\n", getpid());
            }
        }
        getMaxMinResults(server, MINCOUNT, keyword);
    } else if (!strcmp(cmd, "wc")) {
        for (int i = 0; i < server->wNum; i++) {
            if (Util::sendWcCmd(server->writefd[i]) <= 0) {
                printf("Server %d: sendCommand error 4\n", getpid());
            }
        }
        getWCResults(server);
    } else if (!strcmp(cmd, "exit")) {
        for (int i = 0; i < server->wNum; i++) {
            if (Util::sendExitCmd(server->writefd[i]) == -1) {
                printf("Server %d: sendCommand error 5\n", getpid());
            }
        }
    } else {
        printf("Server %d: Unknown command\n", getpid());
    }
}

char* readCommand(Server * server) {
    char * line = NULL;
    char *cmd = NULL;
    char* args = NULL;
    size_t len = 0;
    ssize_t read;
    char del[] = " \n";
    //if there was a signal sent while we were waiting for user input
    //getline will keep returning -1, so we must reset error variables for stdin 
    clearerr(stdin);
    fflush(stdin);
    if ((read = getline(&line, &len, stdin)) != -1 && !Interrupt) {
        //get command
        cmd = strtok(line, del);
        if (cmd != NULL && strcmp(cmd, "")) {
            if (strcmp(cmd, "wc") && strcmp(cmd, "exit")) {
                args = strtok(NULL, "\n");
            }
            //call sendCommand routine to send the command to workers
            sendCommand(server, cmd, args);
        }
    } else if (read == -1 && ChldKilled) {//if a worker's status changed 
        //replace the worker
        replaceWorkers(server, -1, NULL);
    } else if (Interrupt) {//if an interrupt signal was caught exit
        if (cmd != NULL) {
            delete cmd;
        }
        closePipes(server);
        waitWorkersToExit(server);
        exit(0);
    }
    return cmd;
}

void createPipe(Server* server, char path[2][20]) {
    if ((mkfifo(path[0], PERMS) < 0) && (errno != EEXIST)) {
        printf("ERROR : %d\n", getpid());
        perror("ERROR mkfifo:");
        exit(1);
    }
    if ((mkfifo(path[1], PERMS) < 0) && (errno != EEXIST)) {
        unlink(path[0]);
        printf("ERROR : %d\n", getpid());
        perror("ERROR mkfifo:");
        exit(1);
    }

}

void openPipe(Server* server, char path[2][20], int i) {
    while ((server->readfd[i] = open(path[0], O_RDONLY)) < 0);
    while ((server->writefd[i] = open(path[1], O_WRONLY)) < 0);
}

void unlinkPipe(pid_t wpid) {
    char p1[20];
    char p2[20];
    sprintf(p1, "%s.%d", FIFO2, wpid);
    sprintf(p2, "%s.%d", FIFO1, wpid);
    unlink(p1);
    unlink(p2);
}

void closePipes(Server * server) {
    for (int i = 0; i < server->wNum; i++) {
        closePipe(server, i);
    }
}

void closePipe(Server* server, int i) {
    if (close(server->readfd[i]) < 0) {
        printf("ERROR : %d\n", getpid());
        perror("ERROR close:");
        exit(1);
    }
    if (close(server->writefd[i]) < 0) {
        printf("ERROR : %d\n", getpid());
        perror("ERROR close:");
        exit(1);
    }
}

void initializePipes(Server * server) {
    char path[2][20];
    for (int i = 0; i < server->wNum; i++) {
        sprintf(path[0], "%s.%d", FIFO1, server->child_pid[i]);
        sprintf(path[1], "%s.%d", FIFO2, server->child_pid[i]);
        createPipe(server, path);
        openPipe(server, path, i);
    }
}

void getWCResults(Server * server) {
    //create mask to interrupt ppoll when interrupt signal is caught
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    struct pollfd fds[server->wNum];
    for (int i = 0; i < server->wNum; i++) {
        fds[i].fd = server->readfd[i];
        fds[i].events = POLLIN;
        fds[i].revents = 0;
    }
    int totalW = 0, totalC = 0, totalL = 0;
    int ret = 0;
    int count = 0;
    while (count < server->wNum && !Interrupt) {
        if (ChldKilled && !Interrupt) {//if a child status has changed 
            //try to replace it
            replaceWorkers(server, WC, NULL);
        }
        //wait for input from fds
        ret = ppoll(fds, server->wNum, NULL, &mask);
        if (ret < 0) {// error case
            printf("ERROR : poll\n");
            break;
        } else if (ret == 0) {//timeout case
            break;
        } else if (ret > 0) {//input was found
            for (int i = 0; i < server->wNum; i++) {
                //if input came from i process get input
                if (fds[i].revents != 0) {
                    totalW += Util::getInteger(server->readfd[i]);
                    totalC += Util::getInteger(server->readfd[i]);
                    totalL += Util::getInteger(server->readfd[i]);
                    count++;
                }
            }

        }
    }
    printf("Words: %d\n", totalW);
    printf("Characters: %d\n", totalC);
    printf("Lines: %d\n", totalL);
}

void getMaxMinResults(Server* server, int cmd, char* arg) {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    struct pollfd fds[server->wNum];
    for (int i = 0; i < server->wNum; i++) {
        fds[i].fd = server->readfd[i];
        fds[i].events = POLLIN;
        fds[i].revents = 0;
    }
    int ret = 0;
    int result = 0;
    char* path;
    int resCount = 0;
    int count = 0;
    char* wPath = NULL;
    while (resCount < server->wNum && !Interrupt) {
        if (ChldKilled && !Interrupt) {
            replaceWorkers(server, cmd, arg);
        }
        ret = ppoll(fds, server->wNum, NULL, &mask);
        if (ret < 0) {
            printf("ERROR : poll\n");
            break;
        } else if (ret == 0) {
            printf("Timeout: poll\n");
            break;
        } else if (ret > 0) {
            for (int i = 0; i < server->wNum; i++) {
                if (fds[i].revents == POLLIN) {
                    switch (cmd) {
                        case MAXCOUNT:
                            count = Util::getInteger(server->readfd[i]);
                            wPath = Util::getMessage(server->readfd[i]);
                            resCount++;
                            if (count > result) {
                                result = count;
                                path = wPath;
                            }
                            break;
                        case MINCOUNT:
                            count = Util::getInteger(server->readfd[i]);
                            wPath = Util::getMessage(server->readfd[i]);
                            resCount++;
                            if (count > 0 && (count < result || result == 0)) {
                                result = count;
                                path = wPath;
                            }
                    }

                }
            }
        }
    }
    if (result > 0) {
        printf("File: %s\n", path);
        printf("Count: %d\n", result);
    } else {
        printf("Word not found in documents\n");
    }
}

void getSearchResults(Server* server, int timeout) {
    clock_t start_t;
    bool flag = false;
    int count = 0;
    int ret = 0;
    bool finished[server->wNum];
    for (int i = 0; i < server->wNum; i++) {
        finished[i] = false;
    }
    struct pollfd fds[server->wNum];
    for (int i = 0; i < server->wNum; i++) {
        fds[i].fd = server->readfd[i];
        fds[i].events = POLLIN;
        fds[i].revents = 0;
    }

    start_t = clock();
    //while flag == false, meaning not all workers have finished
    //and timeout has not expired
    while (!flag && (double) (clock() - start_t) / CLOCKS_PER_SEC <= timeout) {
        if (ChldKilled && !Interrupt) {
            replaceWorkers(server, -1, NULL);
        }
        ret = poll(fds, server->wNum, 1000);
        if (ret < 0) {
            printf("ERROR : poll\n");
            break;
        } else if (ret > 0) {
            for (int i = 0; i < server->wNum; i++) {
                if (fds[i].revents != 0) {
                    int msg = Util::getCode(server->readfd[i]);
                    switch (msg) {
                        case MSGEND://if a MSGEND code was read this process has
                            finished[i] = true; //finished.
                            count++;
                            break;
                        case MSG://if a MSG code was read, read data
                        {
                            char* path = Util::getMessage(server->readfd[i]);
                            int lineNum = Util::getInteger(server->readfd[i]);
                            char* line = Util::getMessage(server->readfd[i]);
                            line = strtok(line, "\n");
                            printf("Document: %s , line: %d\n", path, lineNum);
                            printf("%s\n", line);
                        }
                    }
                }
            }
            if (count == server->wNum) {//if have gotten MSGEND from all workers
                flag = true;
            }
        }
    }
    for (int i = 0; i < server->wNum; i++) {//send USR1 signal to workers that 
        if (!finished[i]) { //are still running to notify them
            kill(server->child_pid[i], SIGUSR1); //to stop
        }
    }
    if (count < server->wNum) {
        printf("%d workers did not finish\n", server->wNum - count);
    } else {
        printf("All workers finished\n");
    }
}

void waitWorkersToExit(Server * server) {
    int status;
    pid_t wpid;
    while ((wpid = wait(&status)) > 0) {
        //        printf("Server %d: Exit status of %d: %d (%s)\n", getpid(), (int) wpid, status,
        //                (status > 0) ? "reject" : "accept");
        //        if (WIFEXITED(status)) {
        //            printf("Server %d: Exit code was: %d\n", getpid(), WEXITSTATUS(status));
        //        }
    }
}