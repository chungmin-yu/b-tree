#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <limits.h>
#include "index.h"

#define order 80

using namespace std;

int n = order * 2;

// Constructor of bucket
Bucket::Bucket() {
	this->parent = nullptr;          
	this->right = nullptr;
	isLeaf = true;                   
};

// Create a B+ Tree index 
Index::Index(int num_rows, vector<int> key, vector<int> value) {
	root = nullptr;
	for (int i = 0; i < num_rows; i++) {
		insert(root, key[i], value[i]);
	}
}

// Insert a key-vlaue pair
void Index::insert(Bucket *current, int key, int value) {
	
	// nothing in the B+ Tree (first insert)
	if (current == nullptr) {
		current = new Bucket;
		current->pairs.push_back(make_pair(key, value));
		root = current;
	}
	
	// no first insert
	else {          
		
		// Reach a leaf node
		if (current->isLeaf) {
			 
			int newNode = current->pairs.size() + 1;
			for (int i = 0; i < current->pairs.size(); i++) {
				if (key < current->pairs[i].first) {
					newNode = i;
					break;
				}
			}
			
			current->pairs.resize(current->pairs.size() + 1);
			if (newNode == current->pairs.size()) {				
				current->pairs[current->pairs.size() - 1] = make_pair(key, value);
			}
			
			else {
				for (int i = current->pairs.size() - 1; i > newNode; i--) {
					current->pairs[i] = current->pairs[i - 1];
				}
				current->pairs[newNode] = make_pair(key, value);
			}
			
			// Check whether the current bucket is full. If it is full, split it;
			if (current->pairs.size() > n) {
				// Number of key-value pairs moved to new bucket
				int k;
				
				if (current->pairs.size() % 2 == 0)
					k = current->pairs.size() / 2;
				else
					k = current->pairs.size() / 2 + 1;
				
				// Create a new bucket for the new right bucket 
				Bucket *newBucket = new Bucket;
				for (int i = current->pairs.size() - k; i < current->pairs.size(); i++) {
					newBucket->pairs.push_back(current->pairs[i]);
				}
				current->pairs.resize(current->pairs.size() - k);
				newBucket->parent = current->parent;
				newBucket->right = current->right;
				current->right = newBucket;
				
				// If current bucket has no parent, create a new parent
				if (current->parent == nullptr) {
					Bucket *p = new Bucket;
					p->isLeaf = false;
					p->pairs.push_back(newBucket->pairs[0]);
					p->child.push_back(current);
					p->child.push_back(newBucket);
					current->parent = p;
					newBucket->parent = p;
					root = p;
				}
				
				// The current bucket has parent, split into the parent bucket
				else {
					
					current = current->parent;
					int newNode = current->pairs.size() + 1;
					for (int i = 0; i < current->pairs.size(); i++) {
						if (newBucket->pairs[0].first < current->pairs[i].first) {
							newNode = i;
							break;
						}
					}
					
					current->pairs.resize(current->pairs.size() + 1);
					current->child.resize(current->child.size() + 1);
					if (newNode == current->pairs.size()) {						
						current->pairs[current->pairs.size() - 1] = newBucket->pairs[0];
						current->child[current->child.size() - 1] = newBucket;
					}
					
					else {
						for (int i = current->pairs.size() - 1; i > newNode; i--) {
							current->pairs[i] = current->pairs[i - 1];
							current->child[i + 1] = current->child[i];
						}
						current->pairs[newNode] = newBucket->pairs[0];
						current->child[newNode+1] = newBucket;
					}
					
					// Set the parent pointers
					for (int i = 0; i < current->child.size(); i++) {
						current->child[i]->parent = current;
					}
					
					// Check whether the current bucket is full. If it is full, split it. 
					if (current->pairs.size() > n) {
						split_recursive(current);
					}
				}
			}
		}
		
		// Not a leaf, go down to find the right leaf node to insert
		else {
			
			// Go down through the middle children pointers
			for (int i = 0; i < current->pairs.size(); i++) {
				if (key < current->pairs[i].first) {
					insert(current->child[i], key, value);
					return;
				}
			}
			
			// Go down through the last child pointer
			insert(current->child[current->child.size() - 1], key, value);
		}
	}
}


// Split a nonleaf node
void Index::split_recursive(Bucket *current) {
	
	// Number of key-value pairs moved to new bucket
	int k;
	
	if (current->pairs.size() % 2 == 0)
		k = current->pairs.size() / 2;
	else
		k = current->pairs.size() / 2 + 1;
	
	// Create a new bucket for the new right bucket
	Bucket *newBucket = new Bucket;
	pair<int, int> f = current->pairs[current->pairs.size() - k];
	for (int i = current->pairs.size() - k + 1; i < current->pairs.size(); i++) {
		newBucket->pairs.push_back(current->pairs[i]);
	}
	for (int i = current->child.size() - k; i < current->child.size(); i++) {
		newBucket->child.push_back(current->child[i]);
	}
	newBucket->isLeaf = false;
	newBucket->parent = current->parent;
	current->pairs.resize(current->pairs.size() - k);
	current->child.resize(current->child.size() - k);
	
    // Set the parent pointers	
	for (int i = 0; i < current->child.size(); i++) {
		current->child[i]->parent = current;
	}
	for (int i = 0; i < newBucket->child.size(); i++) {
		newBucket->child[i]->parent = newBucket;
	}
    
	// If current bucket has no parent, create a new parent
	if (current->parent == nullptr) {
		Bucket *p = new Bucket;
		p->isLeaf = false;
		p->pairs.push_back(f);
		p->child.push_back(current);
		p->child.push_back(newBucket);
		current->parent = p;
		newBucket->parent = p;
		root = p;
	}
	
	// The current bucket has parent, split into the parent bucket
	else {
		
		current = current->parent;
		int newNode = current->pairs.size() + 1;
		for (int i = 0; i < current->pairs.size(); i++) {
			if (f.first < current->pairs[i].first) {
				newNode = i;
				break;
			}
		}
		
		current->pairs.resize(current->pairs.size() + 1);
		if (newNode == current->pairs.size()) {
			current->pairs[current->pairs.size() - 1] = f;
		}
		
		else {
			for (int i = current->pairs.size() - 1; i > newNode; i--) {
				current->pairs[i] = current->pairs[i - 1];
			}
			current->pairs[newNode] = f;
		}
		
		newNode = current->child.size() + 1;
		for (int i = 0; i < current->child.size(); i++) {
			if (newBucket->pairs[0].first < current->child[i]->pairs[0].first) {
				newNode = i;
				break;
			}
		}
		
		current->child.resize(current->child.size() + 1);
		if (newNode == current->child.size()) {
			current->child[current->child.size() - 1] = newBucket;
		}

		else {
			for (int i = current->child.size() - 1; i > newNode; i--) {
				current->child[i] = current->child[i - 1];
			}
			current->child[newNode] = newBucket;
		}
        
		// Set the parent pointers
		for (int i = 0; i < current->child.size(); i++) {
			current->child[i]->parent = current;
		}
        
		// Check whether the current bucket is full. If it is full, split it.
		if (current->pairs.size() > n) {
			split_recursive(current);
		}
	}
}


void Index::key_query(vector<int> query_keys) {
	fstream f;
	f.open("key_query_out.txt", ios::out);
	int output;
	for (int i = 0; i < query_keys.size(); i++) {
		output = search(root, query_keys[i]);
		f << output << endl;
	}
	f.close();
}

// Search for value by key.(key_query) 
int Index::search(Bucket *current, int key) {
	
	// reach a leaf
	if (current->isLeaf) {
		for (int i = 0; i < current->pairs.size(); i++) {
			
			// find the key
			if (key == current->pairs[i].first)
				return current->pairs[i].second;
			
			// the key is not found
			else if (key < current->pairs[i].first)
				return -1;
		}
		
		// the key is not found 
		return -1;
	}
	
	// If not a leaf, go down to find the right leaf node for search
	else {
		
		// The correct value
		int k;
		
		// Go down through the middle children pointers
		if (key < current->pairs[int(current->pairs.size()/2)].first){
			for (int i = 0; i < int(current->pairs.size()/2)+1; i++) {
				if (key < current->pairs[i].first) {
					k = search(current->child[i], key);
					return k;
				}
			}
		}else{
			for (int i = int(current->pairs.size()/2); i < current->pairs.size(); i++) {
				if (key < current->pairs[i].first) {
					k = search(current->child[i], key);
					return k;
				}
			}
		}
		
		
		// Go down through the last child pointer
		k = search(current->child[current->child.size() - 1], key);
		return k;
	}
}

void Index::range_query(vector<pair<int, int>> query_pairs) {
	fstream f;
	f.open("range_query_out.txt", ios::out);
	int output;
	for (int i = 0; i < query_pairs.size(); i++) {
		output = find(root, query_pairs[i].first, query_pairs[i].second);
		f << output << endl;
	}
	f.close();
}

// Find the maximum value with keys between key1 and key2 (range_query)
int Index::find(Bucket *current, int key1, int key2) {
	
	int max = INT_MIN;
	bool stop = false;
	bool change = false;
	
	// reach a leaf
	if (current->isLeaf) {
		for (int i = 0; i < current->pairs.size(); i++) {
			
			// The key is between key1 and key2 
			// Compare the value with max. If it is greater, update the max.
			if (current->pairs[i].first >= key1 && current->pairs[i].first <= key2) {
				if (current->pairs[i].second > max) {
					max = current->pairs[i].second;
					change = true;
				}
			}
			
			// Key is greater than key2, then stop scanning and return
			// If maximum has been changed, return max
			// If maximum has not been changed, return -1 (no keys found between key1 and key2)
			if (current->pairs[i].first > key2) {			
				if (change) {
					return max;
				}else{
					return -1;
				}
			}
					
		}
		
		// Move to the right leaf node to continue to scan if there is a bucket on the right
		while (current->right != nullptr) {
			current = current->right;
			for (int i = 0; i < current->pairs.size(); i++) {
				
				// The key is between key1 and key2 
			    // Compare the value with max. If it is greater, update the max.
				if (current->pairs[i].first <= key2) {
					if (current->pairs[i].second > max) {
						max = current->pairs[i].second;
						change = 1;
					}
				}
				
				// The key is greater than key2, stop scanning
				if (current->pairs[i].first > key2) {
					stop = 1;
					break;
				}
			}
			
			// If the scan should stop and max has been changed, return max
			if (stop && change){
				return max;
			}
			
			// If the scan should stop and max has not been changed, return -1 (no keys found between key1 and key2)
			if (stop && (!change)){
				return -1;
			}
		}
		
		// If scan to the end and max has been changed, return max
		if (change){
			return max;
		}
		
		// If scan to the end and max has not been changed, return -1 (no keys found between key1 and key2)
		else{
			return -1;
		}
	}
	
	// If not a leaf,go down to find the right leaf node for scan
	else {
		
		// The correct max value
		int k;
		
		// Go down through the middle children pointers
		for (int i = 0; i < current->pairs.size(); i++) {
			if (key1 < current->pairs[i].first) {
				k = find(current->child[i], key1, key2);
				return k;
			}
		}
		
		// Go down through the last child pointer
		k = find(current->child[current->child.size() - 1], key1, key2);
		return k;
	}
	
}

// Free the memory used by the B+ Tree
void Index::clear_index() {
	clear(root);
}

// delete the buckets
void Index::clear(Bucket *current) {
	
	// First delete all children
	for (int i = 0; i < current->child.size(); i++) {
		clear(current->child[i]);
	}
	
	// And delete the current bucket
	delete current;
}