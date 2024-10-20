
#include "ExpressedIn.h"
#include <regex>
#include <cfloat>
#include <iostream>
#include <tuple>
using namespace std;

bool VerifyInput(string name){
    return regex_match(name, regex(R"(^[0-9a-z\-]+$)"));
}

int VerifyMatrix(Eigen::Affine3d transfo_matrix){
    //1) Verify that the rotation matrix is nearly orthogonal (transpose(R) == inverse(R))
    Eigen::Matrix3d rot = transfo_matrix.rotation();
    Eigen::Matrix3d result = rot * rot.transpose() - Eigen::Matrix3d::Identity();
    auto tolerance = 100 * DBL_EPSILON;
    if(result.norm() > tolerance)
        return -1;
    //2) Verify that the determinant of the rotation matrix is 1
    if(abs(rot.determinant() - 1) > tolerance)
        return -2;
    //3) Verify that the last row is [0,0,0,1]
    if( transfo_matrix(3,0) != 0 || transfo_matrix(3,1) != 0 || transfo_matrix(3,2) != 0 || transfo_matrix(3,3) != 1 )
        return -3;
    return 0;
}

//RefFrame constructor taking a transformation matrix as input
RefFrame::RefFrame(string subject_name, string parent_name, Eigen::Affine3d transfo):
    name(subject_name),
    parent_name(parent_name),
    rotation(transfo.rotation()),
    translation(transfo.translation()){
        int code = VerifyMatrix(transfo);
        if(code < 0)
            throw runtime_error("The format of the submitted matrix is wrong ("+to_string(code)+").");
        //Normalize the quaternion
        rotation.normalize();
}

//RefFrame constructor taking a quaternion and a translation vector as input
RefFrame::RefFrame(string subject_name, string parent_name, Eigen::Quaterniond q, Eigen::Vector3d t):
    name(subject_name),
    parent_name(parent_name),
    rotation(q),
    translation(t){
        //Normalize the quaternion
        rotation.normalize();
}

RefFrame::~RefFrame(){}

SetAs::SetAs(string world_name, string subject_name, string basis_name, string csys_name):
    world_name(world_name), 
    subject_name(subject_name), 
    basis_name(basis_name),
    csys_name(csys_name){
    if(!VerifyInput(subject_name) || !VerifyInput(basis_name) || !VerifyInput(csys_name))
        throw runtime_error("Only [a-z], [0-9] and dash (-) is allowed in the frame name.");
    //Set the timeout to 10 seconds
    this->timeout = 10000;
}

SetAs::~SetAs(){}

/*
*    Check if a frame exists in the database
*
*    @param db: The database to check
*    @param subject_name: The name of the frame to check
*
*    @return: True if the frame exists, false otherwise
*/
bool SetAs::FrameExistsInDB(SQLite::Database& db, string subject_name){
    SQLite::Statement   query(db, "SELECT * FROM frames WHERE name IS ?");
    query.bind(1, subject_name);

    //Verify that the frame exists.
    int row_counter = 0;
    while (query.executeStep()){
        row_counter++;
    }
    if(row_counter == 0)
        return false;
    if(row_counter != 1)
        throw runtime_error("Need a single reference frame "+subject_name+".");
    return true;
}

//Write to the database the transformation matrix defining the frame subject_name with respect to the frame basis_name
// and expressed in the frame basis_name, that is X_S_B.
void SetAs::As(Eigen::Matrix4d transformation_matrix){
    Eigen::Affine3d transfo_matrix;
    transfo_matrix.matrix() = transformation_matrix;
    int code = VerifyMatrix(transfo_matrix);
    if(code < 0)
        throw runtime_error("The format of the submitted matrix is wrong ("+to_string(code)+").");
    //List all existing reference names
    SQLite::Database db(this->world_name+".db", SQLite::OPEN_READWRITE, this->timeout);
    db.exec("PRAGMA journal_mode=WAL;");
    db.exec("PRAGMA synchronous = off;");

    /* Cases:
    * 1) R,F,I defined                          : Normal case, will overwrite previous definition
    * 2) R,I defined and F undefined            : Normal case, will introduce a new frame
    * 3) F,I defined and R undefined            : Reverse the command
    * 4) R,F undefined and R != I               : Error
    * 5) R,I undefined and R != I               : Error
    * 6) R,I undefined and R == I and F defined : Permitted, will potentially introduce a disconnected frame.
    * 7) R,F,I undefined and R == I             : Permitted, will potentially introduce a disconnected frame.
    * 8) I undefined and R != I                 : Error
    */

    //Check the existence of the frames
    bool in_frame_exists = this->FrameExistsInDB(db, this->csys_name);
    bool ref_frame_exists = this->FrameExistsInDB(db, this->basis_name);
    bool frame_exists = this->FrameExistsInDB(db, this->subject_name);

    //Case 4
    if(!ref_frame_exists && !frame_exists && this->basis_name != this->csys_name){
        throw runtime_error("Reference frames "+this->basis_name+" and "+this->subject_name+" do not exist in this world.");
    }

    //Case 5
    if(!ref_frame_exists && !in_frame_exists && this->basis_name != this->csys_name){
        //We are now permitting setting a reference frame with respect to a parent frame that has not yet been defined
        // unless the csys_name is different from the basis_name.
        throw runtime_error("Reference frames "+this->basis_name+" and "+this->csys_name+" do not exist in this world.");
    }
    
    //Case 8
    if(!in_frame_exists && this->basis_name != this->csys_name){
        throw runtime_error("Reference frames "+this->csys_name+" does not exist in this world.");
    }

    //Case 3
    //If the ref_frame is undefined BUT the frame is defined, we reverse the command to SET ref_frame WRT frame AS transformation_matrix.inverse()
    if(!ref_frame_exists && frame_exists){
        auto setter = ExpressedInSet(this->world_name, this->basis_name, this->subject_name);
        //Inverse the transformation matrix. In general, reversing a transformation matrix cannot be done by simply taking the inverse
        // as doing so assumes that the ref_frame is the same as the in_frame. This is not necessarily the case here.
        Eigen::Matrix4d inversed_transformation_matrix = Eigen::Matrix4d::Identity();
        // R_r_f = R_f_r_Tran
        inversed_transformation_matrix.block<3,3>(0,0) = transformation_matrix.block<3,3>(0,0).transpose();
        // p_r_f_i = - p_r_f_i
        inversed_transformation_matrix.block<3,1>(0,3) = -1 * transformation_matrix.block<3,1>(0,3);
        
        setter.Ei(this->csys_name).As(inversed_transformation_matrix);
        return;
    }
    
    //By here, we can assume that we are dealing with case 1 or 2.

    Eigen::Matrix3d R_C_B;
    //If the ref_frame is different from the in_frame
    if(this->basis_name != this->csys_name){
        //Take into account the fact that the transformation can be expressed in a frame different from the reference frame
        //       Like: SET object WRT table EI world
        auto getter = ExpressedInGet(this->world_name, this->csys_name, this->basis_name);
        auto X_C_B = getter.Ei(this->basis_name);
        R_C_B = X_C_B(Eigen::seq(0,2), Eigen::seq(0,2));
    }else{
        //If the ref_frame is the same as the in_frame, the identity matrix relates them.
        R_C_B = Eigen::Matrix3d::Identity();
    }

    // The position vector is expressed in the ref_frame through
    // p_S_B = R_C_B * p_S_B_C
    // such that the stored pose is X_S_B = [R_S_B, p_S_B; 0,0,0,1]
    auto R = transfo_matrix.rotation();
    auto t = R_C_B * transfo_matrix.translation(); 

    SQLite::Transaction transaction(db);
    
    //Remove from DB any frame with  __subject_name
    SQLite::Statement   q1(db, "DELETE FROM frames WHERE name IS ?");
    q1.bind(1, this->subject_name);
    q1.executeStep();

    //Store the frame built from R_S_B and p_S_B
    SQLite::Statement   q2(db, "INSERT INTO frames VALUES (?, ?, ?,?,?, ?,?,?, ?,?,?, ?,?,?)");
    q2.bind(1, this->subject_name);
    q2.bind(2, this->basis_name);
    q2.bind(3,  R(0,0));
    q2.bind(4,  R(0,1));
    q2.bind(5,  R(0,2));
    q2.bind(6,  R(1,0));
    q2.bind(7,  R(1,1));
    q2.bind(8,  R(1,2));
    q2.bind(9,  R(2,0));
    q2.bind(10, R(2,1));
    q2.bind(11, R(2,2));
    q2.bind(12, t(0));
    q2.bind(13, t(1));
    q2.bind(14, t(2));
    q2.executeStep();

    //Commit: Either everything is done or nothing is done.
    transaction.commit();
}


ExpressedInGet::ExpressedInGet(string world_name, string subject_name, string basis_name):
    world_name(world_name), 
    subject_name(subject_name), 
    basis_name(basis_name){
    if(!VerifyInput(subject_name) || !VerifyInput(basis_name))
        throw runtime_error("Only [a-z], [0-9] and dash (-) is allowed in the frame name.");
    //Set the timeout to 10 seconds
    this->timeout = 10000;
}

ExpressedInGet::~ExpressedInGet(){}

RefFrame ExpressedInGet::GetParentFrame(string subject_name){
    SQLite::Database db(this->world_name+".db", SQLite::OPEN_READONLY, this->timeout);
    db.exec("PRAGMA journal_mode=WAL;");
    db.exec("PRAGMA synchronous = off;");

    SQLite::Statement   query(db, "SELECT * FROM frames WHERE name IS ?");
    query.bind(1, subject_name);

    //Values to be read from the database
    string name;
    string parent_name;
    double R00, R01, R02;
    double R10, R11, R12;
    double R20, R21, R22;
    double t0, t1, t2;
    //Verify that the frame exists.
    int row_counter = 0;
    while (query.executeStep()){
        row_counter++;
        name = query.getColumn(0).getText();
        parent_name = query.getColumn(1).getText();

        R00    = query.getColumn(2).getDouble();
        R01    = query.getColumn(3).getDouble();
        R02    = query.getColumn(4).getDouble();
        
        R10    = query.getColumn(5).getDouble();
        R11    = query.getColumn(6).getDouble();
        R12    = query.getColumn(7).getDouble();
        
        R20    = query.getColumn(8).getDouble();
        R21    = query.getColumn(9).getDouble();
        R22    = query.getColumn(10).getDouble();

        t0     = query.getColumn(11).getDouble();
        t1     = query.getColumn(12).getDouble();
        t2     = query.getColumn(13).getDouble();
    }
    if(row_counter == 0)
        throw runtime_error("The reference frame "+this->subject_name+" does not exist in this world.");
    if(row_counter != 1)
        throw runtime_error("Need a single reference frame "+this->subject_name+".");

    //If the value is lower than machine precision, set it to zero.
    R00 = (abs(R00) < DBL_EPSILON) ? 0 : R00;
    R01 = (abs(R01) < DBL_EPSILON) ? 0 : R01;
    R02 = (abs(R02) < DBL_EPSILON) ? 0 : R02;
    R10 = (abs(R10) < DBL_EPSILON) ? 0 : R10;
    R11 = (abs(R11) < DBL_EPSILON) ? 0 : R11;
    R12 = (abs(R12) < DBL_EPSILON) ? 0 : R12;
    R20 = (abs(R20) < DBL_EPSILON) ? 0 : R20;
    R21 = (abs(R21) < DBL_EPSILON) ? 0 : R21;
    R22 = (abs(R22) < DBL_EPSILON) ? 0 : R22;
    t0 = (abs(t0) < DBL_EPSILON) ? 0 : t0;
    t1 = (abs(t1) < DBL_EPSILON) ? 0 : t1;
    t2 = (abs(t2) < DBL_EPSILON) ? 0 : t2;

    //Build a transformation matrix
    Eigen::Affine3d tr;
    tr.matrix() <<  R00, R01, R02, t0,
                    R10, R11, R12, t1,
                    R20, R21, R22, t2,
                    0,0,0,1;

    RefFrame parent_frame(name, parent_name, tr);
    return parent_frame;
}

//Return the pose of subject_name relative to world reference frame, expressed in world.
// or if a kinematic loop is detected, return the subject_name as the parent frame.
tuple<Eigen::Affine3d, string> ExpressedInGet::PoseWrtRoot(string subject_name){
    RefFrame f = ExpressedInGet::GetParentFrame(subject_name);
    Eigen::Quaterniond new_ori(f.rotation);
    Eigen::Vector3d new_pos(f.translation);
    
    //The parent_name of the root frame is empty so we can use it as a stop condition.
    while(f.parent_name.length() > 0){
        auto parent_frame = ExpressedInGet::GetParentFrame(f.parent_name);
        Eigen::Quaterniond ori(parent_frame.rotation);
        Eigen::Vector3d pos(parent_frame.translation);
        //Compute the composed orientation
        new_ori = ori * new_ori;
        //Compute the composed position
        new_pos = ori * new_pos + pos;
        //Go deeper in the tree
        f = parent_frame;
        //If the name of the parent frame is the same as the initial frame, we have a loop
        // and its impossible to return the pose of the frame relative to the root frame.
        if(f.name == subject_name){
            throw runtime_error("The frame "+this->subject_name+" is part of a kinematic loop.");
        }
    }

    Eigen::Affine3d new_transfo = Eigen::Affine3d::Identity();
    new_transfo.linear()        = new_ori.toRotationMatrix();
    new_transfo.translation()   = new_pos;

    return {new_transfo, f.parent_name};
}

//Return the pose of subject_name relative to root reference frame, expressed in the root frame.
// This version is independant of the name of the root frame.
// This version performs everything from within the database, increasing drastically the speed.
// Currently this does not use quaternions as rotation matrices are used in the database.
// WARNING: There is a limit of 100 recursions, if you have a kinematic link longer than that, it will fail.
tuple<Eigen::Affine3d, string> ExpressedInGet::PoseWrtRootSQL(string subject_name){
    SQLite::Database db(this->world_name+".db", SQLite::OPEN_READWRITE, this->timeout);
    db.exec("PRAGMA journal_mode=WAL;");
    db.exec("PRAGMA synchronous = off;");

    //This query create a temporary table (CTE) through a recursive query that is started with the
    // first SELECT statement, which generate a row that is then the input to the following SELECT.
    // The second SELECT performs transform composition. So the query go from a leaf to a root,
    // compositing the transform at each step. The third SELECT is used to get the result obtained
    // at the end of the recursive process, without prior knowledge about the name of the root frame.
    SQLite::Statement   query(db, "\
    WITH RECURSIVE get_parent (i, n, p, b00, b01, b02, b10, b11, b12, b20, b21, b22, bx, by, bz) \
    AS ( \
        select 0, frames.* from frames where frames.name = ? \
        UNION ALL \
        SELECT i+1, name, parent, \
        r00*b00+r01*b10+r02*b20, \
        r00*b01+r01*b11+r02*b21, \
        r00*b02+r01*b12+r02*b22, \
        r10*b00+r11*b10+r12*b20, \
        r10*b01+r11*b11+r12*b21, \
        r10*b02+r11*b12+r12*b22, \
        r20*b00+r21*b10+r22*b20, \
        r20*b01+r21*b11+r22*b21, \
        r20*b02+r21*b12+r22*b22, \
        r00*bx+r01*by+r02*bz+t0, \
        r10*bx+r11*by+r12*bz+t1, \
        r20*bx+r21*by+r22*bz+t2 \
        FROM frames, get_parent WHERE name = get_parent.p \
        LIMIT 100 \
    ) \
    SELECT n, p, b00, b01, b02, b10, b11, b12, b20, b21, b22, bx, by, bz FROM get_parent ORDER BY i DESC LIMIT 1; \
    ");
    query.bind(1, subject_name);

    //Values to be read from the database
    string name;
    string parent_name;
    double R00, R01, R02;
    double R10, R11, R12;
    double R20, R21, R22;
    double t0, t1, t2;
    //Verify that the frame exists.
    int row_counter = 0;
    while (query.executeStep()){
        row_counter++;
        name = query.getColumn(0).getText();
        parent_name = query.getColumn(1).getText();

        R00    = query.getColumn(2).getDouble();
        R01    = query.getColumn(3).getDouble();
        R02    = query.getColumn(4).getDouble();
        
        R10    = query.getColumn(5).getDouble();
        R11    = query.getColumn(6).getDouble();
        R12    = query.getColumn(7).getDouble();
        
        R20    = query.getColumn(8).getDouble();
        R21    = query.getColumn(9).getDouble();
        R22    = query.getColumn(10).getDouble();

        t0     = query.getColumn(11).getDouble();
        t1     = query.getColumn(12).getDouble();
        t2     = query.getColumn(13).getDouble();
    }

    string root_name;
    if(row_counter == 0)
        throw runtime_error("The reference frame "+subject_name+" does not exist in this world.");
    else{
        if(name == "world")
            root_name = "world";
        else
            root_name = parent_name;
    }

    //If the value is lower than machine precision, set it to zero.
    R00 = (abs(R00) < DBL_EPSILON) ? 0 : R00;
    R01 = (abs(R01) < DBL_EPSILON) ? 0 : R01;
    R02 = (abs(R02) < DBL_EPSILON) ? 0 : R02;
    R10 = (abs(R10) < DBL_EPSILON) ? 0 : R10;
    R11 = (abs(R11) < DBL_EPSILON) ? 0 : R11;
    R12 = (abs(R12) < DBL_EPSILON) ? 0 : R12;
    R20 = (abs(R20) < DBL_EPSILON) ? 0 : R20;
    R21 = (abs(R21) < DBL_EPSILON) ? 0 : R21;
    R22 = (abs(R22) < DBL_EPSILON) ? 0 : R22;
    t0 = (abs(t0) < DBL_EPSILON) ? 0 : t0;
    t1 = (abs(t1) < DBL_EPSILON) ? 0 : t1;
    t2 = (abs(t2) < DBL_EPSILON) ? 0 : t2;

    //Build a transformation matrix
    Eigen::Affine3d tr;
    tr.matrix() <<  R00, R01, R02, t0,
                    R10, R11, R12, t1,
                    R20, R21, R22, t2,
                    0,0,0,1;

    return {tr, root_name};
}

Eigen::Matrix4d ExpressedInGet::Ei(string csys_name){
    this->csys_name = csys_name;
    if(!VerifyInput(csys_name))
        throw runtime_error("Only [a-z], [0-9] and dash (-) is allowed in the frame name.");

    //Get subject_name WRT root EI root
    auto [X_S_W, frame_root_name] = PoseWrtRootSQL(this->subject_name);

    //Get basis_name WRT root EI root
    auto X_B_W = Eigen::Affine3d::Identity();
    auto ref_root_name = this->basis_name;
    if(frame_root_name == this->basis_name){
        //The root frame is the basis_name so X_B_W is identity and there is nothing to do.
    }else{
        //Otherwise, we need to find its pose relative to the root.
        auto [pose, root_name] = PoseWrtRootSQL(this->basis_name);
        X_B_W = pose;
        ref_root_name = root_name;
    }
    auto R_B_W   = X_B_W.rotation();

    //Get csys_name WRT root EI root
    auto X_C_W = Eigen::Affine3d::Identity();
    if(frame_root_name == ref_root_name){
        if(frame_root_name == csys_name){
            //The root frame is already the frame in which we want to express the transform
            // so X_C_W is identity and there is nothing to do.
        }else{
            //Otherwise, we need to find its pose relative to the root.
            auto [pose, in_root_name] = PoseWrtRootSQL(this->csys_name);
            X_C_W = pose;
            //Make sure all three frames have the same root frame
            if(ref_root_name != in_root_name){
                throw runtime_error("The frame "+this->basis_name+" cannot be defined with respect to "+this->csys_name+". Is the frame graph complete?");
            }
        }
    }else{
        throw runtime_error("The frame "+this->subject_name+" cannot be defined with respect to "+this->basis_name+". Is the frame graph complete?");
    }

    //Get root WRT ref EI root
    auto X_W_B = X_B_W.inverse();
    Eigen::Affine3d X_W_B_W = Eigen::Affine3d::Identity();
    X_W_B_W.linear()        = X_W_B.rotation();
    X_W_B_W.translation()   = -1 * X_B_W.translation();

    //Compute the subject_name WRT basis_name EI root
    auto X_S_B = X_W_B * X_S_W;
    auto R_S_B   = X_S_B.rotation();

    //Change the "expressed in"
    // To represent a position vector (ref_frame --> frame), a coordinate system (in_frame) needs to be chosen.
    // Rotations are not expressed in a coordinate system (no in_frame involved).
    // p_S_B = R_C_B * p_S_B_C
    auto X_W_C = X_C_W.inverse();
    auto R_W_C   = X_W_C.rotation();
    Eigen::Affine3d X_S_B_C = Eigen::Affine3d::Identity();
    X_S_B_C.linear()        = R_S_B;
    X_S_B_C.translation()   = R_W_C * R_B_W * X_S_B.translation();
    return X_S_B_C.matrix();
}


ExpressedInSet::ExpressedInSet(string world_name, string subject_name, string basis_name): 
    world_name(world_name), 
    subject_name(subject_name), 
    basis_name(basis_name){
    //Set the timeout to 10 seconds
    this->timeout = 10000;
}

ExpressedInSet::~ExpressedInSet(){}

SetAs ExpressedInSet::Ei(string csys_name){
    this->csys_name = csys_name;
    return SetAs(this->world_name, this->subject_name, this->basis_name, this->csys_name);
}