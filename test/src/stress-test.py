#!/usr/bin/python3
'''
This script instantiate a client that sends request to the database, as quickly as possible.
Periodically, the average frequency at which the requests are completed is printed. 

Initially, the client creates a pose tree with N levels. The assumption is that as the tree is
deeper, the time to complete a request increases.

The client can be configured to send SET or GET requests, depending on what the user wants to test.
'''
import time
import numpy as np
from spatialmath import SE3
import with_respect_to as WRT
import argparse

def random_pose():
    '''
    Return a random pose that is valid.
    '''
    pose = SE3.Rz(np.random.uniform(0, 360), "deg") * SE3.Ry(np.random.uniform(0, 360), "deg") * SE3.Rx(np.random.uniform(0, 360), "deg")
    pose = pose.A
    pose[0:3, 3] = np.random.uniform(-10, 10, 3)
    return pose

def initialize(db, nb_levels):
    '''
    Create a pose tree with nb_levels levels.
    '''

    db.In('test').Set('0').Wrt('world').Ei('world').As(random_pose())
    for i in range(nb_levels):
        pose = random_pose()
        db.In('test').Set('{}'.format(i+1)).Wrt('{}'.format(i)).Ei('{}'.format(i)).As(pose)
    
    return db

def test_set(db, nb_levels):
    '''
    Send a SET request, compute the time it takes for the database to return the result, and return the time.
    '''
    pose = random_pose()

    subject_name     = '{}'.format(np.random.randint(1, nb_levels))
    basis_name = '{}'.format(np.random.randint(0, int(subject_name)))
    csys_name  = '{}'.format(np.random.randint(0, int(subject_name)))
    #print("Setting {} wrt {} ei {}".format(subject_name, basis_name, csys_name))
    start = time.time()
    db.In('test').Set(subject_name).Wrt(basis_name).Ei(csys_name).As(pose)
    end = time.time()
    return end - start

def test_get(db, nb_levels):
    '''
    Send a GET request, compute the time it takes for the database to return the result, and return the time.
    '''
    subject_name     = '{}'.format(np.random.randint(1, nb_levels))
    basis_name = '{}'.format(np.random.randint(0, int(subject_name)))
    csys_name  = '{}'.format(np.random.randint(0, int(subject_name)))
    start = time.time()
    db.In('test').Get(subject_name).Wrt(basis_name).Ei(csys_name)
    end = time.time()
    return end - start

def init_argparse():
    '''
    Initialize the argument parser.
    '''
    parser = argparse.ArgumentParser(
        usage="%(prog)s [OPTION] [FILE]...",
        description="Stress test for the database",
        formatter_class=argparse.RawTextHelpFormatter,
    )
    parser.add_argument(
        "-n", "--nb-levels", type=int, required=False, default=10, help="Number of levels in the pose tree"
    )
    parser.add_argument(
        "-c", "--create", default=True, help="Create the pose tree", action='store_true'
    )
    parser.add_argument(
        "-t", "--test", type=str, required=False, default="SET", help="Type of request to send (SET or GET)"
    )
    parser.add_argument(
        "-i", "--iterations", type=int, required=False, default=10000, help="Number of iterations to perform before returning statistics"
    )
    return parser

def parse_arguments():
    parser  = init_argparse()
    args    = parser.parse_args()

    # Check if the test type is valid
    args.test = args.test.upper()
    if args.test not in ["SET", "GET"]:
        raise ValueError("Invalid test type: {}".format(args.test))
    
    # Check if the number of levels is a positive integer
    if args.nb_levels < 0:
        raise ValueError("Number of levels must be a positive integer")
    
    # Check if the number of iterations is a positive integer
    if args.iterations < 0:
        raise ValueError("Number of iterations must be a positive integer")
    
    return args

if __name__ == '__main__':
    args = parse_arguments()

    # Initialize the database
    db = WRT.DbConnector()
    if args.create:
        db = initialize(db, args.nb_levels)
    
    # Send requests as quickly as possible
    timing_list = []
    for i in range(args.iterations):
        
        if args.test == "SET":
            time_taken = test_set(db, args.nb_levels)
        else:
            time_taken = test_get(db, args.nb_levels)
        
        timing_list.append(time_taken)

    #Compute statistics
    timing_list = np.array(timing_list)
    timing_list = np.sort(timing_list)
    #Remove the outliers
    outliers_removed = timing_list[timing_list < timing_list[int(0.99*len(timing_list))]]
    print("For 99% of the requests, the time taken is:")
    print("Average time taken: {} s".format(np.mean(outliers_removed)))
    print("Standard deviation: {} s".format(np.std(outliers_removed)))
    print("Minimum time taken: {} s".format(np.min(outliers_removed)))
    print("Maximum time taken: {} s".format(np.max(outliers_removed)))
    print("Average frequency: {} Hz".format(1/np.mean(outliers_removed)))
