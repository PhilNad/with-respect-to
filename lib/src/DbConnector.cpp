#include "DbConnector.h"
#include <regex>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <unistd.h>
using namespace std;

DbConnector::DbConnector(string db_dir_override): db_dir_override(db_dir_override),opened_world({}){
}

DbConnector::DbConnector(): db_dir_override({}),opened_world({}){
}

DbConnector::~DbConnector(){}

//Returns the absolute path to the directory in which this executable is located.
std::filesystem::path get_exe_dir_abs_path() {
    char buff[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buff, sizeof(buff)-1);
    if (len != -1) {
        buff[len] = '\0';
        std::filesystem::path current_exe{string(buff)};
        return current_exe.parent_path();
    }else{
        std::filesystem::path calling_dir{"."};
        return calling_dir;
    }
}

GetSet DbConnector::In(string world_name){
    if(!regex_match(world_name, regex(R"(^[0-9a-z\-]+$)")))
        throw runtime_error("Only [a-z], [0-9] and dash (-) is allowed in the world name.");
    
    auto DB_EXISTS = false;
    //Get the path to the directory of the executable
    std::filesystem::path exe_dir = get_exe_dir_abs_path();
    //Possibly override db directory
    if(db_dir_override.length() > 0){
        exe_dir = std::filesystem::path{db_dir_override};
    }

    //Get a list of existing databases in directory
    //to see if database already exists.
    for (auto const& entry : filesystem::directory_iterator{exe_dir}){
        std::filesystem::path entry_path = entry.path();
        if(entry.is_regular_file() && entry_path.extension() == ".db"){
            auto fn = entry_path.filename();
            if(fn == world_name+".db")
                DB_EXISTS = true;
        }
    }
    world_name = string(std::filesystem::absolute(exe_dir)) + "/" + world_name;
    if(DB_EXISTS == false){
        //Initialize the database.
        //Connects to the database and create it if it doesnt already exist.
        SQLite::Database db(world_name+".db", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
        /*
        Each row describes a single frame with
            - name : Unique string
            - parent: Name of parent frame (reference this frame is defined from)
            - R00,R01,R02: First row of the rotation matrix in the transformation
            - R10,R11,R12: Second row of the rotation matrix in the transformation
            - R20,R21,R22: Third row of the rotation matrix in the transformation
            - t0,t1,t2: Translation vector in the transformation
        The 'world' frame is always the inertial/immobile reference frame, it's parent is set to NULL/None.
        All other frames must have a non-NULL parent, creating a tree with a single root.
        */
        db.exec("CREATE TABLE IF NOT EXISTS frames( \
                        name TEXT PRIMARY KEY, \
                        parent TEXT, \
                        R00 REAL, \
                        R01 REAL, \
                        R02 REAL, \
                        R10 REAL, \
                        R11 REAL, \
                        R12 REAL, \
                        R20 REAL, \
                        R21 REAL, \
                        R22 REAL, \
                        t0 REAL, \
                        t1 REAL, \
                        t2 REAL \
                    );");
        db.exec("INSERT INTO frames VALUES ('world', NULL, 1,0,0, 0,1,0, 0,0,1, 0,0,0)");
    }
    opened_world = world_name;
    return GetSet(world_name);
}