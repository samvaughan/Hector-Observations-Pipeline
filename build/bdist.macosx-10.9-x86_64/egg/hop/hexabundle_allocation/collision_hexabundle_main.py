#!/usr/bin/env python3
import sys
sys.path.append('/home/ayoan/Hector/magnet_position/venv/bin/python')

import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
import csv
import time
start_time = time.time()

from problem_operations.conflicts.functions import find_all_blocked_magnets,\
                                    create_list_of_fully_blocked_magnets
from problem_operations.conflicts.blocked_magnet import print_fully_blocked_magnets
from problem_operations.extract_data import create_list_of_all_magnets_from_file,get_file
from problem_operations.plots import create_magnet_pickup_areas, draw_magnet_pickup_areas, draw_all_magnets
from problem_operations.position_ordering import create_position_ordering_array
from problem_operations.file_arranging import arrange_guidesFile, merge_hexaAndGuides, create_robotFileArray, \
                                    positioningArray_adjust_and_mergetoFile, finalFiles
from hector.plate import HECTOR_plate
from problem_operations.offsets import hexaPositionOffset


clusterNum = 2
tileNum = 0
limit = 86
allBatchOfTiles = ['G9','G12','G15']

galaxyIDrecord = {}

# for batch in allBatchOfTiles:
#     if batch == 'G9':
#         limit = 104
#     elif batch == 'G12':
#         limit = 86
#     elif batch == 'G15':
#         limit = 95
for clusterNum in range(2,3):
    if (clusterNum == 1):
        limit = 15
    elif (clusterNum == 2):
        limit = 29
    elif (clusterNum == 3):
        limit = 24
    elif (clusterNum == 4):
        limit = 10
    elif (clusterNum == 5):
        limit = 26
    elif (clusterNum == 6):
        limit = 16
    for tileNum in range(7,10):

        # fileNameGuides = ('GAMA_'+batch+'/Configuration/HECTORConfig_Guides_GAMA_'+batch+'_tile_%03d.txt' % (tileNum))
        fileNameGuides = ('For_Ayoan_14p5_exclusion_Clusters/Cluster_%d/Configuration/HECTORConfig_Guides_Cluster_%d_tile_%03d.txt' % (clusterNum, clusterNum, tileNum))

        # proxy file to arrange guides in required format to merge with hexa probes
        proxyGuideFile = 'galaxy_fields/newfile.txt'

        # Adding ID column and getting rid of the header line of Guides cluster to add to the hexa cluster
        arrange_guidesFile(fileNameGuides, proxyGuideFile)

        # fileNameHexa = ('GAMA_'+batch+'/Configuration/HECTORConfig_Hexa_GAMA_'+batch+'_tile_%03d.txt' % (tileNum))
        fileNameHexa = ('For_Ayoan_14p5_exclusion_Clusters/Cluster_%d/Configuration/HECTORConfig_Hexa_Cluster_%d_tile_%03d.txt' % (clusterNum, clusterNum, tileNum))

        plate_file = ('For_Ayoan_14p5_exclusion_Clusters/Cluster_%d/Output/Hexa_and_Guides_Cluster_%d_tile_%03d.txt' % (clusterNum, clusterNum, tileNum))
        # plate_file = get_file('GAMA_'+batch+'/Output/Hexa_and_Guides_GAMA_'+batch+'_tile_%03d.txt' % (tileNum))

        # Adding guides cluster txt file to hexa cluster txt file
        merge_hexaAndGuides(fileNameHexa, proxyGuideFile, plate_file, clusterNum, tileNum)

        # extracting all the magnets and making a list of them from the plate_file
        all_magnets = create_list_of_all_magnets_from_file(get_file(plate_file))

        #### Offset functions- still a work in progress- need to determine input source and add column to output file
        all_magnets = hexaPositionOffset(all_magnets)

        # create magnet pickup areas for all the magnets
        create_magnet_pickup_areas(all_magnets)

        #************** # creating plots and drawing pickup areas
        plt.clf()
        plt.close()
        HECTOR_plate().draw_circle('r')
        draw_magnet_pickup_areas(all_magnets, '--c')
        #**************


        # test for collision and detect magnets which have all pickup directions blocked
        conflicted_magnets = find_all_blocked_magnets(all_magnets)

        # create a list of the fully blocked magnets
        fully_blocked_magnets = create_list_of_fully_blocked_magnets(conflicted_magnets)

        # print the fully blocked magnets out in terminal and record in conflicts file
        conflictsRecord = 'galaxy_fields/Conflicts_Index.txt'
        print_fully_blocked_magnets(fully_blocked_magnets,conflictsRecord, fileNameHexa)

        conflictFile = 'galaxy_fields/unresolvable_conflicts.txt'
        flagsFile = 'galaxy_fields/Flags.txt'
        #***  Choose former method OR median method OR larger bundle prioritized method for hexabundle allocation  ***
        positioning_array,galaxyIDrecord = create_position_ordering_array(all_magnets, fully_blocked_magnets, \
                                      conflicted_magnets, galaxyIDrecord, clusterNum, tileNum, conflictFile, flagsFile)

        # draw all the magnets in the plots created earlier
        figureFile = ('figures/Cluster_%d/savedPlot_cluster_%d_tile_%03d.pdf' % (clusterNum,clusterNum,tileNum))
        draw_all_magnets(all_magnets,clusterNum,tileNum,figureFile)  #***********

        # checking positioning_array prints out all desired parameters
        print(positioning_array)

        # insert column heading and print only rectangular magnet rows in the csv file
        newrow = ['Magnet', 'Label', 'Center_x', 'Center_y', 'rot_holdingPosition', 'rot_platePlacing', 'order', 'Pickup_option', 'ID','Index', 'Hexabundle']
        newrow_circular = ['Magnet', 'Label', 'Center_x', 'Center_y', 'holding_position_ang', 'plate_placement_ang', 'order', 'Pickup_option', 'ID', 'Index', 'Hexabundle']

        # final two output files
        outputFile = ('For_Ayoan_14p5_exclusion_Clusters/Cluster_%d/Output_with_Positioning_array/Hexa_and_Guides_with_PositioningArray_Cluster_%d_tile_%03d.txt' \
                    % (clusterNum, clusterNum, tileNum))
                    # ('GAMA_'+batch+'/Output_with_Positioning_array/Hexa_and_Guides_with_PositioningArray_GAMA_'+batch+'_tile_%03d.txt' % (tileNum)
        robotFile = ('For_Ayoan_14p5_exclusion_Clusters/Cluster_%d/Output_for_Robot/Robot_Cluster_%d_tile_%03d.txt' \
                     % (clusterNum, clusterNum, tileNum))
                    #('GAMA_'+batch+'/Output_for_Robot/Robot_GAMA_'+batch+'_tile_%03d.txt' % (tileNum)

        # creating robotFile array and storing it in robot file
        positioning_array, robotFilearray = create_robotFileArray(positioning_array,robotFile,newrow)

        # adjusting the positioning array to merge only selected parameters to the output file
        positioning_array, positioning_array_circular = positioningArray_adjust_and_mergetoFile(positioning_array, \
                                                                        plate_file, outputFile, newrow,newrow_circular)

        # produce final files with consistent layout and no extra commas
        finalFiles(outputFile, robotFile)

        # just to check each tile's whole operation time
        print("\t \t -----   %s seconds   -----" % (time.time() - start_time))


        # Comment out all ***** marked plot functions above(lines 81-84,105s)
        # to run whole batch of tiles fast without plots