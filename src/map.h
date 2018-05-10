#ifndef MAP_H
#define MAP_H

#include <iostream>

#include "mapNode.h"

class Map {
    //the head of the Map list
    MapNode* head;
public:
    Map();
    ~Map();
    //add a new document to the list
    MapNode* add(char*, int, char*, int);
    MapNode* add(MapNode*);
    void print();

};


#endif
