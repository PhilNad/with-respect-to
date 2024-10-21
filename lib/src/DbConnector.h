#pragma once

//Forward declaration
class DbConnector;

#include <SQLiteCpp/SQLiteCpp.h>
#include <string>
#include <filesystem>
#include "GetSet.h"
using namespace std;

/**
 * @brief Connects to a database and provides an interface to access it.
 * 
 * A database connection is made through the DbConnector constructor. The In() method is then used to select a database (also called *world*) to work in.
 * 
 * Example 1: Connecting to the *laboratory* world in a temporary database.
 * 
 *     auto wrt = DbConnector(DbConnector::TEMPORARY_DATABASE);
 *     auto world = wrt.In("laboratory")
 * 
 * Example 2: Connecting to the *kitchen* world in a persistent database.
 * 
 *    auto wrt = DbConnector();
 *    auto world = wrt.In("kitchen")
 * 
 * Example 3: Connecting to the *deed* world in a temporary database located in the directory *\/tmp*.
 * 
 *    auto wrt = DbConnector("/tmp", DbConnector::TEMPORARY_DATABASE);
 *    auto world = wrt.In("deed")
 */
class DbConnector
{
    private:
        /// Used to store the user-specified directory for the database.
        string db_dir_override;
        /// Whether the database is temporary or not.
        bool temporary_db;
        /// Path to the database.
        string db_path;
        /**
         * @brief Check if the directory at the specified path is writable.
         * 
         * @note If the path points to a file, the parent directory is checked.
         * 
         * @param path Either a string or a filesystem::path object pointing to the directory.
         * @return true if the directory is writable, false otherwise.
         */
        bool IsDirectoryWritable(std::filesystem::path path);
        bool IsDirectoryWritable(string path);
    public:
        /**
         * @brief Construct a new DbConnector object to a persistent database located in the directory of the executable or the home directory if the directory of the executable is not writable.
         */
        DbConnector();
        /**
         * @brief Construct a new DbConnector object using the user-specified flags.
         * 
         * @param flags: Options to use when creating the database (by default, no flag is set). 
         * 
         * @see DbConnector::TEMPORARY_DATABASE
         */
        DbConnector(uint8_t flags);
        /**
         * @brief Construct a new DbConnector object using the user-specified directory and flags.
         * 
         * @param path: Path to the directory where the database is stored.
         * @param flags: Options to use when creating the database (by default, no flag is set).
         * 
         * @see DbConnector::TEMPORARY_DATABASE
         */
        DbConnector(string path, uint8_t flags);
        ~DbConnector();
        /**
         * @brief Set up a connection to the database to the specified *world*.
         * 
         * @param world: Name of the world to connect to. Only [a-z], [0-9] and dash (-) is allowed in the world name.
         * 
         * @return GetSet Interface to Get/Set frames in the selected *world*.
         * 
         * @throws filesystem::filesystem_error if the database directory is not writable.
         */
        GetSet In(string world);
        /// Flag specifying that the database should be deleted when the DbConnector object is destroyed.
        static const uint8_t TEMPORARY_DATABASE = 0b00000001;
};