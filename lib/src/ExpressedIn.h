#pragma once

//Forward declaration
class ExpressedInGet;
class ExpressedInSet;

#include "DbConnector.h"
#include <Eigen/Eigen>
#include <Eigen/Geometry>
#include <string>
using namespace std;


class RefFrame{
    public:
        RefFrame(string, string, Eigen::Affine3d);
        RefFrame(string, string, Eigen::Quaterniond, Eigen::Vector3d);
        ~RefFrame();
        string name;
        string parent_name;
        Eigen::Quaterniond rotation;
        Eigen::Vector3d translation;
};

class SetAs{
    private:
        string world_name;
        string frame_name;
        string ref_frame_name;
        string in_frame_name;
    public:
        SetAs(string, string, string, string);
        ~SetAs();
        void As(Eigen::Matrix4d);
};

class ExpressedInGet
{
private:
    string world_name;
    string frame_name;
    string ref_frame_name;
    string in_frame_name;
    RefFrame GetParentFrame(string frame_name);
    tuple<Eigen::Affine3d, string> PoseWrtRoot(string frame_name);
public:
    ExpressedInGet(string, string, string);
    ~ExpressedInGet();
    Eigen::Matrix4d Ei(string);
};

class ExpressedInSet
{
private:
    string world_name;
    string frame_name;
    string ref_frame_name;
    string in_frame_name;
public:
    ExpressedInSet(string, string, string);
    ~ExpressedInSet();
    SetAs Ei(string);
};