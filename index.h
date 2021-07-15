#ifndef INDEX_H_
#define INDEX_H_

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;

class Bucket {
public:
	vector<pair<int, int>> pairs;
	vector<Bucket*> child;
	Bucket *parent;
	Bucket *right;
	bool isLeaf;
	Bucket();
};

class Index{
public:
	Bucket *root;
	Index(int num_rows, vector<int> key, vector<int> value);
	void insert(Bucket *current, int key, int value);
	void split_recursive(Bucket *current);
	void key_query(vector<int> query_keys);
	int search(Bucket *current, int key);
	void range_query(vector<pair<int, int>> query_pairs);
	int find(Bucket *current, int key1, int key2);
	void clear_index();
	void clear(Bucket *current);
};

#endif
