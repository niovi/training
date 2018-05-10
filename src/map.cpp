#include "map.h"

Map::Map() {
    //std::cout << "Map Default Constructor" << std::endl;
    this->head = NULL;
}

Map::~Map() {
//    std::cout << "Map Destructor" << std::endl;
    if (this->head != NULL) {
        delete this->head;
        this->head = NULL;
    }
}

MapNode* Map::add(char* path, int plen, char* doc, int len) {
    //create a new MapNode for the document
    MapNode* newNode = new MapNode(path, plen, doc, len);
    MapNode* node = head;
    if (this->head == NULL) {
        //if Map was empty create a new entry and point head
        this->head = newNode;
        return this->head;
    } else {
        //else go to the last element of the list
        while (node->getNext() != NULL) {
            node = node->getNext();
        }
    }
    //set the new node as the last node in the list
    node->setNext(newNode);
    return newNode;
}

MapNode* Map::add(MapNode* newNode) {
    MapNode* node = head;
    if (this->head == NULL) {
        //if Map was empty create a new entry and point head
        this->head = newNode;
        return this->head;
    } else {
        //else go to the last element of the list
        while (node->getNext() != NULL) {
            node = node->getNext();
        }
    }
    //set the new node as the last node in the list
    node->setNext(newNode);
    return newNode;
}

void Map::print() {
    MapNode* node = head;
    //for every document in the Map print the path
    while (node != NULL) {
        std::cout << node->getPath() << std::endl;
        node = node->getNext();
    }
}
