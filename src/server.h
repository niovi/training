#ifndef SERVER_H
#define SERVER_H
#include <poll.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/ioctl.h>


#include "util.h"
#include "worker.h"

#define PERMS   0666

extern volatile sig_atomic_t ChldKilled;
extern volatile sig_atomic_t Interrupt;
extern volatile sig_atomic_t Pipe;

typedef struct {
    //array for read file descriptors
    int* readfd;
    //array for write file descriptors
    int* writefd;
    //array with catalogs
    char** paths;
    //number of catalogs
    int pathNum;
    //array for worker pids
    int* child_pid;
    //number of workers
    int wNum;
} Server;

//creates Server and initializes variables and handlers 
Server* createServer(int);
//frees Server's dynamic allocated variables
void destroyServer(Server*);
//wait for all worker processes to exit
void waitWorkersToExit(Server*);
//initialize the server
void initializeServer(Server*);
//start server loop
void startServer(Server*);
//creates new worker processes 
void createWorkers(Server*);
//sends a SIGINT to every worker
void destroyWorkers(Server*);
//sends a SIGINT worker with child_pid[index]
void destroyWorker(Server*, int);
//replaces a worker that have been killed with a new one
//if replacement happend during a wc,maxcount or mincount command
//send the command to the worker
void replaceWorker(Server*, int, char*, pid_t);
//replaces all workers that have been killed with a new one
//if replacement happend during a wc,maxcount or mincount command
//send the command to workers
void replaceWorkers(Server*, int, char*);
//sends catalogs to workers
void initializeWorkers(Server*, char*);
//sends catalogs to worker with child_pid[index]
void initializeWorker(Server*, int);
//initialize signal handlers
void initializeSignHandlersS();
//wait for a message from worker saying their work is done
void waitWorkers(Server*);
//wait for a message from worker with index saying their work is done
void waitWorker(Server*, int);
//sends command to workers
void sendCommand(Server*, char*, char*);
//read and forward command from stdin to workers
char* readCommand(Server*);
//create a pipe with pathname from server 
void createPipe(Server*, char path[2][20]);
//open pipe from server with a pathname for worker with child_pid[index]
void openPipe(Server*, char[2][20], int index);
//unlink pipe with worker
void unlinkPipe(pid_t wpid);
//close pipe with child_pid[index]
void closePipe(Server*, int index);
//close all open pipes
void closePipes(Server*);
//initialize pipes
void initializePipes(Server*);
//get results for wc command from workers
void getWCResults(Server*);
//get results for maxcount and mincount command from workers
void getMaxMinResults(Server*, int, char*);
//get results for search command from workers
void getSearchResults(Server*, int);

#endif /* SERVER_H */

