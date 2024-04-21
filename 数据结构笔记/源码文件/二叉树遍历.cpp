#include <iostream>
#include <stack>
#include <queue>
#include <string>
using namespace std;


struct Node {
    int val;
    Node* left;
    Node* right;
    Node(int v) {
        val = v;
        left = nullptr;
        right = nullptr;
    }

    Node(int v, Node* l, Node* r) {
        val = v;
        left = l;
        right = r;
    }
};


void P_dfs(Node* h) {
    if (!h) return;
    cout<<h->val<<endl;
    P_dfs(h->left);
    P_dfs(h->right);
}

void M_dfs(Node* h) {
    if (!h) return;
    P_dfs(h->left);
    cout<<h->val<<endl;
    P_dfs(h->right);
}

void B_dfs(Node* h) {
    if (!h) return;
    P_dfs(h->left);
    P_dfs(h->right);
    cout<<h->val<<endl;
}

void DFS(Node* h) {
    stack<Node*> s1;
    s1.push(h);
    while(!s1.empty()) {
        Node* now = s1.top();
        s1.pop();
        cout<<now->val<<endl;
        if (now->right != nullptr) s1.push(now->right);
        if (now->left != nullptr) s1.push(now->left);
    }
}

void BFS(Node* h) {
    int nowNodes = 1;
    int nxtNodes = 0;
    queue<Node*> q1;
    q1.push(h);
    while(!q1.empty()) {
        Node* now = q1.front();
        q1.pop();
        cout<<now->val;
        if (now->left != nullptr) {
            q1.push(now->left);
            nxtNodes++;
        }
        if (now->right != nullptr) {
            q1.push(now->right);
            nxtNodes++;
        }
        nowNodes--;
        if (!nowNodes) {
            cout<<endl;
            nowNodes = nxtNodes;
            nxtNodes = 0;
        }else {
            cout<<" ";
        }
    }
}
int main() {
    Node* A = new Node(10);
    Node* B = new Node(200);
    Node* C = new Node(20);
    Node* D = new Node(600);
    Node* E = new Node(1000);
    Node* F= new Node(300);
    A->left = B;
    A->right = C;
    B->left = D;
    B->right = E;
    C->left = F;
    BFS(A);

    delete A;
    delete B;
    delete C;
    delete D;
    delete E;

}