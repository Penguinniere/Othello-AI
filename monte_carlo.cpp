#include "monte_carlo.h"
#include <iostream>
Node::Node():Board(),Move(){
    this->parent = NULL;
    this->player = BLACK;
    memset( this->win, 0, sizeof(int)*4);
    memset( this->match, 0, sizeof(int)*2);
    this->pass = 0;
    this->prune = false;
    this->searchAll( (Board&)(*this), this->player);  
}

Node::Node( const Board &board, Node *parent, const int player)
    :Board( board),Move(){
    this->parent = parent;
    this->player = player;
    memset( this->win, 0, sizeof(int)*4);
    memset( this->match, 0, sizeof(int)*2);
    this->pass = 0;
    this->prune = false;
    this->searchAll( (Board&)(*this), this->player);
}

void Node::print_visual(){
    Bitboard pos = 0x8000000000000000ULL;
    for( int i=1; pos!=0; pos>>=1, ++i){
        if( this->m[0] & pos)
            printf("S");
        else if( this->b[BLACK] & pos)
            printf("X");
        else if( this->b[WHITE] & pos)
            printf("O");
        else
            printf("-");
        if( i%8==0)
            printf("\n");
    }
    //this->Board::print_visual();
    //this->Move::print_visual();
}
void Node::print_bit(){
    this->Board::print_bit();
    this->Move::print_bit();
}

void Node::expand(){
    if( this->pass==2) //the end of match, do not expand anymore
        return;
    Board nb;
    for( const auto &c: move_sequence()){
        //printf("%d ", c);
        if( this->play( nb, (Move&)(*this), this->player, c)){
            Node n( nb, this, (this->player)^1);
            this->child.insert(std::make_pair( c, n));
        }
    }
    if( this->child.find(64)!=this->child.end())
        this->child[64].pass = this->pass+1;
    //printf("\n");
    return;
}

bool Node::create( int pos){
    if( this->pass==2) //the end of match, do not create anymore
        return false;
        
    Board nb;
    if( !this->play( nb, (Move&)(*this), this->player, pos))
        return false;
    
    Node n( nb, this, (this->player)^1);
    this->child.insert(std::make_pair( pos, n));
    if( pos==64)
        this->child[64].pass = this->pass+1;
    return true;
}

int Node::simulate( int batch){
    for( int i=0; i < batch; ++i){
        Board b( (Board&)(*this));
        Move m( (Move&)(*this));
        std::vector<int> v;
        int p = this->player;
        bool pass = false;
        
        while(1){
            v = m.move_sequence();
            if( ! m.m[0]){
                if( pass == true)
                    break;
                else
                    pass = true;
            }  
            else{
                int choose = uniform_dist[v.size()](rand_generator);
                b.play( m, p, v[choose]);
                pass = false;
            }
            p ^= 1; //change player 
            m.searchAll( b, p);
        }
        if( (b.count())>0)
            this->win[0][0]++;
        if( (b.count())<0)
            this->win[0][1]++;
        this->match[0]++;
    }
    return batch;
}

void Node::backprogate(){
    Node* ptr = NULL;
    for( ptr = this->parent; ptr!=NULL; ptr = ptr->parent){
        for( int i=0; i<2; i++){
            ptr->win[i][0] += this->win[0][0];
            ptr->win[i][1] += this->win[0][1];
            ptr->match[i] += this->match[0];
        }
    }
}

void Node::pruning( int N, double range, double stdThreas){
    Node *best = get_node( this, N);
    double mean = (double)(best->win[0][this->player])/best->match[0];
    double std = sqrt(mean * (1-mean));
    double leftValue = mean - range * std;
    double tmpM, tmpS, rightValue;
    if( std > stdThreas)
        return;
    for( auto &c: this->child){
        tmpM = (double)(c.second.win[0][this->player])/ c.second.match[0];
        tmpS = sqrt(tmpM * (1-tmpM));
        rightValue = tmpM + range * std;
        if( tmpS < stdThreas && rightValue < leftValue ){
            c.second.prune = true;
            //printf("prune at %d\n", c.first);
        }
        else
            c.second.prune = false;
    }
}

void Node::clear(){
    this->win[0][0] -= this->win[1][0];
    this->win[0][1] -= this->win[1][1];
    this->match[0] -= this->match[1];
    this->win[1][0] = 0;
    this->win[1][1] = 0;
    this->match[1] = 0;
    this->child.clear();
};

int Node::select(){
    double max = -1.0, tmp;
    int pos = -1;
    for( auto &c: this->child){
        if( c.second.prune)
            continue;
        tmp = (double) (c.second.win[0][this->player])/ c.second.match[0];
        if( tmp > max){
            max = tmp;
            pos = c.first;
        }
    }
    return pos;
}

Node* get_node( Node* node, int N){
    double max = -1.0, tmp;
    Node* cand = NULL;
    for( auto &c: node->child){
        if( c.second.prune)
            continue;
        tmp = (double) (c.second.win[0][node->player])/ c.second.match[0];
        if( tmp > max){
            max = tmp;
            cand = &c.second;
        }
    }
    return cand;
}

Node* get_node_ucb( Node* node, int N){
    double max = -1.0, tmp;
    Node* cand = NULL;
    for( auto &c: node->child){
        if( c.second.prune)
            continue;
        tmp = (double) (c.second.win[0][node->player]) / c.second.match[0];
        tmp += sqrt( log((double)N) / (c.second.match[0]))*C;
        if( tmp > max){
            max = tmp;
            cand = &c.second;
        }
    }
    return cand;
}

Node* get_node_ucb_revise( Node* node, int N){
    double max = -1.0, tmp, tmp2, ratio;
    Node* cand = NULL;
    for( auto &c: node->child){
        if( c.second.prune)
            continue;
        ratio = sqrt( log((double)N) / c.second.match[0]);
        tmp = (double) (c.second.win[0][node->player]) / c.second.match[0];
        tmp2 = tmp * (1-tmp) + C1 * sqrt(ratio);
        tmp = tmp + C * sqrt(ratio * (tmp2>C2)? C2: tmp2);
        if( tmp > max){
            max = tmp;
            cand = &c.second;
        }
    }
    return cand;
}
