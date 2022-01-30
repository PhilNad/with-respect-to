#include <iostream>
#include <Eigen/Geometry>
#include <SQLiteCpp/SQLiteCpp.h>
#include <string>
#include <cfloat>
#include "Wrt.h"

using namespace std;
using Eigen::Affine3d;
using Eigen::Matrix3d;
 
int main()
{
    Affine3d m = Affine3d::Identity();
    auto R = m.rotation();
    auto t = m.translation();
    std::cout << m.matrix() << std::endl;
    std::cout << R << std::endl;
    std::cout << t << std::endl;
    std::cout << R(2,2) << std::endl;
    std::cout << t(0) << std::endl;

    auto wrt = DbConnector();
    Affine3d pose;
    pose.matrix() << 1,0,0,1, 0,1,0,1, 0,0,1,1, 0,0,0,1;
    wrt.In("test").Set("a").Wrt("world").Ei("world").As(pose);
    wrt.In("test").Get("a").Wrt("world").Ei("world");
    /* TODO, further testing.
    
    db = WRT.DbConnector()
    pose = np.array([[1,0,0,1],[0,1,0,1],[0,0,1,1],[0,0,0,1]])
    db.In('test').Set('a').Wrt('world').Ei('world').As(pose)

    pose = SE3.Rx(90, "deg")
    db.In('test').Set('b').Wrt('a').Ei('a').As(pose)

    pose = SE3.Rx(0, "deg", t=[1,0,0])
    db.In('test').Set('c').Wrt('b').Ei('b').As(pose)

    pose = SE3.Rz(90, "deg", t=[1,1,0])
    db.In('test').Set('d').Wrt('b').Ei('b').As(pose)
    */

}