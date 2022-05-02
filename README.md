# With-Respect-To (+ Expressed-In)
Simple library that manages databases of 3D transformations with explicit accessors.

## Goals
- Simple. A single 3D convention is used and is explicitly defined.
- Fast. The user should not worry about the overhead of calling accessors.
- Accessible. Information is accessible from a variety of interfaces on Linux (Python, Julia, Bash).
- Minimalist. The API contains as few methods as possible.

## Performances
Currently, a GET operation performed from within Bash takes about 0.009 seconds to execute while a SET operation from Bash takes about 0.04 seconds.
This is reasonable and allows any program that can run bash commands to use the interface.

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
All dependencies should be provided by your preferred package manager. PyBind11 should be downloaded automatically from its repository along with this repository and should be located in `extern/pybind11`. Also, for now, this repository contains a copy of SQLiteCpp in `lib/SQLiteCpp-3.1.1`. PyBind11 and SQLiteCpp should be installed to your system to allow CMake to find all the required files:
```bash
> cd lib/SQLiteCpp-3.1.1
> cmake -S . -B build
> cmake --build build
> sudo cmake --install build
```
and
```bash
> git submodule update --init --recursive
> cd extern/pybind11
> mkdir build
> cd build
> cmake --build .
> sudo make install
```

## Installation
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

## Usage of the Command-Line Interface
```
Usage: WRT [options] 

Optional arguments:
-h --help    	shows help message and exits
-v --version 	prints version information and exits
-q --quiet   	If a problem arise, do now output any information, fails quietly. [default: false]
-c --compact 	Output a compact representation of the matrix as a comma separated list of 16 numbers in row-major order. [default: false]
--In         	The world name the frame lives in ([a-z][0-9]-). [required]
--Get        	Name of the frame to get ([a-z][0-9]-).
--Set        	Name of the frame to set ([a-z][0-9]-).
--Wrt        	Name of the reference frame the frame is described with respect to ([a-z][0-9]-). [required]
--Ei         	Name of the reference frame the frame is expressed in ([a-z][0-9]-). [required]
--As         	If setting a frame, a string representation of the array defining the pose with rotation R and translation t: [[R00,R01,R02,t0],[R10,R11,R12,t1],[R20,R21,R22,t2],[0,0,0,1]]
```

### Example from Bash
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

### Example from C++
See `cli/src/test.cpp`.

### Example from Python
See `python_bindings/test.py`.

## TODO
- Test that using this library from multiple scripts produces the intended results.
- Make Julia bindings to the library.
- Better documentation of the library.
- Remove files related to SQLiteCpp from the repository.
- Make a x64 Linux executables package for easy installation.
