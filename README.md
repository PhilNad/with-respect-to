# With-Respect-To
With-Respect-To (WRT) is a lightweight, high-performance library for managing 3D transformations easily with a centralized database approach. This library is meant to be used whenever you need to read/write the pose of a frame, freeing yourself from the worry of error-prone computations.

## Goals
- Simple. A single 3D convention is used and is explicitly defined.
- Fast. The user should not worry about the overhead of calling accessors.
- Accessible. Information is accessible from a variety of interfaces on Linux (Python, C++, Bash).
- Minimalist. The API contains as few methods as possible.

## Usage
### Example Usage From Python
This is part of [python_bindings/src/test.py](python_bindings/src/test.py).
```python
#Connect to a temporary database
TEMPORARY_DATABASE = 1
db = WRT.DbConnector(TEMPORARY_DATABASE)

#Set the pose of frame b with respect to frame a, which does not exist yet (permitted).
pose = SE3.Rx(90, "deg").A
db.In('test').Set('b').Wrt('a').Ei('a').As(pose)

#Set the pose of frame a with respect to the world (default frame)
pose = np.array([[1,0,0,1],[0,1,0,1],[0,0,1,1],[0,0,0,1]])
db.In('test').Set('a').Wrt('world').Ei('world').As(pose)

#Get the pose of frame a with respect to frame b
T_a_b = SE3(db.In('test').Get('a').Wrt('b').Ei('b'))
```

### Example Usage From C++
This is part of [test/src/test.cpp](test/src/test.cpp).
```cpp
//Connect to a temporary database
auto wrt = DbConnector(DbConnector::TEMPORARY_DATABASE);

//Set the pose of frame a with respect to the world (default frame)
Affine3d pose;
pose.matrix() << 1,0,0,1, 0,1,0,1, 0,0,1,1, 0,0,0,1;
wrt.In("test").Set("a").Wrt("world").Ei("world").As(pose.matrix());
pose = wrt.In("test").Get("a").Wrt("world").Ei("world");

//Set the pose of frame b with respect to frame a
pose.linear() = AngleAxisd(deg_to_rad(90), Vector3d::UnitX());
pose.translation() << 0,0,0;
wrt.In("test").Set("b").Wrt("a").Ei("a").As(pose.matrix());

//Get the pose of frame a with respect to the world
Matrix4d T_b_a = wrt.In("test").Get("a").Wrt("world").Ei("world");
```

### Example Usage From Bash
```bash
> WRT --In test --Get d --Wrt a --Ei a
 0 -1  0  1
 0  0 -1  0
 1  0  0  1
 0  0  0  1
> WRT --compact --In test --Get d --Wrt a --Ei a
0,-1,0,1,0,0,-1,0,1,0,0,1,0,0,0,1
> WRT --In test --Set a --Wrt world --Ei world --As [[1,0,0,1],[0,1,0,1],[0,0,1,1],[0,0,0,0]]
The format of the submitted matrix is wrong (-3).
> WRT --quiet --In test --Set a --Wrt world --Ei world --As [[1,0,0,1],[0,1,0,1],[0,0,1,1],[0,0,0,0]]
> WRT --In test --Set a --Wrt world --Ei world --As [[1,0,0,1],[0,1,0,1],[0,0,1,1],[0,0,0,1]]
> WRT --dir /home/username/other_dir/ --In test --Get d --Wrt a --Ei a
The reference frame a does not exist in this world.
```

### Usage of the Command-Line Interface
```
Usage: WRT [options] 

Optional arguments:
-h --help    	shows help message and exits
-v --version 	prints version information and exits
-q --quiet   	If a problem arise, do now output any information, fails quietly. [default: false]
-c --compact 	Output a compact representation of the matrix as a comma separated list of 16 numbers in row-major order. [default: false]
-d --dir     	Path to the directory in which the database is located.
--In         	The world name the frame lives in ([a-z][0-9]-). [required]
--Get        	Name of the frame to get ([a-z][0-9]-).
--Set        	Name of the frame to set ([a-z][0-9]-).
--Wrt        	Name of the reference frame the frame is described with respect to ([a-z][0-9]-). [required]
--Ei         	Name of the reference frame the frame is expressed in ([a-z][0-9]-). [required]
--As         	If setting a frame, a string representation of the array defining the pose with rotation R and translation t: [[R00,R01,R02,t0],[R10,R11,R12,t1],[R20,R21,R22,t2],[0,0,0,1]]
```

## Differences With tf2
Although this library might seem to be similar to [tf2](http://wiki.ros.org/tf2), there are notable differences that should be taken into account when choosing which one to use:
- ### **1.** tf2 requires running the ROS ecosystem, WRT does not.
  If you are already running ROS for other reasons, it might make more sense to continue using it. However, it might seem excessive to run the whole ROS ecosystem for the sole reason of recording transformations.
- ### **2.**  tf2 does transformation interpolation, WRT does not.
  One of the primary use case for tf2 is to linearly interpolate poses through time. WRT does not do such operation and assumes that all transformations are exact at the time at which they are accessed. If the previous pose of a moving frame must be recorded, a timestamp can be appended to the name of the frame to differentiate between the older and newer frames.
- ### **3.** tf2 is designed as a distributed system while WRT is designed as a centralized system.
  While it is possible (and easy) to use WRT over ethernet through [a network file system](https://ubuntu.com/server/docs/service-nfs), it is certainly [not optimal](https://www.sqlite.org/useovernet.html). Conversely, while it is possible to use tf2 in a setup consisting in a single machine, it really shines when used by many machines over a network. However, be wary of the [bandwidth requirements](http://wiki.ros.org/tf2/Design#tf_messages_do_not_deal_with_low_bandwidth_networks_well) imposed by the frequent transfer of [ROS messages](https://docs.ros.org/en/lunar/api/geometry_msgs/html/msg/TransformStamped.html) over the network.

## Performances
This library is meant to be fast such that the overhead of calling the accessors is negligible, freeing the user from worrying about the performance of the library.

[Stress-testing the library](test/src/stress-test.py) through the Python interface on two standard consumer-level laptops:
1) **Thinkpad X1 Yoga Gen 1 i5-6200U (20FQCTO1WW) 2016**
2) **Thinkpad X1 Gen 12 Intel Ultra 7 165U (20UBCTO1WW) 2025**

The following table shows the average time it takes to perform a GET or SET operation on a tree of a given depth with a given number of concurrent processes. The average time and frequency are calculated over 10,000 iterations.

Note that a pose tree depth of 50 can be considered an edge-case as it is unlikely that a tree would be that deep in practice.

| Operation | # Concurrent Processes | Pose Tree Depth | Machine | Avg. Time (ms) | Avg. Frequency (Hz) |
|-----------|------------------------|-----------------|---------|-----------|----------------|
| GET | 1 | 10 | 2 | 1.43 | 697 |
| SET | 1 | 10 | 2 | 1.46 | 686 |
| GET | 1 | 50 | 2 | 1.55 | 644 |
| SET | 1 | 50 | 2 | 1.65 | 604 |
| GET | 3 | 10 | 1 | 2.6 | 380 |
| GET | 3 | 10 | 2 | 1.883 | 531 |
| SET | 3 | 10 | 1 | 2.6 | 390 |
| SET | 3 | 10 | 2 | 1.79 | 558 |
| GET | 3 | 50 | 1 | 3.2 | 315 |
| GET | 3 | 50 | 2 | 1.988 | 503 |
| SET | 3 | 50 | 1 | 3.1 | 325 |
| SET | 3 | 50 | 2 | 2.038 | 491 |

The results show that the library is fast enough for most applications, even when used concurrently. The performance is not significantly affected by the depth of the tree or the number of concurrent processes.

## Design
- Uses the [Eigen library](https://eigen.tuxfamily.org)
- Produces and consumes 4x4 transformation Eigen matrices
- Store data in a SQLITE database using [sqlite3](https://docs.python.org/3/library/sqlite3.html)
- The scene is described by a tree
  - Re-setting a parent node, also changes the children nodes (i.e. assumes a rigid connection between parent and children)
  - If setting a transform would create a loop, the node is reassigned to a new parent. A frame only has a single parent.

## Dependencies
- [Python 3.x](https://www.python.org/downloads/)
- [Python 3.x Development Files](https://pkgs.org/download/python3-devel)
- [C++ 17 Compiler](https://gcc.gnu.org/) (Yes, you need C++17.)
- [CMake > 3.15](https://cmake.org/download/)
- [Eigen > 3.4](https://eigen.tuxfamily.org/)
- [pybind11](https://pybind11.readthedocs.io/en/stable/)
- [SQLiteCpp](http://srombauts.github.io/SQLiteCpp/)
- [sqlite3](https://sqlite.org/index.html)

### Dependencies Installation
Most dependencies should be provided by your preferred package manager. PyBind11 and SQLiteCpp are included in this repository as submodules and can be easily installed with:
```bash
> git submodule update --init --recursive
> cd extern/pybind11
> cmake -S . -B build
> cmake --build build
> sudo cmake --install build
> cd ../SQLiteCpp
> cmake -S . -B build
> cmake --build build
> sudo cmake --install build
```

## Installation From Pre-Compiled Binaries
The [latest release](https://github.com/PhilNad/with-respect-to/releases) contains pre-compiled 64-bit binaries for Linux, that can be used directly once placed in an appropriate directory.

## Installation From Source Code
### Build Executables
```bash
> git clone https://github.com/PhilNad/with-respect-to.git
> cd with-respect-to
> cmake -S . -B build
> cmake --build build
```
### Install Everything
```bash
> sudo cmake --install build
```

### Install Command-Line Interface (cli)
```bash
> cd cli
> sudo make install
```
### Install Python3 bindings
```bash
> cd ../python_bindings
> sudo make install
```

### Installation Verification
```bash
> cd ~
> WRT
> python3 -c $'import with_respect_to as WRT'
```

## Debugging
You can use [SQLiteStudio](https://github.com/pawelsalawa/sqlitestudio) to open the database file and read its content through a GUI.
