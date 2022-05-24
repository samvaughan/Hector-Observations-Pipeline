import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import string
from pathlib import Path
import matplotlib.colors as colors
from matplotlib.patches import Circle, Rectangle
import matplotlib.lines as mlines

# base_folder = Path("/Users/samvaughan/Science/Hector/Targets/Commissioning/Feb2022/HandMade_Feb2022/results/TilingOutputs/150_m22/FinalOutputs/Reference_Stars_iteration_2/")

# robot = pd.read_csv(base_folder / "Robot_FinalFormat_tile_150_m22_reference_stars_iteration_2_CONFIGURED_correct_header.csv", skiprows=6)
# tile = pd.read_csv(base_folder / "Tile_FinalFormat_tile_150_m22_reference_stars_iteration_2_CONFIGURED_correct_header.csv", skiprows=11)

base_folder = Path("/Users/samvaughan/Science/Hector/Targets/Commissioning/Feb2022/WAVES_North/results/TilingOutputs/G12/FinalOutputs/")
robot = pd.read_csv(base_folder / "Robot_FinalFormat_G12_tile_006_CONFIGURED_correct_header.csv", skiprows=6)
tile = pd.read_csv(base_folder / "Tile_FinalFormat_G12_tile_006_CONFIGURED_correct_header.csv", skiprows=11)


length = 27.2

# Merge the tile and robot files together to get the angs values
#merged = pd.merge(robot, tile.loc[:, ['Hexabundle', 'angs']], left_on='Hexabundle', right_on='Hexabundle')


Hexabundles = list(string.ascii_uppercase[:21]) + ['GS1', 'GS2', 'GS3', 'GS4', 'GS5', 'GS6']

robot_centre_in_mm = [324.47, 297.834]

robot = robot.copy()
fig, ax = plt.subplots()
ax.set_aspect(1)
ax.scatter(robot.Center_x, robot.Center_y, c='k', s=5)
ax.scatter(robot_centre_in_mm[0], robot_centre_in_mm[1], marker='x', c='r', s=30)

for i, Hexabundle in enumerate(Hexabundles):
    cm = robot.loc[(robot['Hexabundle'] == Hexabundle) & (robot['#Magnet'] == 'circular_magnet')]
    rm = robot.loc[(robot['Hexabundle'] == Hexabundle) & (robot['#Magnet'] == 'rectangular_magnet')]
    
    rm_angle = np.radians(270 - rm.rot_holdingPosition - rm.rot_platePlacing)
    ax.plot([rm.Center_x - 27.2 * np.cos(rm_angle), rm.Center_x + 27.2 * np.cos(rm_angle)], [rm.Center_y - 27.2 * np.sin(rm_angle), rm.Center_y + 27.2 * np.sin(rm_angle)], c='b', label='Before Metrology')
    
    cm_angle = np.radians(270 - cm.rot_holdingPosition - cm.rot_platePlacing)
    ax.plot([cm.Center_x, cm.Center_x + 15 * np.cos(cm_angle)], [cm.Center_y, cm.Center_y + 15 * np.sin(cm_angle)], c='b', alpha=0.5)

    # Plot the circular magnet
    c = Circle(xy=(cm.Center_x, cm.Center_y), radius=10, facecolor='None', edgecolor='k')
    ax.add_patch(c)
    ax.annotate(xy=(cm.Center_x, cm.Center_y), text=f"{Hexabundle}")

    # Plot the rectangular magnet

    #r = Rectangle(xy=(rm.Center_x, rm.Center_y), width=27.2, height=14.5, transform=Affine2D().rotate_deg_around(*(rm.Center_x, rm.Center_y), rm_angle)+ax.transData)

plt.show()