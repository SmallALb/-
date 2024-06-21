#include <iostream>

using namespace std;



class graph {
public:
    //ͼ�ĳ�ʼ��
    graph(int n): size(n) {
        mp = new bool*[n+1];
        for (int i=0; i<=n; i++) {
            mp[i] = new bool[n+1];
            for (int j=0; j<=n; j++) mp[i][j] = 0;
        }
    }

    ~graph() {
        //�ڴ�����
        for (int i=0; i<=size; i++) {
            delete mp[i];
        }
        delete[] mp;
    }

    //����·��������a -> b ��Ϊ1����
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
    bool **mp;//�����洢һ��ֻ����Ϊ������±��Ǵ�0��ʼ��
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
