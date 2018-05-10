#ifndef MAPNODE_H
#define MAPNODE_H
#include <iostream>
#include <cstring>
#include <cstdlib>

class MapNode {
    //the path of the file
    char* path;
    //path length
    int plen;
    //document data
    char* doc;
    //document length
    int len;
    //document score(it is set to 0 after every search)
    double score;
    //number of words in document
    int wordCount;
    //next element in the list
    MapNode* next;
public:
    MapNode();
    MapNode(char*, int, char*, int);
    ~MapNode();

    char* getPath();
    void setPath(char*, int);
    char* getDoc();
    void setDoc(char*, int);
    int getLen();
    MapNode* getNext();
    void setNext(MapNode*);
    void setWordCount(int);
    int getWordCount();
    double getScore();
    void setScore(double);
};


#endif
