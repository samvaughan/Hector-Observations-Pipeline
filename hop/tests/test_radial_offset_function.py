import numpy as np
from .. import radialOffset_standaloneFunction_includeMetrologyCalibration as radial
import pytest
import pandas as pd

"""
Test the radialOffset_standaloneFunction_includeMetrologyCalibration code
"""

@pytest.fixture()
def robot_dataframe():
    print("Loading the dataframe...")
    df = pd.read_csv("tests/Robot_file_for_testing.csv", skiprows=6)
    df = df.rename(columns={'#Magnet':'Magnet'})
    yield df

# Share global variables between tests
@pytest.fixture()
def config():
    return dict(robot_centre = [324.470,297.834])

class Test_radialOffset_standaloneFunction_includeMetrologyCalibration:
    
    def test_original_x_values_are_returned_when_no_offset_applied(self, robot_dataframe, config):

        old_x_values = robot_dataframe.Center_x

        updated_df = radial.apply_offsets_to_magnets(robot_dataframe, offset=0, robot_centre=config['robot_centre'], apply_telecentricity_correction=False, verbose=True)

        new_x_values = updated_df.Center_x

        assert np.array_equal(old_x_values, new_x_values)


    def test_original_y_values_are_returned_when_no_offset_applied(self, robot_dataframe, config):

        old_y_values = robot_dataframe.Center_y

        updated_df = radial.apply_offsets_to_magnets(robot_dataframe, offset=0, robot_centre=config['robot_centre'], apply_telecentricity_correction=False, verbose=True)

        new_y_values = updated_df.Center_y

        assert np.array_equal(old_y_values, new_y_values)


    @pytest.mark.parametrize("centre, offset, result", [
        ((50, 50), np.sqrt(2),  (51, 51)), 
        ((50, 50), -np.sqrt(2),  (49, 49)),
        ((-50, 50), np.sqrt(2),  (-51, 51)),
        ((-50, 50), -np.sqrt(2),  (-49, 49)),
        ((50, -50), np.sqrt(2),  (51, -51)),
        ((50, -50), -np.sqrt(2),  (49, -49)),
        ((-50, -50), np.sqrt(2),  (-51, -51)),
        ((-50, -50), -np.sqrt(2),  (-49, -49)),
        ])
    def test_radial_offset_matches_expectations(self, centre, offset, result):
        """ 
        Check that the radial offset function gives the correct results for a wide range of positions on the plate
        """
        circular_magnet = pd.DataFrame(dict(Center_x=centre[0], Center_y=centre[1]), index=[0])

        new_x, new_y = radial.calculate_radial_offset(circular_magnet, offset, robot_centre=[0, 0], apply_telecentricity_correction=False)

        if not np.allclose(a=[new_x - result[0], new_y - result[1]], b=[0, 0]):
            print(centre, offset, result)
        assert np.allclose(a=[new_x - result[0], new_y - result[1]], b=[0, 0])


    @pytest.mark.parametrize("centre, offset", [
        ((50, 50), 1),
        ((50, 50),  12),
        ((-50, 50), -3),
        ((-50, 50),  np.sqrt(6)),
        ((50, -50), -np.pi),
        ((50, -50),  300),
        ((-50, -50), 150),
        ((-50, -50),  np.sqrt(2))
        ])
    def test_radial_offset_has_correct_magnitude(self, centre, offset):

        circular_magnet = pd.DataFrame(dict(Center_x=centre[0], Center_y=centre[1]), index=[0])
        new_x, new_y = radial.calculate_radial_offset(circular_magnet, offset, robot_centre=[0, 0], apply_telecentricity_correction=False)

        offset_length = np.sqrt((circular_magnet.Center_x - new_x)**2 + (circular_magnet.Center_y - new_y)**2)

        assert np.isclose(np.abs(offset_length), np.abs(offset))


    @pytest.mark.parametrize("circular_magnet_centre, angle", [
        ((10, 10), np.radians(0)),
        ((10, 10), np.radians(45)),
        ((-30, 50), np.radians(270))
        ])
    def test_rectangular_magnets_are_along_line_through_centre_of_circular_magnet(self, circular_magnet_centre, angle):

        x_rect, y_rect = radial.calculate_rectangular_magnet_center_coordinates(circular_magnet_centre[0], circular_magnet_centre[1], angle)

        delta_x = circular_magnet_centre[0] + 27.2 * np.cos(angle) - x_rect
        delta_y = circular_magnet_centre[1] + 27.2 * np.sin(angle) - y_rect

        assert np.allclose([delta_x, delta_y], [0, 0])

    @pytest.mark.parametrize("circular_magnet_centre, telecentricity_colour, result", [
        ((10, 10), 'Blu', 20.7*0.001),
        ((10, 10), 'Gre', 42.0*0.001),
        ((-30, 50), 'Yel', 61.2*0.001),
        ((-30, 50), 'Mag', 79.9*0.001),
        ])
    def test_telecentricity_correction_is_correct_magnitude(self, config, circular_magnet_centre, telecentricity_colour, result):

        magnet = pd.DataFrame(dict(Center_x=circular_magnet_centre[0], Center_y=circular_magnet_centre[1], Label=telecentricity_colour), index=[0])
        offset_x, offset_y = radial.calculate_telecentricity_correction(magnet, config['robot_centre'], verbose=False)

        offset_magnitude = np.sqrt(offset_x**2 + offset_y**2)
        assert np.isclose(offset_magnitude, result)

    @pytest.mark.parametrize("circular_magnet_centre, telecentricity_colour", [
        ((10, 10), 'Blu'),
        ((-10, 10), 'Gre'),
        ((30, -50), 'Yel'),
        ((-30, -50), 'Mag'),
        ])
    def test_telecentricity_correction_is_radially_inwards(self, circular_magnet_centre, telecentricity_colour):

        magnet = pd.DataFrame(dict(Center_x=circular_magnet_centre[0], Center_y=circular_magnet_centre[1], Label=telecentricity_colour), index=[0])
        offset_x, offset_y = radial.calculate_telecentricity_correction(magnet, [0, 0], verbose=False)

        new_x = magnet.Center_x.values + offset_x
        new_y = magnet.Center_y.values + offset_y

        # Calculate r and theta for the original position and the new position
        # The thetas should be equal and the r values should be smaller
        old_magnet_radius = np.sqrt(magnet.Center_x**2 + magnet.Center_y**2).values
        old_magnet_theta = np.arctan(magnet.Center_y/magnet.Center_x).values

        new_magnet_radius = np.sqrt(new_x**2 + new_y**2)
        new_magnet_theta = np.arctan(new_y/new_x)
    
        assert np.isclose(old_magnet_theta, new_magnet_theta) & (new_magnet_radius < old_magnet_radius)