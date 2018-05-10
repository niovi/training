#ifndef PAIR_H
#define PAIR_H
#include <iostream>

#include "mapNode.h"

class PostingListEntry {
    //document line
    int line;
    //document line offset
    int offset;
    //document line length
    int length;
    //pointer to the document in the map for quick access and for use
    //of the score value
    MapNode* node;
    //next element in the list
    PostingListEntry* next;
public:
    PostingListEntry();
    PostingListEntry(int, int, int, MapNode*);
    ~PostingListEntry();
    PostingListEntry* getNext();
    void setNext(PostingListEntry*);
    int getLine();
    int getOffset();
    int getLength();
    MapNode* getDocument();
};

#endif
