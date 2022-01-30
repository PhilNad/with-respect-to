#pragma once

//Forward declaration
class GetSet;

#include "DbConnector.h"
#include "WrtGetSet.h"
#include <string>
using namespace std;

class GetSet
{
private:
    string world_name;
    string frame_name;
public:
    GetSet(string);
    ~GetSet();
    WrtGet Get(string);
    WrtSet Set(string);
};