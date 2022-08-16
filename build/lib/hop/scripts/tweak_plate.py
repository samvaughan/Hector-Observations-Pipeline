import pandas as pd
import numpy as np
import argparse
import hop
from astropy.coordinates import SkyCoord
import astropy.units as u
from pathlib import Path
import subprocess
import shlex
"""
Take a Hector tile file and tweak the observation time. 

Steps:
    * Read in the final tile file at the wrong time
    * Split it into a guides file and a targets file and save these as temporary filenames
    * Run the distortion code (with no sky fibre checking) and save these outputs as temporary filenames
    * Read in the new positions and make files ready for the Hector allocation code
    * Run the Hector Allocation code and use this to make new final files. 

Because we're moving magnets on the plate, there's a chance that a field configured for one time wouldn't be safe to configure at another. Running the Hector allocation code again fixes this issue, since it checks to see if any Magnets are blocked. If so, it would be nice to be able to launch Caro's interactive app and fix this. TODO: investigate this. 
"""

if __name__ == "__main__":

    parser = argparse.ArgumentParser()
    parser.add_argument("tile_filename", help="Name of tile file to change observing date and time for")
    args = parser.parse_args()
    tile_fname = args.tile_filename

    date_and_time = input("Enter an observation date and time. Must be **UTC TIME** in the format 'YYYY MM DD HH:MM'\n")

    new_date_and_time = date_and_time.lstrip()
    print(f"\nTweaking the tile to be observed at {new_date_and_time}...")

    df = pd.read_csv(tile_fname, skiprows=11)
    # Now get the header
    with open(tile_fname, 'r') as f:
        header = f.readlines()[:11]

    # Read it into a dict
    header_dict={}
    for line in header:
        key, value = line.split(',', maxsplit=1)
        header_dict[key] = value

    # Get the field centre as an astropy SkyCoord
    tile_RA, tile_DEC = header_dict['CENTRE'].rstrip('#Field centre\n').split(',')
    field_centre = SkyCoord(f'{tile_RA} {tile_DEC}', unit=(u.hourangle, u.deg))

    # Make the new tile and guide files
    tile_file_required_columns = ['ID','RA','DEC','g_mag','r_mag','i_mag','z_mag','y_mag','GAIA_g_mag','GAIA_bp_mag','GAIA_rp_mag','Mstar','Re','z','GAL_MU_E_R','pmRA','pmDEC','priority','MagnetX_noDC','MagnetY_noDC','type']
    guide_file_required_columns = ['ID', 'RA', 'DEC', 'r_mag', 'type', 'pmRA', 'pmDEC', 'MagnetX_noDC', 'MagnetY_noDC']

    new_tile_df = df.loc[df['type'].isin([0, 1]), tile_file_required_columns]
    new_guide_df = df.loc[df['type'].isin([2]), guide_file_required_columns]

    temp_input_tile_name = Path("temp_input_tile.csv")
    temp_input_guide_name = Path("temp_input_guide_tile.csv")

    tmp_header = f"# Temporary File for plate date tweaking\n# {field_centre.ra.value:.5f} {field_centre.dec.value:.5f}\n# Proximity Value {header_dict['PROXIMITY'][:4]}\n"
    with open(temp_input_tile_name, 'w') as f:
        f.write(tmp_header)
        new_tile_df.to_csv(f, index=False)

    with open(temp_input_guide_name, 'w') as f:
        f.write(tmp_header)
        new_guide_df.to_csv(f, index=False, sep=' ')

    robot_temperature = 14
    obs_temperature = 14

    print("\tRunning the Distortion Correction code")

    # Location of various files we need
    base_folder = Path(hop.__file__).parent
    DistortionCorrection_binary_location = base_folder / Path("distortion_correction/HectorTranslationSoftware/HectorConfigUtility/HectorConfigUtil")
    profit_file_dir = "."
    # Locations of all the Hector Config code
    distortion_file = base_folder / Path("distortion_correction/HectorTranslationSoftware/DataFiles/April2022_Pos/HectorDistortion.sds")  
    linearity_file = base_folder / Path("distortion_correction/HectorTranslationSoftware/DataFiles/April2022_Pos/HectorLinear.sds")  
    sky_fibre_file = base_folder / Path("distortion_correction/HectorTranslationSoftware/DataFiles/SkyFibres.csv")

    # Check the model location and print to the screen
    if linearity_file.parent.name == "fit2-b-pos":
        correction_direction = "POSITIVE"
    elif linearity_file.parent.name == "fit2-b-neg":
        correction_direction = "NEGATIVE"
    elif linearity_file.parent.name == 'PreFeb2022':
        correction_direction = "Pre_Feb2022"
    elif linearity_file.parent.name == 'April2022_Pos':
        correction_direction = "April 2022 POSITIVE"
    else:
        correction_direction = "April2022"

    print(f"\nOptical Model Correction is {correction_direction}\n")
    print(f"Tdf linearity file is {linearity_file}, Optical Model is {distortion_file}")

    robot_temp = 14
    obs_temp = 14
    label = "None"
    plateID = 1
    tile_out_fname_after_DC = Path("temp_output_combined.csv")
    # Now call Keith's code
    DC_bash_code = [f"{DistortionCorrection_binary_location}",  f"{temp_input_tile_name}", f"{temp_input_guide_name}", f"{tile_out_fname_after_DC}", f"{label}", f"{plateID}", f"{new_date_and_time}", f"{robot_temp}", f"{obs_temp}", f"{distortion_file}", f'{linearity_file}', f"{sky_fibre_file}", f"{profit_file_dir}", "-nosky"]

    
    # if verbose:
    #     print(shlex.join(DC_bash_code))

    process = subprocess.Popen(DC_bash_code, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    output, error = process.communicate()
    if process.returncode != 0:
        raise ValueError(output.decode("utf-8"))

    new_distortion_corrected_file = pd.read_csv(tile_out_fname_after_DC, comment='#').dropna(subset=['type'])

    required_Hexas_columns = ["probe","X","ID","RA","DEC","g_mag","r_mag","i_mag","z_mag","y_mag","GAIA_g_mag","GAIA_bp_mag","GAIA_rp_mag","Mstar","Re","z","GAL_MU_E_R","pmRA","pmDEC","priority","MagnetX_noDC","MagnetY_noDC","type","MagnetX","MagnetY","SkyPosition","fibre_type","x","y","rads","angs","azAngs","angs_azAng"]
    reaquired_Guides_columns = ['ID','RA','DEC','g_mag','r_mag','i_mag','z_mag','y_mag','GAIA_g_mag','GAIA_bp_mag','GAIA_rp_mag','Mstar','Re','z','GAL_MU_E_R','pmRA','pmDEC','priority','MagnetX_noDC','MagnetY_noDC','type','MagnetX','MagnetY','SkyPosition','fibre_type','x','y','rads','angs','azAngs','angs_azAng']

    df.loc[df['type'].isin([0, 1, 2]), 'x'] = new_distortion_corrected_file['MagnetX'] / 1000.0
    df.loc[df['type'].isin([0, 1, 2]), 'y'] = new_distortion_corrected_file['MagnetY'] / 1000.0

    df.loc[df['type'].isin([0, 1, 2]), 'MagnetX'] = new_distortion_corrected_file['MagnetX']
    df.loc[df['type'].isin([0, 1, 2]), 'MagnetY'] = new_distortion_corrected_file['MagnetY']

    df.loc[df['type'].isin([0, 1, 2]), 'MagnetX_noDC'] = new_distortion_corrected_file['MagnetX_noDC']
    df.loc[df['type'].isin([0, 1, 2]), 'MagnetY_noDC'] = new_distortion_corrected_file['MagnetY_noDC']

    # Now we need to clean up after ourselves
    tile_out_fname_after_DC.unlink()
    temp_input_tile_name.unlink()
    temp_input_guide_name.unlink()

