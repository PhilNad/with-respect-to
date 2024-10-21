#pragma once

//Forward declaration
class WrtGet;
class WrtSet;

#include "DbConnector.h"
#include "ExpressedIn.h"
#include <string>
using namespace std;

/**
 * @brief When performing a Get() operation, this class exposes the Wrt() method that is used to define the basis reference frame.
 * 
 * Although possible, it is not recommended to use this class directly. You should instead perform full queries with:
 * In("world").Get("frame").Wrt("reference_frame").Ei("expressed_in_frame")
 */
class WrtGet
{
private:
    /// Name of the world/database to work in.
    string world_name;
    /// Name of the subject frame.
    string frame_name;
    /// Name of the basis frame.
    string ref_frame_name;
public:
    /**
     * @brief Interface to the Wrt operator. Do not use this class directly. For internal use only.
     * 
     * @param world_name: Name of the world/database to work in.
     * @param subject_frame: Name of the subject frame to Get().
     */
    WrtGet(string world_name, string subject_frame);
    ~WrtGet();
    /**
     * @brief Specify the basis frame with respect to which the subject frame is defined.
     * 
     * @param basis_frame: Name of the basis frame.
     * 
     * @return ExpressedInGet Interface to the Ei() operator.
     */
    ExpressedInGet Wrt(string basis_frame);
};

/**
 * @brief When performing a Set() operation, this class exposes the Wrt() method that is used to define the basis reference frame.
 * 
 * Although possible, it is not recommended to use this class directly. You should instead perform full queries with:
 * In("world").Get("frame").Wrt("reference_frame").Ei("expressed_in_frame")
 */
class WrtSet
{
private:
    /// Name of the world/database to work in.
    string world_name;
    /// Name of the subject frame.
    string frame_name;
    /// Name of the basis frame.
    string ref_frame_name;
public:
    /**
     * @brief Interface to the Wrt operator. Do not use this class directly. For internal use only.
     * 
     * @param world_name: Name of the world/database to work in.
     * @param subject_frame: Name of the subject frame to Set().
     */
    WrtSet(string world_name, string subject_frame);
    ~WrtSet();
    /**
     * @brief Specify the basis frame with respect to which the subject frame is defined.
     * 
     * @param basis_frame: Name of the basis frame.
     * 
     * @return ExpressedInSet Interface to the Ei() operator.
     */
    ExpressedInSet Wrt(string basis_frame);
};