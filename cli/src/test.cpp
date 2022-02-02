#include <iostream>
#include <Eigen/Geometry>
#include <SQLiteCpp/SQLiteCpp.h>
#include <string>
#include <cfloat>
#include <math.h>
#include "Wrt.h"

using namespace std;
using Eigen::Affine3d;
using Eigen::Matrix3d;
using Eigen::AngleAxisd;
using Eigen::Vector3d;
 
double deg_to_rad(double angle){
    return angle * M_PI / 180;
}

int main()
{
    Affine3d m = Affine3d::Identity();
    auto R = m.rotation();
    auto t = m.translation();
    cout << m.matrix() << endl;
    cout << R << endl;
    cout << t << endl;
    cout << R(2,2) << endl;
    cout << t(0) << endl;

    auto wrt = DbConnector();
    Affine3d pose;
    pose.matrix() << 1,0,0,1, 0,1,0,1, 0,0,1,1, 0,0,0,1;
    wrt.In("test").Set("a").Wrt("world").Ei("world").As(pose);
    pose = wrt.In("test").Get("a").Wrt("world").Ei("world");

    Matrix3d rot;
    rot = AngleAxisd(deg_to_rad(90), Vector3d::UnitX());
    pose.linear() = rot;
    pose.translation() << 0,0,0;
    cout << pose.matrix() << endl;
    wrt.In("test").Set("b").Wrt("a").Ei("a").As(pose);
    
    rot = Matrix3d::Identity();
    pose.linear() = rot;
    pose.translation() << 1,0,0;
    cout << pose.matrix() << endl;
    wrt.In("test").Set("c").Wrt("b").Ei("b").As(pose);

    rot = AngleAxisd(deg_to_rad(90), Vector3d::UnitZ());
    pose.linear() = rot;
    pose.translation() << 1,1,0;
    cout << pose.matrix() << endl;
    wrt.In("test").Set("d").Wrt("b").Ei("b").As(pose);

    pose.matrix() << 1,0,0,0, 0,0,1,0, 0,-1,0,0, 0,0,0,1;
    assert(wrt.In("test").Get("a").Wrt("b").Ei("b").matrix().isApprox(pose.matrix()));
    
    pose.matrix() << 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1;
    assert(wrt.In("test").Get("a").Wrt("b").Ei("a").matrix().isApprox(pose.matrix()));

    pose.matrix() << 1,0,0,2, 0,0,-1,1, 0,1,0,1, 0,0,0,1;
    assert(wrt.In("test").Get("c").Wrt("world").Ei("world").matrix().isApprox(pose.matrix()));

    pose.matrix() << 1,0,0,2, 0,1,0,1, 0,0,1,-1, 0,0,0,1;
    assert(wrt.In("test").Get("c").Wrt("world").Ei("c").matrix().isApprox(pose.matrix()));

    pose.matrix() << 1,0,0,2, 0,0,-1,1, 0,1,0,1, 0,0,0,1;
    assert(wrt.In("test").Get("c").Wrt("world").Ei("a").matrix().isApprox(pose.matrix()));

    pose.matrix() << 0,-1,0,1, 0,0,-1,0, 1,0,0,1, 0,0,0,1;
    assert(wrt.In("test").Get("d").Wrt("a").Ei("a").matrix().isApprox(pose.matrix()));

    cout << "Congratulations! All tests passed." << endl;
}