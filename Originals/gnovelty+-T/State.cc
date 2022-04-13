#include "State.hh"


using namespace tr1;

State& State::operator=(const State& state)
{
    exit(0);
    VERBOSER(1, "State::operator="<<endl);

    /*Assert we are not copy constructing a null-state.*/
    assert(state.numPropositions);
    
    this->numPropositions = state.numPropositions;
    //this->data = state.data;

    if(0 != data){
        VERBOSER(1, "Deleting old bitvector."<<endl);
        delete [] data;
    }

    VERBOSER(1, "Allocating new bitvector."<<endl);
    data = new ELEM_TYPE[CEIL(numPropositions)];
    for(uint i = 0; i < CEIL(numPropositions); i++){    
        data[i] = state.data[i];
    }
    
    assert(*this == state);
    
    return *this;
}

State::State(const State& state)
    :numPropositions(state.numPropositions)
{
    //exit(0);
    VERBOSER(1, "State::State() Copy construction"<<endl);
    
    data = new ELEM_TYPE[CEIL(numPropositions)];
    for(uint i = 0; i < CEIL(numPropositions); i ++)data[i] = state.data[i];

    assert(*this == state);
    
}

State::State(uint size)
    :numPropositions(size),
     data(0)
{
    //data = vector<lui>(CEIL(size));
    //data = new lui[CEIL(size)];
    
//     for(uint i =0; i < data.size(); i++){
// 	data[i] = 0;
//     }

//     for(uint i = SIZE_ELEM * data.size(); i > numPropositions; i--){
// 	flipOn(i);
//     }
    assert(size == numPropositions);
    data = new ELEM_TYPE[CEIL(numPropositions)];
    
    for(uint i = 0; i < CEIL(numPropositions); i++){
        VERBOSER(2, "Assignment to element "<<i<<" in data of size "<<(CEIL(numPropositions))<<"\n");
	data[i] = 0;
        VERBOSER(2, "Done assignment to element "<<i<<" in data\n");
    }
    
//     for(uint i = (SIZE_ELEM * CEIL(numPropositions)) - 1 ; i > numPropositions; i--){
//         assert(i >= 0);
//         assert(i > numPropositions);
        
//         VERBOSER(2, "Assignment to element "<<i<<" in data of size "<<(SIZE_ELEM * CEIL(numPropositions))<<"\n");
// 	flipOn(i);
//         VERBOSER(2, "*** Assignment to element "<<i<<" in data of size "<<(SIZE_ELEM * CEIL(numPropositions))<<"\n");
//     }
}

State::~State()
{
    if(0 != data)
        delete [] data;
}


bool State::operator==(const State& state) const
{
    if(numPropositions != state.numPropositions) return false;
    
    for(uint i = 0; i < CEIL(numPropositions); i++){
        if(data[i] != state.data[i]) {
            return false;
        }
    }
    
    return true;//state.data == this->data;
}
 
bool State::operator<(const State& state) const
{
    if(numPropositions > state.numPropositions){
        return false;
    } else if (numPropositions == state.numPropositions) {
        for(uint i = 0; i < CEIL(numPropositions); i++){
            if(data[i] > state.data[i]) {
                return false;
            } else if (data[i] < state.data[i]) {
                return true;
            }
        }
    } else {
        return true;
    }
    
    assert(*this == state);
    
    return false;
    
    
//     return (state.data.size() < this->data.size())?true:
// 	((state.data.size() == this->data.size())?(state.data < this->data):false);
}
    
size_t State::hash_value() const
{
    register size_t seed = 0;
    register uint i;
    
    for(i = 0; i < CEIL(numPropositions); i++){
        boost::hash_combine(seed, data[i]);
    }
    
    return seed;

    /*The following did not seem to be thread safe.*/
    
    //return boost::hash_range(data, data + numPropositions);//data.begin(), data.end());
}

void State::randomize()
{
    register ELEM_TYPE tmp;
    register uint i;
    register unsigned short int j;
	
    for( i = 0; i < numPropositions/*data.size()*/; i++){
	for(j = 0 ; j < sizeof(ELEM_TYPE) * 8; j++){
	    tmp = 1;
	    tmp = tmp<<j;
	    if(random() % 2){
		tmp = tmp^big;
		data[i] &= tmp;
	    } else {
		data[i] |= tmp;
	    }
	}
    }
}

// /*Change individual (at index \argument{uing}) bits of the
//  * state representation (\member{data}).*/
// void State::flipOn(uint index)
// {
//     uint remainder = REMAINDER(index);
//     uint bitVecNum = FLOOR(index);

//     uint tmp = 1;
//     tmp = tmp<<remainder;
//     data[bitVecNum] |= tmp;
    
//     /*Now it's true, some actions may be executable.*/
// }

// void State::flipOff(uint index)
// {
//     uint remainder = REMAINDER(index);
//     uint bitVecNum = FLOOR(index);

//     uint tmp = 1;
//     tmp = tmp<<remainder;
//     tmp = tmp^big;
//     data[bitVecNum] &= tmp;
// }

// void State::flip(uint index)
// {
//     uint remainder = REMAINDER(index);
//     uint bitVecNum = FLOOR(index);

//     uint tmp = 1;
//     tmp = tmp<<remainder;
    
//     if(data[bitVecNum] & tmp){
// 	tmp = tmp^big;
// 	data[bitVecNum] &= tmp;
//     } else {
// 	data[bitVecNum] |= tmp;

// 	/*Now it's true, some actions may be executable.*/
//     }
// }

// bool State::isTrue(uint index) const
// {
//     uint remainder = REMAINDER(index);
//     uint bitVecNum = FLOOR(index);

//     uint tmp = 1;
//     tmp = tmp<<remainder;
    
//     return (data[bitVecNum] & tmp)?true:false;
// }



    
/*Function for STL and boost to access \member{hash_value} of
 * \argument{GroundAction}.*/
std::size_t hash_value(const State& state)
{
	return state.hash_value();
}

