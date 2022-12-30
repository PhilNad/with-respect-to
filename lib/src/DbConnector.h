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
        string db_dir_override;
        bool temporary_db;
        string db_path;
    public:
        DbConnector();
        DbConnector(uint8_t);
        DbConnector(string, uint8_t);
        ~DbConnector();
        GetSet In(string);
        //Flags definitions
        static const uint8_t TEMPORARY_DATABASE = 0b00000001;
};