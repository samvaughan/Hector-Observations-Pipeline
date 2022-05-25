from astropy.io import fits 
from astropy.table import Table
import numpy as np

def load_FITS_table_in_pandas(fname):

    hdu=fits.open(fname)
    t=Table(hdu[1].data)
    return t.to_pandas()

def save_dataframe_as_FITS_table(df, fname, overwrite=True):

    t=Table.from_pandas(df)
    t = t.filled(fill_value=np.nan)
    return t.write(fname, overwrite=overwrite)