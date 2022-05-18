import os
import glob
from pathlib import Path
import pandas as pd
import matplotlib.pyplot as plt
import subprocess
import sys
import itertools
import datetime
import shlex
import shutil
import numpy as np
import shlex

from .misc import misc_tools
from .misc import pandas_tools as P
from .misc import plotting_tools 
from .tiling import tiling_functions as tiling
from .hexabundle_allocation.problem_operations import extract_data, file_arranging, hexabundle, offsets, plots, position_ordering, robot_parameters, conflicts, fibres
from .target_selection import HectorSim

from .hexabundle_allocation.hector.plate import HECTOR_plate

class HectorPipe:

    """
    The Hector Observations pipeline. This class has methods to:

        * Use the Survey Simulator to create an input catalogue from a larger master catalogue
        * Tile an input catalogue using the Hector tiling code
        * Apply distortion correction to each tile
        * Configure each tile
        * Run the HexabundleAllocation code
    """
    def __init__(self, config_filename=None, Profit_files_location=None, config_dictionary=None):
        """
        Must be initialised with a configuration dictionary. This can be made from a configuration file using the yaml library.

        Optional Arguments: Location of the Profit files. If None, default location is $hop_package_location/distortion_correction/HectorTranslationSoftware/DataFiles
        """

        # Firstly, set the directory of the 'hop' folder as a bash environment variable, so that we can access it in R
        os.environ['HECTOROBSPIPELINE_LOC'] = Path(__file__).parent.as_posix()

        if config_filename is None:
            assert config_dictionary is not None, 'You must provide one of config_filename or config_dictionary'
            self.config = config_dictionary
        if config_dictionary is None:
            assert config_filename is not None, 'You must provide one of config_filename or config_dictionary'
            self.config_filename = config_filename
            self.config = misc_tools._load_config(config_filename)


        # Lists for the tile RAs and DECs
        self.best_tile_RAs = []
        self.best_tile_Decs = []

        # Set up the header for this tile we'll append to
        self.header_dictionary = self.make_header_dictionary()

        # Set up the output folders
        subfolders_to_be_made = ['Logs', 'Configuration', 'Tiles', 'Plots', 'DistortionCorrected', "DistortionCorrected/Plots", "Allocation", "Allocation/tile_outputs", "Allocation/robot_outputs", "Fibres", "FinalOutputs"]
        folders = misc_tools.create_output_directories(self.config['output_folder'], subfolders_to_be_made)

        # Add these as class attributes
        self.logfile_location = folders['Logs']
        self.configuration_location = folders['Configuration']
        self.tile_location = folders['Tiles']
        self.plot_location = folders['Plots']
        self.distortion_corrected_tile_location = folders['DistortionCorrected']
        self.distortion_corrected_plot_location = folders['DistortionCorrected/Plots']
        self.allocation_files_location_base = folders['Allocation']
        self.allocation_files_location_tiles = folders['Allocation/tile_outputs']
        self.allocation_files_location_robot = folders['Allocation/robot_outputs']
        self.final_tiles_correct_format_location = folders['FinalOutputs']

        # Set up the log files
        self.logger, self.logger_R_code = misc_tools.set_up_loggers(self.config)

        # The galaxy ID dictionary for the hexabundle allocation
        self.galaxyIDrecord = {}

        # Get the location of the distortion correction executable and the R code
        self.DistortionCorrection_binary_location = Path(__file__).parent / Path("distortion_correction/HectorTranslationSoftware/HectorConfigUtility/HectorConfigUtil")
        # if not self.DistortionCorrection_binary_location.exists():
        #     raise NameError("The Distortion Correction binary seems to not exist")

        # Locations of all the Hector Config code
        self.Hector_distortion_file_location = Path(__file__).parent / Path("distortion_correction/HectorTranslationSoftware/DataFiles/fit2-b-pos/HectorDistortion.sds")  
        self.Hector_linearity_file_location = Path(__file__).parent / Path("distortion_correction/HectorTranslationSoftware/DataFiles/fit2-b-pos/HectorLinear.sds")  
        self.Hector_sky_fibre_location = Path(__file__).parent / Path("distortion_correction/HectorTranslationSoftware/DataFiles/SkyFibres.csv")

        if Profit_files_location is None:
            self.Profit_files_location = Path(__file__).parent / Path("distortion_correction/HectorTranslationSoftware/DataFiles")
        else:
            self.Profit_files_location = Path(Profit_files_location)

        if not self.Hector_distortion_file_location.exists():
            raise FileNotFoundError("The Hector distortion file HectorDistortion.sds seems to not exist")
        os.environ['HECTOR_DISTORTION'] = self.Hector_distortion_file_location.as_posix()
        if not self.Hector_linearity_file_location.exists():
            raise FileNotFoundError("The Hector linearity file HectorLinear.sds seems to not exist")
        os.environ['HECTOR_LINEARITY'] = self.Hector_linearity_file_location.as_posix()

        self.ConfigurationCode_location = Path(__file__).parent / Path("configuration/HECTOR_ClusterFieldsTest.R")
        if not self.ConfigurationCode_location.exists():
            raise FileNotFoundError("The Configuration Code seems to not exist")

        # Files used in the hexabundle allocation
        self.excel_files_for_allocation_location = Path(__file__).parent / Path("hexabundle_allocation")
        # Location of the P and Q offset file
        self.offsetFile = self.excel_files_for_allocation_location / Path("Hexa_final_prism_gluing_PQ_table.xlsx")
        # Location of the Fibre Slit Info file
        self.fibre_file = self.excel_files_for_allocation_location / Path("Fibre_slitInfo_final.csv")

        if not self.offsetFile.exists():
            raise FileNotFoundError("The P and Q offset file seems to not exist")

        if not self.fibre_file.exists():
            raise FileNotFoundError("The Fibre slit info file seems to not exist")
        
        self.have_targets_catalogue = False
        self.have_standard_star_catalogue = False
        self.have_guide_star_catalogue = False


    def make_header_dictionary(self):

        header_dictionary = {"#PROXIMITY":f"{self.config['proximity']} # tiling proximity value in arcseconds",
                             "#TILING_DATE":f"{datetime.date.today().strftime('%Y %m %d')} # Date the tile was created/configured"
                            }

        return header_dictionary

    def read_header_dictionary_from_file(self, filename):
        
        header_dict={}
        with open(filename, 'r') as f:
            
            for l in self._read_file_get_header(filename)[0]:
                key, value = l.split(',', 1)
                header_dict[key] = value
        return header_dict

    def load_input_catalogue(self):
        """
        Load an input target catalogue, guide star catalogue and standard star catalogue
        """
        self.df_targets = misc_tools._read_table(os.path.expanduser(self.config['final_catalogue_name']))
        self.df_guide_stars = misc_tools._read_table(os.path.expanduser(self.config['guide_star_filename']))
        self.df_standard_stars = misc_tools._read_table(os.path.expanduser(self.config['standard_star_filename']))

        # Add some attributes to check that we have the correct catalogues
        self.have_targets_catalogue = True
        self.have_guide_star_catalogue = True
        self.have_standard_star_catalogue = True

    def _add_columns_to_catalogues(self):

        # Add the empty columns which we'll update
        self.df_targets['COMPLETED'] = False
        self.df_targets['Tile_number'] = -999

        # Add the columns about how many observations we need to complete for each galaxy
        # The remaining obsverations column will count down to 0
        self.df_targets['N_observations_to_complete'] = 1
        self.df_targets['remaining_observations'] = self.df_targets['N_observations_to_complete'].copy()
        self.df_targets = self.df_targets.loc[self.df_targets['remaining_observations'] > 0]

        self.logger.info("Adding extra columns to the target dataframe")

    def _rename_columns(self):

        """
        Rename a few columns to make sure things align. TODO: FixMe so this isn't needed!
        """

        # # Rename columns to match the ones the code expects
        # # This will be removed in future
        self.df_targets = self.df_targets.rename(columns=dict(GAL_MAG_R='r_mag', CATID='ID'))
        self.df_guide_stars = self.df_guide_stars.rename(columns=dict(ROWID='ID', R_MAG_AUTO='r_mag'))
        self.df_standard_stars = self.df_standard_stars.rename(columns=dict(ROWID='ID', R_MAG_AUTO='r_mag'))

    def find_premade_tiles(self):
        """
        Find any tiles which have already been configured in the output folders.
        """
        try:
            self.df_targets = pd.read_csv(f"{self.tile_location}/in_progress_targets_dataframe.csv")
        except FileNotFoundError:
            self.logger.info(f"No 'in_progress_targets_dataframe.csv' file found, so no pre-made tiles will be loaded")
            return 0

        pre_made_and_configured_files = np.sort(glob.glob(f"{self.configuration_location}/HECTORConfig_Hexa_*.txt"))
        pre_made_tiles = np.sort(glob.glob(os.path.expanduser(f"{self.tile_location}/tile_*.fld")))

        starting_tile = 0
        for configuration_file, tile_file in zip(pre_made_and_configured_files, pre_made_tiles):
            #tile_number = int(configuration_file.split('_')[-1].split('.')[0])
            #tile_members = pd.read_csv(configuration_file, delim_whitespace=True)
            starting_tile +=1

            # Now get the centres of the tile
            with open(tile_file, 'r') as f:
                lines = f.readlines()[1].split()
            self.best_tile_RAs.append(float(lines[1]))
            self.best_tile_Decs.append(float(lines[2]))

        self.logger.info(f"Loaded {starting_tile} pre-made tiles from a previous run")

        return starting_tile


    def get_remaining_targets(self, df_targets):
        """
        Get the number of targets remaining in the field to tile
        """

        remaining_targets = len(df_targets) - df_targets['COMPLETED'].sum()

        if remaining_targets > 0:
            return remaining_targets
        else:
            self.logger.info("\n\n")
            self.logger.info("Done!")
            return 0


    def add_fibre_type_column(self, tile_filename):

        # Get the dataframe, skipping the header
        tile = pd.read_csv(tile_filename, comment='#')

        tile['fibre_type'] = 'NA'
        # Galaixes and standard stars have type = 1
        tile.loc[tile.type==1, 'fibre_type'] = 'P'
        # Standards also have type "G"
        tile.loc[tile.type == 0, 'fibre_type'] = 'P'
        # Guides have type "G"
        tile.loc[tile.type == 2, 'fibre_type'] = 'G'
        # Sky fibres have type "S"
        tile.loc[~np.isfinite(tile.type), 'fibre_type'] = 'S'

        return tile


    def _run_tiling_code(self, proximity, current_tile, use_galaxy_priorities, tile_out_fname=None, guide_out_fname=None):

        
        df_targets, tile_df, guide_stars_for_tile, standard_stars_for_tile, tile_RA, tile_Dec = tiling.make_best_tile(self.df_targets, self.df_guide_stars, self.df_standard_stars, proximity=proximity, tiling_parameters=self.config, tiling_type=self.config['tiling_type'], selection_type=self.config['allocation_type'], fill_spares_with_repeats=self.config['fill_spares_with_repeats'], use_galaxy_priorities=use_galaxy_priorities)

        tiling.save_tile_outputs(f"{self.config['output_folder']}", self.df_targets, tile_df, guide_stars_for_tile, standard_stars_for_tile, tile_RA, tile_Dec, tiling_parameters=self.config, tile_number=current_tile, plot=True, columns_in_order=self.config['columns_for_target_tile_saving'], guide_columns_in_order=self.config['columns_for_guide_tile_saving'], tile_out_name=tile_out_fname, guide_out_name=guide_out_fname)

        return df_targets, tile_df, guide_stars_for_tile, tile_RA, tile_Dec



    def tile_field(self, configure_tiles=True, apply_distortion_correction=True, plot=True, config_timeout=None, use_galaxy_priorities=True, robot_temperature=12, obs_temperature=12, label="a test_label", plateID="a test_plateID", date=datetime.date.today().strftime("%Y %m %d"), check_sky_fibres=True):

        """
        Tile an entire input catalogue. Optionally apply distortion correction to go from RA/DEC to locations on the plate and optionally run the configuration code to arrange the hexabundles on the plate. 
        """

        self.logger.info(f"Robot Temperature: {robot_temperature} C, Obs Temperature: {obs_temperature} C. date={date}\n\n")
        self.robot_temperature = robot_temperature
        self.obs_temperature = obs_temperature

        # Make an empty data frame to store the properties of each tile
        self.tile_database = pd.DataFrame()

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
        remaining_targets = self.get_remaining_targets(self.df_targets)
        self.logger.info(f"Total Targets in field to tile: {remaining_targets}\n")

        for count in itertools.count():

            remaining_targets = self.get_remaining_targets(self.df_targets)
            if remaining_targets == 0:
                break

            current_tile = count + starting_tile
            self.logger.info(f"Tile {current_tile}:")
            self.logger.info(f"\tRemaining Targets: {remaining_targets}")

            # Set up the tile filenames
            tile_out_fname = Path(f"{self.tile_location}/tile_{current_tile:03}.fld")
            guide_tile_out_fname = Path(f"{self.tile_location}/guide_tile_{current_tile:03}.fld")
            tile_out_fname_after_DC = Path(f"{self.distortion_corrected_tile_location}/DC_tile_{current_tile:03}.fld")
            guide_tile_out_fname_after_DC = Path(f"{self.distortion_corrected_tile_location}/guide_DC_tile_{current_tile:03}.fld")
            configuration_output_filename = Path(f"{self.configuration_location}/HECTORConfig_Hexa_{self.config['output_filename_stem']}_{current_tile:03}.txt")
            configuration_guide_filename = Path(f"{self.configuration_location}/HECTORConfig_Guides_{self.config['output_filename_stem']}_{current_tile:03}.txt")
            configuration_plot_filename = Path(f"{self.configuration_location}/HECTORConfig_Plot_{self.config['output_filename_stem']}_{current_tile:03}.pdf")

            # filename for a plot which compares the tile before/after distortion correction
            DC_correction_comparison_plot_filename = Path(f"{self.distortion_corrected_plot_location}/Tile_{current_tile:03}_comparison.pdf")

            # Make sure that the configuration code is looking at the correct file, whether or not we're applying the distortion correction
            if configure_tiles:
                if apply_distortion_correction:
                    tile_file_for_configuration = tile_out_fname_after_DC
                    guide_tile_for_configuration = guide_tile_out_fname_after_DC
                else:
                    tile_file_for_configuration = tile_out_fname
                    guide_tile_for_configuration = guide_tile_out_fname

            # Run the tiling
            self.df_targets, tile_df, guide_stars_for_tile, tile_RA, tile_Dec = self._run_tiling_code(proximity=self.config['proximity'], current_tile=current_tile, use_galaxy_priorities=use_galaxy_priorities)

            self.best_tile_RAs.append(tile_RA)
            self.best_tile_Decs.append(tile_Dec)

            if apply_distortion_correction == True:
                # Run the distortion correction
                DC_corrected_output_file = self.apply_distortion_correction(tile_out_fname, guide_tile_out_fname, tile_out_fname_after_DC, guide_tile_out_fname_after_DC, tile_RA, tile_Dec, guide_stars_for_tile, plot_save_filename=DC_correction_comparison_plot_filename, date=date, robot_temp=robot_temperature, obs_temp=obs_temperature, label=f"{self.config['SourceCat']} Tile {count:03}", plateID=plateID, distortion_file=self.Hector_distortion_file_location, linearity_file=self.Hector_linearity_file_location, sky_fibre_file=self.Hector_sky_fibre_location, profit_file_dir=self.Profit_files_location, check_sky_fibres=check_sky_fibres)
                self.logger.info(f"\tDistortion-corrected tile saved at {tile_out_fname_after_DC}...")


            if configure_tiles == True:
                # Place holder variable to see if the configuration code has completed
                configured = False
                for j in range(self.config['MAX_TRIES']):

                    # If we're having trouble with a tile, rescale the proximity by 10 % for every five tiles we've tried
                    if self.config['Rescale_proximity'] is True:
                        proximity = self.config['proximity'] * (1 + (j // 5) * 0.1)

                    # Set up the filenames of the configureation
                    return_code = self.run_configuration_code(tile_file_for_configuration, guide_tile_for_configuration, configuration_output_filename, configuration_guide_filename, configuration_plot_filename, config_timeout)
                        
                        
                    # If we've done the tile, break out of the inner 'Max tries' loop
                    if return_code == 0:
                        configured = True
                        break
                    elif return_code == 10:
                        timeout_message = f"\t**Tiling code couldn't configure and timed out... Trying again with a new input tile.**\n\t**Proximity value is currently {proximity:.3f}, {remaining_targets} targets remaining**"
                        self.logger.info(timeout_message)
                        self.logger_R_code.info(timeout_message)

                        

                    # If we get here, it means the configuration code hasn't managed to configure. So we'll give it another tile. 
                    self.df_targets, tile_df, guide_stars_for_tile, tile_RA, tile_Dec = self._run_tiling_code(proximity=proximity, current_tile=current_tile, use_galaxy_priorities=use_galaxy_priorities)

                    if apply_distortion_correction == True:
                        DC_corrected_output_file = self.apply_distortion_correction(tile_out_fname, guide_tile_out_fname, tile_out_fname_after_DC, guide_tile_out_fname_after_DC, tile_RA, tile_Dec, guide_stars_for_tile, plot_save_filename=DC_correction_comparison_plot_filename, date=date, robot_temp=robot_temperature, obs_temp=obs_temperature, label=label, plateID=plateID, distortion_file=self.Hector_distortion_file_location, linearity_file=self.Hector_linearity_file_location, sky_fibre_file=self.Hector_sky_fibre_location, profit_file_dir=self.Profit_files_location, check_sky_fibres=check_sky_fibres)


                if not configured:
                    raise ValueError(f"Couldn't configure this tile after {self.config['MAX_TRIES']} attempts.")

            # Update the header of the files from the config code
            if configure_tiles:
                self.header_dictionary = misc_tools.update_header(configuration_output_filename, self.header_dictionary)
                self.header_dictionary = misc_tools.update_header(configuration_guide_filename, self.header_dictionary)

            # Now find which targets have been selected in this tile
            selected_targets_mask = self.df_targets['ID'].isin(tile_df['ID'])
            # Find which targets are being tiled and haven't been completed
            new_targets = (selected_targets_mask) & (self.df_targets['COMPLETED'] == False)
            # Find the targets which have been completed and are just filling out the numbers
            repeated_targets = (selected_targets_mask) & (self.df_targets['COMPLETED'] == True)

            # Subtract one from the remaining observations for everything
            self.df_targets.loc[new_targets, 'remaining_observations'] -= 1

            # Change the TILED flag for targets we've finished
            self.df_targets.loc[new_targets & (self.df_targets.loc[new_targets, 'remaining_observations'] == 0), 'COMPLETED'] = True
            # And include the tile number for each target
            self.df_targets.loc[new_targets, 'Tile_number'] = current_tile

            # Now update the tile database

            tile_df['tile_number'] = current_tile
            tile_df['tile_centre_RA'] = tile_RA
            tile_df['tile_centre_DEC'] = tile_Dec
            tile_df['Filler_Galaxy'] = False
            tile_df.loc[repeated_targets, 'Filler_Galaxy'] = True
            self.tile_database = pd.concat((self.tile_database, tile_df), ignore_index=True)
            #self.tile_database.set_index("tile_number", inplace=True)
            self.logger.info(f"\tTile contains {len(tile_df)} galaxies, of which {tile_df['Filler_Galaxy'].sum()} are repeats")
            self.logger.info(f"FINISHED TILE {current_tile}\n\n")

            # Save the overall database in its current form
            self.df_targets.to_csv(f"{self.config['output_folder']}/Tiles/in_progress_targets_dataframe.csv")
            
            self.tile_database.to_csv(f"{self.config['output_folder']}/Tiles/tiles_dataframe.csv")


        self.tile_positions = np.array([self.best_tile_RAs, self.best_tile_Decs])
        

        if plot:
            fig, ax = tiling.plot_survey_completeness_and_tile_positions(self.tile_positions, self.df_targets, self.config, fig=None, ax=None, completion_fraction_to_calculate=0.95, verbose=True)
            fig.savefig(f"{self.config['output_folder']}/Plots/{self.config['output_filename_stem']}_field_plot.pdf", bbox_inches='tight')
            plt.close('all')
        
        #Save the overall dataframes
        self.df_targets.to_csv(f"{self.config['output_folder']}/Tiles/completed_targets_dataframe.csv")

        self.N_tiles = current_tile


    def apply_distortion_correction(self, tile_out_fname, guide_tile_out_fname, tile_out_fname_after_DC, guide_tile_out_fname_after_DC, tile_RA, tile_Dec, guide_stars_for_tile, verbose=False, plot_save_filename=None, date="", robot_temp=12, obs_temp=12, label="test_label", plateID="test_plateID", distortion_file="", linearity_file="", sky_fibre_file="", profit_file_dir="", check_sky_fibres=True, turn_off_linearity=False, rot_matrix='', apply_PM_corr=True, debug_levels=None):

        """
        Take a tile file from the tiling process and apply Keith's distortion correction code. Then turn his one output file into the two output files which Caro's configuration code expects. We also need to do some work to edit the headers/etc.

        If plot_save_filename is not None, save an image of the tile before/after the correction is applied
        """

        # Touch the output file so it already exists
        Path(tile_out_fname_after_DC).touch()
        # Now call Keith's code
        DC_bash_code = [f"{self.DistortionCorrection_binary_location}",  f"{tile_out_fname}", f"{guide_tile_out_fname}", f"{tile_out_fname_after_DC}", f"{label}", f"{plateID}", f"{date}", f"{robot_temp}", f"{obs_temp}", f"{distortion_file}", f'{linearity_file}', f"{sky_fibre_file}", f"{profit_file_dir}"]

        # Turn off the sky fibre checking if check_sky_fibres is False
        if rot_matrix != '':
            DC_bash_code +=[f"xymatrix={rot_matrix}"]
        if not check_sky_fibres:
            DC_bash_code += ["-nosky"]
        if not apply_PM_corr:
            DC_bash_code += ['-nopm']
        if debug_levels is not None:
            DC_bash_code += ['-debug', f"{debug_levels}"]

        if turn_off_linearity:
            DC_bash_code += ["-nolin"]

        if verbose:
            print(shlex.join(DC_bash_code))

        process = subprocess.Popen(DC_bash_code, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        output, error = process.communicate()
        if verbose:
            self.logger.info(f"Bash command: {shlex.join(DC_bash_code)}")
            self.logger.info(output.decode("utf-8"))
        if process.returncode != 0:
            raise ValueError(output.decode("utf-8"))


        # Now take Keith's code and separate the one output file which contains galaxies, standard stars and guides into two: one which has galaxies and standard stars and one which just has the guides. This is a bit messy, but reflects the way that Caro's code reads things in. It's easier to do it in Python, where I know exactly which rows are guides/galaxies/standards, rather than guessing in R using the naming convention 

        # First, add the fibre_type column
        DC_corrected_output_file = self.add_fibre_type_column(tile_out_fname_after_DC)

        with open(tile_out_fname_after_DC,"r") as fi:
            header = []
            for ln in fi:
                if ln.startswith("#"):
                    header.append(ln)

        #Keith's code spits out the header in the reverse order to the one we want, so fix that here:
        header = list(reversed(header))


        # Convert the header list to a header dictionary
        # The extra argument to split is "maxsplits", i.e. we only want to split on the first occurence
        header_keys = [h.strip().split(',', 1)[0] for h in header]
        header_values = [h.strip().split(',', 1)[1] for h in header]

        new_header_values = dict(zip(header_keys, header_values))
        self.header_dictionary.update(new_header_values)
        #self.header_dictionary.update(zip(["#CONFIGTEMP", "#OBSTEMP"], [robot_temp, obs_temp]))


        guide_rows = DC_corrected_output_file.ID.isin(guide_stars_for_tile.ID.astype(str))
        with open(guide_tile_out_fname_after_DC, 'w') as f:
            for ln in header:
                f.write(ln)
            DC_corrected_output_file.loc[guide_rows].to_csv(f, index=False)

        with open(tile_out_fname_after_DC, 'w') as g:
            for ln in header:
                g.write(ln)
            DC_corrected_output_file.loc[~guide_rows].to_csv(g, index=False)

        if plot_save_filename is not None:
            fig, ax = plotting_tools.plot_distortion_correction_before_after(tile_out_fname_after_DC, title_text=f"{tile_out_fname_after_DC}")
            fig.savefig(f"{plot_save_filename}", bbox_inches='tight')
            plt.close(fig)

        return DC_corrected_output_file

    def apply_config_code_to_tile(self, tile_number, config_timeout=None):

        tile_file_for_configuration = Path(f"{self.distortion_corrected_tile_location}/DC_tile_{tile_number:03}.fld")
        guide_tile_for_configuration = Path(f"{self.distortion_corrected_tile_location}/guide_DC_tile_{tile_number:03}.fld")
        configuration_output_filename = Path(f"{self.configuration_location}/HECTORConfig_Hexa_{self.config['output_filename_stem']}_{tile_number:03}.txt")
        configuration_guide_filename = Path(f"{self.configuration_location}/HECTORConfig_Guides_{self.config['output_filename_stem']}_{tile_number:03}.txt")
        configuration_plot_filename = Path(f"{self.configuration_location}/HECTORConfig_Plot_{self.config['output_filename_stem']}_{tile_number:03}.pdf")

        print(f"\tRunning Configuration code on Tile {tile_number}")
        print(f"\tSaving to {configuration_output_filename}")
        if config_timeout is not None:
            print(f"\tTimeout is {config_timeout} seconds")

        return_code = self.run_configuration_code(tile_file_for_configuration, guide_tile_for_configuration, configuration_output_filename, configuration_guide_filename, configuration_plot_filename, config_timeout, log=False, print_output=True)

        return return_code

    def run_configuration_code(self, tile_file_for_configuration, guide_tile_for_configuration, configuration_output_filename, configuration_guide_filename, configuration_plot_filename, config_timeout, log=True, print_output=False):
        # Now call Caro's code
        if log:
            self.logger.info(f"\n\tRunning Configuration code on file {tile_file_for_configuration}...")
        Configuration_bash_code = ["Rscript", self.ConfigurationCode_location, tile_file_for_configuration, guide_tile_for_configuration, configuration_output_filename, configuration_guide_filename, '--plot_filename', configuration_plot_filename]
        try:
            process = subprocess.run(Configuration_bash_code, text=True, capture_output=True, timeout=config_timeout)
            if log:
                self.logger_R_code.info(process.stdout)
            if print_output:
                print(process.stdout)
            return_code = process.returncode
        except subprocess.TimeoutExpired:
            return_code = 10

        if return_code == 1:
            tiling_error_message = "\t***Error in the tiling code! Exiting to debug***\n"
            self.logger_R_code.info(process.stderr)
            self.logger_R_code.error(tiling_error_message)
            self.logger.info(tiling_error_message)
            raise subprocess.CalledProcessError(return_code, Configuration_bash_code)
        
        return return_code

    @staticmethod
    def _read_file_get_header(filename):

        """ Read in the output file and get the header, as well as the line numbers for the header"""
        with open(filename, 'r') as f:
            header_lines = []
            header_linenumbers = []
            for linenumber, line in enumerate(f):
                if line.startswith('#'):
                    header_lines.append(line)
                    header_linenumbers.append(linenumber)
        
        return header_lines, header_linenumbers

    @staticmethod
    def _write_file_with_header(outfilename, header, dataframe):
        """ Take a header and a pandas dataframe. Write the header to a file and then append the dataframe. There will be a # before the start of the column names"""
        # Now write the correct output tile file
        with open(outfilename, 'a') as f:

            for line in header:
                f.write(line.lstrip('#'))

            # Now write a comment character before the column names get written
            # Tony requires us to do this 
            f.write('#')
            dataframe.to_csv(f, index=False)


    def make_output_file_for_Tony(self, tile_file, robot_file):

        """
        Take our output files and make them into a format which Tony can read in.
        This means:
            * '#' is reserved for a comment. Header key/value pairs should not start with a #, but the row names (which are only included for readability, Tony's code doesn't use them) should
            * Missing values should be blank- i.e. na_rep=''
            * things which are specified as an integer must be made an integer, not a floating point number
        """

        tile_file = Path(f"{tile_file}")
        robot_file = Path(f"{robot_file}")

        filename_stem = tile_file.stem.split('HECTOROutput_Hexas_')[1]

        outputTileFile = Path(f"{self.final_tiles_correct_format_location}/Tile_FinalFormat_{filename_stem}.csv")
        outputRobotFile = Path(f"{self.final_tiles_correct_format_location}/Robot_FinalFormat_{filename_stem}.csv")

        # Clear the output files if they already exist
        if outputTileFile.exists():
            os.remove(outputTileFile)
        if outputRobotFile.exists():
            os.remove(outputRobotFile)


        ### The Tile File

        tile_header_lines, tile_header_linenumbers = self._read_file_get_header(tile_file)

        # Get the contents of the file:
        output_tile_contents = pd.read_csv(tile_file, comment='#')
        # Convert things which are nan values in integer columns to -99
        # The integer columns are: probe, priority, type, SkyPosition, order, order_c
        integer_columns = ['probe', 'priority', 'type', 'SkyPosition', 'order', 'order_C']
        # Replace NAs with -99
        output_tile_contents.loc[:, integer_columns] = output_tile_contents.loc[:, integer_columns].fillna(-99).astype(int)
        # Now change the types of these columns to be integers
        output_tile_contents = output_tile_contents.astype(dict(zip(integer_columns, ['int64']*len(integer_columns))))

        # Only select the required columns
        columns = ["probe","ID","x","y","rads","angs","azAngs","angs_azAng","RA","DEC","g_mag","r_mag","i_mag","z_mag","y_mag","GAIA_g_mag","GAIA_bp_mag","GAIA_rp_mag","Mstar","Re","z","GAL_MU_E_R","pmRA","pmDEC","priority","MagnetX_noDC","MagnetY_noDC","type","MagnetX","MagnetY","SkyPosition","fibre_type","Magnet","Label","order","Pickup_option","Index","Hexabundle","probe_orientation","rectMag_inputOrientation","Magnet_C","Label_C","order_C","Pickup_option_C","offset_P","offset_Q"]

        # Make sure that our IDs aren't written in standard form
        def _fix_integer_value(val):
            # This takes a value and makes sure it's an integer (e.g. 5.135143786217539e+18 becomes 5135143786217538560).
            # The try/except clause is to not change the skyfibre IDs (which are strings, e.g Sky-H7-4)
            try:
                return int(float(val))
            except ValueError:
                return val

        output_tile_contents['ID'] = output_tile_contents['ID'].apply(_fix_integer_value)

        self._write_file_with_header(outfilename=outputTileFile, header=tile_header_lines, dataframe=output_tile_contents.loc[:, columns])
        

        # Now do the robot file
        robot_header_lines, robot_header_linenumbers = self._read_file_get_header(robot_file)


        output_robot_contents = pd.read_csv(robot_file, comment='#')
        self._write_file_with_header(outfilename=outputRobotFile, header=robot_header_lines, dataframe=output_robot_contents)
        


    def allocate_hexabundles_for_single_tile(self, fileNameGuides, fileNameHexa, robot_temperature, obs_temperature, plot=False):

        ### FIXME- add documentation here



        # Input config files to be read from
        # Input file 1
        # fileNameGuides = f"{self.configuration_location}/HECTORConfig_Guides_{self.config['output_filename_stem']}_{tile_number:03d}.txt"
        # # Input file 2
        # fileNameHexa = f"{self.configuration_location}/HECTORConfig_Hexa_{self.config['output_filename_stem']}_{tile_number:03d}.txt"
        fileNameGuides = Path(fileNameGuides)
        fileNameHexa = Path(fileNameHexa)

        fileNameGuides_stem = fileNameGuides.stem.replace("Guides_", "")
        fileNameHexa_stem= fileNameHexa.stem.replace("Hexas_", "")
        print('\n\n'+str(fileNameHexa))

        # Read in the tile file to get the header, if we don't have a 'LABEL' keyword in it already
        try:
            label_string = self.header_dictionary['LABEL']
        except KeyError:
            header_dict={}
            for l in self._read_file_get_header(fileNameHexa)[0]:
                key, value = l.lstrip('#').split(',', 1)
                header_dict[key] = value
            label_string = header_dict['LABEL']


        # files to be output to after arranging the guide and hexa probe data
        # proxy output file
        plate_file = f"{self.allocation_files_location_base}/Hexa_and_Guides_{fileNameHexa_stem}.txt"
        # Output file 1
        guide_outputFile = f"{self.allocation_files_location_tiles}/HECTOROutput_Guides_{fileNameGuides_stem}.txt"

        # Adding ID column and removing the header line of Guides cluster to add to the hexa cluster
        df_guideFile, guideFileList = file_arranging.arrange_guidesFile(fileNameHexa, fileNameGuides, guide_outputFile)

        # Adding guides cluster txt file to hexa cluster txt file
        df_combined = file_arranging.merge_hexaAndGuides(fileNameHexa, df_guideFile, plate_file)

        # extracting all the magnets and making a list of them from the plate_file
        all_magnets = extract_data.create_list_of_all_magnets_from_file(extract_data.get_file(plate_file), guideFileList)

        ## UPDATE: fixed radial shift of the circular magnets (with the rectangular magnets moving in tandem with them)
        ## dependent on which annulus the circular magnet sits in
        # Offset function: similar to the standalone thermal coefficient based movement of magnet pair as a whole
        offset_circularAnnulus = {'Blu':0.0, 'Gre':0.0, 'Yel':0.0, 'Mag':0.0}
        all_magnets = offsets.magnetPair_radialPositionOffset_circularAnnulus(offset_circularAnnulus, all_magnets)

        # file to report flags regarding special cases of hexabundle allocation
        flagsFile = f'{self.allocation_files_location_base}/Flags.txt'

        # variable for the hexabundle allocation using version 3
        mu_1re_cutoff = 22.5

        # carrying out the whole hexabundle allocation algorithm from hexabundles.py script
        self.galaxyIDrecord, MagnetDict = hexabundle.overall_hexabundle_size_allocation_operation_version3_largerBundlePriority(all_magnets, \
                                     self.galaxyIDrecord, mu_1re_cutoff, self.config['output_filename_stem'], flagsFile)

        #### Offset functions- still a work in progress- need to determine input source and add column to output file
        # Input file 3 - offsets
        
        all_magnets = offsets.hexaPositionOffset(all_magnets,self.offsetFile)

        # create magnet pickup areas for all the magnets
        plots.create_magnet_pickup_areas(all_magnets)

        if plot:
            #************** # creating plots and drawing pickup areas
            fig_hexa, ax_hexa = plt.subplots()
            fig_robot, ax_robot = plt.subplots() 
            #plt.figure(1)
            # plt.clf()
            # plt.close()
            HECTOR_plate().draw_circle(colour='r', ax1=ax_hexa, ax2=ax_robot)
            # plots.draw_magnet_pickup_areas(all_magnets, '--c')
            #**************


        # test for collision and detect magnets which have all pickup directions blocked
        conflicted_magnets = conflicts.functions.find_all_blocked_magnets(all_magnets)

        # create a list of the fully blocked magnets
        fully_blocked_magnets = conflicts.functions.create_list_of_fully_blocked_magnets(conflicted_magnets)

        fully_blocked_magnets_dictionary = conflicts.functions.blocking_magnets_for_fully_blocked_magnets(conflicted_magnets)

        # print the fully blocked magnets out in terminal and record in conflicts file
        conflictsRecord = f'{self.allocation_files_location_base}/Conflicts_Index.txt'
        conflicts.blocked_magnet.print_fully_blocked_magnets(fully_blocked_magnets,conflictsRecord, fileNameHexa)

        # record the magnet and tile list in unresolvable conflicts file
        conflictFile = f'{self.allocation_files_location_base}/unresolvable_conflicts.txt'

        #***  Choose former method OR median method OR larger bundle prioritized method for hexabundle allocation  ***
        positioning_array,self.galaxyIDrecord = position_ordering.create_position_ordering_array(all_magnets, fully_blocked_magnets, \
                                                                         conflicted_magnets, MagnetDict, self.galaxyIDrecord, \
                                                                         self.config['output_filename_stem'], fileNameHexa, conflictFile)

        if plot:
            # draw all the magnets in the plots created earlier
            # Output file- figure 1
            robot_figureFile = f"{self.plot_location}/robotPlot_{fileNameHexa_stem}.pdf"
            # Output file- figure 2
            hexabundle_figureFile = f"{self.plot_location}/hexabundlePlot_{fileNameHexa_stem}.pdf"
            plots.draw_all_magnets(all_magnets, self.config['output_filename_stem'], fileNameHexa, robot_figureFile, hexabundle_figureFile, fig_hexa, ax_hexa, fig_robot, ax_robot)  #***********
            
        # checking positioning_array prints out all desired parameters
        # print(positioning_array)

        # insert column heading and print only rectangular magnet rows in the csv file
        newrow = ['Magnet', 'Label', 'Center_x', 'Center_y', 'rot_holdingPosition', 'rot_platePlacing', 'order', 'Pickup_option', 'ID','Index', 'Hexabundle', 'probe_orientation', 'rectMag_inputOrientation']
        newrow_circular = ['Magnet_C', 'Label_C', 'Center_x', 'Center_y', 'holding_position_ang', 'plate_placement_ang', 'order_C', 'Pickup_option_C', 'ID', 'Index', 'Hexabundle', 'probe_orientation', 'rectMag_inputOrientation']

        # final two output files
        # Output file 2
        outputFile = f"{self.allocation_files_location_tiles}/HECTOROutput_Hexas_{fileNameHexa_stem}.txt"
        # Output file 3
        robotFile = f"{self.allocation_files_location_robot}/Robot_{fileNameHexa_stem}.txt"

        ## DUMMY VALUES TO BE REMOVED ##
        # creating robotFile array and storing it in robot file
        positioning_array, robotFilearray = file_arranging.create_robotFileArray(label_string,positioning_array,robotFile,newrow,fully_blocked_magnets_dictionary, robot_temp=robot_temperature, obs_temp=obs_temperature)

        # adjusting the positioning array to merge only selected parameters to the output file
        positioning_array, positioning_array_circular = file_arranging.positioningArray_adjust_and_mergetoFile(positioning_array, plate_file, outputFile, newrow,newrow_circular)

        # produce final files with consistent layout and no extra commas
        file_arranging.finalFiles(all_magnets, outputFile, fileNameHexa)


        # fibre slit info output file
        output_fibreSlitInfo = f"{self.allocation_files_location_tiles}/Fibre_slitInfo_{fileNameHexa_stem}.csv"

        # Output files printed only once, not for each tile
        output_fibreAAOmega = f"{self.allocation_files_location_tiles}/Hector_fibres_AAOmega.txt"
        output_fibreSpector = f"{self.allocation_files_location_tiles}/Hector_fibres_Spector.txt"

        output_hexabundle_coordData = f"{self.allocation_files_location_tiles}/Fibre_coordData_"
        #{self.config['output_filename_stem']}_tile_{tile_number:03d}.txt"

        # create fibre slit info file for each tile updating the skyfibres with position 0
        fibres.create_fibreSlit_info_file(fileNameHexa,self.fibre_file,output_fibreSlitInfo)

        # create the fibre spectrograph files for each of AAOmega and Spector
        new_arrayAAOmega,new_arraySpector = fibres.extract_fibreInfo(self.fibre_file,output_fibreAAOmega,output_fibreSpector)

        # create the hexabundle fibre coordinate data files
        fibres.create_hexabundleFibre_coordData(output_hexabundle_coordData)

        # Output file- figure 3 fibres
        fibreFigure_AAOmega = f"{self.plot_location}/fibre_slitletAAOmega_{fileNameHexa_stem}.pdf"
        # Output file- figure 4 fibres
        fibreFigure_Spector = f"{self.plot_location}/fibre_slitletSpector_{fileNameHexa_stem}.pdf"
        # create figure of slitlets
        fibres.create_slitletFigure(new_arrayAAOmega,new_arraySpector,fibreFigure_AAOmega,fibreFigure_Spector)

        # Output file- figure 5 fibres
        skyFibre_AAOmegaFigure = f"{self.plot_location}/skyfibre_slitletAAOmega_{fileNameHexa_stem}.pdf"
        # Output file- figure 6 fibres
        skyFibre_SpectorFigure = f"{self.plot_location}/skyfibre_slitletSpector_{fileNameHexa_stem}.pdf"
        # create figure of magnified sky fibre positioning in slitlets
        fibres.create_skyFibreSlitlet_figure(fileNameHexa=fileNameHexa, new_arrayAAOmega=new_arrayAAOmega, new_arraySpector=new_arraySpector, skyFibre_AAOmegaFigure=skyFibre_AAOmegaFigure, skyFibre_SpectorFigure=skyFibre_SpectorFigure)

        plt.close('all')

        # just to check each tile's whole operation time
        # print("\t \t -----   %s seconds   -----" % (time.time() - start_time))



    def allocate_hexabundles_for_all_tiles(self):

        all_configured_tiles = np.sort(glob.glob(f"{self.configuration_location}/HECTORConfig_Hexa_*.txt"))

        tile_numbers = [int(t.split("_")[-1].split(".")[0]) for t in all_configured_tiles]

        for tile_number in tile_numbers:
            self.allocate_hexabundles_for_single_tile(tile_number)
            self.make_output_file_for_Tony(tile_number)


    def run_target_selection(self, plot=False, save=False):


        full_catalogue_table = HectorSim.load_table(self.config)
        HectorSim_input_parameters = ['BoundaryType','zlimit','MstarMin','MstarMax','SparseFunction','MSparseCut1','minRe', 'total_area', 'SourceCat']
        HectorSim_args = {k:self.config[k] for k in HectorSim_input_parameters}
        HectorSim_args['entire_table'] = full_catalogue_table

        self.target_selection = HectorSim.HectorSim(**HectorSim_args)

        if save:

            final_catalogue_path = Path(self.config['final_catalogue_name'])
            P.save_dataframe_as_FITS_table(self.target_selection.selection_function_sparsely_sampled, final_catalogue_path.expanduser().as_posix())

        if plot:
            return self._make_target_selection_plot()

    def _make_target_selection_plot(self):
        
        plt.style.use((Path(__file__).parent / "target_selection/HectorSim.mplstyle").as_posix())
        fig, axs = plt.subplots(nrows=3, ncols=6, figsize=(18.87, 5.94))
        self.target_selection.make_Julias_plot(fig, axs)
        fig.tight_layout()
        fig.subplots_adjust(hspace=0.4, wspace=0.28)

        return fig, axs

    def show_skyfibreChanges_and_magnetCountPerAnnuli(self, tile_fname_1=None, tile_fname_2=None, tile_1_guide=None, tile_2_guide=None, tile_batch=None, tileBatch_count=None, tileBatch_skyFibreChange = 'OFF'):

        ''' FUNCTION TO SHOW CHANGES IN SKYFIBRE SUB-PLATE NUMBERS AND MAGNET COUNT CHECK PERFORMED BETWEEN TWO TILES
            TWO OPTIONS TO RUN THIS FUNCTION

        INPUTS FOR OPTION 1: 'tile_number_1' and 'tile_number_2'
                             Produces the plots for the first tile and the second tile with changes in skyfibre sub-plate
                             number of second tile compared to first tile being highlighted, and conducts magnet count
                             check per annulus to record any warnings of magnet count in any annulus exceeding the
                             available number of casings on text file.

        INPUTS FOR OPTION 2: 'tile_batch' and 'tileBatch_count'
                             'tileBatch_skyFibreChange' - set as 'ON' to generate a full set of sky fibre change plots
                             Carries out a magnet count check for each tile pair in a whole batch without any repeats
                             and generates histogram plots for each annulus to show distribution of magnets count for
                             each annulus in a batch. 'tileBatch_skyFibreChange' is OFF by default, and if set to ON
                             will also produce a full set of sky fibre change plots for each tile pair in batch.

        '''

        # max magnet casings to be allowed per annuli
        annuli_count_magnetCasing = {'Blu': 18, 'Gre': 19, 'Yel': 22, 'Mag': 21}

        annuliCount_batch = {'Blu': [], 'Gre': [], 'Yel': [], 'Mag': []}

        pistonChange_countBatch = []

        if (tile_fname_1 != None) and (tile_fname_2 != None):

            # ### PRODUCING PLOT FOR THE SECOND TILE BASED ON CHANGES IN SKYFIBRE SUB-PLATE NUMBERS COMPARED TO FIRST TILE ###
            tile_1_fname_stem = Path(tile_fname_1).stem.strip('Hexas_')
            tile_2_fname_stem = Path(tile_fname_2).stem.strip('Hexas_')

            subplateSkyfibre_figureFile_tile1 = f"{self.plot_location}/subPlate_changeSkyfibrePlot_{tile_1_fname_stem}_previous.jpg"
            subplateSkyfibre_figureFile_tile2 = f"{self.plot_location}/subPlate_changeSkyfibrePlot_subPlate_changeSkyfibrePlot_{tile_2_fname_stem}_current.jpg"
            subplateSkyfibre_figureFile = f"{self.plot_location}/subPlate_changeSkyfibrePlot__previous_{tile_1_fname_stem}_new_{tile_2_fname_stem}.jpg"
            fibres.createskyfibreChanges_plot(self, tile_fname_1, tile_fname_2, subplateSkyfibre_figureFile_tile1, subplateSkyfibre_figureFile_tile2, subplateSkyfibre_figureFile)

            # check for magnet count per annulus and record any warnings on text file
            #fibres.check_magnetCount_perAnnulus(self, tile_fname_1, tile_fname_2, tile_1_guide, tile_2_guide, annuliCount_batch, annuli_count_magnetCasing)

        elif (tile_batch != None) and (tileBatch_count != None):

            # nested for loops to test out each possible pair in a batch of tiles without any repeat
            for i in range(tileBatch_count+1):

                for j in range(i, tileBatch_count+1):

                    if i != j:

                        annuliCount_batch = fibres.check_magnetCount_perAnnulus(self, i, j, annuliCount_batch, annuli_count_magnetCasing)
                        print(annuliCount_batch)

                        # produce plots for sky fibre changes if input is provided as 'on'
                        if tileBatch_skyFibreChange == 'ON':
                            subplateSkyfibre_figureFile_tile1 = f"{self.plot_location}/subPlate_changeSkyfibrePlot_{self.config['output_filename_stem']}_tile_{i:03d}_previous.jpg"
                            subplateSkyfibre_figureFile_tile2 = f"{self.plot_location}/subPlate_changeSkyfibrePlot_{self.config['output_filename_stem']}_tile_{j:03d}_current.jpg"
                            subplateSkyfibre_figureFile = f"{self.plot_location}/subPlate_changeSkyfibrePlot_{self.config['output_filename_stem']}_tile_orginal-{i:03d}_new-{j:03d}.jpg"
                            # pistonChange_count = fibres.createskyfibreChanges_plot(self, i, j, subplateSkyfibre_figureFile_tile1, subplateSkyfibre_figureFile_tile2, subplateSkyfibre_figureFile)

                            pistonChange_count = fibres.skyfibreChanges_pistonChange_count(self, i, j)
                            pistonChange_countBatch += [pistonChange_count]

            # create histogram plot
            fibres.plotHist_annuliCount_batch(self, annuliCount_batch, tile_batch, tileBatch_count)

            if tileBatch_skyFibreChange == 'ON':
                fibres.plotHist_pistonChange_count_batch(self, pistonChange_countBatch, tile_batch, tileBatch_count)

            print(len(annuliCount_batch['Blu']))
        else:
            print('\nEither pass two tile numbers or a batch of tile in the format shown:\n'+\
                  '      HP.show_sky_fibre_changes(tile number 1,tile number 2)\n'+
                  '                        OR                                  \n'+
                  '      HP.show_sky_fibre_changes( None, None, tile batch) \n')