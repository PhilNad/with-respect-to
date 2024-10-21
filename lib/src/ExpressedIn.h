#pragma once

//Forward declaration
class ExpressedInGet;
class ExpressedInSet;

#include "DbConnector.h"
#include <Eigen/Eigen>
#include <Eigen/Geometry>
#include <string>
using namespace std;

/**
 * @brief Defines a reference frame in relation to its parent frame through a rigid transformation.
 */
class RefFrame{
    public:
        /**
         * @brief Construct a new RefFrame object with a 4x4 transformation matrix.
         * 
         * @param subject_name: Name of the frame to define.
         * @param parent_name: Name of the parent frame that acts as the basis for the frame definition.
         * @param rigid_transformation_matrix: 4x4 transformation matrix defining the pose of the frame with respect to the parent and expressed in the parent frame.
         * 
         * @throw runtime_error: If the transformation matrix is not valid.
         */
        RefFrame(string, string, Eigen::Affine3d);
        /**
         * @brief Construct a new RefFrame object with a quaternion and a translation vector.
         * 
         * @param subject_name: Name of the frame to define.
         * @param parent_name: Name of the parent frame that acts as the basis for the frame definition.
         * @param q: Quaternion defining the rotation of the frame with respect to the parent frame.
         * @param t: Translation vector defining the position of the frame with respect to the parent frame.
         */
        RefFrame(string subject_name, string parent_name, Eigen::Quaterniond q, Eigen::Vector3d t);
        ~RefFrame();
        /// Name of the reference frame.
        string name;
        /// Name of the parent frame that acts as the basis.
        string parent_name;
        /// Quaternion encoding the orientation of the frame relative to its parent.
        Eigen::Quaterniond rotation;
        /// Position of the origin of the frame relative to its parent reference frame.
        Eigen::Vector3d translation;
};

/**
 * @brief Interface to the As() method to which the rigid transformation matrix is passed.
 */
class SetAs{
    private:
        /// Name of the world/database to work in.
        string world_name;
        /// Name of the subject frame.
        string subject_name;
        /// Name of the basis frame.
        string basis_name;
        /// Name of the coordinate system in which the transformation/pose is expressed.
        string csys_name;
        /// Timeout in milliseconds for the operation, default is 10 seconds.
        int timeout;
        /**
         * @brief Check if the specified frame exists in the supplied (opened) database.
         * 
         * @param database: A reference to an open connection to the database.
         * @param frame: Name of the frame to check.
         * 
         * @return true if the frame exists, false otherwise.
         */
        bool FrameExistsInDB(SQLite::Database& database, string frame);
    public:
        /**
         * @brief Interface to the As() operator. Do not use this class directly. For internal use only.
         * 
         * @param world_name: Name of the world/database to work in.
         * @param subject_name: Name of the subject frame to Set().
         * @param basis_name: Name of the basis frame.
         * @param csys_name: Name of the coordinate system in which the transformation/pose is expressed.
         * 
         * @throw runtime_error: If the name of any frame contains invalid characters.
         */
        SetAs(string world_name, string subject_name, string basis_name, string csys_name);
        ~SetAs();
        /**
         * @brief Used to specify the transformation defining the pose of the frame with respect to the basis frame and expressed in the selected coordinate system.
         * 
         * @note Calling this function will overwrite any previously defined frame with the same name.
         * 
         * @throw runtime_error: If the query is incorrect or if the transformation matrix is invalid.
         */
        void As(Eigen::Matrix4d transformation_matrix);
};

/**
 * @brief Interface to the Ei() method to which the name of the coordinate system is passed when performing a Get() operation.
 */
class ExpressedInGet
{
private:
    /// Name of the world/database to work in.
    string world_name;
    /// Name of the subject frame.
    string subject_name;
    /// Name of the basis frame.
    string basis_name;
    /// Name of the coordinate system in which the transformation/pose is expressed.
    string csys_name;
    /// Timeout in milliseconds for the operation, default is 10 seconds.
    int timeout;
    /**
     * @brief Get the definition of the parent frame of the specified frame as a RefFrame object.
     * 
     * @param subject_name: Frame whose parent is desired.
     * @return RefFrame Definition of the parent frame relative to its parent.
     */
    RefFrame GetParentFrame(string subject_name);
    /**
     * @brief (DEPRECATED) Compute the pose of the specified frame relative to the root of its tree (the only frame with no parent in the tree).
     * 
     * @note DEPRECATED. PoseWrtRootSQL() is much faster.
     * 
     * @param subject_name Name of the frame whose pose is desired.
     * @return tuple<Eigen::Affine3d pose, string> where the pose is the transformation matrix defining the pose of the frame with respect to the root frame and expressed in the root frame. The string is empty. 
     */
    tuple<Eigen::Affine3d, string> PoseWrtRoot(string subject_name);
    /**
     * @brief Compute the pose of the specified frame relative to the root of its tree (the only frame with no parent in the tree).
     * 
     * @note The name of the function comes from the fact that all computations are done directly from within the database, hence its speed.
     * 
     * @param subject_name Name of the frame whose pose is desired.
     * @return tuple<Eigen::Affine3d pose, string root_name> where the pose is the transformation matrix defining the pose of the frame with respect to the root frame whose name is root_name. 
     */
    tuple<Eigen::Affine3d, string> PoseWrtRootSQL(string subject_name);
public:
    /**
     * @brief Interface to the Ei() operator. Do not use this class directly. For internal use only.
     * 
     * @param world_name: Name of the world/database to work in.
     * @param subject_name: Name of the subject frame to Get().
     * @param basis_name: Name of the basis frame.
     * 
     * @throw runtime_error: If the name of any frame contains invalid characters.
     */
    ExpressedInGet(string world_name, string subject_name, string basis_name);
    ~ExpressedInGet();
    /**
     * @brief Used to specify the name of the coordinate system used to represent the pose of the subject frame relative to the basis frame.
     * 
     * @note Calling this function will trigger reading the database to answer the query.
     * 
     * @throw runtime_error: If the name of any frame contains invalid characters or if there is a problem with the pose graph.
     * 
     * @param csys_name: Name of the coordinate system used to represent the pose of the frame.
     * @return Eigen::Matrix4d Pose of the frame with respect to the selected basis and expressed in the chosen coordinate system.
     */
    Eigen::Matrix4d Ei(string csys_name);
};

/**
 * @brief Interface to the Ei() method to which the name of the coordinate system is passed when performing a Set() operation.
 */
class ExpressedInSet
{
private:
    /// Name of the world/database to work in.
    string world_name;
    /// Name of the subject frame.
    string subject_name;
    /// Name of the basis frame.
    string basis_name;
    /// Name of the coordinate system in which the transformation/pose is expressed.
    string csys_name;
    /// Timeout in milliseconds for the operation, default is 10 seconds.
    int timeout;
public:
    /**
     * @brief Interface to the Ei() operator. Do not use this class directly. For internal use only.
     * 
     * @param world_name: Name of the world/database to work in.
     * @param subject_name: Name of the subject frame to Get().
     * @param basis_name: Name of the basis frame.
     * 
     * @throw runtime_error: If the name of any frame contains invalid characters.
     */
    ExpressedInSet(string world_name, string subject_name, string basis_name);
    ~ExpressedInSet();
    /**
     * @brief Used to specify the name of the coordinate system used to represent the pose of the subject frame relative to the basis frame.
     * 
     * @throw runtime_error: If the name of any frame contains invalid characters.
     * 
     * @param csys_name: Name of the coordinate system used to represent the pose of the frame.
     * @return SetAs Interface to the As() function used to specify the transformation matrix.
     */
    SetAs Ei(string csys_name);
};