from astropy.io import fits 
from astropy.table import Table

def load_FITS_table_in_pandas(fname):

    hdu=fits.open(fname)
    t=Table(hdu[1].data)
    return t.to_pandas()

def save_dataframe_as_FITS_table(df, fname, overwrite=True):

    t=Table.from_pandas(df)
    return t.write(fname, overwrite=overwrite)