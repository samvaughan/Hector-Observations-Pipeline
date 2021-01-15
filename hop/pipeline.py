import numpy as np 
import os
import glob
from pathlib import Path
import pandas as pd
import matplotlib.pyplot as plt
import subprocess
import sys

from hop.misc import misc_tools
from hop.misc import pandas_tools as P
from hop.tiling import tiling_functions as tiling
from hop.hexabundle_allocation.problem_operations import extract_data, file_arranging, hexabundle, offsets, plots, position_ordering, robot_parameters, conflicts

from hop.hexabundle_allocation.hector.plate import HECTOR_plate

class HectorPipe:

    """
    The Hector Observations pipeline. This class has methods to:

        * Use the Survey Simulator to create an input catalogue from a larger master catalogue
        * Tile an input catalogue using the Hector tiling code
        * Apply distortion correction to each tile
        * Configure each tile
        * Run the HexabundleAllocation code
    """
    def __init__(self, config_filename):
        """
        Must be initialised with a configuration dictionary. This can be made from a configuration file using the yaml library.
        """

        # Firstly, set the directory of the 'hop' folder as a bash environment variable, so that we can access it in R
        os.environ['HECTOROBSPIPELINE_LOC'] = Path(__file__).parent.as_posix()

        self.config_filename = config_filename
        self.config = misc_tools._load_config(config_filename)

        # Lists for the tile RAs and DECs
        self.best_tile_RAs = []
        self.best_tile_Decs = []

        # Set up the output folders
        folders = misc_tools.create_output_directories(self.config['output_folder'])
        # Add these as class attributes

        ubfolders_to_be_made = ['Logs', 'Configuration', 'Tiles', 'Plots', 'DistortionCorrected', "Allocation", "Allocation/tile_outputs", "Allocation/robot_outputs"]


        self.logfile_location = folders['Logs']
        self.configuration_location = folders['Configuration']
        self.tile_location = folders['Tiles']
        self.plot_location = folders['Plots']
        self.distortion_corrected_tile_location = folders['DistortionCorrected']
        self.allocation_files_location_base = folders['Allocation']
        self.allocation_files_location_tiles = folders['Allocation/tile_outputs']
        self.allocation_files_location_robot = folders['Allocation/robot_outputs']


        # Set up the log files
        self.logger, self.logger_R_code = misc_tools.set_up_loggers(self.config)

        # The galaxy ID dictionary for the hexabundle allocation
        self.galaxyIDrecord = {}

        # Get the location of the distortion correction executable and the R code
        self.DistortionCorrection_binary_location = Path(__file__).parent / Path("distortion_correction/HectorTranslationSoftware/Code/HectorConfigUtil")
        if not self.DistortionCorrection_binary_location.exists():
            raise NameError("The Distortion Correction binary seems to not exist")

            Path(hop.__file__).parent
        self.TdF_distortion_file_location = Path(__file__).parent / Path("distortion_correction/HectorTranslationSoftware/Code/tdFdistortion0.sds")  
        if not self.TdF_distortion_file_location.exists():
            raise FileNotFoundError("The 2dF distortion file tdFdistortion0.sds seems to not exist")
        os.environ['TDF_DISTORTION'] = self.TdF_distortion_file_location.as_posix()

        self.ConfigurationCode_location = Path(__file__).parent / Path("configuration/HECTOR_ClusterFieldsTest.R")
        if not self.ConfigurationCode_location.exists():
            raise FileNotFoundError("The Configuration Code seems to not exist")


    def load_input_catalogue(self):
        """
        Load an input target catalogue, guide star catalogue and standard star catalogue
        """
        self.df_targets = P.load_FITS_table_in_pandas(os.path.expanduser(self.config['final_catalogue_name']))
        self.df_guide_stars = P.load_FITS_table_in_pandas(os.path.expanduser(self.config['guide_star_filename']))
        self.df_standard_stars = P.load_FITS_table_in_pandas(os.path.expanduser(self.config['standard_star_filename']))

        # Add some attributes to check that we have the correct catalogues
        self.have_targets_catalogue = True
        self.have_guide_star_catalogue = True
        self.have_standard_star_catalogue = True

    def _add_columns_to_catalogues(self):

        # Add the empty columns which we'll update
        self.df_targets['ALREADY_TILED'] = False
        self.df_targets['Tile_number'] = -999

        self.df_targets['remaining_observations'] = 1
        self.df_targets = self.df_targets.loc[self.df_targets['remaining_observations'] > 0]

        self.logger.info("Adding extra columns to the target dataframe")

    def _rename_columns(self):

        """
        Rename a few columns to make sure things align. TODO: FixMe so this isn't needed!
        """

        # # Rename columns to match the ones the code expects
        # # This will be removed in future
        self.df_targets = self.df_targets.rename(columns=dict(GAL_MAG_R='r_mag', CATID='CATAID'))
        self.df_guide_stars = self.df_guide_stars.rename(columns=dict(ROWID='CoADD_ID'))
        self.df_standard_stars = self.df_standard_stars.rename(columns=dict(ROWID='CoADD_ID'))

    def find_premade_tiles(self):
        """
        Find any tiles which have already been configured in the output folders.
        """
        pre_made_and_configured_files = np.sort(glob.glob(os.path.expanduser(f"{self.config['output_folder']}/Configuration/HECTORConfig_Hexa_{self.config['output_filename_stem']}_tile_*.txt")))
        pre_made_tiles = np.sort(glob.glob(os.path.expanduser(f"{self.config['output_folder']}/Tiles/tile_*.fld")))

        starting_tile = 0
        for configuration_file, tile_file in zip(pre_made_and_configured_files, pre_made_tiles):
            tile_number = int(configuration_file.split('_')[-1].split('.')[0])
            tile_members = pd.read_csv(configuration_file, delim_whitespace=True)
            already_tiled_mask = self.df_targets['CATAID'].isin(tile_members.IDs)
            # Change the TILED flag for targets we've added to a tile
            self.df_targets.loc[already_tiled_mask, 'ALREADY_TILED'] = True
            # And include the tile number for each target
            self.df_targets.loc[already_tiled_mask, 'Tile_number'] = tile_number
            starting_tile +=1

            # Now get the centres of the tile
            with open(tile_file, 'r') as f:
                lines = f.readlines()[1].split()
            self.best_tile_RAs.append(float(lines[1]))
            self.best_tile_Decs.append(float(lines[2]))

        self.logger.info(f"Loaded {starting_tile} pre-made tiles from a previous run")

        return starting_tile


    def tile_field(self, configure_tiles=True, apply_distortion_correction=True, plot=True):

        """
        Tile an entire input catalogue. Optionally apply distortion correction to go from RA/DEC to locations on the plate and optionally run the configuration code to arrange the hexabundles on the plate. 
        """

        # Check that we have our three input catalogues
        if not self.have_targets_catalogue & self.have_standard_star_catalogue & self.have_guide_star_catalogue:
            raise ValueError("We must have a targets, guide star and standard star catalogue! Did you forget to load them?")

        # Add the empty columns to our dataframes
        self._add_columns_to_catalogues()
        self._rename_columns() ## ToDo: FixMe so this isn't necessary

        starting_tile = 0 
        if not self.config['fresh_start']:
            starting_tile = self.find_premade_tiles()
        

        # Now do the tiling
        self.logger.info(f"Total Targets in field to tile: {len(self.df_targets[self.df_targets['ALREADY_TILED']==False])}")
        N_tiles = 9999
        for current_tile in range(starting_tile, N_tiles):

            if self.df_targets['ALREADY_TILED'].sum() == len(self.df_targets):
                self.logger.info("\n\n")
                self.logger.info("Done!")
                break

            tile_out_fname = os.path.expanduser(f"{self.config['output_folder']}/Tiles/tile_{current_tile:03}.fld")
            guide_tile_out_fname = os.path.expanduser(f"{self.config['output_folder']}/Tiles/guide_tile_{current_tile:03}.fld")
            tile_out_fname_after_DC = f"{self.config['output_folder']}/DistortionCorrected/DC_tile_{current_tile:03}.fld"
            guide_tile_out_fname_after_DC = f"{self.config['output_folder']}/DistortionCorrected/guide_DC_tile_{current_tile:03}.fld"

            # If we're applying distortion correction, make sure that the configuration code is looking at the correct file
            if configure_tiles:
                if apply_distortion_correction:
                    tile_file_for_configuration = tile_out_fname_after_DC
                else:
                    tile_file_for_configuration = tile_out_fname

            self.logger.info(f"Tile {current_tile}:")
            
            self.df_targets, tile_df, guide_stars_for_tile, standard_stars_for_tile, tile_RA, tile_Dec = tiling.make_best_tile(self.df_targets, self.df_guide_stars, self.df_standard_stars, tiling_parameters=self.config, tiling_type=self.config['tiling_type'], selection_type=self.config['allocation_type'], fill_spares_with_repeats=self.config['fill_spares_with_repeats'])

            self.logger.info(f"\tSaving to {tile_out_fname}...")

            tiling.save_tile_outputs(f"{self.config['output_folder']}", self.df_targets, tile_df, guide_stars_for_tile, standard_stars_for_tile, tile_RA, tile_Dec, tiling_parameters=self.config, tile_number=current_tile, plot=True)

            if apply_distortion_correction == True:
                DC_corrected_output_file = self.apply_distortion_correction(tile_out_fname, guide_tile_out_fname, tile_out_fname_after_DC, guide_tile_out_fname_after_DC, tile_RA, tile_Dec, guide_stars_for_tile)
                self.logger.info(f"\tDistortion-corrected tile saved at {tile_out_fname_after_DC}...")

            if configure_tiles == True:
                # Place holder variable to see if the configuration code has completed
                configured = False
                for j in range(self.config['MAX_TRIES']):
    
                    # Now call Caro's code
                    self.logger.info(f"Tile {current_tile}: \n\tRunning Configuration code on file {tile_out_fname_after_DC}...")
                    Configuration_bash_code = ["Rscript", f"{self.ConfigurationCode_location}",  f"{tile_file_for_configuration}", f'{self.config["output_filename_stem"]}_', '--out-dir', f'{self.config["output_folder"]}/Configuration/', '--run_local']
                    #proc = subprocess.check_call(, stdout=f, stderr=g, universal_newlines=True)
                    
                    process = subprocess.run(Configuration_bash_code, text=True, capture_output=True)
                    self.logger_R_code.info(process.stdout)
                        
                    # If we've done the tile, break out of the inner 'Max tries' loop
                    if process.returncode == 0:
                        configured = True
                        break
                    elif process.returncode == 10:
                        self.logger.info("\t**Tiling code couldn't configure after 10 minutes... Trying again with a new input tile**")
                        self.logger_R_code.info("\t**Tiling code couldn't configure after 10 minutes... Trying again with a new input tile**")
                    else:
                        self.logger_R_code.error("\n***Error in the tiling code! Exiting to debug***\n")
                        self.logger.info("\t**Error in the configuration code! Exiting to debug**")
                        sys.exit()
                        #logger_R_code.error(error.decode("utf-8"))

                    # If we get here, it means the configuration code hasn't managed to configure. So we'll give it another tile. 
                    self.df_targets, tile_df, guide_stars_for_tile, standard_stars_for_tile, tile_RA, tile_Dec = tiling.make_best_tile(self.df_targets, self.df_guide_stars, self.df_standard_stars, tiling_parameters=self.config, tiling_type=self.config['tiling_type'], selection_type=self.config['allocation_type'], fill_spares_with_repeats=self.config['fill_spares_with_repeats'])

                    tiling.save_tile_outputs(f"{self.config['output_folder']}", self.df_targets, tile_df, guide_stars_for_tile, standard_stars_for_tile, tile_RA, tile_Dec, tiling_parameters=self.config, tile_number=current_tile, plot=True)

                    if apply_distortion_correction == True:
                        DC_corrected_output_file = self.apply_distortion_correction(tile_out_fname, guide_tile_out_fname, tile_out_fname_after_DC, guide_tile_out_fname_after_DC, tile_RA, tile_Dec, guide_stars_for_tile)

                if not configured:
                    raise ValueError(f"Couldn't configure this tile after {self.config['MAX_TRIES']} attempts.")

            # Now find which targets have been selected in this tile
            selected_targets_mask = self.df_targets['CATAID'].isin(tile_df['CATAID'])

            # Find which targets are being tiled for the first time
            new_targets = (selected_targets_mask) & (self.df_targets['ALREADY_TILED'] == False)
            # Change the TILED flag for targets we've added to a tile
            self.df_targets.loc[new_targets, 'ALREADY_TILED'] = True
            # And include the tile number for each target
            self.df_targets.loc[new_targets, 'Tile_number'] = current_tile

            self.best_tile_RAs.append(tile_RA)
            self.best_tile_Decs.append(tile_Dec)

            self.logger.info(f"FINISHED TILE {current_tile}\n\n")

        self.tile_positions = np.array([self.best_tile_RAs, self.best_tile_Decs])

        if plot:
            fig, ax = tiling.plot_survey_completeness_and_tile_positions(self.tile_positions, self.df_targets, self.config, fig=None, ax=None, completion_fraction_to_calculate=0.95, verbose=True)
            fig.savefig(f"{self.config['output_folder']}/Plots/{self.config['output_filename_stem']}_field_plot.pdf", bbox_inches='tight')
        
        #Save the overall dataframe
        self.df_targets.to_csv(f"{self.config['output_folder']}/Tiles/overall_targets_dataframe.csv")

        self.N_tiles = current_tile


    def apply_distortion_correction(self, tile_out_fname, guide_tile_out_fname, tile_out_fname_after_DC, guide_tile_out_fname_after_DC, tile_RA, tile_Dec, guide_stars_for_tile, verbose=False):

        """
        Take a tile file from the tiling process and apply Keith's distortion correction code. Then turn his one output file into the two output files which Caro's configuration code expects. We also need to do some work to edit the headers/etc.
        """

        # Now call Keith's code
        DC_bash_code = [f"{self.DistortionCorrection_binary_location}",  tile_out_fname, guide_tile_out_fname, tile_out_fname_after_DC]

        process = subprocess.Popen(DC_bash_code, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        output, error = process.communicate()
        if verbose:
            self.logger.info(output.decode("utf-8"))
        if process.returncode != 0:
            raise ValueError(output.decode("utf-8"))


        # Now take Keith's code and separate the one output file which contains galaxies, standard stars and guides into two: one which has galaxies and standard stars and one which just has the guides. This is a bit messy, but reflects the way that Caro's code reads things in. It's easier to do it in Python, where I know exactly which rows are guides/galaxies/standards, rather than guessing in R using the naming convention 

        with open(tile_out_fname_after_DC,"r") as fi:
            header = []
            for ln in fi:
                if ln.startswith("#"):
                    header.append(ln)

        header.insert(0, f"# Tile file after distortion correction applied\n")
        header.insert(1, f"# {tile_RA} {tile_Dec}\n")

        DC_corrected_output_file = pd.read_csv(tile_out_fname_after_DC, skiprows=6)
        guide_rows = DC_corrected_output_file.ID.isin(guide_stars_for_tile.CoADD_ID)
        with open(guide_tile_out_fname_after_DC, 'w') as f:
            for ln in header:
                f.write(ln)
            DC_corrected_output_file.loc[guide_rows].to_csv(f, index=False)

        with open(tile_out_fname_after_DC, 'w') as g:
            for ln in header:
                g.write(ln)
            DC_corrected_output_file.loc[~guide_rows].to_csv(g, index=False)
        
        return DC_corrected_output_file




    def allocate_hexabundles_for_single_tile(self, tile_number, plot=False):

        ### FIXME- add documentation here
        
        # fileNameGuides = ('GAMA_'+batch+'/Configuration/HECTORConfig_Guides_GAMA_'+batch+'_tile_%03d.txt' % (tileNum))
        fileNameGuides = f"{self.configuration_location}/HECTORConfig_Guides_{self.config['output_filename_stem']}_tile_{tile_number:03d}.txt"

        # proxy file to arrange guides in required format to merge with hexa probes
        proxyGuideFile = f'{self.allocation_files_location_base}/newfile.txt'

        # Adding ID column and getting rid of the header line of Guides cluster to add to the hexa cluster
        file_arranging.arrange_guidesFile(fileNameGuides, proxyGuideFile)

        # fileNameHexa = ('GAMA_'+batch+'/Configuration/HECTORConfig_Hexa_GAMA_'+batch+'_tile_%03d.txt' % (tileNum))
        fileNameHexa = f"{self.configuration_location}/HECTORConfig_Hexa_{self.config['output_filename_stem']}_tile_{tile_number:03d}.txt"

        plate_file = f"{self.allocation_files_location_base}/Hexa_and_Guides_{self.config['output_filename_stem']}_tile_{tile_number:03d}.txt"
        # plate_file = get_file('GAMA_'+batch+'/Output/Hexa_and_Guides_GAMA_'+batch+'_tile_%03d.txt' % (tileNum))

        #### Offset function: thermal coefficient based movement of magnet pair as a whole
        plate_file,magnetPair_offset = offsets.magnetPair_radialPositionOffset(plate_file)

        # Adding guides cluster txt file to hexa cluster txt file
        file_arranging.merge_hexaAndGuides(fileNameHexa, proxyGuideFile, plate_file)

        # extracting all the magnets and making a list of them from the plate_file
        all_magnets = extract_data.create_list_of_all_magnets_from_file(extract_data.get_file(plate_file), magnetPair_offset)

        #### Offset functions- still a work in progress- need to determine input source and add column to output file
        all_magnets = offsets.hexaPositionOffset(all_magnets)

        # create magnet pickup areas for all the magnets
        plots.create_magnet_pickup_areas(all_magnets)

        if plot:
            #************** # creating plots and drawing pickup areas
            plt.clf()
            plt.close()
            HECTOR_plate().draw_circle('r')
            plots.draw_magnet_pickup_areas(all_magnets, '--c')
            #**************


        # test for collision and detect magnets which have all pickup directions blocked
        conflicted_magnets = conflicts.functions.find_all_blocked_magnets(all_magnets)

        # create a list of the fully blocked magnets
        fully_blocked_magnets = conflicts.functions.create_list_of_fully_blocked_magnets(conflicted_magnets)

        fully_blocked_magnets_dictionary = conflicts.functions.blocking_magnets_for_fully_blocked_magnets(conflicted_magnets)

        # print the fully blocked magnets out in terminal and record in conflicts file
        conflictsRecord = f'{self.allocation_files_location_base}/Conflicts_Index.txt'
        conflicts.blocked_magnet.print_fully_blocked_magnets(fully_blocked_magnets,conflictsRecord, fileNameHexa)

        conflictFile = f'{self.allocation_files_location_base}/unresolvable_conflicts.txt'
        flagsFile = f'{self.allocation_files_location_base}/Flags.txt'
        #***  Choose former method OR median method OR larger bundle prioritized method for hexabundle allocation  ***
        positioning_array,self.galaxyIDrecord = position_ordering.create_position_ordering_array(all_magnets, fully_blocked_magnets, conflicted_magnets, self.galaxyIDrecord, self.config['output_filename_stem'], tile_number, conflictFile, flagsFile)

        if plot:
            # draw all the magnets in the plots created earlier
            figureFile = f"{self.plot_location}/savedPlot_{self.config['output_filename_stem']}_tile_{tile_number:03d}.pdf"
            plots.draw_all_magnets(all_magnets, self.config['output_filename_stem'], tile_number,figureFile)  #***********

        # checking positioning_array prints out all desired parameters
        print(positioning_array)

        # insert column heading and print only rectangular magnet rows in the csv file
        newrow = ['Magnet', 'Label', 'Center_x', 'Center_y', 'rot_holdingPosition', 'rot_platePlacing', 'order', 'Pickup_option', 'ID','Index', 'Hexabundle']
        newrow_circular = ['Magnet', 'Label', 'Center_x', 'Center_y', 'holding_position_ang', 'plate_placement_ang', 'order', 'Pickup_option', 'ID', 'Index', 'Hexabundle']

        # final two output files
        outputFile = f"{self.allocation_files_location_tiles}/Hexa_and_Guides_with_PositioningArray_{self.config['output_filename_stem']}_tile_{tile_number:03d}.txt"
        robotFile = f"{self.allocation_files_location_tiles}/Robot_{self.config['output_filename_stem']}_tile_{tile_number:03d}.txt"

        # creating robotFile array and storing it in robot file
        positioning_array, robotFilearray = file_arranging.create_robotFileArray(positioning_array,robotFile,newrow,fully_blocked_magnets_dictionary)

        # adjusting the positioning array to merge only selected parameters to the output file
        positioning_array, positioning_array_circular = file_arranging.positioningArray_adjust_and_mergetoFile(positioning_array, plate_file, outputFile, newrow,newrow_circular)

        # produce final files with consistent layout and no extra commas
        file_arranging.finalFiles(outputFile, robotFile)

        # just to check each tile's whole operation time
        # print("\t \t -----   %s seconds   -----" % (time.time() - start_time))

        # Comment out all ***** marked plot functions above(lines 81-84,105s)
        # to run whole batch of tiles fast without plots


    def allocate_hexabundles_for_all_tiles(self):

        for tile_number in range(self.N_tiles):
            self.allocate_hexabundles_for_single_tile(tile_number)
