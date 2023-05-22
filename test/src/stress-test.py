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

    frame_name     = '{}'.format(np.random.randint(1, nb_levels))
    ref_frame_name = '{}'.format(np.random.randint(0, int(frame_name)))
    in_frame_name  = '{}'.format(np.random.randint(0, int(frame_name)))
    #print("Setting {} wrt {} ei {}".format(frame_name, ref_frame_name, in_frame_name))
    start = time.time()
    db.In('test').Set(frame_name).Wrt(ref_frame_name).Ei(in_frame_name).As(pose)
    end = time.time()
    return end - start

def test_get(db, nb_levels):
    '''
    Send a GET request, compute the time it takes for the database to return the result, and return the time.
    '''
    frame_name     = '{}'.format(np.random.randint(1, nb_levels))
    ref_frame_name = '{}'.format(np.random.randint(0, int(frame_name)))
    in_frame_name  = '{}'.format(np.random.randint(0, int(frame_name)))
    start = time.time()
    db.In('test').Get(frame_name).Wrt(ref_frame_name).Ei(in_frame_name)
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
    
    return args

if __name__ == '__main__':
    args = parse_arguments()

    # Initialize the database
    db = WRT.DbConnector()
    if args.create:
        db = initialize(db, args.nb_levels)
    
    # Send requests as quickly as possible
    timing_list = []
    while True:
        
        if args.test == "SET":
            time_taken = test_set(db, args.nb_levels)
        else:
            time_taken = test_get(db, args.nb_levels)
        timing_list.append(time_taken)

        if len(timing_list) % 100 == 0:
            moving_average_time = np.mean(timing_list)
            print("Average frequency: {} Hz".format(1/moving_average_time))
            #Do a quick pause to allow the user to Ctrl+C
            time.sleep(0.1)
            timing_list = []