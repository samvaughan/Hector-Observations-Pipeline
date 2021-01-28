import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
import csv
import re

def extract_fibreInfo(fibre_file):

    fibre_data = pd.read_excel(fibre_file)

    print(fibre_data['Spectrograph'])

    return fibre_data