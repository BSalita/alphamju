/*
   Alphamju - on the way to a single Dummy Solver
   Copyright (C) 2024 by Kalle Prorok

   NOT READY YET - WORK IN PROGRESS

   Based on the paper
   The alpha mju Search Algorithm for the Game of Bridge (greek letters replaced)
   Tristan Cazenave and Veronique Ventos 2019

   Uses DDS, a bridge double dummy solver 
   by Bo Haglund & Soren Hein.

   See LICENSE and README.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/dll.h"
#include "hands.h"
#include <set>
#include <list>
#include <chrono>

#include "hands.cpp" // To get definitions and useful functions

using namespace std;
using namespace std::chrono;

#define WORLDSIZE 2

char Worlds[WORLDSIZE][80] = {
  "W:.Q76.K. .AJT.7. .543.J .K98.5."      ,
  "W:.543.K. .AJT.7. .Q76.J .K98.5."      ,
//  "W:.Q76.. .AJT.. .543.. .K98.."      ,
//  "W:.543.. .AJT.. .Q76.. .K98.."      ,
};

unsigned int holdingsc[WORLDSIZE][DDS_HANDS][DDS_SUITS];

int contract = 3;
int trumpc = NOTRUMP;
int firstc = SOUTH;
int dealerc = EAST;
int vulc = VUL_NONE;


struct State
{
    bool Min_node;
    int declarerTricks;
    int defenseTricks;
    int handtoplay;
    int handsplayed;
    int firstc;
    int playno;
    int playSuit[13];
    int playRank[13];
};

struct Result
{
    int r[WORLDSIZE]; // Result
    char m[WORLDSIZE]; // Mask for possible worlds
};

inline bool operator<(const Result& L, const Result& R)
{
    return L.r[0] < R.r[0];
}

struct Playable
{
    int suit;
    int rank;
    int world;
};

set<Playable> legalMoves(int world, State state)
{
    set<Playable> moves;
    Playable p;
    p.world = world;
    if (state.handsplayed>0)
    {

    }
    else
    {
        for (int s = 0; s < DDS_SUITS; s++)
            {
                p.suit = s;
                unsigned int m = RA;
                bool adjacent = false;
                for (int r = 14; r >1; r--)
                {
                    if (holdingsc[world][state.handtoplay][s] & m)
                    {
                        if (!adjacent)
                        {
                            p.rank = r; 
                            p.world = world;
                            moves.insert(p);
                            adjacent = true;
                        }
                    }
                    else adjacent = false;
                    m >>= 1;
                }
            }
    }
    return moves;
}

inline bool operator<(const Playable& L,const Playable& R)
{
    if (L.suit < R.suit)
        return true;
    else if (L.suit > R.suit)
        return false;
    else if (L.rank < R.rank)
        return true;
    else if (L.rank > R.rank)
        return false;
    else if (L.world < R.world)
        return true;
    else return false;
}


int init()
{
    for (int i =0; i< WORLDSIZE; i++)
        ConvertPBN(Worlds[i], holdingsc[i]); // Translate PBN to BIN representation
    return 0;
}

void binprint(int v)
{
    unsigned int mask = 1 << ((sizeof(int) << 3) - 1);
    while (mask) {
        printf("%d", (v & mask ? 1 : 0));
        mask >>= 1;
    }
}

int doubleDummy(int world, State state)
{
    deal dl;
    futureTricks fut; 
    int target;
    int solutions;
    int mode;
    int threadIndex = 0;
    int res;
    char line[80], lin[80];
    strcpy_s(line, 80, "DD:"); //Double Dummy result
    strcpy_s(lin, 80, "Original hand:"); //Double Dummy result

    dl.trump = trumpc;// [handno] ;
    dl.first = SOUTH; // state.handtoplay;// [handno] ;

    dl.currentTrickSuit[0] = state.playSuit[0]; //0;
    dl.currentTrickSuit[1] = 0;
    dl.currentTrickSuit[2] = 0;

    dl.currentTrickRank[0] = state.playRank[0]; 0;
    dl.currentTrickRank[1] = 0;
    dl.currentTrickRank[2] = 0;
    printf("play %c%c ", "SHDC"[state.playSuit[0]], dcardRank[state.playRank[0]]);

        for (int h = 0; h < DDS_HANDS; h++)
            for (int s = 0; s < DDS_SUITS; s++)
                dl.remainCards[h][s] = holdingsc[world][h][s];
    PrintHand(lin, dl.remainCards);

    unsigned int mask= (1 << (state.playRank[0] )); // remove played card
//        printf("S:"); binprint(dl.remainCards[SOUTH][HE]);
//        printf(" playmask %d:", mask);
//        binprint(mask);
    mask = ~mask;
 //       printf(" iplaymask %d:", mask);
 //       binprint(mask);
    dl.remainCards[state.firstc][state.playSuit[state.playno-1]] &= mask;
 //       printf("SM:"); binprint(dl.remainCards[SOUTH][HE]);

    target = -1;
    solutions = 1;
    mode = 1;
    res = SolveBoard(dl, target, solutions, mode, &fut, threadIndex);

    if (res != RETURN_NO_FAULT)
    {
        ErrorMessage(res, line);
        printf("DDS error: %s\n", line);
    }
    printf("World %d. \n", world);
    PrintFut(line, &fut);
    PrintHand(line, dl.remainCards);
    return fut.score[0];
}

bool stopc(State state, int M, set<int> worlds, Result& result)
{
    if (state.declarerTricks >= contract)
    {
        for (int w : worlds)
        {
            result.r[w] = 1;
        }
        return true;
    }
    else if (state.defenseTricks > 13 - contract)
    {
        for (int w : worlds)
        {
            result.r[w] = 0;
        }
        return true;
    }
    else if (M == 0)
    {
        for (int w : worlds)
        {
            result.r[w] = doubleDummy(w,state);
            result.m[w] = '1';
        }
        return true;

    }
    else return false;
}

list<Result> minr(list<Result> front, list<Result> f)
{
    set<Result> result;
    return front;

}

list<Result> maxr(list<Result> front,list<Result> f)
{
    set<Result> result;
    return f;

}

State playc(Playable move, State state)
{
    state.playSuit[state.playno] = move.suit;
    state.playRank[state.playno] = move.rank;
    state.playno++;
    if (state.handtoplay < WEST)
        state.handtoplay++;
    else state.handtoplay = NORTH;

    return state;
}

list<Result> au(State state, int M, set<int> worlds)
{
    Result result;
    if (stopc(state, M, worlds, result))
    {
       list<Result> res;
       res.push_back(result);
       return res;
    }
    list<Result> front;
    list<Result> f;
    if (state.Min_node)
    {
        set<Playable> allMoves;
        for (int w:worlds)
        {
            set<Playable> l;
            l.merge(legalMoves(w,state));
            allMoves.merge(l);
        }
        for (Playable move : allMoves)
        {
            State s = playc(move, state);
            set<int> W1;
            for (Playable mov : allMoves)
            {
                if (move.rank == mov.rank && move.suit == mov.suit && move.world != mov.suit)
                    W1.insert(mov.world);
            }
            f = au(s, M, W1);
            front = minr(front, f);
        }
    }
    else // Max node
    {
        set<Playable> allMoves;
        for (int w : worlds)
        {
            set<Playable> l;
            l.merge(legalMoves(w,state));
            allMoves.merge(l);
        }
        for (Playable move : allMoves)
        {
            State s = playc(move, state);
            set<int> W1;
            W1.insert(move.world);
            for (Playable mov : allMoves)
            {
                if (move.rank == mov.rank && move.suit == mov.suit && move.world != mov.suit)
                    W1.insert(mov.world); // TODO; same card might be played again in another world?
            }
            f = au(s, M - 1, W1);
            front = maxr(front, f);
        }
    }
    return front;
}

State start;

int main()
{
    set<int> w;
    list<Result> r;

    State state = start;
    state.firstc = SOUTH; 
    state.handtoplay = SOUTH;
    auto start = high_resolution_clock::now();
    init();
    for (int i = 0; i < WORLDSIZE; i++)
        w.insert(i);
//    r.merge(au(state, 1, w));
    r = au(state, 1, w);
    for (Result elem : r)
    {

        for (int w = 0; w < WORLDSIZE; w++)
            printf(" %d", elem.r[w]);
        printf("\n");
    }
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);
    printf("Time taken by function : %f seconds\n", duration.count()/1000000.0);

    return 0;
}

