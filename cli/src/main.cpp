#include <iostream>
#include <Eigen/Geometry>
#include <string>
#include <algorithm>
#include <regex>
#include <fstream>
#include "Wrt.h"
#include "argparse.hpp"
using namespace std;

int main(int argc, char *argv[]) {

    argparse::ArgumentParser program("WRT", "0.1.1");

    program.add_argument("-q","--quiet")
        .help("If a problem arise, do now output any information, fails quietly.")
        .default_value(false)
        .implicit_value(true);

    program.add_argument("-c","--compact")
        //https://en.wikipedia.org/wiki/Row-_and_column-major_order
        .help("Output a compact representation of the matrix as a comma separated list of 16 numbers in row-major order.")
        .default_value(false)
        .implicit_value(true);

     program.add_argument("-d","--dir")
        .help("Path to the directory in which the database is located.");

    program.add_argument("--In")
        .required()
        .help("The world name the frame lives in ([a-z][0-9]-).");

    program.add_argument("--Get")
        .help("Name of the frame to get ([a-z][0-9]-).");

    program.add_argument("--Set")
        .help("Name of the frame to set ([a-z][0-9]-).");

    program.add_argument("--Wrt")
        .required()
        .help("Name of the reference frame the frame is described with respect to ([a-z][0-9]-).");
    
    program.add_argument("--Ei")
        .required()
        .help("Name of the reference frame the frame is expressed in ([a-z][0-9]-).");
    
    program.add_argument("--As")
        .help("If setting a frame, a string representation of the array defining the pose with rotation R and translation t: [[R00,R01,R02,t0],[R10,R11,R12,t1],[R20,R21,R22,t2],[0,0,0,1]]");

    try {
        program.parse_args(argc, argv);
    }
    catch (const std::runtime_error& err) {
        cerr << err.what() << endl;
        cerr << program;
        exit(1);
    }

    auto has_get = program.is_used("--Get");
    auto has_set = program.is_used("--Set");
    auto has_as  = program.is_used("--As");

    if(has_get && has_set){
        cerr << "Error: Cannot use both --Get and --Set, only one or the other." << endl;
        exit(1);
    }

    if(!has_get && !has_set){
        cerr << "Error: Must specify --Get or --Set." << endl;
        exit(1);
    }

    if(has_set && !has_as){
        cerr << "Error: Must specify --As when using --Set." << endl;
        exit(1);
    }

    //Output in this block depends on the "-q" flag.
    try{
        //If the user want to Set a frame
        if(has_set){
            //Build a matrix from the string representation of the pose
            auto str_pose = program.get<std::string>("--As");
            //Pre-process the input string [[R00,R01,R02,t0],[R10,R11,R12,t1],[R20,R21,R22,t2],[0,0,0,1]]
            char chars_to_remove[] = "[] \r\n\t";
            for (unsigned int i = 0; i < strlen(chars_to_remove); ++i){
                str_pose.erase (remove(str_pose.begin(), str_pose.end(), chars_to_remove[i]), str_pose.end());
            }
            //Verify the the input string is now a list of 16 real numbers
            auto valid = regex_match(str_pose, regex(R"(^(((\+|-)?([0-9]+)(\.[0-9]+)?),){15}((\+|-)?([0-9]+)(\.[0-9]+)?)$)"));
            
            if(valid){
                //Extract all the numbers
                size_t pos = 0;
                string token;
                double n[16];
                int counter = 0;
                while ((pos = str_pose.find(",")) != string::npos) {
                    token = str_pose.substr(0, pos);
                    n[counter] = stod(token);
                    str_pose.erase(0, pos + 1);
                    counter++;
                }
                n[15] = stod(str_pose);
                //Build the pose matrix
                Eigen::Matrix4d pose = Eigen::Matrix4d::Identity();
                pose << n[0],n[1],n[2],n[3], n[4],n[5],n[6],n[7], n[8],n[9],n[10],n[11], n[12],n[13],n[14],n[15];
                //Retrieve pertinent arguments
                auto world_name     = program.get<std::string>("--In");
                auto subject_name     = program.get<std::string>("--Set");
                auto basis_name = program.get<std::string>("--Wrt");
                auto csys_name  = program.get<std::string>("--Ei");
                //Set pose
                DbConnector wrt;
                if(program.is_used("--dir")){
                    string dir_path = program.get<std::string>("--dir");
                    wrt = DbConnector(dir_path, 0);
                    cout << "called constructor with path." << endl;
                }else{
                    wrt = DbConnector();
                }
                wrt.In(world_name).Set(subject_name).Wrt(basis_name).Ei(csys_name).As(pose);
            }
        }
        
        //If the user want to Get a frame
        if(has_get){
            //Retrieve pertinent arguments
            auto world_name     = program.get<std::string>("--In");
            auto subject_name     = program.get<std::string>("--Get");
            auto basis_name = program.get<std::string>("--Wrt");
            auto csys_name  = program.get<std::string>("--Ei");
            //Get pose
            DbConnector wrt;
            if(program.is_used("--dir")){
                string dir_path = program.get<std::string>("--dir");
                wrt = DbConnector(dir_path, 0);
            }else{
                wrt = DbConnector();
            }
            Eigen::Matrix4d pose = wrt.In(world_name).Get(subject_name).Wrt(basis_name).Ei(csys_name);

            //If the output should be compact
            if(program["--compact"] == true){
                auto n = pose;
                cout << n(0,0) << "," << n(0,1) << "," << n(0,2) << "," << n(0,3) << ",";
                cout << n(1,0) << "," << n(1,1) << "," << n(1,2) << "," << n(1,3) << ",";
                cout << n(2,0) << "," << n(2,1) << "," << n(2,2) << "," << n(2,3) << ",";
                cout << n(3,0) << "," << n(3,1) << "," << n(3,2) << "," << n(3,3) << endl;
            }else{
                cout << pose.matrix() << endl;
            }
        }
    }catch (const std::runtime_error& err) {
        if(program["--quiet"] == true){
            exit(1);
        }else{
            cerr << err.what() << endl;
            exit(1);
        }
    }
}