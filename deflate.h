#include <bits/stdc++.h>
#define USIZE uint32_t
#define SHIFT 16
#define MASK 0xFFFF
struct node1
{
    long long key_value;
    char code;
    bool leaf;
    node1 *left;
    node1 *right;

    node1(char code, long long key_value, bool leaf)
    {
        this->code = code;
        this->key_value = key_value;
        left = NULL;
        right = NULL;
        this->leaf = leaf;
    }
};

struct compare1
{
    bool operator()(node1 *leftNode, node1 *rightNode)
    {
        return (leftNode->key_value > rightNode->key_value);
    }
};
struct node2
{
    long long key_value;
    USIZE code;
    bool leaf;
    node2 *left;
    node2 *right;

    node2(USIZE code, long long key_value, bool leaf)
    {
        this->code = code;
        this->key_value = key_value;
        left = NULL;
        right = NULL;
        this->leaf = leaf;
    }
};

struct compare2
{
    bool operator()(node2 *leftNode, node2 *rightNode)
    {
        return (leftNode->key_value > rightNode->key_value);
    }
};