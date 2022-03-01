import pandas as pd
from hop.hexabundle_allocation.problem_operations.fibres import check_tile_pair_magnet_counts
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("Robot_file_1", help="Robot file of the plate currently on the telescope")
parser.add_argument("Robot_file_2", help="Robot file of the plate we want to configure")
parser.parse_args()

robot_file_1 = args.Robot_file_1
robot_file_2 = args.Robot_file_2

check_tile_pair_magnet_counts(robot_file_1, robot_file_2)

