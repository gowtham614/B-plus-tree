#include <iostream>
#include <iomanip>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utility>
#include <math.h>
#include <vector>
using namespace std;

int m; //order of the B+ tree

struct node {
	int size; //number of key + 1 or think as #child pointers present = degree
	vector<node*> child; //child pointers
	vector<pair<double, vector<string> > > keys;
	node* next; //only leaf will have this
	node* prev; //only leaf will have this
	node* par; //parent pointer
	bool leaf;
};

void printvec_out(vector<string> &value, ofstream &outfile) {
	// helper function; prints the vector to the output stream
	int print = 0;
	for (vector<string>::iterator it = value.begin(); it != value.end(); ++it) {
		if (print++)
			outfile << ", ";
		outfile << *it;
	}
	outfile << endl;
}

void printvec(vector<string> &value) {
	//helper function; prints the vector
	int print = 0;
	for (vector<string>::iterator it = value.begin(); it != value.end(); ++it) {
		if (print++)
			cout << ", ";
		cout << *it;
	}
	cout << endl;
}

void printnode(node* head) {
	//helper function prints single node
	int size = head->size - 1;
	for (int i = 0; i < size; i++) {
		cout << "key = " << head->keys[i].first;
		cout << "  value = ";
		printvec(head->keys[i].second);
	}
	cout << endl;
}

void recprint(node* head) {
	// used for debugging; recursively prints the tree rooted at head
	printnode(head);
	vector<node*> child = head->child;
	for (vector<node*>::iterator it = child.begin(); it != child.end(); ++it) {
		recprint(*it);
	}
}

void printpar(node* head) {
	// used for debugging; recursively prints all the parent starting from head
	vector<node*> child = head->child;
	for (vector<node*>::iterator it = child.begin(); it != child.end(); ++it) {
		printnode(*it);
		node *x = *it;
		cout << "parent = ";
		printnode(x->par);
		printpar(x);
	}
}

node* findnode(node *head, double key) {
	// helper function which is used for finding the node where
	// key is present

	//base case
	if (head->leaf)
		return head;
	int size = head->size - 1;
	int found = 0, i;
	for (i = 0; i < size; i++) {
		if (head->keys[i].first > key) {
			found = 1;
			break;
		}
	}
	if (!found)
		i = size;
//	cout<<"i = "<<i<<endl;

	// recursive call;
	findnode(head->child[i], key);
}

int search(node *head, double key, ofstream &outfile) {
	// finds the node at which key is present and prints the values
	node* x = findnode(head, key);
	int size = x->size - 1;
	int found = 0;
//	cout<<"search fucn\n";
//	printnode(x);
	for (int i = 0; i < size; i++) {
		if (x->keys[i].first == key) {
			printvec_out(x->keys[i].second, outfile);
			found = 1;
			break;
		}
	}
	if (found == 0)
		outfile << "Null\n";
	return found;
}

int printkey(double key, vector<string> value, int print, ofstream &outfile) {
	for (vector<string>::iterator it = value.begin(); it != value.end(); ++it) {
		if (print++)
			outfile << ", ";
		outfile << "(" << key << "," << *it << ")";
	}
	return print;
}

void search_range(node *head, double key1, double key2, ofstream &outfile) {
	// finds the node where the key1 is present and starts printing the keys
	// key1 <= keys <= key2
	node* x = findnode(head, key1);
	int startPrint = 0;
	int finish = 0;
	int found = 0;
	while (x) {
		int size = x->size - 1;
		for (int i = 0; i < size; i++) {
			double key = x->keys[i].first;
			if (key < key1)
				continue;
			else if (key > key2) {
				finish = 1;
				break;
			} else if ((key1 <= key) && (key <= key2)) {
				found = 1;
				startPrint = printkey(key, x->keys[i].second, startPrint,
						outfile);
			}
		}
		if (finish)
			break;
		x = x->next;
	}
	if (found == 0)
		outfile << "Null";
	outfile << endl; //just for printing in one line
}

void insertPar(node* x, pair<double, vector<string> > p, node* childnode,
		node* & head) {
	//cout << "$$$insertPar\n";
	// pair p is inserted in x.keys and child node in x.child
	double key = p.first;
	int size = x->size - 1;
	int i;
	for (i = 0; i < size; i++) {
		if (x->keys[i].first > key) {
			break;
		}
	}
	x->keys.insert(x->keys.begin() + i, p);
	x->child.insert(x->child.begin() + i + 1, childnode);
	x->size += 1;

	if (x->size > m) { //split node
		//cout << "split node\n";
		float nodesize = x->size - 1;
		int nNodesize = ceil(nodesize / 2) - 1;
		int eNodesize = nodesize - nNodesize - 1; //1st key is not included in extra

		node * newnode = new node;
		newnode->size = eNodesize + 1;
		vector<node*> newchild(x->child.begin() + nNodesize + 1,
				x->child.end());
		vector<pair<double, vector<string> > > newkeys(
				x->keys.begin() + nNodesize + 1, x->keys.end()); //check this

		newnode->keys = newkeys;
		newnode->child = newchild;
		// changing the parent pointer of newnode->child; due to splitting
		for (vector<node*>::iterator it = newchild.begin();
				it != newchild.end(); ++it) {
			node *child = *it;
			child->par = newnode;
		}
		newnode->leaf = false;

		pair<double, vector<string> > value = x->keys[nNodesize];

		x->size = nNodesize + 1;
		x->keys.erase(x->keys.begin() + nNodesize, x->keys.end()); //check this
		x->child.erase(x->child.begin() + nNodesize + 1, x->child.end());

		if (x->par) {
			newnode->par = x->par;
			// recursively insert in parent
			insertPar(x->par, value, newnode, head);
		} else {
			//x.par doesn't exist; x is root, create new root; basecase for recursion
//			cout << "root node\n";
			node *root = new node;
			root->size = 2;
			root->par = NULL;
			root->leaf = false;
			root->next = NULL;
			root->prev = NULL;
			root->keys.push_back(value);
			root->child.push_back(x);
			root->child.push_back(newnode);
			x->par = root;
			newnode->par = root;
			head = root;
		}
	}
}

void insertLeaf(node * x, int i, double key, string value, node* &head) {
	//inserts the key in the leaf at index i
//	cout << "$$$insertleaf called\n";
	vector<string> temp;
	temp.push_back(value);
	pair<double, vector<string> > p(key, temp);

	x->keys.insert(x->keys.begin() + i, p);
	x->size += 1;

	if (x->size > m) { //split leaf
//		cout << "$$$split leaf\n";
		float nodesize = x->size - 1;
		int nNodesize = ceil(nodesize / 2) - 1;
		int eNodesize = nodesize - nNodesize;

		node * newnode = new node;
		newnode->size = eNodesize + 1;
		vector<pair<double, vector<string> > > newkeys(
				x->keys.begin() + nNodesize, x->keys.end()); //check this
		newnode->keys = newkeys;
		newnode->leaf = true;
		newnode->par = x->par;

		//newnode->keys[0] new value to be inserted in parent of x

		x->size = nNodesize + 1;
		x->keys.erase(x->keys.begin() + nNodesize, x->keys.end()); //check this

		//y may not exist
		if (x->next) {
			// creating doubly linked list; case where there is no next node
//			cout << "y exist \n";
			node*y = x->next;
			x->next = newnode;
			y->prev = newnode;
			newnode->prev = x;
			newnode->next = y;
		} else {
			//creating doubly linked list
//			cout << "y not exit\n";
			x->next = newnode;
			newnode->prev = x;
			newnode->next = NULL;
		}
		vector<string> temp;
		pair<double, vector<string> > p(newnode->keys[0].first, temp);
//		cout << "par value inserted = " << p.first << endl;
		if (x->par)
			insertPar(x->par, p, newnode, head);
		else { //create root
			// parent doesn't exist; given node is root
			node *root = new node;
			root->size = 2;
			root->par = NULL;
			root->leaf = false;
			root->next = NULL;
			root->prev = NULL;
			root->keys.push_back(p);
			root->child.push_back(x);
			root->child.push_back(newnode);
			x->par = root;
			newnode->par = root;
			head = root;

		}
	}
}

void insert(node* &head, double key, string value) {
	node* x = findnode(head, key); // find which node to be inserted
	int size = x->size - 1;
	int found = 0, i;
	for (i = 0; i < size; i++) {
		if (x->keys[i].first == key) { //duplicate key
//			cout << "duplicate key, key = " << key << endl;
			found = 1;
			x->keys[i].second.push_back(value);
			break;
		} else if (x->keys[i].first > key) {
			break;
		}
	}
	if (found == 0) { //new key
//		cout << "$$$$ newkey, i = " << i << endl;
		insertLeaf(x, i, key, value, head);
	}
}

void parseIns(string x, node* &head) {
	//parse insert it calls insert
	size_t pos1 = x.find("(");
	size_t pos2 = x.find(",");
	size_t pos3 = x.find(")");

	string temp = x.substr(pos1 + 1, pos2 - pos1 - 1);
	double key = atof(temp.c_str());
	string value = x.substr(pos2 + 1, pos3 - pos2 - 1);

	//cout << "INS  " << ",key = " << key << "  " << ",value = " << value << endl;
	insert(head, key, value);
}

void parseSearch(string x, bool par2, node*head, ofstream &outfile) {
	//paresearch it calls the search or search_range based on the parameterss
	size_t pos1 = x.find("(");
	size_t pos2 = x.find(",");
	size_t pos3 = x.find(")");

	if (par2) {
//		cout << "SRH  2 param  ";
		string temp = x.substr(pos1 + 1, pos2 - pos1 - 1);
		double key1 = atof(temp.c_str());
//		cout << "key1 = " << key1 << "  ";

		temp = x.substr(pos2 + 1, pos3 - pos2 - 1);
		double key2 = atof(temp.c_str());
//		cout << "key2 = " << key2 << endl;
		search_range(head, key1, key2, outfile);
	} else { //single parameter
//		cout << "SRH  ";
		string temp = x.substr(pos1 + 1, pos3 - pos1 - 1);
		double key = atof(temp.c_str());
//		cout << "key = " << key << endl;
		search(head, key, outfile);
	}
}

void parse(ifstream &f, node* &head, ofstream &outfile) { // basic input parsing
	string linebuffer;
	getline(f, linebuffer);
	m = atoi(linebuffer.c_str());
	//cout << "m = " << m << endl;

	while (f && getline(f, linebuffer)) {

		size_t pos1 = linebuffer.find("(");
		size_t pos2 = linebuffer.find(",");
		size_t pos3 = linebuffer.find(")");
		bool par2 = true;
		if (pos2 == string::npos) { // ',' notfound
			par2 = false;
		}
		string cmd = linebuffer.substr(0, pos1);
		if (cmd == "Insert")
			parseIns(linebuffer, head);
		else if (cmd == "Search")
			parseSearch(linebuffer, par2, head, outfile);
	}
}

void recdel(node* head) { //deleting the node recursively; cleanup
	if (!head->leaf) {
		for (int i = 0; i < head->size; i++) {
			recdel(head->child[i]);
		}
	}
	delete head;
}

int main(int argc, char** argv) {

	ifstream f;
	f.open(argv[1]);
	node *head = new node;
	ofstream outfile("output_file.txt");
	head->size = 1;
	head->next = NULL;
	head->prev = NULL;
	head->par = NULL;
	head->leaf = true;
	parse(f, head, outfile);
	f.close();
	recdel(head);

	return 0;
}
