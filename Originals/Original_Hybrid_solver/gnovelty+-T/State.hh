#ifndef STATE_HH
#define STATE_HH

typedef long unsigned int lui;

#define ELEM_TYPE lui

/*Number of bits in each element of the bitvector*/
#define SIZE_ELEM (sizeof(ELEM_TYPE) * 8)


#define CEIL(X) (( X % SIZE_ELEM )?( ( X / SIZE_ELEM ) + 1):(X / SIZE_ELEM))
#define FLOOR(X) (X / SIZE_ELEM)
#define REMAINDER(X) (X % SIZE_ELEM)

/*---------------------------------

  Old fashioned debugging

  ---------------------------------*/
#define DEBUG_LEVEL 3
#define DEBUG_GET_CHAR(Y) {if(Y > DEBUG_LEVEL) {char ch; cin>>ch;} }

#define VERBOSE(X) {cerr<<"INFO :: "<<X<<endl;}
#define VERBOSER(Y, X) {if(Y > DEBUG_LEVEL)cerr<<"INFO ("<<Y<<") :: "<<X;}

/*---------------------------------

  Macros for fatal and lower-level errors.

  ---------------------------------*/
#define UNRECOVERABLE_ERROR(X) {cerr<<"UNRECOVERABLE ERROR :: "<<X;assert(0);exit(0);}

#define QUERY_UNRECOVERABLE_ERROR(Q,X) {if(Q)UNRECOVERABLE_ERROR(X);}

#define WARNING(X) {}//{cerr<<"WARNING :: "<<X<<endl;}

/*---------------------------------

  Usual suspects C++::1998

  ---------------------------------*/

/*IO, string, files*/
#include<string>
#include<iostream>
#include<iomanip>
#include<sstream>
#include<fstream>

/*Storage*/
#include<vector>
#include<map>
#include<set>

/*Sorting, and set operations*/
#include<algorithm>

/*---------------------------------

  Usual suspects C

  ---------------------------------*/
#include<cstddef>
#include<cstdio>
#include<cmath>
//#include<math.h>
#include<cstdlib>
//#include<cmalloc>
#include<cassert>
#include<cctype>
#include<csignal>
#include<cstdarg>
#include<cstddef>
#include<cstring>

/*---------------------------------

  Usual suspects C++::TR1

  ---------------------------------*/

#include<tr1/unordered_map>
#include<tr1/unordered_set>

/*Inlcude for boost hashing et al.*/
#include<boost/functional/hash.hpp>

/*Model is stored in "atom" and the range is "1 to numAtoms+1".*/

using namespace std;


class State
{
public:	
    /*Initially nothing is true (all bits in \member{data} are
     * false). \argument{size} is the number of propositions that
     * this state is required to keep the truth value of.*/
    State(uint size = 0);

    ~State();
    
    /*Copy construction clones \member{numPropositions},
     * \member{data}, \member{time} and
     * \member{possibleActions}. NOTE: We DO NOT copy
     * \member{propositionsChanged}.*/
    State(const State&);

    /*This clones the same elements as the copy constructor.*/
    State& operator=(const State&);
	
    /*Comparison is based on the size of \member{data} first, and
     * then on the actual elements.*/
    bool operator==(const State& state) const;
    bool operator<(const State& state) const;
	
    static const ELEM_TYPE big = -1;

    
//     /*Change individual (at index \argument{uing}) bits of the
//      * state representation (\member{data}).*/
//     void flipOn(uint);
//     void flipOff(uint);
//     void flip(uint);

//     /*Is proposition \argument{uint} true?*/
//     bool isTrue(uint) const;
//     bool isFalse(uint in) const {return !isTrue(in);};


    /*Change individual (at index \argument{uing}) bits of the
     * state representation (\member{data}).*/
    inline void flipOn(uint index)
    {
        register uint remainder = REMAINDER(index);
        register uint bitVecNum = FLOOR(index);

        assert(bitVecNum < CEIL(numPropositions));
        assert(bitVecNum >=0);
        
//         VERBOSER(2, "Flipping ::"<<index<<endl
//                  <<remainder<<endl
//                  <<bitVecNum<<endl);
        
        register ELEM_TYPE mask = 1;
        mask = mask<<remainder;
        data[bitVecNum] |= mask;
    
        /*Now it's true, some actions may be executable.*/
    }

    inline void flipOff(uint index)
    {
        register uint remainder = REMAINDER(index);
        register uint bitVecNum = FLOOR(index);

        register ELEM_TYPE mask = 1;
        mask = mask<<remainder;
        mask = mask^big;
        data[bitVecNum] &= mask;
    }

    inline void flip(uint index)
    {
        register uint remainder = REMAINDER(index);
        register uint bitVecNum = FLOOR(index);

        register ELEM_TYPE mask = 1;
        mask = mask<<remainder;
    
        if(data[bitVecNum] & mask){
            mask = mask^big;
            data[bitVecNum] &= mask;
        } else {
            data[bitVecNum] |= mask;

            /*Now it's true, some actions may be executable.*/
        }
    }

    inline bool isTrue(uint index) const
    {
        register uint remainder = REMAINDER(index);
        register uint bitVecNum = FLOOR(index);

        register ELEM_TYPE mask = 1;
        mask = mask<<remainder;
    
        return (data[bitVecNum] & mask)?true:false;
    }





    
    void randomize();
    
    size_t hash_value() const;

    /*see \member{numPropositions}.*/
    uint getNumPropositions() const{return numPropositions;};
private:
//     /* Bitmask for use by accessor functions such as flipOn, flipOff,
//      * isTrue, etc.*/
//     register uint mask;

//     /*(see \member{flipOff}, \member{flipOn}, \member{}flipOn, etc.)*/
//     register uint bitVecNum;
    
//     /*(see \member{flipOff}, \member{flipOn}, \member{}flipOn, etc.)*/
//     register uint remainder;


    
    /*Number of propositions in the state description.*/
    uint numPropositions;
	
    /*64 bits per element in this vector, each bit represents the
     * truth value of a proposition.*/
    //vector<lui> data;
    ELEM_TYPE* data;
};
    
/*Function for STL and boost to access \member{hash_value} of
 * \argument{GroundAction}.*/
std::size_t hash_value(const State& );


class SetOfStateHashes //: public tr1::unordered_set<size_t, boost::hash<size_t> >
{
public:
    SetOfStateHashes(uint SIZE = 1000000)
        :array_size(CEIL(SIZE)),
         entries(SIZE)
    {
//         saturation = 0;
        hash_array = new ELEM_TYPE[CEIL(SIZE)];
        for(uint i = 0; i < CEIL(SIZE); i++){
            hash_array[i] = 0;
        }
        
        saturation = 0;
    }

    /* Copy construction does not actually copy the bitarray, but
     * rather just the size parameters. */
    SetOfStateHashes(const SetOfStateHashes& setOfStateHashes)
    {
        this->saturation = 0;//setOfStateHashes.saturation;
        
        this->entries = setOfStateHashes.entries;
        uint SIZE = this->entries;
        
        this->array_size = CEIL(SIZE);
        this->entries = SIZE;
       
        hash_array = new ELEM_TYPE[CEIL(SIZE)];
        for(uint i = 0; i < CEIL(SIZE); i++){
            hash_array[i] = 0;
        } 
    }
    
    SetOfStateHashes& operator=(const SetOfStateHashes& setOfStateHashes)
    {
        saturation = 0;
        this->entries = setOfStateHashes.entries;
        uint SIZE = this->entries;
        
        this->array_size = CEIL(SIZE);
        this->entries = SIZE;
        
        if(hash_array)
            delete [] hash_array;
        
        hash_array = new ELEM_TYPE[CEIL(SIZE)];
        for(uint i = 0; i < CEIL(SIZE); i++){
            hash_array[i] = 0;
        }

        return *this;
    }
    
    ~SetOfStateHashes()
    {
        if(hash_array)
            delete [] hash_array;
    }
    
    inline uint find(const State& state) 
    {
        register uint index = state.hash_value() % entries;
        register uint remainder = REMAINDER(index);
        register uint bitVecNum = FLOOR(index);
        register ELEM_TYPE mask = 1;
        mask = mask<<remainder;


        if(hash_array[bitVecNum] & mask){
            return 0;
        } else {
            return entries + 1;
        }
    }

    inline uint end()
    {
        return entries + 1;
    }
    
    
    //tr1::unordered_set<size_t, boost::hash<size_t> >::const_iterator end();

    inline void insert(const State& state)
    {
        
        register uint index = state.hash_value() % entries;
        register uint remainder = REMAINDER(index);
        register uint bitVecNum = FLOOR(index);
        
        register ELEM_TYPE mask = 1;
        mask = mask<<remainder;
        
        if(!(hash_array[bitVecNum] & mask)){
            hash_array[bitVecNum] |= mask;
            saturation++;
        }
    }

    
    bool is_saturated()
    {
        assert(saturation <= entries);
        
        if((saturation / entries) > 0.8){
            return true;
        } else {
            return false;
        }
    }
    
    
private:
    /* Number of states in the hash so far.*/
    uint saturation;
    
    /* Number of elements sizeof(ELEM_TYPE) in \member{hash_array}.*/
    uint array_size;

    /* Number of boolean elements that we are trying to keep track of.*/
    uint entries;

    /* Data in bitvector. 1 means "state that hashes to that entry --
     * i.e., the index of the boolean, has been seen. 0 means state
     * that hashes to that entry has not been seen. */
    ELEM_TYPE* hash_array;
};

/*Set of states. This is the TABU.*/
typedef SetOfStateHashes SetOfStates;

#endif
