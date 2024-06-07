#include <iostream>

using namespace std;

class AVLTree {
    struct Node {
        int val;
        Node *left, *right;
        Node(int x) {
            val = x;
            left = nullptr;
            right = nullptr;
        }
        Node(int x, Node* l, Node* r) {
            val = x;
            left = l;
            right = r;
        }

    };
public:
    AVLTree(){

    }

    ~AVLTree() {
        de(head);
    }
    //插入调用函数
    void insert(int x) {
        //调用插入，返回一个新的树
        head = MakebalanceInsert(head, x);
    }
    //删除调用函数
    void earse(int x) {
        //调用删除，返回一个新的树
        head = MakebalanceEarse(head, x);
    }

    Node* search(int x) {
        Node* p = head;
        while(p && p->val != x) (x > p->val) ? p = p->right : p = p->left;
        return p;
    }

    void printTree() {
        Pdfs(head);
    }



private:
    //中序输出
    void Pdfs(Node* p) {
        if (!p) return;
        Pdfs(p->left);
        cout<<p->val<<" ";
        Pdfs(p->right);
    }

    void de(Node* p) {
        if (!p) return;
        de(p->right);
        de(p->left);
        delete p;
    }


    Node* findMin(Node* p) {
        while(p && p->left) p = p->left;
        return p;
    }

    Node* MakebalanceInsert(Node* p, int x) {
        if (!p) return new Node(x);
        else if (x > p->val) p->right = MakebalanceInsert(p->right,x);
        else if (x < p->val) p ->left = MakebalanceInsert(p->left, x);
        else return p;

        int balance = getbalance(p);
        if (balance >= -1 && balance <= 1) return p;
        if (balance < -1) {
            if (getbalance(p->right) > 0) p->right = RRoate(p->right);
            return LRoate(p);
        }
        else if (balance > 1) {
             if (getbalance(p->left) < 0) p->left = LRoate(p->left);
             return RRoate(p);
        }

    }

    Node* MakebalanceEarse(Node* p, int x) {
        if (!p) return nullptr;
        if (x < p->val) p->left = MakebalanceEarse(p->left, x);
        else if (x > p->val) p->right = MakebalanceEarse(p->right, x);
        else {
            if (!p->right && !p->left) {
                delete p;
                p = nullptr;
                return nullptr;
            }
            else if (!p->right) {
                Node* tmp = p->left;
                delete p;
                p = nullptr;
                return tmp;
            }
            else if (!p->left) {
                Node* tmp = p->right;
                delete p;
                p = nullptr;
                return tmp;
            }
            Node* s = p->right;
            Node* mi = findMin(s);
            mi->left = p->left;
            delete p;
            p = nullptr;
            return s;
        }
        if (p == nullptr) return nullptr;
        int balance = getbalance(p);
        if (balance < -1) {
            if (getbalance(p->right) > 0) p->right = RRoate(p->right);
            return LRoate(p);
        }
        else if (balance > 1) {
            if (getbalance(p->left) < 0) p->left = LRoate(p->left);
            return RRoate(p);
        }
        return p;
    }

    //右旋
    Node* RRoate(Node* p) {
        if (!p) return p;
        Node* q = p->left;
        p->left = q->right;
        q->right = p;
        return q;
    }
    //左旋
    Node* LRoate(Node* p) {
        if (!p) return p;
        Node* q = p->right;
        p->right = q->left;
        q->left = p;
        return q;
    }

    int getbalance(Node* p) {
        return getheight(p->left) - getheight(p->right);
    }

    int getheight(Node* p) {
        return !p ? 0 : 1 + max(getheight(p->left), getheight(p->right));
    }
private:
    Node* head{nullptr};
};


int main() {
    AVLTree T;
    T.insert(10);
    T.insert(3);
    T.insert(2);
    T.insert(4);
    T.insert(13);
    T.insert(15);
    T.insert(14);
    T.insert(12);
    T.insert(11);
    T.earse(13);
    T.earse(12);
    T.printTree();
    cout<<"\n";
    cout<<T.search(5);

}