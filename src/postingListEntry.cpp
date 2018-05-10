#include "postingListEntry.h"

PostingListEntry::PostingListEntry() {
    //std::cout << "Pair Default Constructor" << std::endl;
    this->node = NULL;
    this->next = NULL;
}

PostingListEntry::PostingListEntry(int line, int offset, int length, MapNode* node) {
    //std::cout << "Pair Constructor" << std::endl;
    this->length = length;
    this->line = line;
    this->offset = offset;
    this->node = node;
    this->next = NULL;
}

PostingListEntry::~PostingListEntry() {
    //std::cout << "Pair Destructor" << std::endl;
    if (this->next != NULL) {
        delete this->next;
        this->next = NULL;
    }
    this->node = NULL;
}

MapNode* PostingListEntry::getDocument() {
    return this->node;
}

PostingListEntry* PostingListEntry::getNext() {
    return this->next;
}

void PostingListEntry::setNext(PostingListEntry* next) {
    this->next = next;
}

int PostingListEntry::getLine() {
    return this->line;
}

int PostingListEntry::getLength() {
    return this->length;
}

int PostingListEntry::getOffset() {
    return this->offset;
}
