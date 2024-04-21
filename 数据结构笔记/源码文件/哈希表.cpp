#include <iostream>
using namespace std;


class Mhash {
    struct Node {
        int val;
        Node* nxt{nullptr};
        Node() {

        }
        Node(int v) {
            val = v;
            nxt = nullptr;
        }

    };
public:
    Mhash() {
        map = new Node*[13]();
    }
    ~Mhash() {
        for (int i=0; i<7; i++) {
            Node* p = map[i];
            while(p) {
                Node* np = p->nxt;
                delete p;
                p = np;
            }
        }
        delete[] map;
    }

    void insert(int val) {
        int indx = rule(val);
        if (!map[indx]) {
            map[indx] = new Node(val);
            return;
        }
        Node* p = map[indx];
        while(p->nxt) {
            p = p->nxt;
        }
        p->nxt = new Node(val);
    }


    bool find(int val) {
        Node* p = map[rule(val)];
        while (p) {
            if (p->val == val) return 1;
            p = p->nxt;
        }
        return 0;
    }

    void printHash() {
        for (int i =0; i<13; i++) {
            Node* p = map[i];
            while (p) {
                cout<<p->val<<"->";
                p = p->nxt;
            }
            cout<<"null"<<endl;
        }
    };
private:
    inline int rule(int val) { return abs(val % 13); }
private:
    Node** map;
};


int main() {
    Mhash h1;
    for (auto x : {10,21,3,4,5,114,678, 990, 356, 1021, 72}) h1.insert(x);

    cout<<h1.find(10)<<endl;
    cout<<h1.find(114)<<endl;
    cout<<h1.find(514)<<endl;
    h1.printHash();
}