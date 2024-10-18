#include "DbConnector.h"
#include <regex>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <pwd.h>
using namespace std;

DbConnector::DbConnector(string db_dir_override, uint8_t flags): db_dir_override(db_dir_override){
    //Each bit set to 1 corresponds to a flag being raised.
    //TEMPORARY_DATABASE: Delete database file when DbConnector is destroyed
    this->temporary_db = flags & this->TEMPORARY_DATABASE; 
}

//Delegated constructors
DbConnector::DbConnector(uint8_t flags): DbConnector("", flags){}
DbConnector::DbConnector(): DbConnector("", 0){}

DbConnector::~DbConnector(){
    //If the temporary flag was set
    if(this->temporary_db){
        //Remove the database file if it exists.
        auto p = std::filesystem::path{this->db_path};
        if(std::filesystem::exists(p)){
            std::filesystem::remove(p);
        }
        //Remove the test.db-shm file if it exists.
        p = std::filesystem::path{this->db_path + "-shm"};
        if(std::filesystem::exists(p)){
            std::filesystem::remove(p);
        }
        //Remove the test.db-wal file if it exists.
        p = std::filesystem::path{this->db_path + "-wal"};
        if(std::filesystem::exists(p)){
            std::filesystem::remove(p);
        }
    }
}

//Return the path to the user's home directory
std::filesystem::path get_home_dir(){
    char *homedir = getenv("HOME");

    if (homedir == NULL){
        homedir = getpwuid(getuid())->pw_dir;
        std::filesystem::path home_path{string(homedir)};
        return home_path;
    }else{
        std::filesystem::path calling_dir{homedir};
        return calling_dir;
    }
}

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

/*
* Check if a directory is writable.
*
* @param path: Path to the directory to check.
* @return: True if the directory is writable, false otherwise.
*/
bool DbConnector::IsDirectoryWritable(string path){
    return IsDirectoryWritable(std::filesystem::path{path});
}

/*
* Check if a directory is writable.
*
* @param path: Path to the directory to check.
* @return: True if the directory is writable, false otherwise.
*/
bool DbConnector::IsDirectoryWritable(std::filesystem::path path){
    if(std::filesystem::is_directory(path)){
        if(!std::filesystem::exists(path)){
            return false;
        }else{
            //The path is a directory and it exists, so test if it is writable.
            ofstream test_file(path.string()+"/test_if_directory_writable.txt");
            if(test_file.is_open()){
                test_file.close();
                std::filesystem::remove(path.string()+"/test_if_directory_writable.txt");
                return true;
            }else{
                return false;
            }
        }
    }else{
        //If the path is not a directory, check if the parent directory is writable
        return IsDirectoryWritable(path.parent_path());
    }
}

GetSet DbConnector::In(string world_name){
    if(!regex_match(world_name, regex(R"(^[0-9a-z\-]+$)")))
        throw runtime_error("Only [a-z], [0-9] and dash (-) is allowed in the world name.");
    
    /*
    The following rules are used to determine the directory in which the database is stored:
    1. If the db_dir_override is set, use that directory. Throw an exception if the directory is not writable.
    2. If this->temporary_db == True, use the /tmp directory if it is writable, otherwise use the home directory. 
    3. If this->temporary_db == False, and the executable is located in a directory that is writable, use that directory.
    4. If this->temporary_db == False, and the executable is located in a directory that is NOT writable, use the home directory.
    */

    auto DB_EXISTS = false;
    //Get the path to the directory of the executable
    std::filesystem::path exe_dir = get_exe_dir_abs_path();

    if(db_dir_override.length() > 0){
        //Use the user specified directory
        exe_dir = std::filesystem::path{db_dir_override};
    }else{
        //If the temporary flag is set, use the /tmp directory if it is writable, otherwise use the home directory.
        if(this->temporary_db){
            exe_dir = std::filesystem::path{"/tmp"};
            if(!IsDirectoryWritable(exe_dir)){
                exe_dir = get_home_dir();
            }
        }else{
            //If the directory of the executable is not writable, use the home directory.
            if(!IsDirectoryWritable(exe_dir)){
                exe_dir = get_home_dir();
            }
        }
    }

    //If the directory is not writable, throw an exception as we need to write a file somewhere.
    if(!IsDirectoryWritable(exe_dir)){
        throw runtime_error("The directory "+exe_dir.string()+" is not writable.");
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
    this->db_path = world_name+".db";

    if(DB_EXISTS == false){
        //Initialize the database.
        //Connects to the database and create it if it doesnt already exist.
        SQLite::Database db(this->db_path, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
        db.exec("PRAGMA journal_mode=WAL;");
        db.exec("PRAGMA synchronous = off;");
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
    return GetSet(world_name);
}