#include "worker.h"

volatile sig_atomic_t Usr1W = 0;
volatile sig_atomic_t InterruptW = 0;

void handler(int sign) {
    Usr1W = 1;
}

void interruptHandelerW(int sign) {
    printf("Worker %d SIGINT: %d\n", getpid(), sign);
    InterruptW = 1;
}

void initializeSignHandlersW() {
    struct sigaction saU, saI;
    saU.sa_handler = handler;
    saU.sa_flags = 0;
    sigfillset(&saU.sa_mask);
    sigaction(SIGUSR1, &saU, NULL);

    saI.sa_handler = interruptHandelerW;
    saI.sa_flags = 0;
    sigfillset(&saI.sa_mask);
    sigaction(SIGINT, &saI, NULL);
    sigaction(SIGTERM, &saI, NULL);

}

int initialize(Worker* worker) {
    if (openPipe(worker) == ERROR) {//open pipe
        return ERROR;
    }
    int code = Util::getCode(worker->readfd);
    if (code != MSGSTART) {
        printf("Worker %d: Unexpected code\n", getpid());
        return ERROR;
    }
    //get catalogs from server and process
    while (!InterruptW) {
        code = Util::getCode(worker->readfd);
        if (code == MSG) {
            char* msg = Util::getMessage(worker->readfd);
            if (msg != NULL) {
                processCatalog(worker, msg);
                delete[] msg;
            }
        } else if (code == MSGEND) {
            break;
        } else {
            return ERROR;
        }
    }
    if (InterruptW)
        return ERROR;
    return 0;
}

void startWorker(Worker* worker) {
    if (initialize(worker) == ERROR) {//initialize worker aka get catalogs and process them
        return;
    }
    sendReady(worker); //notify server that worker is ready to process commands
    while (!InterruptW && processCommand(worker) != EXIT);
    //if interrupt or terminate signal was caught unlink and destroy 
    unlinkPipe(worker);
    destroyWorker(worker);
    exit(0);
}

void destroyWorker(Worker* worker) {
    if (worker != NULL) {
        delete worker;
        worker = NULL;
    }
}

void processCatalog(Worker* worker, char* nameDir) {
    worker->totalCNum = 0;
    worker->totalLNum = 0;
    worker->totalWNum = 0;
    DIR* dir;
    if ((dir = opendir(nameDir)) == NULL) {//open dir
        fprintf(stdout, "Error : Failed to open input directory %s - %s\n", nameDir, strerror(errno));
        exit(0);
    }
    struct dirent* in_file; //create dirent structure with directory info
    while ((in_file = readdir(dir)) != NULL) {
        //exclude current and parent directories
        if (!strcmp(in_file->d_name, "."))
            continue;
        if (!strcmp(in_file->d_name, ".."))
            continue;
        //allocate space and copy path
        char* path = new char[strlen(nameDir) + strlen(in_file->d_name) + 1];
        strncpy(path, nameDir, strlen(nameDir));
        strncpy(path + strlen(nameDir), in_file->d_name, strlen(in_file->d_name));
        path[strlen(nameDir) + strlen(in_file->d_name)] = '\0';
        //call processFile procedure
        processFile(worker, path);
        delete[] path;
    }
    if (closedir(dir) == -1) {
        fprintf(stdout, "Error : Failed to close input directory %s - %s\n", nameDir, strerror(errno));
        exit(0);
    }

}

void processFile(Worker* worker, char* path) {
    FILE * fp = NULL;
    char* fileData = NULL;
    fp = fopen(path, "r");
    if (fp == NULL) {
        printf("Worker: processFile: Error opening file: %s\n", path);
        return;
    }
    //go to the end of the file
    if (fseek(fp, 0L, SEEK_END) == 0) {
        //get the size of the file
        long bufsize = ftell(fp);
        if (bufsize == -1) {
            printf("Worker: processFile: Error Unknown file size\n");
            fclose(fp);
            return;
        }
        //allocate our buffer to that size
        fileData = new char[bufsize + 1];
        //go back to the start of the file
        if (fseek(fp, 0L, SEEK_SET) != 0) {
            delete fileData;
            printf("Worker: processFile: Error Unknown file size\n");
            return;
        }
        char* line = NULL;
        size_t len = 0;
        ssize_t read;
        int lineOffset = 0;
        int lineLength = 0;
        int lineNum = 0;
        int wNum = 0;
        char del[] = " \t\n";
        //create new document (aka MapNode)
        MapNode* newNode = new MapNode();
        newNode->setPath(path, strlen(path));
        //read file line by line
        while ((read = getline(&line, &len, fp)) != -1) {
            lineLength = read;
            strncpy(fileData + lineOffset, line, lineLength);
            lineNum++;
            //for every word in line insert to index
            char* word = strtok(line, del);
            while (word != NULL) {
                wNum++;
                worker->index.insert(lineNum, lineOffset, lineLength, word, strlen(word), newNode);
                word = strtok(NULL, del);
            }
            lineOffset += lineLength;
        }
        //set last character in fileData buffer to \0 for safety
        fileData[bufsize] = '\0';
        //set document data
        newNode->setDoc(fileData, bufsize + 1);
        //add document to Map
        worker->map.add(newNode);
        //set total character, line and word numbers
        worker->totalCNum += bufsize;
        worker->totalLNum += lineNum;
        worker->totalWNum += wNum;
    } else {
        printf("Worker: processFile: Error Unknown file size\n");
        fclose(fp);
        return;
    }
    fclose(fp);
}

int processCommand(Worker* worker) {
    int cmd = -1;
    cmd = Util::getCode(worker->readfd); //get command code
    switch (cmd) {
        case SEARCH:
        {
            Usr1W = 0;
            char* args = Util::getMessage(worker->readfd);
            if (Util::sendCode(worker->writefd, MSGSTART) == -1) {
                printf("Worker %d: processCommand error 1\n", getpid());
            }
            char* word = strtok(args, " ");
            //for every word in args search
            while (word != NULL && !InterruptW) {
                if (!Usr1W && strncmp(word, " ", 1) && strncmp(word, "\t", 1)) {
                    searchWord(worker, word);
                } else if (Usr1W) {//if a USR1 signal was send from server
                    interrupt(worker); //stop searching and go to main loop
                }
                word = strtok(NULL, " ");
            }
            if (!Usr1W) {//notify server that searching has finished
                if (Util::sendCode(worker->writefd, MSGEND) == -1) {
                    printf("Worker %d: processCommand error 2\n", getpid());
                }
            }
            break;
        }
        case MAXCOUNT:
        {
            //get keyword for maxcount command
            char* keyword = Util::getMessage(worker->readfd);
            //search index for maxcount
            KeywordResult file = worker->index.maxCount(keyword);
            //send data to server
            if (Util::sendInteger(worker->writefd, file.count) == -1) {
                printf("Worker %d: processCommand error 4\n", getpid());
            }
            //if word is not in the index send an empty message
            if (file.count == 0) {
                if (Util::sendMessage(worker->writefd, "", 0) == -1) {
                    printf("Worker %d: processCommand error 3\n", getpid());
                }
            } else {//else send data to server
                if (Util::sendMessage(worker->writefd, file.file, strlen(file.file) + 1) == -1) {
                    printf("Worker %d: processCommand error 4\n", getpid());
                }
            }
            break;
        }
        case MINCOUNT:
        {
            char* keyword = Util::getMessage(worker->readfd);
            KeywordResult file = worker->index.minCount(keyword);
            if (Util::sendInteger(worker->writefd, file.count) == -1) {
                printf("Worker %d: processCommand error 5\n", getpid());
            }
            if (file.count == 0) {
                if (Util::sendMessage(worker->writefd, "", 0) == -1) {
                    printf("Worker %d: processCommand error 6\n", getpid());
                }
            } else {
                if (Util::sendMessage(worker->writefd, file.file, strlen(file.file) + 1) == -1) {
                    printf("Worker %d: processCommand error 7\n", getpid());
                }
            }
            break;
        }
        case WC:
            if (Util::sendWC(worker->writefd, worker->totalWNum, worker->totalCNum, worker->totalLNum) == -1) {
                printf("Worker %d: processCommand error 8\n", getpid());
            }
            break;
        default:
            cmd = EXIT;
    }
    return cmd;
}

void sendReady(Worker* worker) {
    if (Util::sendCode(worker->writefd, READY) == -1) {
        printf("Worker %d: sendReady write error\n", getpid());
    }
}

int openPipe(Worker* worker) {
    char p1[20];
    char p2[20];
    sprintf(p1, "%s.%d", FIFO2, getpid());
    sprintf(p2, "%s.%d", FIFO1, getpid());
    struct stat info;
    while (!InterruptW && (worker->writefd = open(p2, O_WRONLY)) < 0);
    while (!InterruptW && (worker->readfd = open(p1, O_RDONLY)) < 0);
    if (InterruptW || worker->writefd < 0 || worker->readfd < 0) {
        return ERROR;
    }
    return 0;
}

void unlinkPipe(Worker* worker) {
    char p1[20];
    char p2[20];
    sprintf(p1, "%s.%d", FIFO2, getpid());
    sprintf(p2, "%s.%d", FIFO1, getpid());
    unlink(p1);
    unlink(p2);
}

void searchWord(Worker* worker, char* word) {
    //get node for word
    IndexNode* node = worker->index.getNode(word, strlen(word));
    if (node != NULL) {
        //for every entry in postinglistentry list
        PostingListEntry* eNode = node->getHead();
        int lastline = -1;
        while (eNode != NULL) {
            //get data and document
            int lineNum = eNode->getLine();
            int length = eNode->getLength();
            int offset = eNode->getOffset();
            MapNode* mNode = eNode->getDocument();
            char* document = mNode->getDoc();
            char* line = new char[length + 1];
            char* path = mNode->getPath();
            strncpy(line, document + offset, length);
            line[length] = '\0';
            //if this is a new line
            if (lastline != lineNum) {
                if (Util::sendCode(worker->writefd, MSG) == -1) {
                    printf("Worker %d: Search word: write error 1\n", getpid());
                }
                if (Util::sendMessage(worker->writefd, path, strlen(path) + 1) == -1) {
                    printf("Worker %d: Search word: write error 2\n", getpid());
                }
                if (Util::sendInteger(worker->writefd, lineNum) == -1) {
                    printf("Worker %d: Search word: write error 3\n", getpid());
                }
                if (Util::sendMessage(worker->writefd, line, length + 1) == -1) {
                    printf("Worker %d: Search word: write error 4\n", getpid());
                }
                lastline = lineNum;
            }
            delete[] line;
            eNode = eNode->getNext();
        }
    }
}

void interrupt(Worker* worker) {
    while (processCommand(worker) != EXIT);
    unlinkPipe(worker);
    destroyWorker(worker);
    exit(0);
}