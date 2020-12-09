import numpy as np 
import os
import glob

from hop.misc import misc_tools
from hop.misc import pandas_tools as P
from hop.tiling import tiling_functions as tiling

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

        self.config_filename = config_filename
        self.config = misc_tools._load_config(config_filename)

        # Lists for the tile RAs and DECs
        self.best_tile_RAs = []
        self.best_tile_Decs = []

        # Set up the output folders
        misc_tools.create_output_directories(self.config['output_folder'])

        # Set up the log files
        self.logger, self.logger_R_code = misc_tools.set_up_loggers(self.config)

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
        Tile an enture input catalogue
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
            keith_output_filename = f"{self.config['output_folder']}/DistortionCorrected/DC_tile_{current_tile:03}.fld"
            guide_tile_out_fname_after_DC = f"{self.config['output_folder']}/DistortionCorrected/guide_DC_tile_{current_tile:03}.fld"
            


            if configure_tiles == True:
                raise NotImplementedError("ToDo")

            else:
                self.logger.info(f"Tile {current_tile}: \n\tSaving to {tile_out_fname}...")

                self.df_targets, tile_df, guide_stars_for_tile, standard_stars_for_tile, tile_RA, tile_Dec = tiling.make_best_tile(self.df_targets, self.df_guide_stars, self.df_standard_stars, tiling_parameters=self.config, tiling_type=self.config['tiling_type'], selection_type=self.config['allocation_type'], fill_spares_with_repeats=self.config['fill_spares_with_repeats'])

                tiling.save_tile_outputs(f"{self.config['output_folder']}", self.df_targets, tile_df, guide_stars_for_tile, standard_stars_for_tile, tile_RA, tile_Dec, tiling_parameters=self.config, tile_number=current_tile, plot=True)

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