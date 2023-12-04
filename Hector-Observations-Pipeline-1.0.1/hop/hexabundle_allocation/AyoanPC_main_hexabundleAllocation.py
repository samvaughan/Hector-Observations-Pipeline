#!/usr/bin/env python3
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
import csv
import time
start_time = time.time()

from hop.hexabundle_allocation.problem_operations import extract_data, file_arranging, hexabundle, offsets, plots, \
                                                        position_ordering, robot_parameters, conflicts
from hop.hexabundle_allocation.hector.plate import HECTOR_plate

clusterNum = 2
tileNum = 0
limit = 86
allBatchOfTiles = ['G9','G12','G15']

Location = 'D:/Hector/Hector_project_files/Hector/magnet_position'
galaxyIDrecord = {}

def allocate_hexas(clusterNum, tileNum, galaxyIDrecord, plot):

    # check for galaxyIDrecord python dictionary
    if 'galaxyIDrecord' in globals() or 'galaxyIDrecord' in locals():
        print("galaxyIDrecord exists.")
    else:
        print("galaxyIDrecord being created.")
        galaxyIDrecord = {}

    # fileNameGuides = ('GAMA_'+batch+'/Configuration/HECTORConfig_Guides_GAMA_'+batch+'_tile_%03d.txt' % (tileNum))
    fileNameGuides = (Location+'/For_Ayoan_14p5_exclusion_Clusters/Cluster_%d/Configuration/HECTORConfig_Guides_Cluster_%d_tile_%03d.txt' % ( clusterNum, clusterNum, tileNum))

    # proxy file to arrange guides in required format to merge with hexa probes
    proxyGuideFile = Location+'/galaxy_fields/newfile.txt'

    # Adding ID column and getting rid of the header line of Guides cluster to add to the hexa cluster
    file_arranging.arrange_guidesFile(fileNameGuides, proxyGuideFile)

    # fileNameHexa = ('GAMA_'+batch+'/Configuration/HECTORConfig_Hexa_GAMA_'+batch+'_tile_%03d.txt' % (tileNum))
    fileNameHexa = (Location+'/For_Ayoan_14p5_exclusion_Clusters/Cluster_%d/Configuration/HECTORConfig_Hexa_Cluster_%d_tile_%03d.txt' % (clusterNum, clusterNum, tileNum))

    plate_file = (Location+'/For_Ayoan_14p5_exclusion_Clusters/Cluster_%d/Output/Hexa_and_Guides_Cluster_%d_tile_%03d.txt' % (clusterNum, clusterNum, tileNum))
    # plate_file = get_file('GAMA_'+batch+'/Output/Hexa_and_Guides_GAMA_'+batch+'_tile_%03d.txt' % (tileNum))

    #### Offset function: thermal coefficient based movement of magnet pair as a whole
    plate_file,magnetPair_offset = offsets.magnetPair_radialPositionOffset(plate_file)

    # Adding guides cluster txt file to hexa cluster txt file
    file_arranging.merge_hexaAndGuides(fileNameHexa, proxyGuideFile, plate_file)

    # extracting all the magnets and making a list of them from the plate_file
    all_magnets = extract_data.create_list_of_all_magnets_from_file(extract_data.get_file(plate_file),magnetPair_offset)

    #### Offset functions- still a work in progress- need to determine input source and add column to output file
    all_magnets = offsets.hexaPositionOffset(all_magnets)

    # create magnet pickup areas for all the magnets
    plots.create_magnet_pickup_areas(all_magnets)

    if plot:
        # ************** # creating plots and drawing pickup areas
        plt.clf()
        plt.close()
        HECTOR_plate().draw_circle('r')
        plots.draw_magnet_pickup_areas(all_magnets, '--c')
        # **************


    # test for collision and detect magnets which have all pickup directions blocked
    conflicted_magnets = conflicts.functions.find_all_blocked_magnets(all_magnets)

    # create a list of the fully blocked magnets
    fully_blocked_magnets = conflicts.functions.create_list_of_fully_blocked_magnets(conflicted_magnets)

    conflicts.functions.blocking_magnets_for_fully_blocked_magnets(conflicted_magnets)

    # print the fully blocked magnets out in terminal and record in conflicts file
    conflictsRecord = Location+'/galaxy_fields/Conflicts_Index.txt'
    conflicts.blocked_magnet.print_fully_blocked_magnets(fully_blocked_magnets,conflictsRecord, fileNameHexa)

    conflictFile = Location+'/galaxy_fields/unresolvable_conflicts.txt'
    flagsFile = Location+'/galaxy_fields/Flags.txt'
    #***  Choose former method OR median method OR larger bundle prioritized method for hexabundle allocation  ***
    positioning_array,galaxyIDrecord = position_ordering.create_position_ordering_array(all_magnets, fully_blocked_magnets, \
                                  conflicted_magnets, galaxyIDrecord, clusterNum, tileNum, conflictFile, flagsFile)

    if plot:
        # draw all the magnets in the plots created earlier
        figureFile = (Location+'/figures/Cluster_%d/savedPlot_cluster_%d_tile_%03d.pdf' % (clusterNum,clusterNum,tileNum))
        plots.draw_all_magnets(all_magnets,clusterNum,tileNum,figureFile)  #***********

    # checking positioning_array prints out all desired parameters
    print(positioning_array)

    # insert column heading and print only rectangular magnet rows in the csv file
    newrow = ['Magnet', 'Label', 'Center_x', 'Center_y', 'rot_holdingPosition', 'rot_platePlacing', 'order', 'Pickup_option', 'ID','Index', 'Hexabundle']
    newrow_circular = ['Magnet', 'Label', 'Center_x', 'Center_y', 'holding_position_ang', 'plate_placement_ang', 'order', 'Pickup_option', 'ID', 'Index', 'Hexabundle']

    # final two output files
    outputFile = (Location+'/For_Ayoan_14p5_exclusion_Clusters/Cluster_%d/Output_with_Positioning_array/Hexa_and_Guides_with_PositioningArray_Cluster_%d_tile_%03d.txt' \
                % (clusterNum, clusterNum, tileNum))
                # ('GAMA_'+batch+'/Output_with_Positioning_array/Hexa_and_Guides_with_PositioningArray_GAMA_'+batch+'_tile_%03d.txt' % (tileNum)
    robotFile = (Location+'/For_Ayoan_14p5_exclusion_Clusters/Cluster_%d/Output_for_Robot/Robot_Cluster_%d_tile_%03d.txt' \
                 % (clusterNum, clusterNum, tileNum))
                #('GAMA_'+batch+'/Output_for_Robot/Robot_GAMA_'+batch+'_tile_%03d.txt' % (tileNum)

    # creating robotFile array and storing it in robot file
    positioning_array, robotFilearray = file_arranging.create_robotFileArray(positioning_array,robotFile,newrow)

    # adjusting the positioning array to merge only selected parameters to the output file
    positioning_array, positioning_array_circular = file_arranging.positioningArray_adjust_and_mergetoFile(positioning_array, plate_file, outputFile, newrow,newrow_circular)

    # produce final files with consistent layout and no extra commas
    file_arranging.finalFiles(outputFile, robotFile)

    print(galaxyIDrecord)

    # just to check each tile's whole operation time
    print("\t \t -----   %s seconds   -----" % (time.time() - start_time))

    # Comment out all ***** marked plot functions above(lines 81-84,105s)
    # to run whole batch of tiles fast without plots


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
    for tileNum in range(21,22):
        allocate_hexas(clusterNum,tileNum,galaxyIDrecord, plot=True)