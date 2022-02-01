
#include "ExpressedIn.h"
#include <regex>
#include <cfloat>
#include <iostream>
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

RefFrame::RefFrame(string frame_name, string parent_frame_name, Eigen::Affine3d transfo):
    name(frame_name),
    parent_name(parent_frame_name),
    transformation(transfo){
    int code = VerifyMatrix(transfo);
    if(code < 0)
        throw runtime_error("The format of the submitted matrix is wrong ("+to_string(code)+").");
}

RefFrame::~RefFrame(){}

SetAs::SetAs(string world_name, string frame_name, string ref_frame_name, string in_frame_name): 
    world_name(world_name), 
    frame_name(frame_name), 
    ref_frame_name(ref_frame_name),
    in_frame_name(in_frame_name){
    if(!VerifyInput(frame_name) || !VerifyInput(ref_frame_name) || !VerifyInput(in_frame_name))
        throw runtime_error("Only [a-z], [0-9] and dash (-) is allowed in the frame name.");
}

SetAs::~SetAs(){}

void SetAs::As(Eigen::Affine3d transfo_matrix){
    int code = VerifyMatrix(transfo_matrix);
    if(code < 0)
        throw runtime_error("The format of the submitted matrix is wrong ("+to_string(code)+").");
    //List all existing reference names
    SQLite::Database db(this->world_name+".db", SQLite::OPEN_READWRITE);

    SQLite::Statement   query(db, "SELECT * FROM frames WHERE name IS ?");
    query.bind(1, this->ref_frame_name);

    //Verify that the reference frame ref_frame_name exists.
    int row_counter = 0;
    while (query.executeStep()){
        row_counter++;
    }
    if(row_counter == 0){
        throw runtime_error("The reference frame "+this->ref_frame_name+" does not exist in this world.");
    }
    if(row_counter != 1){
        throw runtime_error("Need a single reference frame "+this->ref_frame_name+".");
    }
    //Take into account the fact that the transformation can be expressed in a frame different from the reference frame
    //       Like: SET object WRT table EI world
    auto getter = ExpressedInGet(this->world_name, this->in_frame_name, this->ref_frame_name);
    auto X_RI_R = getter.Ei(this->ref_frame_name);
    
    auto R = X_RI_R.rotation() * transfo_matrix.rotation();
    auto t = X_RI_R.rotation() * transfo_matrix.translation();

    SQLite::Transaction transaction(db);
    
    //Remove from DB any frame with  __frame_name
    SQLite::Statement   q1(db, "DELETE FROM frames WHERE name IS ?");
    q1.bind(1, this->frame_name);
    q1.executeStep();

    //Add new frame
    SQLite::Statement   q2(db, "INSERT INTO frames VALUES (?, ?, ?,?,?, ?,?,?, ?,?,?, ?,?,?)");
    q2.bind(1, this->frame_name);
    q2.bind(2, this->ref_frame_name);
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


ExpressedInGet::ExpressedInGet(string world_name, string frame_name, string ref_frame_name): 
    world_name(world_name), 
    frame_name(frame_name), 
    ref_frame_name(ref_frame_name){
    if(!VerifyInput(frame_name) || !VerifyInput(ref_frame_name))
        throw runtime_error("Only [a-z], [0-9] and dash (-) is allowed in the frame name.");
}

ExpressedInGet::~ExpressedInGet(){}

RefFrame ExpressedInGet::GetParentFrame(string frame_name){
    SQLite::Database db(this->world_name+".db", SQLite::OPEN_READONLY);

    SQLite::Statement   query(db, "SELECT * FROM frames WHERE name IS ?");
    query.bind(1, frame_name);

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
        throw runtime_error("The reference frame "+this->ref_frame_name+" does not exist in this world.");
    if(row_counter != 1)
        throw runtime_error("Need a single reference frame "+this->ref_frame_name+".");

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

//Return the pose of frame_name relative to world reference frame, expressed in world.
Eigen::Affine3d ExpressedInGet::PoseWrtWorld(string frame_name){
    RefFrame f = ExpressedInGet::GetParentFrame(frame_name);
    Eigen::Affine3d new_transfo;
    new_transfo.matrix() = f.transformation.matrix();
    
    while(f.parent_name.length() > 0){
        f = ExpressedInGet::GetParentFrame(f.parent_name);
        new_transfo = f.transformation * new_transfo;
    }
    return new_transfo;
}

Eigen::Affine3d ExpressedInGet::Ei(string in_frame_name){
    this->in_frame_name = in_frame_name;
    if(!VerifyInput(in_frame_name))
        throw runtime_error("Only [a-z], [0-9] and dash (-) is allowed in the frame name.");
    
    //Using Drake's monogram notation (https://drake.mit.edu/doxygen_cxx/group__multibody__notation__basics.html)

    //1) Make sure frame_name, ref_frame_name and in_frame_name exist in the DB
    //2) Get frame_name WRT world EI world
    auto X_WF_W = PoseWrtWorld(this->frame_name);
    //3) Get ref_frame_name WRT world EI world
    auto X_WR_W = PoseWrtWorld(this->ref_frame_name);
    Eigen::Affine3d X_RW_W = Eigen::Affine3d::Identity();
    X_RW_W.translation() = -1 * X_WR_W.translation();
    //4) Get in_frame_name WRT world EI world
    auto X_WI_W = PoseWrtWorld(this->in_frame_name);
    auto X_IW_I = X_WI_W.inverse();
    //5) Compute the frame_name WRT ref_frame_name EI world
    auto X_RF_W = X_RW_W * X_WF_W;
    //6) Change the "expressed in"
    Eigen::Affine3d mat = Eigen::Affine3d::Identity();
    mat.linear()        = X_IW_I.rotation() * X_RF_W.rotation();
    mat.translation()   = X_IW_I.rotation() * X_RF_W.translation();
    return mat;
}


ExpressedInSet::ExpressedInSet(string world_name, string frame_name, string ref_frame_name): 
    world_name(world_name), 
    frame_name(frame_name), 
    ref_frame_name(ref_frame_name){

}

ExpressedInSet::~ExpressedInSet(){}

SetAs ExpressedInSet::Ei(string in_frame_name){
    this->in_frame_name = in_frame_name;
    return SetAs(this->world_name, this->frame_name, this->ref_frame_name, this->in_frame_name);
}