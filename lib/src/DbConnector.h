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
        string db_dir_override;

    public:
        DbConnector();
        DbConnector(string);
        ~DbConnector();
        GetSet In(string);
};