#pragma once

//Forward declaration
class WrtGet;
class WrtSet;

#include "DbConnector.h"
#include "ExpressedIn.h"
#include <string>
using namespace std;

class WrtGet
{
private:
    string world_name;
    string frame_name;
    string ref_frame_name;
public:
    WrtGet(string, string);
    ~WrtGet();
    ExpressedInGet Wrt(string);
};

class WrtSet
{
private:
    string world_name;
    string frame_name;
    string ref_frame_name;
public:
    WrtSet(string, string);
    ~WrtSet();
    ExpressedInSet Wrt(string);
};