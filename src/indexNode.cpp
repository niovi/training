#include "indexNode.h"
#include "postingListEntry.h"

IndexNode::IndexNode() {
    //std::cout << "IndexNode Default Constructor" << std::endl;
    this->key = '\0';
    this->next = NULL;
    this->child = NULL;
    this->head = NULL;
    this->word = NULL;
}

IndexNode::IndexNode(char key) {
    //std::cout << "IndexNode Constructor" << std::endl;
    this->key = key;
    this->next = NULL;
    this->child = NULL;
    this->head = NULL;
    this->word = NULL;
}

IndexNode::~IndexNode() {
    //std::cout << "IndexNode Destructor" << std::endl;
    if (this->word != NULL) {
        delete[] this->word;
        this->word = NULL;
    }
    if (this->head != NULL) {
        delete this->head;
        this->head = NULL;
    }
    if (this->child != NULL) {
        delete this->child;
        this->child = NULL;
    }
    if (this->next != NULL) {
        delete this->next;
        this->next = NULL;
    }



}

//void IndexNode::printDfsSingle() {
//    if (this->word != NULL) {
//        std::cout << this->word << " " << this->df << std::endl;
//    }
//}

void IndexNode::printDfs() {
    if (this->word != NULL) {
        std::cout << this->word << std::endl;
    }
    //call this method for its first child
    if (this->child != NULL) {
        this->child->printDfs();
    }
    //call this method for every "brother" node
    if (this->next != NULL) {
        this->next->printDfs();
    }
}

void IndexNode::add(int line, int offset, int llength, char* word, int len, MapNode* node) {
    PostingListEntry* newEntry = new PostingListEntry(line, offset, llength, node);
    PostingListEntry* entry = this->head;

    //if this is the first time this word was found in a document
    if (entry == NULL) {
        //create a new pair as its pairList head
        this->head = new PostingListEntry(line, offset, llength, node);
        //copy word and return
        this->word = new char[len + 1];
        strncpy(this->word, word, len);
        this->word[len] = '\0';
        return;
    } else {
        while (entry->getNext() != NULL) {
            entry = entry->getNext();
        }
    }
    entry->setNext(newEntry);
}

char* IndexNode::getWord() {
    return this->word;
}

char IndexNode::getKey() {
    return key;
}

void IndexNode::setKey(char key) {
    this->key = key;
}

IndexNode* IndexNode::getNext() {
    return this->next;
}

void IndexNode::setNext(IndexNode* node) {
    this->next = node;
}

IndexNode* IndexNode::getChild() {
    return this->child;
}

void IndexNode::setChild(IndexNode *node) {
    this->child = node;

}

PostingListEntry* IndexNode::getHead() {
    return this->head;
}

void IndexNode::setHead(PostingListEntry* head) {
    this->head = head;
}
