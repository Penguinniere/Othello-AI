#include "monte_carlo.h"

int node_number(Node *node){
    int number = 0;
    if( node->child.size()==0)
        return 1;
    for( auto &c: node->child)
        number +=node_number( &(c.second));
    return number + 1;
}