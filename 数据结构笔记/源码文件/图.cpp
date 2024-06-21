#include <iostream>

using namespace std;



class graph {
public:
    //图的初始化
    graph(int n): size(n) {
        mp = new bool*[n+1];
        for (int i=0; i<=n; i++) {
            mp[i] = new bool[n+1];
            for (int j=0; j<=n; j++) mp[i][j] = 0;
        }
    }

    ~graph() {
        //内存清理
        for (int i=0; i<=size; i++) {
            delete mp[i];
        }
        delete[] mp;
    }

    //插入路径，就让a -> b 变为1即可
    void insert(int a, int b) {
        mp[a][b] = 1;
    }

    void printGra() {
        for (int i=1; i<=size; i++) {
            for (int j =1; j<=size; j++) cout<<mp[i][j]<<" ";
            cout<<endl;
        }
    }


private:
    int size;
    bool **mp;//这里多存储一行只是因为数组的下标是从0开始的
};
int main() {
    graph G(3);


    G.printGra();

    cout<<endl;

    G.insert(1,2);
    G.insert(2,1);
    G.insert(3,2);

    G.printGra();
}
