#include "board.h"
#include "monte_carlo.h"

const int shift[9] = { 0, 9, 8, 7, 1, 1, 7, 8, 9};
const Bitboard mask[9] = {
                    0x8000000000000000ULL,
                    0xFEFEFEFEFEFEFE00ULL,
                    0xFFFFFFFFFFFFFF00ULL,
                    0x7F7F7F7F7F7F7F00ULL,
                    0xFEFEFEFEFEFEFEFEULL,
                    0x7F7F7F7F7F7F7F7FULL,
                    0x00FEFEFEFEFEFEFEULL,
                    0x00FFFFFFFFFFFFFFULL,
                    0x007F7F7F7F7F7F7FULL
                   };
                   
/***
board function
***/
Board::Board(){
    this->b[BLACK] = BOARD_INIT_BLACK;
    this->b[WHITE] = BOARD_INIT_WHITE;
    this->b[EMPTY] = ~(this->b[BLACK] | this->b[WHITE]);
}

Board::Board( const Board &board){
    memcpy( this, &board, sizeof(Board));
}

void Board::print_visual(){
    Bitboard pos = mask[0];
    for( int i=1; pos!=0; pos>>=1, ++i){
        if( this->b[BLACK] & pos)
            printf("X");
        else if( this->b[WHITE] & pos)
            printf("O");
        else
            printf("-");
        if( i%8==0)
            printf("\n");
    }
}


void Board::print_bit(){
    printf("BLACK:%.16llX\n", this->b[BLACK]);
    printf("WHITE:%.16llX\n", this->b[WHITE]);
    printf("EMPTY:%.16llX\n", this->b[EMPTY]);
}

int Board::count(){
    return  (__builtin_popcountll( this->b[BLACK])-  
        __builtin_popcountll( this->b[WHITE]));
}

bool Board::play( const Move& move, int player, int position){
    Bitboard init = mask[0] >> position;
    Bitboard pos = init;
    if( !move.m[0] && position == 64) //pass
        return true;
    if( !(move.m[0] & pos))  //when invalid move occur
        return false;
    
    this->b[player] |= pos;
    for( int i = 1; i < 5; ++i)
        if( move.m[i] & init){
            pos = init >> shift[i];
            while( this->b[player^1] & pos){
                this->b[player^1] ^= pos;
                this->b[player] |= pos;
                pos >>= shift[i];
            }
        }
    
    for( int i = 5; i < 9; ++i)
        if( move.m[i] & init){
            pos = init << shift[i];
            while( this->b[player^1] & pos){
                this->b[player^1] ^= pos;
                this->b[player] |= pos;
                pos <<= shift[i];
            }
        }
    this->b[EMPTY] = ~(this->b[BLACK] | this->b[WHITE]);
    return true;
}

bool Board::play( Board& newBoard, const Move& move, int player, int position){
    memcpy( &newBoard, this, sizeof(Board));
    return newBoard.play( move, player, position );
}
/***
move function
***/

Move::Move(){
    memset( this->m, 0, sizeof(Bitboard)*9);
}

Move::Move( const Move &move){
    memcpy( this, &move, sizeof(Move));
}

void Move::searchAll( const Board &board, int player){
    memset( this->m, 0, sizeof(Bitboard)*9);
    for( int i = 1; i < 5; ++i){
        this->search_l( board, player, i);
        this->m[0] |= this->m[i];
    }
    
    for( int i = 5; i < 9; ++i){
        this->search_r( board, player, i);
        this->m[0] |= this->m[i];
    }
}

void Move::search_l( const Board &board, int player, int dir){
    Bitboard p = board.b[player];
    Bitboard o = board.b[player ^ 1];
    Bitboard e = board.b[EMPTY];
    
    Bitboard c = o & ( ( p << shift[dir]) & mask[dir]);
    while( c){
        this->m[dir] |= e & ( (c << shift[dir]) & mask[dir]);
        c = o & ( (c << shift[dir]) & mask[dir]);
    }
}

void Move::search_r( const Board &board, int player, int dir){
    Bitboard p = board.b[player];
    Bitboard o = board.b[player ^ 1];
    Bitboard e = board.b[EMPTY];
    
    Bitboard c = o & ( ( p >> shift[dir]) & mask[dir]);
    while( c){
        this->m[dir] |= e & ( (c >> shift[dir]) & mask[dir]);
        c = o & ( (c >> shift[dir]) & mask[dir]);
    }
}

void Move::print_visual(){
    Bitboard pos = mask[0];
    for( int m=0; m<9; ++m){
        printf("\n");
        for( int i=1; pos!=0; pos>>=1, ++i){
            if( this->m[m] & pos)
                printf("S");
            else
                printf("-");
            if( i%8==0)
                printf("\n");
        };
    }
}
void Move::print_bit(){
    for( int i=0; i<9; ++i)
        printf("MOVE:%.16llX\n", this->m[i]);
}

std::vector<int> Move::move_sequence(){
    std::vector<int> seq;
    Bitboard pos = mask[0];
    for( int i=0; pos!=0; pos>>=1, ++i)
        if( this->m[0]&pos)
            seq.push_back(i);
    if(seq.size()==0)
        seq.push_back(PASS);
    return seq;
}
