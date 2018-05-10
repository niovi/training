#ifndef TRIENODE_H
#define TRIENODE_H

#include <iostream>

#include "postingListEntry.h"

class IndexNode {
    char* word = NULL;
    char key;
    IndexNode* next;
    IndexNode* child;
    PostingListEntry* head;
public:
    IndexNode();
    IndexNode(char);
    ~IndexNode();

    char getKey();
    void setKey(char);
    IndexNode* getNext();
    void setNext(IndexNode*);
    IndexNode* getChild();
    void setChild(IndexNode*);
    char* getWord();
    void setWord(char*);
    PostingListEntry* getHead();
    void setHead(PostingListEntry*);
    //print the document frecuency for every word
    void printDfs();
    //print df for only this node
    //    void printDfsSingle();
    //add/update node
    void add(int, int, int, char*, int, MapNode*);


};

#endif


