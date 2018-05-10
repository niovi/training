#ifndef TRIE_H
#define TRIE_H

#include <iostream>
#include <stdio.h>
#include "indexNode.h"
#include "postingListEntry.h"
#include "util.h"

struct KeywordResult{
    int count;
    char* file;
};

class Index {
    //pointer to the head of the Index
    IndexNode* head;
public:
    Index();
    ~Index();
    //returns an IndexNode pointer based on the word input
    //if there is no node returns NULL
    IndexNode* getNode(char*, int);
    //inserts/updates an IndexNode pointer
    //MapNode* is used for 
    void insert(int, int, int, char*, int, MapNode*);
    //df command routine
    void df();

    KeywordResult maxCount(char*);
    KeywordResult minCount(char*);
    //    void df(char*, int);
    //tf command routine
    //    void tf(char*, int, int);
    //search command routine for one word
    //    void searchWord(char*, int, int, double);
};

#endif
