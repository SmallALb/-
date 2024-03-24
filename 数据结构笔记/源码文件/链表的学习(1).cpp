#include <iostream>
using namespace std;
//结构体定义 
struct Node {
	int val;
	Node* nxt;
};
//打印节点 
void printNode(Node* h) {
	Node* p = h;
	while(p) {
		cout<<p->val<<"-->";
		p = p->nxt;
	}
}
//批量创建 
Node* make_NodeList(int num) {
	Node* head = new Node;
	Node* p = head;
	for (int i=0; i<num; i++) {
		int val; cin>>val;
		p->nxt = new Node;
		p->nxt->val = val;
		p = p->nxt;
	}
	p->nxt = nullptr;
	Node* nhead = head->nxt;
	delete head;
	return nhead;
}
//递归的批量创建 
void make_NodeList2(int num, Node* &h) {
	if (num == 0) {
		h = nullptr;
		return;
	}
	int val; cin>>val;
	h = new Node;
	h->val = val;
	make_NodeList2(num-1, h->nxt);
}
//删除整个链表 
void deleteNodelist(Node* head) {
	Node* lp = head;
	Node* np = head->nxt;
	while(lp) {
		cout<<lp->val<<" deleted"<<endl;
		delete lp;
		lp = np;
		if (np) np = np->nxt;
	}
}
//递归的输出整个链表 
void printNode2(Node* p) {
	if (!p) return;
	cout<<p->val<<"-->";
	printNode2(p->nxt);
}
//前插 
Node* push_front(Node* p, int num) {
	Node* np = new Node;
	np->val = num;
	np->nxt = p;
	return np;
}
//后插 
void push_back(Node* h, int num) {
	Node* p = h;
	while(p->nxt) {
		p = p->nxt; 
	}
	p->nxt = new Node;
	p->nxt->val = num;
	p->nxt->nxt = nullptr;
	
}
//中插 
void insert(Node* h, int n, int num) {
	Node* lp = h;
	Node* np = h->nxt;
	for (int i = 1; i<n && lp; i++) {
		lp = np;
		np = np->nxt;
	}
	if (lp == nullptr) return;
	Node* x = new Node;
	x->val = num;
	x->nxt = np;
	lp->nxt = x;
}
//前删 
Node* pop_front(Node* h) {
	Node* p = h->nxt;
	delete h;
	return p;
}
//后删 
void pop_back(Node* h) {
	Node* p = h;
	while(p->nxt->nxt) {
		p = p->nxt;
	}
	delete p->nxt;
	p->nxt = nullptr;
}
//中间删除 
void deleVal(Node* h, int val) {
	if (h->val == val) {
		pop_front(h);
		return;
	}
	Node* p = h;
	while(p->nxt->val != val && p->nxt) {
		p = p->nxt;
	}
	if (!p->nxt) return;
	Node* np = p->nxt->nxt;
	delete np->nxt;
	p->nxt = np;
}


 int main() {
	//调用函数 
 }
 
 
 
 
 
