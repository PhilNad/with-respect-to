#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/eigen.h>
#include "Wrt.h"
namespace py = pybind11;

PYBIND11_MODULE(with_respect_to, m) {
    m.doc() = "Provides an interface to set and get the pose of reference frames as homogeneous transformation matrices.";
    py::class_<DbConnector>(m, "DbConnector")
        .def(py::init<std::string &>(), "Initialize access to the database located in the directory specified in argument.")
        .def(py::init<>(),              "Initialize access to the database located in the user's home directory.")
        .def("In", &DbConnector::In, "Creates or connects to the database named as specified in argument. The specified name can only include characters in ([a-z][0-9]-).");

    py::class_<GetSet>(m, "GetSet")
        .def(py::init<std::string &>())
        .def("Get", &GetSet::Get, "Name of the frame to get, which can only include characters in ([a-z][0-9]-).")
        .def("Set", &GetSet::Set, "Name of the frame to set, which can only include characters in ([a-z][0-9]-).");

    py::class_<WrtGet>(m, "WrtGet")
        .def(py::init<std::string &, std::string &>())
        .def("Wrt", &WrtGet::Wrt, "Name of the reference frame the frame is described with respect to, frame names can only include characters in ([a-z][0-9]-).");
    
    py::class_<WrtSet>(m, "WrtSet")
        .def(py::init<std::string &, std::string &>())
        .def("Wrt", &WrtSet::Wrt, "Name of the reference frame the frame is described with respect to, frame names can only include characters in ([a-z][0-9]-).");

    py::class_<ExpressedInGet>(m, "ExpressedInGet")
        .def(py::init<std::string &, std::string &, std::string &>())
        .def("Ei", &ExpressedInGet::Ei, "Name of the reference frame the frame is expressed in, which can only include characters in ([a-z][0-9]-).");

    py::class_<ExpressedInSet>(m, "ExpressedInSet")
        .def(py::init<std::string &, std::string &, std::string &>())
        .def("Ei", &ExpressedInSet::Ei, "Name of the reference frame the frame is expressed in, which can only include characters in ([a-z][0-9]-).");
    
    py::class_<SetAs>(m, "SetAs")
        .def(py::init<std::string &, std::string &, std::string &, std::string &>())
        .def("As", &SetAs::As, "Homogeneous 4x4 transformation numpy.ndarray defining the pose with rotation R and translation t like such: [[R00,R01,R02,t0],[R10,R11,R12,t1],[R20,R21,R22,t2],[0,0,0,1]]");
}
