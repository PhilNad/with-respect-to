#pragma once

//Forward declaration
class ExpressedInGet;
class ExpressedInSet;

#include "DbConnector.h"
#include <Eigen/Eigen>
#include <string>
using namespace std;


class RefFrame{
    public:
        RefFrame(string, string, Eigen::Affine3d);
        ~RefFrame();
        string name;
        string parent_name;
        Eigen::Affine3d transformation;
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
        void As(Eigen::Affine3d);
};

class ExpressedInGet
{
private:
    string world_name;
    string frame_name;
    string ref_frame_name;
    string in_frame_name;
public:
    ExpressedInGet(string, string, string);
    ~ExpressedInGet();
    RefFrame GetParentFrame(string frame_name);
    Eigen::Affine3d PoseWrtWorld(string frame_name);
    Eigen::Affine3d Ei(string);
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