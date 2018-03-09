#ifndef BOARD_H
#define BOARD_H

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <vector>

#define BOARD_INIT_BLACK 0x0000000810000000ULL
#define BOARD_INIT_WHITE 0x0000001008000000ULL
#define BOARD_POS_START  0x8000000000000000ULL
#define BLACK 0
#define WHITE 1
#define EMPTY 2
#define PASS 64

typedef unsigned long long Bitboard;
class Board;
class Move;

class Board{
    public:
    
    Bitboard b[3];
    
    Board();
    Board( const Board&);
    
    virtual void print_visual();
    virtual void print_bit();
    int count();
    bool play( const Move&, int ,int);
    bool play( Board&, const Move&, int, int);
    void copy( Board&);
};

class Move{
    public:
    Bitboard m[9];
    
    Move();
    Move( const Move&);
    
    virtual void print_visual();
    virtual void print_bit();
    void searchAll( const Board&, int);
    void search_l( const Board&, int, int);
    void search_r( const Board&, int, int);
    std::vector<int> move_sequence();
    void copy( Move&);
};

#endif
