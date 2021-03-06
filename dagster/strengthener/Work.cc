//
//     MiniRed/GlucoRed
//
//     Siert Wieringa 
//     siert.wieringa@aalto.fi
// (c) Aalto University 2012/2013
//
//
#include"Work.h"
//#include"utils/Options.h"

/*
#ifdef MINIRED
using namespace MiniRed;
#elif defined GLUCORED
using namespace GlucoRed;
#endif
*/

// TODO arbitrary constant (same setting aalto had, not necesserily optimal)
static const int opt_reducer_work = 1000; //(VERSION_STRING, "work", "Maximum number of clauses in reducer work set", 1000, IntRange(2, INT32_MAX));

Work::Work()
    : free    (0)
    , workset (new E  [(int) opt_reducer_work])
    , space   (new E* [(int) opt_reducer_work])
    , newest  (NULL)
    , oldest  (NULL)
    , best    (NULL)
{
    for( int i = opt_reducer_work-1; i>=0; i-- ) space[i]=&(workset[free++]);
    assert(free==opt_reducer_work);
}

Work::~Work() {
    delete [] space;
    delete [] workset;
}

WorkWithPosArrayWithLength* Work::insert(WorkWithPosArrayWithLength* work, int smetric) {
    WorkWithPosArrayWithLength* ret = NULL;
    E* w;
    if ( free == 0) {
	w      = oldest;
	ret    = w->work;
	oldest = w->newer;
	if ( w == best   ) best   = w->worse;
	if ( w == newest ) newest = w->older;
	w->remove();
    }
    else w = space[--free];

    w->insert(work, smetric, best, newest);

    newest = w;
    if ( !w->better ) best   = w;
    if ( !w->older  ) oldest = w;

    return ret;
}

std::tuple<WorkWithPosArrayWithLength*, int> Work::get() {
    assert(available());

    E* b = best;
    WorkWithPosArrayWithLength* w = b->work;

    best = b->worse;
    if ( b == oldest ) oldest = b->newer;
    if ( b == newest ) newest = b->older;

    b->remove();
    space[free++] = b;

    std::tuple<WorkWithPosArrayWithLength*, int> retVal (w, b->smetric);
    return retVal;
}


/*

vec<Lit>* Work::insert(vec<Lit>* work, int smetric) {
    vec<Lit>* ret = NULL;
    E* w;
    if ( free == 0) {
	w      = oldest;
	ret    = w->work;
	oldest = w->newer;
	if ( w == best   ) best   = w->worse;
	if ( w == newest ) newest = w->older;
	w->remove();
    }
    else w = space[--free];

    w->insert(work, smetric, best, newest);

    newest = w;
    if ( !w->better ) best   = w;
    if ( !w->older  ) oldest = w;

    return ret;
}

vec<Lit>* Work::get() {
    assert(available());

    E* b = best;
    vec<Lit>* w = b->work;

    best = b->worse;
    if ( b == oldest ) oldest = b->newer;
    if ( b == newest ) newest = b->older;

    b->remove();
    space[free++] = b;

    return w;
}

 */
