#include "index.h"

Index::Index() {
    this->head = new IndexNode();
}

Index::~Index() {
//    std::cout << "Index Destructor" << std::endl;
    if (this->head != NULL) {
        delete this->head;
        this->head = NULL;
    }
}

void Index::insert(int line, int offset, int llength, char* word, int len, MapNode* mapNode) {
    //if the head of the index has no children (first entry case)
    //create a new node as the head's child
    if (this->head->getChild() == NULL) {
        IndexNode* newNode = new IndexNode(word[0]);
        head->setChild(newNode);
    }

    IndexNode* node = this->head->getChild();
    int i = 0;
    //while there are still leters to look up
    while (i < len) {
        //if the node has the same key as the character in position i
        if (node->getKey() == word[i]) {
            //if its the last character
            if (i == len - 1) {
                //add the new entry to the node and return
                node->add(line, offset, llength, word, len, mapNode);
                return;
            } else {
                //if the node has no children
                if (node->getChild() == NULL) {
                    //create a new node as the nodes child
                    IndexNode* newNode = new IndexNode(word[i + 1]);
                    node->setChild(newNode);
                    node = newNode;
                } else {
                    //else check the children of the node
                    node = node->getChild();
                }
                i++;
            }
        } else {
            //if the node doesn't have the same key as the character
            //check the other children
            if (node->getNext() == NULL) {
                //if there are no more "brother" nodes 
                //create a new one, set its key as the character we are 
                //looking for and continue
                IndexNode* newNode = new IndexNode(word[i]);
                node->setNext(newNode);
                node = newNode;
            } else {
                //else just check the next "brother" node
                node = node->getNext();
            }
        }
    }
}

IndexNode* Index::getNode(char* word, int len) {
    //start from head's first child
    IndexNode* node = this->head->getChild();
    int i = 0;
    //for every letter in word
    while (i < len) {
        //if there is no more nodes to check return NULL
        if (node == NULL) {
            return NULL;
        } else if (node->getKey() == word[i]) {
            //if the node has the same key as the character in pos i
            if (i == len - 1) {//if its the last character return the node
                return node;
            }
            //else check its children
            node = node->getChild();
            i++;
        } else {
            //if it does not have the same key check the other children
            node = node->getNext();
        }
    }
    //return the node (may be NULL)
    return node;
}

KeywordResult Index::maxCount(char* word) {
    KeywordResult r;
    IndexNode* node = getNode(word, strlen(word));
    if (node == NULL) {
        r.count = 0;
        r.file = NULL;
        return r;
    }
    PostingListEntry* plEntry = node->getHead();
    MapNode* mN = plEntry->getDocument();
    int count = 1;
    int maxCount = 0;
    MapNode* max = mN;
    while (plEntry->getNext() != NULL) {
        plEntry = plEntry->getNext();
        if (plEntry->getDocument() == mN) {
            count++;
        } else if (count > maxCount ||
                (count == maxCount
                && strlen(plEntry->getDocument()->getPath())
                < strlen(max->getPath()))) {
            maxCount = count;
            max = mN;
            count = 1;
            mN = plEntry->getDocument();
        } else {
            mN = plEntry->getDocument();
            count = 1;
        }
    }
    if (count > maxCount ||
            (count == maxCount
            && strlen(plEntry->getDocument()->getPath())
            < strlen(max->getPath()))) {
        r.count = count;
        r.file = plEntry->getDocument()->getPath();
        return r;
    }

    r.count = maxCount;
    r.file = max->getPath();
    return r;
}

KeywordResult Index::minCount(char* word) {
    KeywordResult r;
    IndexNode* node = getNode(word, strlen(word));
    if (node == NULL) {
        r.count = 0;
        r.file = NULL;
        return r;
    }
    PostingListEntry* plEntry = node->getHead();
    MapNode* mN = plEntry->getDocument();
    int count = 1;
    int minCount = -1;
    MapNode* max = mN;
    while (plEntry->getNext() != NULL) {
        plEntry = plEntry->getNext();
        if (plEntry->getDocument() == mN) {
            count++;
        } else if (count < minCount ||
                (count == minCount
                && strlen(plEntry->getDocument()->getPath())
                < strlen(max->getPath())) || minCount == -1) {
            minCount = count;
            max = mN;
            count = 1;
            mN = plEntry->getDocument();
        } else {
            mN = plEntry->getDocument();
            count = 1;
        }
    }
    if (count < minCount ||
            (count == minCount
            && strlen(plEntry->getDocument()->getPath())
            < strlen(max->getPath()))) {
        r.count = count;
        r.file = plEntry->getDocument()->getPath();
        return r;
    }
    r.count = minCount;
    r.file = max->getPath();
    return r;
}

//void Index::tf(char* word, int len, int doc) {
//    //get the node that represents the word if any
//    IndexNode* node = getNode(word, len);
//    if (node != NULL) {
//        //for evert pair (aka (id,frequency)) in the node
//        PostingListEntry* p = node->getHead();
//        while (p != NULL) {
//            //if there is an entry with id=docId print and return
//            if (p->getKey() == doc) {
//                std::cout << doc << " " << node->getWord() << " "
//                        << p->getValue() << std::endl;
//                return;
//            }
//            //else check the next node
//            p = p->getNext();
//        }
//    }
//    printf("%d %.*s 0\n", doc, len - 1, word);
//
//}

//void Index::df(char* word, int len) {
//    //get the node that represents the word if any
//    IndexNode* node = getNode(word, len);
//    if (node != NULL) {
//        //if the node exists print the frequency
//        node->printDfsSingle();
//    } else
//        std::cout << "No word" << std::endl;
//}

void Index::df() {
    IndexNode* node = this->head->getChild();
    node->printDfs();
}

//void Index::searchWord(char* word, int len, int N, double avgdl) {
//    //get the node that represents the word if any
//    IndexNode* node = this->getNode(word, len);
//    if (node != NULL) {
//        //if the node exists check its pairs (aka (id,docFrequency))
//        PostingListEntry* p = node->getHead();
//        while (p != NULL) {
//            int id = p->getKey();
//            int wdf = p->getValue();
//            int df = node->getDf();
//            int D = p->getDocument()->getWordCount();
//            //calculate the score for this word
//            double newScore
//                    = Util::score(N, df, wdf, D, avgdl);
//            //get the pointer to the Map for the particular document
//            MapNode* mapNode = p->getDocument();
//            //update score
//            mapNode->setScore(mapNode->getScore() + newScore);
//            //proceed to the next pair
//            p = p->getNext();
//        }
//    }
//}
