import numpy as np
import pytest
import pandas as pd
import os
from tqdm import tqdm 
import matplotlib.pyplot as plt
 
import hop.misc.pandas_tools as P
import hop.tiling.tiling_functions as T

import logging 

logger = logging.getLogger(__name__)

np.random.seed(12345)

@pytest.fixture(scope='module')
def input_dataframes():
    file_location = os.path.dirname(os.path.abspath(__file__))

    df_targets = P.load_FITS_table_in_pandas(os.path.expanduser(f'{file_location}/test_save_files/input_test_data/targets.fits'))
    df_guide_stars = P.load_FITS_table_in_pandas(os.path.expanduser(f'{file_location}/test_save_files/input_test_data/guides.fits'))
    df_standard_stars = P.load_FITS_table_in_pandas(os.path.expanduser(f'{file_location}/test_save_files/input_test_data/standards.fits'))

    df_targets = df_targets.rename(columns=dict(r_Petro='r_mag', CATID='ID'))
    df_guide_stars = df_guide_stars.rename(columns=dict(ROWID='ID', R_MAG_AUTO=
        'r_mag'))
    df_standard_stars = df_standard_stars.rename(columns=dict(ROWID='ID', R_MAG_AUTO=
        'r_mag'))

    # Add the empty columns which we'll update
    df_targets['COMPLETED'] = False
    df_targets['Tile_number'] = -999
    df_targets['isel'] = -999
    df_targets['priority'] = 1
    df_targets['remaining_observations'] = 1.0
    df_targets['N_observations_to_complete'] = 1.0

    input_dataframes = [df_targets, df_standard_stars, df_guide_stars]
    yield input_dataframes

@pytest.fixture(scope='module')
def output_folder(input_dataframes):
    
    df_targets, df_standard_stars, df_guide_stars = input_dataframes

    tiling_type = 'greedy'
    proximity = 200
    Nsel = 19
    N_targets_per_Hector_field = 19
    Nsel_guides = 100
    Nsel_standards = 100
    TwoDF_FOV_radius = 1.0
    exclusion_zone = 10 * 15.008 / 3600.0  # This is 10mm in arcseconds
    Hector_FOV_outer_radius = TwoDF_FOV_radius - exclusion_zone - proximity / 3600.0  # Things can't have their centre be nearer than 'proximity' to the edge of the zone
    Hector_FOV_inner_radius = 0.0
    fill_spares_with_repeats = True

    tiling_parameters = dict(Hector_FOV_outer_radius=Hector_FOV_outer_radius, Hector_FOV_inner_radius=Hector_FOV_inner_radius, proximity=proximity, Nsel=Nsel, Nsel_guides=Nsel_guides, Nsel_standards=Nsel_standards, N_targets_per_Hector_field=N_targets_per_Hector_field)

    best_tile_RAs = []
    best_tile_Decs = []
    starting_tile = 0
    N_tiles = 5000

    file_location = os.path.dirname(os.path.abspath(__file__))
    output_folder = f"{file_location}/tests/test_save_files/test_field/"

    for current_tile in tqdm(range(0, N_tiles), total=58):

        if df_targets['COMPLETED'].sum() == len(df_targets):
            break

        tile_out_fname = os.path.expanduser(f"{output_folder}/Tiles/tile_{current_tile:03}.fld")

        df_targets, tile_df, guide_stars_for_tile, standard_stars_for_tile, tile_RA, tile_Dec = T.make_best_tile(df_targets, df_guide_stars, df_standard_stars, proximity=proximity, tiling_parameters=tiling_parameters, tiling_type=tiling_type, selection_type='random', fill_spares_with_repeats=fill_spares_with_repeats)
        T.save_tile_outputs(f'{output_folder}', df_targets, tile_df, guide_stars_for_tile, standard_stars_for_tile, tile_RA, tile_Dec, tiling_parameters, tile_number=current_tile, plot=True, columns_in_order=df_targets.columns.tolist(), guide_columns_in_order=df_guide_stars.columns.tolist())

        best_tile_RAs.append(tile_RA)
        best_tile_Decs.append(tile_Dec)

        selected_targets_mask = df_targets['ID'].isin(tile_df['ID'])

        # Find which ones are being tiled for the first time
        new_targets = (selected_targets_mask) & (df_targets['COMPLETED'] == False)

        # Change the TILED flag for targets we've added to a tile
        df_targets.loc[new_targets, 'COMPLETED'] = True
        # And include the tile number for each target
        df_targets.loc[new_targets, 'Tile_number'] = current_tile
    df_targets.to_csv(f'{output_folder}/Tiles/overall_targets_dataframe.csv')
    tile_positions = [best_tile_RAs, best_tile_Decs]
    fig, ax = T.plot_survey_completeness_and_tile_positions(tile_positions, df_targets, tiling_parameters, verbose=False)
    plt.close('all')

    yield output_folder


class Test_outputs_of_known_tiling():

    @pytest.mark.skip(reason="Needs to be fixed")
    def test_number_of_unique_tiles(self, output_folder):

        """
        Make sure that our tiling has the expected number of tiles
        """
        df = pd.read_csv(f'{output_folder}/Tiles/overall_targets_dataframe.csv')
        assert df['Tile_number'].unique().size == 59

