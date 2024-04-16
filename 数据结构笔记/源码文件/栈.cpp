#include <iostream>
using namespace std;

class stack{
    struct Node {
        int val;
        Node* nxt;
        Node(int _val, Node* _nxt):val(_val), nxt(_nxt) {}
        Node(int _val):val(_val),nxt(nullptr){}
    };
public:
    stack() {}

    ~stack() {}

};