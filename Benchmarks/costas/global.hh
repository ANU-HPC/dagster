#include<string>
#include<sstream>
#include<iostream>
#include<map>
#include<tuple>
#include<set>
#include<vector>
#include <algorithm>
#include <cmath>
#include <cassert>
#include <unistd.h>

using namespace std;


#define PARSE_ARGUMENT(X, Y){\
    std::istringstream iss(optarg);\
    iss>>X;\
    if (iss.fail()) {std::cerr << "unable to parse command link argument associated with "<<Y<<" flag.";} \
    }
