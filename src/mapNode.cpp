#include "mapNode.h"

MapNode::MapNode() {
//    std::cout << "MapNode Default Constructor" << std::endl;
    this->path = NULL;
    this->plen = -1;
    this->doc = NULL;
    this->len = -1;
    this->next = NULL;
    this->score = 0;
}

MapNode::MapNode(char* path, int plen, char* doc, int dlen) {
    this->path = new char[plen];
    strncpy(this->path, path, plen);
    this->plen = plen;
    this->doc = new char[dlen];
    strncpy(this->doc, doc, dlen);
    this->len = dlen;
    this-> next = NULL;
    this->score = 0;
}

MapNode::~MapNode() {
//    std::cout << "MapNode Destructor" << std::endl;
    if (this->doc != NULL) {
        delete[] this->doc;
        this->doc = NULL;
    }
    if (this->path != NULL) {
        delete[] this->path;
        this->path = NULL;
    }
    if (this->next != NULL) {
        delete this->next;
        this->next = NULL;
    }
}

int MapNode::getWordCount() {
    return this->wordCount;
}

void MapNode::setWordCount(int count) {
    this->wordCount = count;
}

char* MapNode::getPath() {
    return this->path;
}

void MapNode::setPath(char* path, int plen) {
    if (this->path == NULL) {
        this->path = new char[plen + 1];
        strncpy(this->path, path, plen);
        this->path[plen] = '\0';
    } else {
        if (plen > this->plen) {
            realloc(this->path, plen + 1);
        }
        strncpy(this->path, path, plen);
        this->path[plen] = '\0';
    }
}

char* MapNode::getDoc() {
    return this->doc;
}

void MapNode::setDoc(char* doc, int dlen) {
    if (this->doc == NULL) {
        this->doc = new char[dlen + 1];
        strncpy(this->doc, doc, dlen);
        this->doc[dlen] = '\0';
    } else {
        if (dlen > this->len) {
            realloc(this->doc, dlen + 1);
        }
        strncpy(this->doc, doc, dlen);
        this->doc[dlen] = '\0';
    }
}

int MapNode::getLen() {
    return this->len;
}

MapNode* MapNode::getNext() {
    return this->next;
}

void MapNode::setNext(MapNode* node) {
    this->next = node;
}

double MapNode::getScore() {
    return this->score;
}

void MapNode::setScore(double score) {
    this->score = score;
}
