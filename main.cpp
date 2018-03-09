#include "board.h"
#include "monte_carlo.h"
#include "my_socket.h"
#include "nodeuse.h"
#include <ctime>
#include <chrono>
using namespace std::chrono;

std::vector <int> node_count;

//random variable
std::mt19937 rand_generator;
std::vector <std::uniform_int_distribution<int> > uniform_dist;

//constant and variable
double C = 0.5, C1 = 0.2 , C2 = 0.1;
double range = 1.0, threas = 0.3;
int preBatch = 128, batch = 64, matchCoe = 256, move;
bool newTree = false;

Node *root = NULL, *curr = NULL;
duration< long long, std::milli> gen_limit(9000);
duration< long long, std::milli> oth_limit(500);

//common function and pointer
void arg_process( int, char**);
void init_generator();
void message_process( skt &socket);
int (*genmove_method)( system_clock::time_point);
Node* (*score_method)( Node* node, int);

//select function
int UCB( system_clock::time_point);
int UCT( system_clock::time_point);
int PPT( system_clock::time_point);

int main(int argc, char **argv){    

    genmove_method = PPT;
    score_method = get_node_ucb;
    
    arg_process( argc, argv);
    init_generator();
    skt socket(argv[1], atoi(argv[2]));  
    message_process(socket);
    
    printf("node use:");
    for( auto &i: node_count)
        printf("%d ", i);
    printf("\n");
    return 0;
}
void arg_process( int argc, char** argv){
    if(argc < 3){
        printf("Oth ip port [genmove_method] [score_method]");
        printf(" [-r]|[-nr] [-p preBatch] [-b batch] [-m matchThres]\n");
        printf("genmove_methold: UCB UCT PPT\n");
        printf("PPT [range threashold]\n");
        printf("score_method: SWIN SUCB SUCBR\n");
        printf("SUCB [C]\n");
        printf("SUCBR [C C1 C2]\n");
        exit(1);
    }
    for( int i=3; i<argc; ++i){
        if( !strcmp(argv[i], "UCB"))
            genmove_method = UCB;
        else if( !strcmp(argv[i], "UCT"))
            genmove_method = UCT;
        else if( !strcmp(argv[i], "PPT")){
            genmove_method = PPT;
            for( int j=i+1; j<argc && atof(argv[j])>=0.0; ++j, ++i){
                if( j-i==1)
                    range = atof(argv[j]);
                else if( j-1==2)
                    threas = atof(argv[j]);
            }
        }
        else if( !strcmp(argv[i], "SWIN"))
            score_method = get_node;
        else if( !strcmp(argv[i], "SUCB")){
            score_method = get_node_ucb;
            for( int j=i+1; j<argc && atof(argv[j])>=0.0; ++j, ++i)
                if( j-i==1)
                    C = atof(argv[j]);
        }
        else if( !strcmp(argv[i], "SUCBR")){
            score_method = get_node_ucb_revise; 
            for( int j=i+1; j<argc && atof(argv[j])>=0.0; ++j, ++i){
                if( j-i==1)
                    C = atof(argv[j]);
                else if( j-1==2)
                    C1 = atof(argv[j]);
                else if( j-1==3)
                    C2 = atof(argv[j]);
            }
        }
        else if( !strcmp(argv[i], "-p"))
            for( int j=i+1; j<argc && atoi(argv[j])>=0; ++j, ++i)
                if( j-i==1)
                    preBatch = atoi(argv[j]);
        else if( !strcmp(argv[i], "-b"))       
            for( int j=i+1; j<argc && atoi(argv[j])>=0; ++j, ++i)
                if( j-i==1)
                    batch = atoi(argv[j]);   
        else if( !strcmp(argv[i], "-m"))       
            for( int j=i+1; j<argc && atoi(argv[j])>=0; ++j, ++i)
                if( j-i==1)
                    matchCoe = atoi(argv[j]);                       
        else if( !strcmp(argv[i], "-r"))
            newTree = true;
        else if( !strcmp(argv[i], "-nr"))
            newTree = false;
    }
}

void init_generator(){
    unsigned seed = system_clock::now().time_since_epoch().count();
    rand_generator.seed(seed);
    uniform_dist.push_back(std::uniform_int_distribution<int>()); //dummy
    for( int i=1; i<33; ++i)
        uniform_dist.push_back(std::uniform_int_distribution<int>(0, i-1)); 
}

void message_process(skt &socket){
    char ibuf[1024],obuf[1024];
    bool not_quit = true;
    move = 0;
    while(not_quit){
        if( socket.Recv_r( ibuf,1023)!= ibuf){
            //if( curr)
            //    curr->print_visual();
            if( ibuf[0]=='g'){ //genmove
                int pos;
                if( strstr( ibuf, "genmove")){
                    move ++;
                    pos = genmove_method(system_clock::now() + gen_limit);
                    curr = &curr->child[pos];
                    sprintf( obuf, "genmove %d %d", pos>>3, pos&0x00000007);
                }
            } 
            else if( ibuf[0]=='p'){ //play    
                int row, col, pos;
                if( strstr( ibuf, "play") && 
                    sscanf( ibuf, "%*s %d %d", &row, &col)==2){
                    move++;
                    pos = (row<<3)+col;
                    if( !curr->create(pos)){
                        printf("error: can't find child at pos %d\n", pos);
                        //curr->print_visual();
                        exit(1);
                    }
                    curr = &curr->child[pos];
                    sprintf( obuf, "play");
                }
            }
            else if( ibuf[0]=='n') {//name
                if( strstr( ibuf, "name"))
                    sprintf( obuf,"name r04922030");
            }
            else if( ibuf[0]=='c'){ //clear_board
                if( strstr( ibuf, "clear_board")){
                    if( root && curr){
                        curr->print_visual();
                        printf("%d\n", curr->count());
                        node_count.push_back(node_number(root));
                        delete root;
                    }
                    root = new Node();
                    curr = root;
                    sprintf( obuf, "clear_board");
                }
            }
            else if( ibuf[0]=='q'){ //quit
                if( strstr( ibuf, "quit")){
                    if( root && curr){
                        curr->print_visual();
                        printf("%d\n", curr->count());
                        node_count.push_back(node_number(root));
                        delete root;
                    }    
                    root = NULL;
                    curr = NULL;
                    not_quit = false;
                    sprintf( obuf, "quit");
                }
            }
            else if( ibuf[0]=='u'){ //undo may not use
                if( strstr( ibuf, "undo")){
                    ;
                }
            }
            else if( ibuf[0]=='s') //showboard may not uee
                ;
            else if( ibuf[0]=='f') //final_score may not use
                ;
            socket.Send_r( obuf); 
        }
    }
}

int UCB( system_clock::time_point deadline){
    curr->expand();
    //printf("%d %d\n", curr->pass, curr->child.size());
    //pre_simulate
    int total_match = 0;
    for( auto &c: curr->child)
        total_match += c.second.simulate( preBatch);
    //bonus_simulate
    while( system_clock::now() < deadline){
        Node *next_node = score_method( curr, total_match);
        total_match += next_node->simulate( batch);
    }
    return curr->select();
}

int UCT( system_clock::time_point deadline){
    Node *ptr = curr;
    if( newTree) //build new Tree 
        ptr->clear();
    while( system_clock::now() < deadline){
        ptr = curr;
        //selection
        //int i=0;
        while( !ptr->child.empty()){
            ptr = score_method( ptr, ptr->match[1]);
            //i++;
        }
        //printf("###%d###", i);
        //expand
        
        ptr->expand();
        if( ptr->child.empty())
            break;
        //simulate
        int total_match = 0;
        
        //pre_simulate
        for( auto &c: ptr->child) 
            total_match += c.second.simulate( preBatch);
        //bonus simulate
        while( system_clock::now() < deadline &&
            total_match <= ptr->child.size()*matchCoe ){
            Node *next_node = score_method( ptr, total_match);
            total_match += next_node->simulate( batch); 
        }
        //backprogation
        for( auto &c: ptr->child)
            c.second.backprogate();
    }
    return curr->select();
}

int PPT( system_clock::time_point deadline){
        Node *ptr = curr;
    if( newTree) //build new Tree 
        ptr->clear();
    while( system_clock::now() < deadline){
        ptr = curr;
        //selection
        //int i=0;
        while( !ptr->child.empty()){
            ptr = score_method( ptr, ptr->match[1]);
            //i++;
        }
        //printf("###%d###", i);
        //expand
        ptr->expand();
        if( ptr->child.empty())
            break;
        //simulate
        int total_match = 0;
        
        //pre_simulate
        for( auto &c: ptr->child) 
            total_match += c.second.simulate( preBatch);
        //bonus simulate
        while( system_clock::now() < deadline &&
            total_match <= ptr->child.size()*matchCoe ){
            Node *next_node = score_method( ptr, total_match);
            total_match += next_node->simulate( batch); 
        }
        //backprogation and pruning
        ptr->pruning( total_match, range, threas);
        for( auto &c: ptr->child)
            if( !c.second.prune)
                c.second.backprogate();
    }
    return curr->select();
}