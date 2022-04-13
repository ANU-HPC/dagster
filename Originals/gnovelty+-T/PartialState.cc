#include "PartialState.hh"


PartialState::PartialState(const vector<uint>& toTrack)
    :State(toTrack.size())
{
    for(uint i = 0; i < toTrack.size(); i++){
        mapPropToStatePosition[toTrack[i]] = i;
        CLONE__mapPropToStatePosition.push_back(pair<uint, uint>(toTrack[i], i));
    }
}

PartialState::PartialState(const PartialState& partialState)
    :State::State(partialState)
{
    mapPropToStatePosition = partialState.mapPropToStatePosition;
    CLONE__mapPropToStatePosition = partialState.CLONE__mapPropToStatePosition;
}

PartialState& PartialState::operator=(const PartialState& partialState)
{
    this->mapPropToStatePosition = partialState.mapPropToStatePosition;
    this->CLONE__mapPropToStatePosition = partialState.CLONE__mapPropToStatePosition;
    State::operator=(partialState);
    return *this;
}



bool PartialState::operator==(const PartialState& partialState) const
{
    return (CLONE__mapPropToStatePosition.size() == partialState.CLONE__mapPropToStatePosition.size()) &&
        (CLONE__mapPropToStatePosition == CLONE__mapPropToStatePosition) &&
        (State::operator==(partialState));
}

bool PartialState::operator<(const PartialState& partialState) const
{
//     CLONE__mapPropToStatePosition < CLONE__mapPropToStatePosition;
    
    if(CLONE__mapPropToStatePosition.size() < partialState.CLONE__mapPropToStatePosition.size()){
        return true;
    } else if (CLONE__mapPropToStatePosition.size() == partialState.CLONE__mapPropToStatePosition.size()) {
        if(CLONE__mapPropToStatePosition < partialState.CLONE__mapPropToStatePosition){
            return true;
        } else if (CLONE__mapPropToStatePosition == partialState.CLONE__mapPropToStatePosition){
            return State::operator<(partialState);
        }
    }

    return false;
}

