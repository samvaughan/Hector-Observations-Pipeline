import numpy as np
import pytest
import unittest
import pandas as pd

import hector_tiling.tiling_functions as T
import hector_tiling.pandas_tools as P

"""
Test that the save_tile_text_file and save_guide_text_file functions are doing sensible things
"""

def test_pandas_tools_saving():

    """
    Make sure pandas_tools can save fits files
    """

    df = pd.DataFrame(dict(RA=np.random.randn(10), DEC=np.random.randn(10)))
    P.save_dataframe_as_FITS_table(df, "tests/test_save_files/Tiles/example_saved_fits_file.fits")


@pytest.fixture()
def tile_RA():
    tile_RA = 0.0
    yield tile_RA


@pytest.fixture()
def tile_Dec():
    tile_Dec = 0.0
    yield tile_Dec


@pytest.fixture(scope='function')
def tile_for_testing(tile_RA, tile_Dec):

    """
    Make a standard tile from fake data which we can use to test with.
    This tile will have two fake galaxy targets and two fake standard star targets
    """

    outfolder = "tests/test_save_files"
    out_name = 'test_save_targets.fld'

    tiling_parameters = dict(proximity=200)

    columns = ['ID', 'RA', 'DEC', 'Re', 'Mstar', 'z', 'GAL_MAG_G', 'GAL_MAG_I', 'GAL_MU_0_G', 'GAL_MU_0_I', 'GAL_MU_0_R', 'GAL_MU_0_U', 'GAL_MU_0_Z', 'GAL_MU_E_G', 'GAL_MU_E_I', 'GAL_MU_E_R', 'GAL_MU_E_U', 'GAL_MU_E_Z', 'GAL_MU_R_at_2Re', 'GAL_MU_R_at_3Re', 'Dingoflag', 'Ellipticity_r', 'IFU_diam_2Re', 'MassHIpred',  'PRI_SAMI',   'SersicIndex_r', 'WALLABYflag', 'g_m_i', 'isel', 'mag','priority', 'remaining_observations', 'Tile_number', 'ALREADY_TILED', 'type']

    tile_df = pd.DataFrame(np.zeros((2, len(columns))), columns=columns)
    tile_df['type'] = 1.0
    tile_df['ID'] = [1, 2]
    tile_df['RA'] = [0.0, 0.1]
    tile_df['DEC'] = [-1, -2]
    tile_df['PRI_SAMI'] = [8, 8]
    tile_df['isel'] = [9, 9]

    standard_stars_for_tile = pd.DataFrame(dict(CoADD_ID=[100, 200], R_MAG_AUTO=[9, 10], RA=[2, 1.9], DEC=[-0.5, -1], isel=[27, 27]))

    T.save_tile_text_file(outfolder, out_name, tile_df, standard_stars_for_tile, tile_RA, tile_Dec, tiling_parameters)

    tile_for_testing = f"{outfolder}/Tiles/{out_name}"

    yield tile_for_testing


@pytest.fixture(scope='function')
def guidetile_for_testing(tile_RA, tile_Dec):

    outfolder = "tests/test_save_files"
    out_name = 'test_save_guides.fld'

    tiling_parameters = dict(proximity=200)

    guide_stars_for_tile = pd.DataFrame(dict(CoADD_ID=[100, 200], R_MAG_AUTO=[9, 10], RA=[2, 1.9], DEC=[-0.5, -1]))

    T.save_guide_text_file(outfolder, out_name, guide_stars_for_tile, tile_RA, tile_Dec, tiling_parameters)

    guidetile_for_testing = f"{outfolder}/Tiles/{out_name}"

    yield guidetile_for_testing


class Test_save_target_outfile():

    @pytest.mark.usefixtures("tile_for_testing", 'tile_RA', 'tile_Dec')
    def test_RA_DEC_on_second_line(self, tile_for_testing, tile_RA, tile_Dec):

        with open(tile_for_testing, 'r') as f:
            lines = f.readlines()

        # Check that the second line is the correct RA and DEC
        r, d = lines[1].strip("#").strip().split()

        assert (float(r) == tile_RA) & (float(d) == tile_Dec)

    def test_correct_type_entered(self, tile_for_testing):

        """
        Check that we're reading in the correct type for the standard stars and the target galaxies
        """

        test_df = pd.read_csv(tile_for_testing, comment='#')

        # Check that this has types equal to [1, 1, 0, 0]
        assert np.array_equal(test_df['type'].values, np.array([1, 1, 0, 0]))

    def test_correct_column_names_for_targets(self, tile_for_testing):

        columns = ['ID', 'RA', 'DEC', 'Re', 'Mstar', 'z', 'GAL_MAG_G', 'GAL_MAG_I', 'GAL_MU_0_G', 'GAL_MU_0_I', 'GAL_MU_0_R', 'GAL_MU_0_U', 'GAL_MU_0_Z', 'GAL_MU_E_G', 'GAL_MU_E_I', 'GAL_MU_E_R', 'GAL_MU_E_U', 'GAL_MU_E_Z', 'GAL_MU_R_at_2Re', 'GAL_MU_R_at_3Re', 'Dingoflag', 'Ellipticity_r', 'IFU_diam_2Re', 'MassHIpred', 'PRI_SAMI', 'SersicIndex_r', 'WALLABYflag', 'g_m_i', 'isel', 'mag','priority', 'remaining_observations', 'Tile_number', 'ALREADY_TILED', 'type']

        test_df = pd.read_csv(tile_for_testing, comment='#')

        # Check that this has column names 'ID', 'RA', 'DEC' and 'mag' and type
        assert np.array_equal(test_df.columns.tolist(), columns)

    def test_index_not_included(self, tile_for_testing):

        test_df = pd.read_csv(tile_for_testing, comment='#')
        data = np.genfromtxt(tile_for_testing, skip_header=3, delimiter=',')

        # If we have an index column, numpy will pick it up but pandas won't. Therefore these two things won't be equal
        assert data.shape[1] == len(test_df.columns.tolist())

class Test_save_guide_outfile():

    @pytest.mark.usefixtures("guidetile_for_testing", 'tile_RA', 'tile_Dec')
    def test_RA_DEC_on_second_line(self, guidetile_for_testing, tile_RA, tile_Dec):

        with open(guidetile_for_testing, 'r') as f:
            lines = f.readlines()

        # Check that the second line is the correct RA and DEC
        r, d = lines[1].strip("#").strip().split()

        assert ((float(r) == tile_RA) & (float(d) == tile_Dec))

    def test_correct_type_entered(self, guidetile_for_testing):

        test_df = pd.read_csv(guidetile_for_testing, comment='#', delim_whitespace=True)

        # Check that this has types equal to [2, 2]
        assert np.array_equal(test_df['type'].values, np.array([2, 2]))

    def test_if_correct_column_names_for_guide(self, guidetile_for_testing):

        test_df = pd.read_csv(guidetile_for_testing, comment='#', delim_whitespace=True)

        # Check that this has column names 'ID', 'RA', 'DEC' and 'mag' and type
        assert np.array_equal(test_df.columns.tolist(), np.array(['ID', 'RA', 'DEC', 'mag', 'type']))

    def test_index_not_included(self, guidetile_for_testing):

        test_df = pd.read_csv(guidetile_for_testing, comment='#', delim_whitespace=True)
        data = np.genfromtxt(guidetile_for_testing, skip_header=3)

        # If we have an index column, numpy will pick it up but pandas won't. Therefore these two things won't be equal
        assert data.shape[1] == len(test_df.columns.tolist())
