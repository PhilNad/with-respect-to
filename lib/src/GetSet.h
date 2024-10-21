#pragma once

//Forward declaration
class GetSet;

#include "DbConnector.h"
#include "WrtGetSet.h"
#include <string>
using namespace std;

/**
 * @brief Expose the Get() and Set() interfaces, to perform the eponymous operations.
 * 
 * Although possible, it is not recommended to use this class directly. You should instead perform full queries with:
 * In("world").Get("frame").Wrt("reference_frame").Ei("expressed_in_frame") 
 * or 
 * In("world").Set("frame").Wrt("reference_frame").Ei("expressed_in_frame").As(matrix).
 */
class GetSet
{
private:
    /// Name of the world/database to work in.
    string world_name;
    /// Name of the frame to Get/Set.
    string frame_name;
public:
    /**
     * @brief Interface to the Get/Set operators. Do not use this class directly. For internal use only.
     * 
     * @param world_name: Name of the frame to Get/Set.
     */
    GetSet(string world_name);
    ~GetSet();
    /**
     * @brief Define the operation type (Get) and the frame to perform it on.
     * 
     * @note You should use this method as part of a full query, such as In("world").Get("frame").Wrt("reference_frame").Ei("expressed_in_frame").
     * 
     * @param subject_frame: Name of the frame to Get.
     * @return WrtGet Interface to the Wrt() operator.
     */
    WrtGet Get(string subject_frame);
    /**
     * @brief Define the operation type (Set) and the frame to perform it on.
     * 
     * @note You should use this method as part of a full query, such as In("world").Set("frame").Wrt("reference_frame").Ei("expressed_in_frame").As(matrix).
     * 
     * @throw runtime_error: If the frame to set is named 'world' as the 'world' frame is assumed to be an inertial/immobile frame.
     * 
     * @param subject_frame: Name of the frame to Set.
     * @return WrtSet Interface to the Wrt() operator.
     */
    WrtSet Set(string subject_frame);
};