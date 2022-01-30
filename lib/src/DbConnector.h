#pragma once

//Forward declaration
class DbConnector;

#include <SQLiteCpp/SQLiteCpp.h>
#include <string>
#include "GetSet.h"
using namespace std;

class DbConnector
{
    private:
        string opened_world;

    public:
        DbConnector();
        ~DbConnector();
        GetSet In(string);
};