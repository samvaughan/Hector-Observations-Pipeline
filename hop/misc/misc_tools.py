import numpy as np
import pandas as pd
import yaml
# For reading fits tables
from . import pandas_tools as P 
import os
import logging
from logging import handlers
from datetime import datetime
from pathlib import Path


def _load_config(filename):

    with open(filename, 'r') as ymlfile:
        config = yaml.safe_load(ymlfile)
    return config


def _read_table(fname):
    """
    Read in a table, either with fits or pandas
    """

    # Read in the whole table. May be an issue with massive tables?
    _, extension = os.path.splitext(os.path.expanduser(fname))

    fname = os.path.expanduser(fname)
    if extension == '.csv':
        table = pd.read_csv(fname)
    elif extension == '.tsv':
        table = pd.read_csv(fname, sep='\t')
    elif extension == '.fits':
        table = P.load_FITS_table_in_pandas(fname)
        #raise TypeError("Not yet written the code for fits... Exiting!")
    else:
        raise TypeError(f"Catalogue {fname} data type \
            not understood")
    return table


def create_output_directories(output_folder):

        # Make the folder if it doesn't exist
    if not os.path.exists(output_folder):
        os.makedirs(output_folder)

    # Make the subdirectories if they don't exist
    subfolders_to_be_made = ['Logs', 'Configuration', 'Tiles', 'Plots', 'DistortionCorrected', "Allocation", "Allocation/tile_outputs", "Allocation/robot_outputs"]
    folders = dict(zip(subfolders_to_be_made, [Path(f'{output_folder}/{subfolder}') for subfolder in subfolders_to_be_made]))
    for name, p in folders.items():
        p.mkdir(parents=True, exist_ok=True)

    return folders

def set_up_loggers(config):
    """
    Set up a pair of loggers so we can save our progress to text files
    """

    output_folder = config['output_folder']
    output_filename_stem = config['output_filename_stem']

    now = datetime.now()
    current_time = now.strftime("%d_%m_%Y-%H_%M_%S")
    logger = logging.getLogger('tiling_progress')
    logger_R_code = logging.getLogger('R_code')
    logger.setLevel(logging.INFO)
    logger_R_code.setLevel(logging.INFO)


    formatter = logging.Formatter('%(asctime)s: - %(message)s')


    logger.handlers = []
    logger_R_code.handlers = []
    # This if statement is to avoid us having multiple loggers if we run the code twice in the same session. 
    if not logger.handlers:
        # Set up our console and file handlers
        console = logging.StreamHandler()
        console.setLevel(logging.INFO)

        file_handler = handlers.WatchedFileHandler(filename=f"{output_folder}/Logs/{output_filename_stem}_{current_time}.log")
        file_handler.setLevel(logging.INFO)

        console.setFormatter(formatter)
        file_handler.setFormatter(formatter)

        logger.addHandler(console)
        logger.addHandler(file_handler)

    # Now do the same for the R code log
    if not logger_R_code.handlers:
        file_handler_R_code = handlers.WatchedFileHandler(filename=f"{output_folder}/Logs/{output_filename_stem}_{current_time}_R_Output.log")

        file_handler_R_code.setLevel(logging.INFO)

        file_handler_R_code.setFormatter(formatter)
        logger_R_code.addHandler(file_handler_R_code)

    return logger, logger_R_code