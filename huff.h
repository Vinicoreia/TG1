#include <bits/stdc++.h>

struct node
{
    long long key_value;
    char code;
    bool leaf;
    node *left;
    node *right;

    node(char code, long long key_value, bool leaf){
        this->code = code;
        this-> key_value = key_value;
        left =NULL;
        right = NULL;
        this->leaf = leaf;
    }
};

struct compare{
    bool operator()(node *leftNode, node *rightNode)
    {
        return (leftNode->key_value > rightNode->key_value);
    }
};