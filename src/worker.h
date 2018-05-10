#ifndef WORKER_H
#define WORKER_H
#include <dirent.h>

#include "util.h"
#include "map.h"
#include "index.h"

//1 when a SIGUSR1 is caught
extern volatile sig_atomic_t Usr1W;
//1 when an SIGINT (cntr+c) signal is caught
extern volatile sig_atomic_t InterruptW;

typedef struct {
    int readfd;
    int writefd;
    Map map;
    Index index;
    int totalWNum;
    int totalCNum;
    int totalLNum;
} Worker;

//initializes signal handlers for worker process
void initializeSignHandlersW();
//initialize worker (pipes,catalogs)
int initialize(Worker*);
//start worker main loop
void startWorker(Worker*);
//destroy worker
void destroyWorker(Worker*);
//process catalog
void processCatalog(Worker*, char*);
//process file
void processFile(Worker*, char*);
//opens pipe
int openPipe(Worker*);
//unlinks pipe
void unlinkPipe(Worker*);
//searches word from index and sends data to server
void searchWord(Worker*, char*);
//notify server that worker is ready to process commands
void sendReady(Worker*);
//stop searching and go to main loop
void interrupt(Worker*);
//get command from server and process
int processCommand(Worker*);

#endif /* WORKER_H */

