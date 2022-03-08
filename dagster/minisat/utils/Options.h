/***************************************************************************************[Options.h]
Copyright (c) 2008-2010, Niklas Sorensson

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT
OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**************************************************************************************************/

#ifndef Minisat_Options_h
#define Minisat_Options_h

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "mtl/IntTypes.h"
#include "mtl/Vec.h"






namespace Minisat {



//==================================================================================================
// Options is an abstract class that gives the interface for all types options:


class Option
{
 protected:
    const char* name;
    const char* description;
    const char* category;
    const char* type_name;

    static vec<Option*>& getOptionList () { static vec<Option*> options; return options; }

    Option(const char* name_, 
           const char* desc_,
           const char* cate_,
           const char* type_) : 
      name       (name_)
    , description(desc_)
    , category   (cate_)
    , type_name  (type_)
    { 
        getOptionList().push(this); 
    }

 public:
    virtual ~Option() {}
};


//==================================================================================================
// Range classes with specialization for floating types:


struct IntRange {
    int begin;
    int end;
    IntRange(int b, int e) : begin(b), end(e) {}
};

struct Int64Range {
    int64_t begin;
    int64_t end;
    Int64Range(int64_t b, int64_t e) : begin(b), end(e) {}
};

struct DoubleRange {
    double begin;
    double end;
    bool  begin_inclusive;
    bool  end_inclusive;
    DoubleRange(double b, bool binc, double e, bool einc) : begin(b), end(e), begin_inclusive(binc), end_inclusive(einc) {}
};


//==================================================================================================
// Double options:


class DoubleOption : public Option
{
 protected:
    DoubleRange range;
    double      value;

 public:
    DoubleOption(const char* c, const char* n, const char* d, double def = double(), DoubleRange r = DoubleRange(-HUGE_VAL, false, HUGE_VAL, false))
        : Option(n, d, c, "<double>"), range(r), value(def) {
        // FIXME: set LC_NUMERIC to "C" to make sure that strtof/strtod parses decimal point correctly.
    }

    operator      double   (void) const { return value; }
    operator      double&  (void)       { return value; }
    DoubleOption& operator=(double x)   { value = x; return *this; }
};


//==================================================================================================
// Int options:


class IntOption : public Option
{
 protected:
    IntRange range;
    int32_t  value;

 public:
    IntOption(const char* c, const char* n, const char* d, int32_t def = int32_t(), IntRange r = IntRange(INT32_MIN, INT32_MAX))
        : Option(n, d, c, "<int32>"), range(r), value(def) {}
 
    operator   int32_t   (void) const { return value; }
    operator   int32_t&  (void)       { return value; }
    IntOption& operator= (int32_t x)  { value = x; return *this; }
};


// Leave this out for visual C++ until Microsoft implements C99 and gets support for strtoll.
#ifndef _MSC_VER

class Int64Option : public Option
{
 protected:
    Int64Range range;
    int64_t  value;

 public:
    Int64Option(const char* c, const char* n, const char* d, int64_t def = int64_t(), Int64Range r = Int64Range(INT64_MIN, INT64_MAX))
        : Option(n, d, c, "<int64>"), range(r), value(def) {}
 
    operator     int64_t   (void) const { return value; }
    operator     int64_t&  (void)       { return value; }
    Int64Option& operator= (int64_t x)  { value = x; return *this; }
};
#endif

//==================================================================================================
// String option:


class StringOption : public Option
{
    const char* value;
 public:
    StringOption(const char* c, const char* n, const char* d, const char* def = NULL) 
        : Option(n, d, c, "<string>"), value(def) {}

    operator      const char*  (void) const     { return value; }
    operator      const char*& (void)           { return value; }
    StringOption& operator=    (const char* x)  { value = x; return *this; }
};


//==================================================================================================
// Bool option:


class BoolOption : public Option
{
    bool value;

 public:
    BoolOption(const char* c, const char* n, const char* d, bool v) 
        : Option(n, d, c, "<bool>"), value(v) {}

    operator    bool     (void) const { return value; }
    operator    bool&    (void)       { return value; }
    BoolOption& operator=(bool b)     { value = b; return *this; }
};

//=================================================================================================
}

#endif
