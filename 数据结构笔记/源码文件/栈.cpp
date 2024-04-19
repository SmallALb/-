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
    stack() {

    }

    ~stack() {
        Node* p = head;
        while (p) {
            Node* np = p->nxt;
            delete p;
            p = np;
        }
        head = nullptr;
    }

    void push(int val) {
        size++;
        if (!head) {
            head = new Node(val);
            return;
        }

        Node* p  = new Node(val, head);
        head = p;
    }

    void pop() {
        if (!head) return;
        size--;
        Node* p = head->nxt;
        delete head;
        head = p;
    }

    int top() const{
        return head->val;
    }

    int getsize() const{
        return size;
    }
private:
    Node* head{nullptr};
    int size{ 0 };
};


class queue {
    struct Node {
        int val;
        Node* nxt;
        Node(int _val, Node* _nxt):val(_val), nxt(_nxt) {}
        Node(int _val):val(_val),nxt(nullptr){}
    };
public:
    queue() {

    }

    ~queue(){
        Node* p = head;
        while (p) {
            Node* np = p->nxt;
            delete p;
            p = np;
        }
        head = nullptr;
    }

    void push(int val) {
        size++;
        if (!end) {
            end = new Node(val);
            head = end;
            return;
        }
        end->nxt = new Node(val);
        end = end->nxt;
    }

    void pop() {
        if (!head) return;
        size--;
        Node* p = head->nxt;
        delete head;
        head = p;
    }

    int top() const{
        return head->val;
    }

    int getsize() const{
        return size;
    }
private:
    Node* head{ nullptr };
    Node* end{ nullptr };
    int size{ 0 };
};
int main() {
    cout<<"stack:"<<endl;
    stack s1;
    s1.push(1);
    s1.push(2);
    cout<<s1.top()<<endl;
    s1.pop();
    cout<<s1.top()<<endl;
    s1.pop();

    cout<<"queue:"<<endl;
    queue q1;
    q1.push(1);
    q1.push(2);
    q1.push(3);
    q1.push(3);
    while (q1.getsize()) {
        cout << q1.top() << " ";
        q1.pop();
    }
}