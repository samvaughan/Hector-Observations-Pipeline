import numpy as np
import pytest
import itertools
import pandas as pd
from collections import Counter
import os

from hop.tiling import tiling_functions as T
import logging 

logger = logging.getLogger(__name__)

np.random.seed(12345)
class Test_find_clashes():

    def test_no_clashes_in_data_when_proximity_small(self):

        RA = np.array([1, 2, 2.01, 4, 5])
        DEC = np.array([1, 2, 2.01, 4, 5])

        df = pd.DataFrame(dict(RA=RA, DEC=DEC))

        # The actual distance between these points is 0.01*np.sqrt(2) degrees, which is 50 arcseconds. Therefore setting this to 50 should not cause a clash

        clashes = T.find_clashes(df, df, proximity=50)

        assert (clashes.sum() == 0.0)

    def test_recover_clashes_when_two_points_are_close(self):

        RA = np.array([1, 2, 2.01, 4, 5])
        DEC = np.array([1, 2, 2.01, 4, 5])

        df = pd.DataFrame(dict(RA=RA, DEC=DEC))

        # The actual distance between these points is 0.01*np.sqrt(2) degrees, which is 50 arcseconds. Therefore setting this to 51 should cause a clash
        clashes = T.find_clashes(df, df, proximity=51)

        assert clashes.sum() != 0.0

    def test_recover_clashes_with_single_new_point(self):

        # This is what we'll be doing in the code- adding a point and checking for a clash

        RA = np.array([1, 1.1, 0.98, 1.05, 0.97, 1.1, 1.04])
        DEC = np.array([0.0, -0.1, 0.02, -0.05, 0.3, 0.2, 0.1])
        tile_df = pd.DataFrame(dict(RA=RA, DEC=DEC))

        RA2 = np.array([1.041])
        DEC2 = np.array([0.101])
        new_point = pd.DataFrame(dict(RA=RA2, DEC=DEC2))

        # Distance between the new point is 5.09, so this shouldn't give a clash...
        clashes = T.find_clashes(tile_df, new_point, proximity=5)
        assert clashes.sum() == 0.0

        # ... but this should:
        clashes = T.find_clashes(tile_df, new_point, proximity=6)
        assert clashes.sum() != 0.0

    def test_raises_error_when_proximity_is_negative(self):

        RA = np.array([1, 1.1, 0.98, 1.05, 0.97, 1.1, 1.04])
        DEC = np.array([0.0, -0.1, 0.02, -0.05, 0.3, 0.2, 0.1])
        tile_df = pd.DataFrame(dict(RA=RA, DEC=DEC))

        RA2 = np.array([1.041])
        DEC2 = np.array([0.101])
        new_point = pd.DataFrame(dict(RA=RA2, DEC=DEC2))

        with pytest.raises(ValueError):
            T.find_clashes(tile_df, new_point, proximity=-10)


class Test_get_best_tile_centre_greedy():

    def test_get_best_tile_centre_recovers_loc_with_most_stars_in_FOV(self):

        """
        Make a cluster of targets at (10, 10), with width 0.1. Then add a few more going from 8 to 15.
        Get_best_tile_centre with a FOV of 0.1 degree and a fine grid should recover (10, 10) as the best tile centre
        """
        file_location = os.path.dirname(os.path.abspath(__file__))

        RA, DEC = np.genfromtxt(f'{file_location}/test_save_files/input_test_data/input_gaussian_cluster.txt', unpack=True)
        targets_df = pd.DataFrame(dict(RA=RA, DEC=DEC))
        priorities = np.ones(len(targets_df))

        centre = np.round(T.get_best_tile_centre_greedy(targets_df, inner_FoV_radius=0.0, outer_FOV_radius=0.2, n_xx_yy=100, priorities=priorities), 1)
        assert np.array_equal([10, 10], centre)


@pytest.fixture(scope='function')
def FOV_test_data():
    file_location = os.path.dirname(os.path.abspath(__file__))
    test_data = pd.read_csv(f'{file_location}/test_save_files/input_test_data/test_data_for_FoV.csv')

    yield test_data


class Test_check_if_in_FOV():

    def test_object_returned_from_check_if_in_FOV_is_boolean(self, FOV_test_data):

        FOV_mask = T.check_if_in_fov(FOV_test_data, 0.0, 0.0, inner_radius=0.0, outer_radius=1)

        assert FOV_mask.dtype == bool

    def test_reject_when_FOV_is_small(self, FOV_test_data):

        # Check that we don't get any galaxies in the FoV when both radii are small

        FOV_mask = T.check_if_in_fov(FOV_test_data, 0.0, 0.0, inner_radius=0.0, outer_radius=0.001)

        assert FOV_mask.sum() == 0

    def test_inner_radius_is_adhered_to(self, FOV_test_data):

        # With this known number of points, we should have 364 galaxies within an annulus of radii 0.1 and 1.0

        FOV_mask = T.check_if_in_fov(FOV_test_data, 0.0, 0.0, inner_radius=0.1, outer_radius=1.0)

        assert FOV_mask.sum() == 364

    def test_all_galaxies_recovered_when_radius_is_large(self, FOV_test_data):

        # With this known number of points, we should have 364 galaxies within an annulus of radii 0.1 and 1.0

        FOV_mask = T.check_if_in_fov(FOV_test_data, 0.0, 0.0, inner_radius=0.0, outer_radius=10.0)

        assert FOV_mask.sum() == len(FOV_test_data)

    def test_no_galaxies_recovered_when_outer_radius_is_large_and_inner_radius_is_too(self, FOV_test_data):

        # With this known number of points, we should have 364 galaxies within an annulus of radii 0.1 and 1.0

        FOV_mask = T.check_if_in_fov(FOV_test_data, 0.0, 0.0, inner_radius=9.0, outer_radius=10.0)

        assert FOV_mask.sum() == 0.0


class Test_find_nearest():

    def test_find_nearest_raises_error_with_x_y_unequal(self):

        RA = np.array([1, 2])
        DEC = np.array([1, 2, 2.01, 4, 5])

        xx = np.linspace(RA.min(), RA.max(), 100)
        yy = np.linspace(DEC.min(), DEC.max(), 100)

        grid_coords = np.array(list(itertools.product(xx, yy)))

        with pytest.raises(ValueError):
            T.find_nearest(RA, DEC, grid_coords)

    def test_raise_error_when_grid_not_two_dimensional(self):

        RA = np.array([1, 2, 2.01, 4, 5])
        DEC = np.array([1, 2, 2.01, 4, 5])

        xx = np.linspace(RA.min(), RA.max(), 100)

        with pytest.raises(ValueError):
            T.find_nearest(RA, DEC, xx)

    def test_find_nearest_picks_correct_value_known_data(self):

        # Make a random sample of 10k points around (0, 0), then find the closest grid point
        # This is enough points that it _should_ always be (0, 0).
        # TODO- read in this data
        points = np.random.randn(10000, 2)
        RA = points[:, 0]
        DEC = points[:, 1]

        xx = np.linspace(-5, 5, 11)
        yy = np.linspace(-5, 5, 11)
        grid_coords = np.array(list(itertools.product(xx, yy)))

        vals = T.find_nearest(RA, DEC, grid_coords)

        mode = int(Counter(vals).most_common(1)[0][0])

        assert np.array_equal(grid_coords[mode], np.array([0.0, 0.0]))

class Test_select_stars_for_tile():

    def test_wrong_star_type_raises_error(self):

        star_df = pd.DataFrame(dict(RA=[0, 1, 2, 3], DEC=[0, 1, 2, 3]))
        tile_df = pd.DataFrame(dict(RA=[0], DEC=[0]))

        with pytest.raises(NameError):
            T.select_stars_for_tile(star_df, tile_df, proximity=100, Nsel=10, star_type='wrong_value')

    def test_stars_sorted_properly_standards(self):

        star_df = pd.DataFrame(dict(RA=[0, 1, 2, 3], DEC=[0, 1, 2, 3], priority=[1, 2, 10, 3], r_mag=[10, 11, 10.5, 9]))
        tile_df = pd.DataFrame(dict(RA=[-1], DEC=[-1]))

        non_clashing_stars = T.select_stars_for_tile(star_df, tile_df, proximity=100, Nsel=10, star_type='standards')

        # This array should be sorted by priority in a _descending_ order. Check this is true...
        vals = non_clashing_stars['priority'].values
        # This is the array sorted descening by numpy
        sorted_desending = -np.sort(-star_df['priority'])

        assert np.array_equal(vals, sorted_desending)

    def test_stars_sorted_properly_guides(self):

        star_df = pd.DataFrame(dict(RA=[0, 1, 2, 3], DEC=[0, 1, 2, 3], priority=[1, 2, 10, 3], r_mag=[10, 11, 10.5, 9]))
        tile_df = pd.DataFrame(dict(RA=[-1], DEC=[-1]))

        non_clashing_stars = T.select_stars_for_tile(star_df, tile_df, proximity=100, Nsel=10, star_type='guides')

        # This array should be sorted by priority in a _descending_ order. Check this is true...
        vals = non_clashing_stars['r_mag'].values
        # This is the array sorted descening by numpy
        sorted_ascending = np.sort(star_df['r_mag'])

        assert np.array_equal(vals, sorted_ascending)

@pytest.fixture(scope='function')
def inputs_for_select_targets():

    file_location = os.path.dirname(os.path.abspath(__file__))
    test_data = pd.read_csv(f'{file_location}/test_save_files/input_test_data/test_data_for_select_targets.csv')
    priorities = np.ones(len(test_data))

    yield dict(all_targets_df=test_data, priorities=priorities)


class Test_select_targets():

    @pytest.mark.parametrize('Nsel', [1, 7, 12, 19, 26])
    def test_select_targets_gives_Nsel_objects_in_output(self, inputs_for_select_targets, Nsel):

        """
        When there are many untiled things in a field, check that our tiles contain NSel galaxies
        """

        proximity = 200
        tile_members, isels = T.select_targets(**inputs_for_select_targets, proximity=proximity, Nsel=Nsel, selection_type='random', fill_spares_with_repeats=False)

        assert (len(tile_members) == Nsel)

    def test_select_targets_doesnt_raise_value_error(self, inputs_for_select_targets):

        """
        Make sure that select targets doesn't raise a value error when it checks that our tile doesn't clash with itself
        """
        proximity = 200
        Nsel = 19

        try:
            tile_members, isels = T.select_targets(**inputs_for_select_targets, proximity=proximity, Nsel=Nsel, selection_type='random', fill_spares_with_repeats=False)
        except ValueError:
            pytest.fail("Value error raised implies our tile clashes with itself!")

    def test_fill_with_repeats_adds_galaxies_to_fill_tile(self, inputs_for_select_targets):

        """
        When there are fewer than Nsel things left to tile in the field, check that the 'fill_spares_with_repeats' flag does add in spare galaxies to reach Nsel targets
        """
        proximity = 200
        Nsel = 19
        # Make it so that everything in this field has been observed except one galaxy
        targets_df = inputs_for_select_targets['all_targets_df']
        targets_df['COMPLETED'] = True
        targets_df.loc[targets_df.CATAID==8706, 'COMPLETED'] = False
        priorities = inputs_for_select_targets['priorities']

        tile_members, isels = T.select_targets(all_targets_df=targets_df, priorities=priorities, proximity=proximity, Nsel=Nsel, selection_type='random', fill_spares_with_repeats=True)

        assert len(tile_members) == Nsel

    def test_fill_with_repeats_being_false_doesnt_add_galaxies_to_fill_tile(self, inputs_for_select_targets):


        """
        When there are fewer than Nsel things left to tile in a field, check that not having the fill_spares_with_repeats flag means that we only return the number of unobserved galaxies for this tile
        """
        proximity = 200
        Nsel = 19
        # Make it so that everything in this field has been observed except one galaxy
        targets_df = inputs_for_select_targets['all_targets_df']
        targets_df['COMPLETED'] = True
        targets_df.loc[targets_df.CATAID==8706, 'COMPLETED'] = False
        priorities = inputs_for_select_targets['priorities']


        tile_members, isels = T.select_targets(all_targets_df=targets_df, priorities=priorities, proximity=proximity, Nsel=Nsel, selection_type='random', fill_spares_with_repeats=False)

        assert len(tile_members) == 1


    def test_select_targets_raises_error_when_wrong_selection_type(self, inputs_for_select_targets):

        """
        When we add in a selection type which isn't 'random' or 'clashing', check this raises an error
        """

        proximity = 200
        Nsel = 19

        with pytest.raises(NameError):
            tile_members, isels = T.select_targets(**inputs_for_select_targets, proximity=proximity, Nsel=Nsel, selection_type='wrong_value', fill_spares_with_repeats=False)


@pytest.fixture(scope='function')
def inputs_for_calc_completeness():

    file_location = os.path.dirname(os.path.abspath(__file__))
    df = pd.read_csv(f"{file_location}/test_save_files/input_test_data/example_tile_distribution_for_completeness_calcs.csv")
    df['COMPLETED'] = True
    df['N_observations_to_complete'] = 1

    yield df


class Test_calculate_completeness_functions():

    def test_calc_completeness_gives_overall_correct_answer(self, inputs_for_calc_completeness):

        df = inputs_for_calc_completeness
        Nsel_max_per_tile = 19

        # This is the completeness of the tiles
        theoretical_completeness = np.ceil(len(df) / Nsel_max_per_tile)
        actual_number_used = df['Tile_number'].max() + 1 ##  Remember to add one since we index from 0

        efficiency = theoretical_completeness / actual_number_used

        # Now get the values from the code
        completeness, completion_fraction_to_calculate, used_tiles_to_get_to_x, minimum_number_of_tiles_for_x, efficiency_xpc, efficiency_from_code = T.calculate_completeness_stats(df, Nsel_max_per_tile, verbose=False)

        assert efficiency == efficiency_from_code

    def test_running_completeness_gives_correct_answer(self, inputs_for_calc_completeness):

        """
        Test that we get the right answer when calculating the completeness as a function of tile number
        """

        completeness_from_code = T._calc_completeness(inputs_for_calc_completeness)

        galaxies_per_tile = inputs_for_calc_completeness.groupby("Tile_number").size()
        running_completeness = np.cumsum(galaxies_per_tile)/len(inputs_for_calc_completeness)

        assert np.allclose(running_completeness.values, completeness_from_code)