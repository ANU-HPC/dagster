#ifndef PARTIALSTATE_HH
#define PARTIALSTATE_HH

#include "State.hh"

/* This is a partial state that only tracks the truth value of
 * propositions in constructor \argument{toTrack}.
 *
 * TODO: Turn conditional statements revolving around
 * \member{mapPropToStatePosition} and its clones into
 * assertions. Also, eventually remove the clone, as it will only be
 * useful in those assertions.
 */
class PartialState : public State
{
public:
    PartialState(const vector<uint>& toTrack = vector<uint>());
    PartialState(const PartialState&);
    PartialState& operator=(const PartialState&);

    bool operator==(const PartialState&) const;
    bool operator<(const PartialState&) const;

    inline bool tracking(uint proposition)
    {
        return (mapPropToStatePosition.find(proposition) 
                != mapPropToStatePosition.end());
    }

    /* (see \member{flipOff()}). Also check that \argument{proposition}
     * is indexed in \member{mapPropToStatePosition}.*/
    inline void flipOff__withTrackingTest(uint proposition)
    {
        if(tracking(proposition)){    
            flipOff(proposition);
        }
    }

    /* (see \member{flipOn()}). Also check that \argument{proposition}
     * is indexed in \member{mapPropToStatePosition}.*/
    inline void flipOn__withTrackingTest(uint proposition)
    {
        if(tracking(proposition)){    
            return flipOn(proposition);
        }
    }

    /* (see \member{flip()}). Also check that \argument{proposition}
     * is indexed in \member{mapPropToStatePosition}.*/
    inline void flip__withTrackingTest(uint proposition)
    {
        if(tracking(proposition)){    
            return flip(proposition);
        }
    }

    /* (see \member{isTrue()}). Also check that \argument{proposition}
     * is indexed in \member{mapPropToStatePosition}.*/
    inline bool isTrue__withTrackingTest(uint proposition)
    {
        if(tracking(proposition)){    
            return isTrue(proposition);
        }
    }

    /* (see \member{isFalse()}). Also check that
     * \argument{proposition} is indexed in
     * \member{mapPropToStatePosition}.*/
    inline bool isFalse__withTrackingTest(uint proposition)
    {
        if(tracking(proposition)){    
            return isFalse(proposition);
        }
        
        return true;
    }

    
    inline void flipOff(uint proposition)
    {
        State::flipOff(mapPropToStatePosition[proposition]);
    }

    inline void flipOn(uint proposition)
    {
        State::flipOn(mapPropToStatePosition[proposition]);
    }

    inline void flip(uint proposition)
    {
        State::flip(mapPropToStatePosition[proposition]);
    }

    inline bool isTrue(uint proposition)
    {
        return State::isTrue(mapPropToStatePosition[proposition]);
    }

    inline bool isFalse(uint proposition)
    {
        return !isTrue(proposition);
    }
private:
    typedef tr1::unordered_map<uint, uint, boost::hash<uint> > MapPropToStatePosition;

    /*Vector-based clone of member \member{mapPropToStatePosition}
     * (see \member{()}).*/
    vector<pair<uint, uint> > CLONE__mapPropToStatePosition;
    
    /* Maps a proposition (as an integer) to a truth value in the
     * parent \class{State} bitvector (see \member{PartialState()}).*/
    MapPropToStatePosition mapPropToStatePosition;
};

#endif
