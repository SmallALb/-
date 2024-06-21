#include <iostream>

using namespace std;

class graph {
    struct Node {
        Node();
        Node(int nx): to(nx) {}
        int to;
        Node* nxt{nullptr};
    };
public:
    graph(int n): size(n) {
        mp = new Node*[n+1](); //一个道理多开辟一位
    }

    ~graph() {
        for (int i=0; i<=size; i++) {
            dfs(mp[i]);
        }
        delete[] mp;
    }

    void insert(int a, int b){
        //要特判一下，链表头为空的情况
        if (!mp[a]) {
            mp[a] = new Node(b);
            return;
        }
        //在链表数组的第a个插入b，代表 a->b有路径
        Node* p = mp[a];
        while(p->nxt) p = p->nxt;
        p->nxt = new Node(b);
    }

    void printGra() {
        for (int i=1; i<=size; i++){
            cout<<i<<": ";
            for (Node* p = mp[i]; p!= nullptr; p =p->nxt) cout<<p->to<<" ";
            cout<<"\n";
        }
    }
private:
    //删除用的dfs
    void dfs(Node* p) {
        if (!p) return;
        dfs(p->nxt);
        delete p;
    }

private:
    int size;
    Node** mp;
};


int main() {
    graph G(5);

    G.insert(1,2);
    G.printGra();

}