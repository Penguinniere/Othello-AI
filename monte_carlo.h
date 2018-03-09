#ifndef MONTE_CARLO_H
#define MONTE_CARLO_H
#include <map>
#include <random>
#include "board.h"

class Node: public Board, public Move{
    public:
    
    std::map < int, Node> child;
    Node *parent;
    
    int player;
    int win[2][2];
    int match[2];
    int pass; //calculte pass num to prevent infinite expand
    bool prune;
    Node();
    Node( const Board &, Node* , const int);
    
    void print_visual();
    void print_bit();
    void expand();
    bool create( int);
    int simulate( int);
    void backprogate();
    void pruning( int, double, double);
    void clear();
    //bool clear_except( int);
    int select();
};

extern std::mt19937 rand_generator;
extern std::vector < std::uniform_int_distribution<int> > uniform_dist;
extern double C, C1, C2;

Node* get_node( Node* node, int);
Node* get_node_ucb( Node* node, int);
Node* get_node_ucb_revise( Node* node, int);

#endif