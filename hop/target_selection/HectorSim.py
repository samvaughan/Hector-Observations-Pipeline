import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import yaml
import bisect
# For reading fits tables
from ..misc import pandas_tools as P 
import os

from ..misc import misc_tools as misc

# def _load_config(SourceCat):

#     with open(f"Config/{SourceCat}.yml", 'r') as ymlfile:
#         config = yaml.safe_load(ymlfile)

#     return config


# def _read_table(fname):
#     """
#     Read in a table, either with fits or pandas
#     """
#     # Read in the whole table. May be an issue with massive tables?
#     extension = fname.split('.')[-1]
#     if extension == 'csv':
#         table = pd.read_csv(fname)
#     elif extension == 'tsv':
#         table = pd.read_csv(fname, sep='\t')
#     elif extension == 'fits':
#         table = P.load_FITS_table_in_pandas(fname)
#         #raise TypeError("Not yet written the code for fits... Exiting!")
#     else:
#         raise TypeError(f"Catalogue {fname} data type \
#             not understood")
#     return table


def load_table(config, quiet=False):
    """
    Read in the table, apply the selection cuts and make our cut-down table
    """

    table = misc._read_table(config['input_master_catalogue_filename'])

    selection = config['selection']
    if not quiet:
        print(f"Selected rows with: {selection}")

    # Cut down our table based on the python code in the config file
    reduced_table = table[eval(selection)]

    # Get the names of each column we want and their index in this
    # specific table
    column_names = np.array(list(config['Columns'].keys()))
    column_indices = np.array(list(config['Columns'].values()))

    # Find the columns we want which exist in this table- i.e. those which
    # have an index which isn't "None"
    if np.any(column_indices == 'None'):
        not_none_mask = column_indices != 'None'
    else:
        not_none_mask = np.ones_like(column_indices, dtype=bool)

    # Columns which we'll select from the table
    columns_to_select = column_indices[not_none_mask]
    assert np.unique(columns_to_select).size == columns_to_select.size, \
        'Looks like we have a repeated index in the config file'

    # ...and the names we'll rename them
    column_names_for_renaming = column_names[not_none_mask]

    # final table- select our columns
    final_table = reduced_table.iloc[:, columns_to_select.astype(int)]

    # Rename the columns- this is a dictionary of old_name : new_name
    renamed_columns = dict(zip(final_table.columns.tolist(),
                               column_names_for_renaming))

    final_table = final_table.rename(index=str, columns=renamed_columns)
    if not quiet:
        print("Renaming Columns:")
        for old_name, new_name in renamed_columns.items():
            print(f"\t {old_name} --> {new_name}")
            # Check everything which should be a float actually is
            if new_name in ['Mstar', 'z', 'Re', 'GAL_MAG_R', 'GAL_MAG_I']:
                final_table[f'{new_name}'] = pd.to_numeric(
                    final_table[f'{new_name}'], errors='coerce')

    # Now add in the columns which didn't exist in the table as nans
    none_values = np.where(column_indices == 'None')[0]
    for index in none_values:
        column_name = column_names[index]
        final_table[column_name] = np.nan

    return final_table



def sparse_select_flat_in_Mass(table, bins, MStarSparseCutBin):
    """
    Take a table of stellar masses and bin into 5 bins between MstarMin and
    MstarMax.
    Take the number in the MSparseCut1 bin (call it n) and then flatten all
    bins to have at most n galaxies in
    """

    # Get the bin which each value should be in
    bin_indices = np.digitize(table.Mstar, bins=bins)
    numMstar, edges = np.histogram(table.Mstar, bins=bins)

    Num_per_Mstarbin = numMstar[MStarSparseCutBin]

    # Now loop through each bin and select which bin members to keep
    # import ipdb; ipdb.set_trace()
    # indices_to_keep = []
    # for bin_num in np.unique(bin_indices):
    #     indices_in_bin = np.where(bin_indices == bin_num)[0]
    #     N_in_bin = indices_in_bin.size
    #     if N_in_bin <= Num_per_Mstarbin:
    #         indices_to_keep += (indices_in_bin.tolist())
    #     else:
    #         random_choice = np.random.choice(indices_in_bin, Num_per_Mstarbin, replace=False)
    #         indices_to_keep += random_choice.tolist()

    indices_to_keep = []
    for bin_num in np.unique(bin_indices):
        indices_in_bin = np.where(bin_indices == bin_num)[0]
        N_in_bin = indices_in_bin.size
        if bin_num <= MStarSparseCutBin:
            indices_to_keep += (indices_in_bin.tolist())
        else:
            if N_in_bin <= Num_per_Mstarbin:
                indices_to_keep += (indices_in_bin.tolist())
            else:
                random_choice = np.random.choice(indices_in_bin, Num_per_Mstarbin, replace=False)
                indices_to_keep += random_choice.tolist()

    indices_to_keep = np.sort(np.array(indices_to_keep))

    return table.iloc[indices_to_keep, :]


def safe_div(x, y):
    if y == 0:
        return 0
    else:
        return x / y


def add_boxes_to_Mstar_z(fig, ax, table_data):
    """
    Taken from original code, lines 1169 to 1204
    """

    zdivisions = (0.0, 0.02, 0.04, 0.06, 0.08, 0.1)
    Mstardivisions = (9.0, 9.4, 10.3, 10.9, 12)
    Frac_Re_Cut = 2.0
    for i in [0, 1, 2, 3, 4]:
        for j in [0, 1, 2, 3]:
            ax.plot([zdivisions[i], zdivisions[i + 1]], [Mstardivisions[j], Mstardivisions[j]], '-.', color='black')
            # ax.plot([zdivisions[i],zdivisions[i] ], [pz_max(zdivisions[i]),12], '-.', color='black')
            ax.plot([zdivisions[i], zdivisions[i]], [9, 12], '-.', color='black')
            # plt.plot([0.0,0.02 ], [9.0,9.0], '.', color='black')
            # plt.plot([0.0,0.04 ], [9.4,9.4], '.', color='black')
            # plt.plot([0.0,0.1 ], [10.3,10.3], '.', color='black')
            # plt.plot([0.0,0.1 ], [10.9,10.9], '.', color='black')
            # plt.plot([0.02,0.02 ], [pz_max(0.02),12], '.', color='black')
            # plt.plot([0.04,0.04 ], [pz_max(0.04),12], '.', color='black')
            # plt.plot([0.06,0.06 ], [pz_max(0.06),12], '.', color='black')
            # plt.plot([0.08,0.08 ], [pz_max(0.08),12], '.', color='black')
            # plt.plot([0.1,0.1 ], [pz_max(0.1),12], '.', color='black')
            Num_per_boxarray = table_data[np.logical_and.reduce((table_data.z > zdivisions[i], table_data.z < zdivisions[(i + 1)], table_data.Mstar > Mstardivisions[j], table_data.Mstar < Mstardivisions[(j + 1)]))]
            # print 'Num_per_boxarray[:,Re_col]', Num_per_boxarray[:,Re_col]
            # print 'np.shape(Num_per_boxarray)', np.shape(Num_per_boxarray)
            # print 'np.shape(Num_per_boxarray[:,Re_col])', np.shape(Num_per_boxarray[:,Re_col])
            Num_per_boxtemp = Num_per_boxarray.Re
            Num_per_box = Num_per_boxtemp.size
            # Num_per_box =Num_per_boxtemp[0]
            # print 'Num_per_boxtemp, Num_per_box, Num_per_box[0]', Num_per_boxtemp, Num_per_box, Num_per_box[0]
            Num_per_box_reGT1pt5 = np.shape(Num_per_boxtemp[(Num_per_boxtemp > Frac_Re_Cut)])  # Not sure why this is two and not our cut from the command line?

            # print 'Num_per_boxtemp[(Num_per_boxtemp>Frac_Re_Cut)]',Num_per_boxtemp[(Num_per_boxtemp>Frac_Re_Cut)]
            # print 'Num_per_box_reGT1pt5', Num_per_box_reGT1pt5
            FracGTRe1pt5 = int(100 * (safe_div((float(Num_per_box_reGT1pt5[0])), (float(Num_per_box)))))
            # print 'Num_per_boxarray, Num_per_box[0], Num_per_box_reGT1pt5[0],FracGTRe1pt5', Num_per_boxarray, Num_per_box[0], Num_per_box_reGT1pt5[0],FracGTRe1pt5
            # print '(zdivisions[(i+1)]+zdivisions[(i)])/2., (Mstardivisions[(j+1)]+Mstardivisions[(j)])/2.', (zdivisions[(i+1)]+zdivisions[(i)])/2., (Mstardivisions[(j+1)]+Mstardivisions[(j)])/2.
            zboxcentre = ((zdivisions[(i + 1)] + zdivisions[(i)]) / 2.)
            Mboxcentre = ((Mstardivisions[(j + 1)] + Mstardivisions[(j)]) / 2.)
            ax.text(zboxcentre, Mboxcentre, str(FracGTRe1pt5), color='black', horizontalalignment='center', verticalalignment='center')

    return fig, ax


def calculate_SB_at_R(Sersic_index, Mu_e, multiple_of_Re):
    """
    Calculate a galaxy's surface brightness at a given multiple of Re from its surface brightness at Re

    """

    # Sersic indices
    n_table = np.arange(1, 11)
    # Values of b from the Sersic Formula
    bvalue_table = np.array([1.6783470, 3.6720608, 5.6701554, 7.6692495, 9.6687149, 11.668363, 13.667757, 
        15.667704, 17.667636, 19.667567])
    b = np.interp(Sersic_index, n_table, bvalue_table)

    # Equation from https://ned.ipac.caltech.edu/level5/March05/Graham/Graham2.html
    SB_at_R = Mu_e + 2.5 * (b / np.log(10)) * ((multiple_of_Re)**(1. / Sersic_index) - 1)
    
    return SB_at_R


class HectorSim():
    """
    A class to simulate obsverations for Hector. 

    We load in a catalogue, select the columns we want, then make plots with (in general) three samples:
        * all objects above some limiting surface brightness (plotted in RED)
        * all objects given some selection function in mass  (plotted in BLUE)
        * a subsample of the blue objects, given some sampling technique (plotted in MAGENTA)
    """

    def __init__(self, entire_table, BoundaryType, zlimit, MstarMin, MstarMax, SparseFunction, SourceCat, MSparseCut1, minRe, total_area=np.nan, quiet=False, Dec_Min=np.nan, Dec_Max=np.nan):
        """
        Args:
            BoundaryType (str): This controls both the type of selection done in stellar mass and the limiting surface brightness of the sample. Must be one of "Taipan" or "SAMI" (case insensitive). 

            zlimit (float): The maximum z to which we select galaxies. All objects at z > zlimit will be ignored in our BLUE and MAGENTA samples.

            MstarMin (float): Used to set the leftmost edge of the smallest mass bin in the mass histograms. 

            MstarMax (float): Used to set the rightmost edge of the largest mass bin in the mass histograms. 

            SparseFunction (int): Method to go from our BLUE sample to our MAGENTA sample. Must be 1 or 2. Method 1 selects MAGENTA galaxies using the following method- all galaxies below MSparseCut1 are selected. For galaxies above MSparseCut1, we take a random sample in mass such that each bin contains the same number of galaxies as in the MSparseCut1 bin. Method 2 is the same as method 1 except we upweight star-forming objects (those on the blue cloud in colour-magnitude space). 

            SourceCat (str): Name of the input catalogue to use. Must have a file names <SourceCat>.yml in the Config/ directory, which we will use to read in the catalogue. 

            MSparseCut1 (float): The bin at which we start our sparse sampling to flatten off the mass distribution. 

            minRe (float): A cut applied on the Re of targets. Can be 0.0 to not select on Re

            quiet (bool, optional): Suppress print statements. 
        """

        # Some checking on the Boundary Type
        if BoundaryType in ['taipan', "Taipan", "TAIPAN"]:
            self.BoundaryType = "Taipan"
        elif BoundaryType in ['sami', "Sami", "SAMI"]:
            self.BoundaryType = "SAMI"
        else:
            if os.path.exists(f'{BoundaryType}'):
                self.BoundaryType = 'General'
                self.BoundaryFile = f'{BoundaryType}'
            else:
                raise NameError(f"Bounday Type {BoundaryType} not understood or file does not exist")

        # Save our input parameters as class attributes
        self.zlimit = zlimit
        self.MstarMin = MstarMin
        self.MstarMax = MstarMax
        self.SparseFunction = SparseFunction
        self.MSparseCut1 = MSparseCut1
        self.minRe = minRe
        self.SourceCat = SourceCat
        self.quiet = quiet
        self.entire_table = entire_table
        self.total_area = total_area

        if not self.quiet: 
            print(f"Maximum redshift is {self.zlimit}")
            print(f"Source Catalogue is {self.SourceCat}")
            print(f"Boundary Type is {self.BoundaryType}")
            print(f"Sparse Function is type {self.SparseFunction}")
            print(f"MSparse Cut is {self.MSparseCut1}")
            print(f"Mstar is between {self.MstarMin} and {self.MstarMax}")
            print(f"Selecting objects with Re greater than {self.minRe}")

        # Check if we have HI information and update an HI_info attribute if we do
        self.HI_info = False
        if not np.all(np.isnan(self.entire_table['WALLABYflag'])) or np.all(np.isnan(self.entire_table['Dingoflag'])):
            self.HI_info = True

        # Colourmap for plotting. Should add this as an option...
        self.cm = plt.cm.get_cmap('RdYlBu_r')

        # Make a mask to get rid of obviously silly numbers
        # We'll add a check on the surface brightness values too later if we can create them
        good_data_mask = (self.entire_table.Mstar > 7) & (self.entire_table.Mstar < self.MstarMax) & (self.entire_table.Re > 0) & (self.entire_table.Re < 100)
        # Add in columns we'll need, such as g-i colour and the IFU diameter to get to 2Re
        self.entire_table['g_m_i'] = self.entire_table.loc[:, 'GAL_MAG_G'] - self.entire_table.loc[:, 'GAL_MAG_I']
        self.entire_table['IFU_diam_2Re'] = 4 * self.entire_table.loc[:, 'Re']

        # Work out surface brightness at 2 and 3 Re if we can
        if not np.all(np.isnan(self.entire_table.GAL_MU_E_R)) | np.all(np.isnan(self.entire_table.SersicIndex_r)):
            self.entire_table['GAL_MU_R_at_2Re'] = calculate_SB_at_R(self.entire_table.SersicIndex_r, self.entire_table.GAL_MU_E_R, 2)
            self.entire_table['GAL_MU_R_at_3Re'] = calculate_SB_at_R(self.entire_table.SersicIndex_r, self.entire_table.GAL_MU_E_R, 3)
            good_data_mask = good_data_mask & (self.entire_table['GAL_MU_R_at_2Re'] < 40.) & (self.entire_table['GAL_MU_R_at_2Re'] > 0.0) & (self.entire_table['GAL_MU_R_at_3Re'] < 40.) & (self.entire_table['GAL_MU_R_at_3Re'] > 0.0)
        else:
            self.entire_table['GAL_MU_R_at_2Re'] = np.nan
            self.entire_table['GAL_MU_R_at_3Re'] = np.nan


       
        # This is our entire table, which we'll now select our samples from
        self.entire_table = self.entire_table.loc[good_data_mask]

        # Check how many rows our 'entire table' has
        assert self.entire_table.shape[0]>0, "Looks like our table doesn't have any rows after getting rid of bad values of M and R_e"
        if not self.quiet:
            print(f"\nAfter getting rid of unphysical values, the master table has {self.entire_table.shape[0]} rows")



        # Get our various limits and cuts limits
        # Mag_mask is a boolean True/False which selects all galaxies brighter than this cut
        # Mstar_cut_pz is a boolean True/False which selects galaxies above pz(z) curve
        # Mstar_cut_qz is a boolean True/False which selects galaxies above qz(z) curve
        # Z_mask is a boolean True/False which selects galaxies less than our maximum z
        # Min_Re cut is a boolean True/False which selects galaxies larger than the minimum Re

        if self.BoundaryType == 'SAMI':
            self.maglimitcut = 19.8
            maglimit = f'r<{self.maglimitcut}'
            self.mag_mask = self.entire_table.GAL_MAG_R < self.maglimitcut

        elif self.BoundaryType == 'Taipan':
            self.maglimitcut = 17.0
            maglimit = f'i<{self.maglimitcut}'
            self.mag_mask = self.entire_table.GAL_MAG_I < self.maglimitcut

        elif self.BoundaryType == 'General':
            self.maglimitcut = 19.8
            maglimit = f'r<{self.maglimitcut}'
            self.mag_mask = self.entire_table.GAL_MAG_R < self.maglimitcut
            self.sf_config = self._load_selection_function_from_file(self.BoundaryFile)

        # And now select a strip in Dec, if we want to
        if np.all(np.isfinite([Dec_Min, Dec_Max])):
            self.Dec_Min = Dec_Min
            self.Dec_Max = Dec_Max
            self.Dec_mask = (self.entire_table.DEC > self.Dec_Min) & (self.entire_table.DEC < self.Dec_Max)
            if not self.quiet:
                print(f"Selecting a strip in DEC between {self.Dec_Min} and {self.Dec_Max}")
                print(f"\tIn this stripe, we have {self.entire_table.loc[self.Dec_mask].shape[0]} rows\n")

        print(f"\nMagnitude Limit: {maglimit}")

        # These are curves/lines in the M*/Z plane which tell us which galaxies to select
        # the Taipain selection is a smooth curve, SAMI is a series of steps
        qz, pz = self.get_qz_pz(self.BoundaryType)
        self.Mstar_cut_pz = self.entire_table.Mstar > pz(self.entire_table.z)
        self.Mstar_cut_qz = self.entire_table.Mstar > qz(self.entire_table.z)
        self.zcut = self.entire_table.z < self.zlimit
        self.Re_cut = self.entire_table.Re > self.minRe

        # Now we select down our samples
        # We'll have one greater than the qz(z) curve, brighter than the mag limit and z<0.1. This is self.possible_to_observe and will be plotted in RED
        # One greater than the pz(z) curve, brighter than the mag limit and less than 0.1. This is self.selection_function and will be plotted in BLUE
        # And one the same as above but sparsely sampled. This is self.selection_function_sparsely_sampled and is plotted in MAGENTA

        if np.all(np.isfinite([Dec_Min, Dec_Max])):
            self.possible_to_observe = self.entire_table.loc[self.Mstar_cut_qz & self.mag_mask & self.zcut & self.Re_cut & self.Dec_mask]
            self.selection_function = self.entire_table.loc[self.Mstar_cut_pz & self.mag_mask & self.zcut & self.Re_cut& self.Dec_mask]
        else:
            self.possible_to_observe = self.entire_table.loc[self.Mstar_cut_qz & self.mag_mask & self.zcut & self.Re_cut]
            self.selection_function = self.entire_table.loc[self.Mstar_cut_pz & self.mag_mask & self.zcut & self.Re_cut]

        nbins = int(np.ceil((self.MstarMax - self.MstarMin) * 5.))
        self.mass_bins = np.linspace(self.MstarMin, self.MstarMax, nbins + 1)
        MStarSparseCutBin = int(np.ceil((self.MSparseCut1 - self.MstarMin) * 5))

        self.selection_function_sparsely_sampled = sparse_select_flat_in_Mass(self.selection_function, self.mass_bins, MStarSparseCutBin)


    @staticmethod
    def qz_function():

        SAMIsel2_points_x = np.array([0, 0.06, 0.10, 0.15, 0.2, 0.3])
        SAMIsel2_points_y = np.array([7.0, 9.7, 9.7, 10.0, 9.7, 7.0])
        # SAMIsel2_points_x = np.array([0,0.15])
        # SAMIsel2_points_y = np.array([9.0,10.0])
        qz = np.poly1d(np.polyfit(SAMIsel2_points_x, SAMIsel2_points_y, 2))

        return qz

    @staticmethod
    def Taipan_Selection(x):
        """
        Taipan Selection cut. Lines 760-765
        """

        # Now do a selection that follows roughly the apparent stellar mass
        # completeness in the xVSMstar_SAMI_HI_iLT17.pdf TAIPAN selection -
        # which is GAMA with i<17 cut (maglimitcut value).
        SAMIsel_points_x = np.array([0.004, 0.02, 0.03, 0.04, 0.05, 0.06, 0.08,
                                     0.09, 0.1, 0.11, 0.12, 0.14, 0.20, 0.25])
        SAMIsel_points_y = np.array([7.4, 8.9, 9.6, 9.7, 10.1, 10.1, 10.2, 10.2,
                                     10.2, 10.2, 10.2, 10.3, 10.6, 10.8])

        pz = np.poly1d(np.polyfit(SAMIsel_points_x, SAMIsel_points_y, 2))

        return pz(x)

    @staticmethod
    def _load_selection_function_from_file(filename):

        with open(f"{filename}", 'r') as ymlfile:
            selection_function = yaml.safe_load(ymlfile)

        return selection_function

    def general_selection(self, z):

        if type(z) == float:
            if (z < self.sf_config['lower_lim']):
                y = self.sf_config['lower_gradient'] * z + self.sf_config['lower_constant_offset']
            if (z >= self.sf_config['lower_lim']) & (z < self.sf_config['limit_2']):
                y = self.sf_config['mass_step_2']
            if (z >= self.sf_config['limit_2']) & (z < self.sf_config['limit_3']):
                y = self.sf_config['mass_step_3']
            if (z >= self.sf_config['limit_3']) & (z < self.sf_config['limit_4']):
                y = self.sf_config['mass_step_4']
            if (z >= self.sf_config['limit_4']) & (z < self.sf_config['limit_5']):
                y = self.sf_config['mass_step_5']
            if (z >= self.sf_config['limit_5']):
                y = self.sf_config['final_mass_step']

        else:
            y = np.zeros_like(z)
            y[z < self.sf_config['lower_lim']] = self.sf_config['lower_gradient'] * z[z < self.sf_config['lower_lim']] + self.sf_config['lower_constant_offset']
            y[(z >= self.sf_config['lower_lim']) & (z < self.sf_config['limit_2'])] =self.sf_config['mass_step_2']
            y[(z >= self.sf_config['limit_2']) & (z < self.sf_config['limit_3'])] =self.sf_config['mass_step_3']
            y[(z >= self.sf_config['limit_3']) & (z < self.sf_config['limit_4'])] =self.sf_config['mass_step_4']
            y[(z >= self.sf_config['limit_4']) & (z < self.sf_config['limit_5'])] = self.sf_config['mass_step_5']
            y[(z >= self.sf_config['limit_5'])] = self.sf_config['final_mass_step']

        return y

    @staticmethod
    def sami_stepfunc(a):
        """
        Get the SAMI steps. Vectorised version of Lines 786-823.
        """

        if type(a) == float:
            if (a < 0.02):
                y = 62.5 * a + 6.95
            if (a >= 0.02) & (a < 0.03):
                y = 8.2
            if (a >= 0.03) & (a < 0.045):
                y = 8.6
            if (a >= 0.045) & (a < 0.06):
                y = 9.4
            if (a >= 0.06) & (a < 0.095):
                y = 10.3
            if (a >= 0.095):
                y = 10.9

        else:
            y = np.zeros_like(a)
            y[a < 0.02] = 62.5 * a[a < 0.02] + 6.95
            y[(a >= 0.02) & (a < 0.03)] = 8.2
            y[(a >= 0.03) & (a < 0.045)] = 8.6
            y[(a >= 0.045) & (a < 0.06)] = 9.4
            y[(a >= 0.06) & (a < 0.095)] = 10.3
            y[(a >= 0.095)] = 10.9

        return y

    def get_qz_pz(self, BoundaryType):

        qz = self.qz_function()
        if BoundaryType == "Taipan":
            return qz, self.Taipan_Selection
        elif BoundaryType == "SAMI":
            return qz, self.sami_stepfunc
        elif BoundaryType == 'General':
            return qz, self.general_selection


        # TODO: Add in sparse selection method 2

    def plot_parent_sample(self, fig, ax):
        """
        Make a plot of the parent sample in M*-z coordinates
        """

        # Linewidths and sizes of each point
        lw = 0
        s = 1

        # Check if we have surface brightness info
        if not np.all(np.isnan(self.entire_table.GAL_MU_E_R)):

            fig, ax = self._Mstar_z_plot(self.entire_table, fig, ax, 
                colour_by=self.entire_table.GAL_MU_E_R, cbar_label="Surface Brightness", vmin=20, vmax=24, lw=lw, s=s)

        else:
            fig, ax = self._Mstar_z_plot(self.entire_table, fig, ax, lw=lw, s=s)

        # Outline each point in black if we think it has HI. 
        if self.HI_info:
            HI_mask = (self.entire_table.WALLABYflag > 0) | (self.entire_table.Dingoflag > 0)
            ax.scatter(self.entire_table.z.loc[HI_mask], self.entire_table.Mstar.loc[HI_mask], lw=0.2, facecolor='None', s=s, edgecolors='k', alpha=0.5)
            ax.text(0.10, 7.5, "black=HI", color='black', fontsize=8)

        ax.text(0.005, 12.5, "Parent sample", color='green', fontsize=8)

        return fig, ax

    def plot_Mstar_z_gi_colour(self, fig, ax):
        """
        Make a plot of M* vs z and colour by G-I colour
        """

        if not np.all(np.isnan(self.entire_table.GAL_MAG_I)):
            fig, ax = self._Mstar_z_plot(self.entire_table, fig, ax, colour_by=self.entire_table['g_m_i'], cbar_label='g - i colour', vmin=0, vmax=2)
        else:
            fig, ax = self._Mstar_z_plot(self.entire_table, fig, ax)
            ax.text(0.005, 12.5, "No I-band magnitude", color='green', fontsize=8)

        return fig, ax

    def _Mstar_z_plot(self, table, fig, ax, colour_by=None, cbar_label=None, lw=0, s=1, **kwargs):

        # FIXME Add in H1 
        """
        Plot the stellar mass against redshift for the catalogue
        """
        xp = np.linspace(0, 0.15, 1000)
        # Check if we have surfave brightnesses and colour by them if we do
        if colour_by is None:
            ax.scatter(
                x=table.z, 
                y=table.Mstar,
                marker='.', lw=lw, s=s, c='grey')
            ax.scatter(
                x=table.z[self.mag_mask], 
                y=table.Mstar[self.mag_mask], 
                marker='o', lw=lw, s=s, c='green')

        else:
            ax.scatter(
                x=table.z, 
                y=table.Mstar, 
                c=colour_by, 
                cmap=self.cm, marker='.', lw=lw, s=s, **kwargs)

            sc = ax.scatter(
                x=table.z[self.mag_mask], 
                y=table.Mstar[self.mag_mask], 
                c=colour_by[self.mag_mask], 
                cmap=self.cm, marker='o', lw=lw, s=s, **kwargs)
            cbar = fig.colorbar(sc, ax=ax)
            cbar.set_label(cbar_label)

        qz, pz = self.get_qz_pz(self.BoundaryType)

        ax.plot(xp, pz(xp), '-', color='blue')
        # plt.plot(xp, pz(xp), '-', color='orange')
        ax.plot(xp, qz(xp), '-', color='red')
        ax.set_ylim(7, 13)
        ax.set_xlim(0, 0.15)
        ax.set_xlabel('z')
        ax.set_ylabel('Mstar')

        return fig, ax

    def plot_z_Re(self, fig, ax):

        sc = ax.scatter(
            x=self.entire_table.z[self.mag_mask], 
            y=self.entire_table.Re[self.mag_mask], 
            c=(self.entire_table.g_m_i)[self.mag_mask], 
            vmin=0, vmax=2.0, cmap=self.cm, marker='o', lw=0, s=1)

        cbar = fig.colorbar(sc, ax=ax)
        cbar.set_label('g-i colour')

        # plt.plot(xp, pz(xp), '-', color='blue')
        # plt.plot(xp, pz(xp), '-', color='orange')
        # plt.plot(xp, qz(xp), '-', color='red')
        ax.set_ylim(0, 15)
        ax.set_xlim(0, 0.15)
        ax.set_xlabel('z')
        ax.set_ylabel('Re')

        return fig, ax

    def plot_selected_sample_MZ(self, fig, ax):
        """
        Make another M*-Z plot, but this time mark on each sample. This is going to be a new function which doesn't reusue _plot_M_z- I can't see an easy way to do what I want with that function without changing everything!
        """

        ax.scatter(self.entire_table.z, self.entire_table.Mstar, color='grey', marker='.')
        ax.scatter(self.possible_to_observe.z, self.possible_to_observe.Mstar, color='red', marker='.', lw=0, s=2)
        ax.scatter(self.selection_function.z, self.selection_function.Mstar, color='blue', marker='o', lw=0, s=7)
        ax.scatter(self.selection_function_sparsely_sampled.z, self.selection_function_sparsely_sampled.Mstar, color='magenta', marker='o', lw=0, s=7)

        if self.HI_info:
            HI_mask = (self.selection_function_sparsely_sampled.WALLABYflag > 0) | (self.selection_function_sparsely_sampled.Dingoflag > 0)
            ax.scatter(self.selection_function_sparsely_sampled.z.loc[HI_mask], self.selection_function_sparsely_sampled.Mstar.loc[HI_mask], color='black', marker='o', s=1)
            ax.text(0.45, 0.1, "Black: Magenta sample\nwith HI", color='black', transform=ax.transAxes, fontsize=8, bbox=dict(facecolor='white', edgecolor='k', alpha=0.8, boxstyle='round,pad=0.1'))

        xp = np.linspace(0, 0.15, 1000)
        qz, pz = self.get_qz_pz(self.BoundaryType)

        ax.plot(xp, pz(xp), '-', color='blue')
        # plt.plot(xp, pz(xp), '-', color='orange')
        ax.plot(xp, qz(xp), '-', color='red')

        ax.set_ylim(7, 13)
        ax.set_xlim(0, 0.15)
        ax.set_xlabel('z')
        ax.set_ylabel('Mstar')
        ax.text(0.05, 0.9, "Selected samples", color='green', transform=ax.transAxes, fontsize=8)
        
        return fig, ax

    def plot_selected_sample_MZ_colour_Re(self, fig, ax):

        cmap_kwargs = dict(vmin=0, vmax=5, cmap=self.cm)
        sc = ax.scatter(self.possible_to_observe.z, self.possible_to_observe.Mstar, c=self.possible_to_observe.Re, marker='.', linewidths=0, s=0.5, **cmap_kwargs)
        sc = ax.scatter(self.selection_function.z, self.selection_function.Mstar, c=self.selection_function.Re, marker='.', linewidths=0, s=1, **cmap_kwargs)
        sc = ax.scatter(self.selection_function_sparsely_sampled.z, self.selection_function_sparsely_sampled.Mstar, c=self.selection_function_sparsely_sampled.Re, marker='.', linewidths=0, s=2, **cmap_kwargs)

        cbar = fig.colorbar(sc, ax=ax)

        Frac_Re_Cut = 2  # Hardcoded number? Line 1171 in Original code
        cbar.set_label(f'Re (% Re>{Frac_Re_Cut} in black boxes')

        # Add the lines
        qz, pz = self.get_qz_pz(self.BoundaryType)
        xp = np.linspace(0, 0.15, 1000)

        # Not sure why +0.05 here?
        ax.plot(xp, pz(xp) + 0.05, '-', color='cyan')

        ax.plot(xp[xp <= 0.1], pz(xp)[xp <= 0.1], '-', color='blue')
        ax.plot(xp[xp <= 0.1], qz(xp)[xp <= 0.1], '-', color='red')
        ax.plot([0.1, 0.1], [pz(0.1), 12], '-', color='blue')
        ax.plot([0.1005, 0.1005], [qz(0.1), 12], '-', color='red')
        ax.plot([self.zlimit, self.zlimit], [pz(self.zlimit), 12], '-', color='magenta')
        ax.plot([0.03, 0.045], [8.6, 8.6], '-', color='black')  # plotting SAMI filler lower limits
        ax.plot([0.045, 0.06], [9.4, 9.4], '-', color='black')  # plotting SAMI filler lower limits
        ax.plot([0.06, 0.095], [10.3, 10.3], '-', color='black')  # plotting SAMI filler lower limits
        ax.plot([0.095, 0.115], [10.9, 10.9], '-', color='black')  # plotting SAMI filler lower limits

        # Add in the lines and fractions of galaxies in each box
        fig, ax = add_boxes_to_Mstar_z(fig, ax, self.selection_function)

        ax.set_ylim(7, 13)
        ax.set_xlim(0, 0.15)
        ax.set_xlabel('z')
        ax.set_ylabel('Mstar')

        # Add in the source densities
        if self.total_area > 0:
            density_in_selection = np.round(self.selection_function.shape[0] * np.pi / self.total_area, decimals=0)
            density_above_SB_limit = np.round(self.possible_to_observe.shape[0] * np.pi / self.total_area, decimals=0)
            density_in_sparse_sampled_selection = np.round(self.selection_function_sparsely_sampled.shape[0] * np.pi / self.total_area, decimals=0)
        else:
            density_in_selection = 'N/A'
            density_above_SB_limit = 'N/A'
            density_in_sparse_sampled_selection = 'N/A'

        ax.text(0.11, 7.5, density_above_SB_limit, color='red')
        ax.text(0.05, 7.5, density_in_selection, color='blue')
        ax.text(0.08, 7.5, density_in_sparse_sampled_selection, color='magenta')
        ax.text(0.04, 8.0, "Gal/2dF(3.14sq.deg)=", color='black')
        return fig, ax

    def _plot_hist(self, column_name, bins, fig, ax):
        """
        Plot a histogram of a column.
        If given, mask should be a list of three masks for each table. 

        """

        red = self.possible_to_observe[f'{column_name}']
        blue = self.selection_function[f'{column_name}']
        magenta = self.selection_function_sparsely_sampled[f'{column_name}']

        ax.hist(red, bins=bins, facecolor='r')    
        ax.hist(blue, bins=bins, facecolor='b')
        ax.hist(magenta, bins=bins, facecolor='magenta')
        ax.set_ylabel("No. galaxies")

        return fig, ax

    def plot_mass_hist(self, fig, ax):

        fig, ax = self._plot_hist('Mstar', self.mass_bins, fig, ax)
        ax.set_xlabel("Stellar Mass")
        return fig, ax

    def plot_z_hist(self, fig, ax):

        bins = np.linspace(0.0, 0.12, 31)
        fig, ax = self._plot_hist('z', bins, fig, ax)
        ax.set_xlabel("Redshift")
        return fig, ax

    def plot_Re_hist(self, fig, ax):

        Remin = 0
        Remax = 15
        bins = np.linspace(Remin, Remax, int(np.ceil((Remax - Remin) * 4.)))

        fig, ax = self._plot_hist('Re', bins, fig, ax)

        ax.text(0.7, 0.92, "median Re", color='black', fontsize=8, transform=ax.transAxes)  # was 13, 600
        ax.text(0.75, 0.85, np.round(np.nanmedian(self.possible_to_observe.Re), decimals=2), color='red', transform=ax.transAxes)
        ax.text(0.75, 0.8, np.round(np.nanmedian(self.selection_function.Re), decimals=2), color='blue', transform=ax.transAxes)
        ax.text(0.75, 0.75, np.round(np.nanmedian(self.selection_function_sparsely_sampled.Re), decimals=2), color='magenta', transform=ax.transAxes)

        ax.set_xlabel("R_e (arcsec)")
        return fig, ax

    def plot_gmi_hist(self, fig, ax):

        bins = np.linspace(0, 5, 21)
        fig, ax = self._plot_hist('g_m_i', bins, fig, ax)

        ax.set_xlabel("g-i colour")
        ax.set_xlim(0, 3)
        return fig, ax

    def plot_ellipticity_hist(self, fig, ax):

        bins = np.linspace(0, 1, 21)
        fig, ax = self._plot_hist('Ellipticity_r', bins, fig, ax)

        ax.set_xlabel("Ellipticity")
        return fig, ax

    def plot_SB_Re(self, fig, ax):

        # plt.plot(HECTORdata_Retemp,HECTORdata_SB, '.', color='magenta')
        ax.plot(self.selection_function.Re, self.selection_function.GAL_MU_E_R, 'o', color='blue')
        ax.plot(self.selection_function_sparsely_sampled.Re, self.selection_function_sparsely_sampled.GAL_MU_E_R, '.', color='magenta')
        ax.set_xlim(0, 15)
        ax.set_ylim(10, 30)

        ax.set_xlabel('Re (arcsec)')
        ax.set_ylabel('Surface Brightness\n (mag/arcsec^2)')

        return fig, ax

    def plot_IFU_needed_for_2Re_hist(self, fig, ax):

        fig, ax = self._plot_hist('IFU_diam_2Re', bins=100, fig=fig, ax=ax)
        ax.set_xlim(0, 60.0)
        ax.set_xlabel("IFU diameter required for 2Re (arcsec)")
        return fig, ax

    def _get_fraction_of_sample_which_fits_in_IFU_diameter(self):
        """
        Bin up the sample using np.histogram in terms of the diameter of a 2Re circle for each galaxy. We then work out the fraction of the sample which is less than each diameter (by dividing by N_total). This is a bit like a KDE of table.IFU_diam_2Re, but not quite! KDEs integrate to 1 whereas these values _sum_ to 1
        Order is [red, blue, magenta]

        Returns: A list of fractions, A list of bin edges.
        """

        N_pto = self.possible_to_observe.IFU_diam_2Re.size
        pto_binned_IFU_diam, bin_edges_pto = np.histogram(self.possible_to_observe.IFU_diam_2Re, density=False, bins=200)

        N_sf = self.selection_function.IFU_diam_2Re.size
        sf_binned_IFU_diam, bin_edges_sf = np.histogram(self.selection_function.IFU_diam_2Re, density=False, bins=200)

        N_sfss = self.selection_function_sparsely_sampled.IFU_diam_2Re.size
        sfss_binned_IFU_diam, bin_edges_sfss = np.histogram(self.selection_function_sparsely_sampled.IFU_diam_2Re, density=False, bins=200)

        fractions = [pto_binned_IFU_diam / N_pto, sf_binned_IFU_diam / N_sf, sfss_binned_IFU_diam / N_sfss]
        bins = [bin_edges_pto, bin_edges_sf, bin_edges_sfss]

        return fractions, bins

    def plot_IFU_needed_for_2Re_kde(self, fig, ax):

        fractions, bins = self._get_fraction_of_sample_which_fits_in_IFU_diameter()

        bin_edges_pto, bin_edges_sf, bin_edges_sfss = bins
        fractions_pto, fractions_sf, fractions_sfss = fractions

        ax.plot(bin_edges_sfss[:-1], fractions_sfss, color='magenta')
        ax.plot(bin_edges_sf[:-1], fractions_sf, color='blue')
        ax.plot(bin_edges_pto[:-1], fractions_pto, color='red')

        ax.set_xlabel("Bundle diameter to reach 2Re (arcsec)")
        ax.set_ylabel("Cumulative frac. sample\n for 2Re every galaxy")

        ax.set_xlabel("IFU diameter required for 2Re (arcsec)")
        ax.set_ylabel("Fraction of Sample")
        ax.set_xlim(0, 100)
        return fig, ax

    def plot_cumulative_fraction_imaged_vs_bundle_diameter(self, fig, ax):

        ax.set_xlim(0, 100)
        ax.text(16, 0.15, "127", color='black')
        ax.text(10, 0.25, "num. 105um fibres/bundle", color='black')
        ax.text(36, 0.15, "469", color='black')
        ax.text(55, 0.15, "1027", color='black')
        ax.text(75, 0.15, "1801", color='black')

        fractions, bins = self._get_fraction_of_sample_which_fits_in_IFU_diameter()

        bin_edges_pto, bin_edges_sf, bin_edges_sfss = bins
        fractions_pto, fractions_sf, fractions_sfss = fractions

        # cumulative_IFU_diam_pto = np.cumsum(fractions_pto)
        cumulative_IFU_diam_sf = np.cumsum(fractions_sf)
        cumulative_IFU_diam_sfss = np.cumsum(fractions_sfss)

        ax.plot(bin_edges_sf[:-1], cumulative_IFU_diam_sf, color='blue')
        ax.plot(bin_edges_sfss[:-1], cumulative_IFU_diam_sfss, color='magenta')
        ax.set_xlabel("Bundle diameter to reach 2Re (arcsec)")
        ax.set_ylabel("Cumulative frac. sample\n for 2Re every galaxy")

        return fig, ax

    def cumulative_fraction_sample_imaged_to_2Re(self, fig, ax):
        """
        Lines 1435-1486
        """

        fractions, bins = self._get_fraction_of_sample_which_fits_in_IFU_diameter()

        bin_edges_pto, bin_edges_sf, bin_edges_sfss = bins
        fractions_pto, fractions_sf, fractions_sfss = fractions

        DIA = np.array([1.676190476, 5.028571429, 8.380952381, 11.73333333, 15.08571429, 18.43809524, 21.79047619, 25.14285714, 28.4952381, 31.84761905, 35.2, 38.55238095, 41.9047619, 45.25714286, 48.60952381, 51.96190476, 55.31428571, 58.66666667, 62.01904762, 65.37142857, 68.72380952, 72.07619048, 75.42857143, 78.78095238, 82.13333333, 85.48571429, 88.83809524, 92.19047619, 95.54285714, 98.8952381, 102.247619, 105.6])  # assumes 105um cores with 5um cladding.
        # coresize = 105
        CORES = np.array([1, 7, 19, 37, 61, 91, 127, 169, 217, 271, 331, 397, 469, 547, 631, 721, 817, 919, 1027, 1141, 1261, 1387, 1519, 1657, 1801, 1951, 2107, 2269, 2437, 2611, 2791, 2977])

        # Now want cumulative fraction of bundles at each diameter and plot
        # Set maximum bundle size that is likely as 469=core, or the 13th element of the CORES and DIA arrays, which is overkill, but set here (-1 because 0 to 12 array):
        MAX_bundle = 13 - 1
        coresplot = CORES[0:MAX_bundle]
        bundles_frac_magenta = []  # to make a zero length array that I will append to during the loop.
        bundles_frac_blue = []
        for i in range(0, MAX_bundle):
            # test = np.where(CORES==i) #find the index at position i, as a tuple
            # tempindex = np.int(test[0]) # make theindex an integer
            # tempDIA[i] = DIA[tempindex[i]] # find the DIA value for that index

            bundles = bisect.bisect(bin_edges_sf[:-1], DIA[i]) - 1  # find first bin in the cumulative bundle diameter for 2Re plot
            # print "bundles", bundles
            # print "binsb", binsb
            # print "DIA", DIA[i]
            bundles_zLT0pt1 = bisect.bisect(bin_edges_sfss[:-1], DIA[i]) - 1
            # Now sum the fract_bundlediam_2Re fraction of galaxies at 2Re from the 0th element to the "bundles" element (should be equivalent to the bundles-th element of cumulative array:
            bundles_frac_magenta.append(np.sum(fractions_sfss[0:(bundles + 1)]))
            # print "bundles_frac", bundles_frac
            bundles_frac_blue.append(np.sum(fractions_sf[0:(bundles_zLT0pt1 + 1)]))
            # bundlesar = np.asarray(bundles)
            # for j in bundles:
        # total_fraction = cumulative[j] #find the total fraction of galaxies with 2Re<the diameter of the bundle for all bundle sizes. eg: all 2Re bins smaller than an nth-core hexabundle, are summed, so we get the cumulative fraction of galaxies that would fit within that bundle size.
        bundles_frac_magenta = np.hstack(bundles_frac_magenta)  # This converts the list to an array.
        # print "bundles_frac_arr", bundles_frac_arr
        bundles_frac_blue = np.hstack(bundles_frac_blue)

        # Now plot the bundles size in no. of cores vs fraction of galaxies:
        ax.plot(coresplot, bundles_frac_magenta, color='magenta')
        ax.plot(coresplot, bundles_frac_blue, color='blue')
        ax.text(19, 0.15, "19", color='black', ha='center')
        ax.plot([19, 19], [0.2, 1.0], ':', color='black')
        ax.text(37, 0.15, "37", color='black', ha='center')
        ax.plot([37, 37], [0.2, 1.0], ':', color='black')
        ax.text(61, 0.15, "61", color='black', ha='center')
        ax.plot([61, 61], [0.2, 1.0], ':', color='black')
        ax.text(91, 0.15, "91", color='black', ha='center')
        ax.plot([91, 91], [0.2, 1.0], ':', color='black')
        ax.text(127, 0.15, "127", color='black', ha='center')
        ax.plot([127, 127], [0.2, 1.0], ':', color='black')
        ax.text(169, 0.15, "169", color='black', ha='center')
        ax.plot([169, 169], [0.2, 1.0], ':', color='black')
        ax.text(217, 0.15, "217", color='black', ha='center')
        ax.plot([217, 217], [0.2, 1.0], ':', color='black')
        ax.set_ylabel("cumulative frac. of galaxies")
        ax.set_xlabel("No.cores in the bundle that just fits 2Re")

        return fig, ax

    def plot_SB_hist(self, fig, ax):
        """
        Plot histograms of effective surface brightness within 1Re and surface brightness at 2 and 3Re. 
        """

        bins = 50
        # Check if surface brightness is nan, as otherwise hist complains
        if not np.all(np.isnan(self.possible_to_observe.GAL_MU_E_R)):
            # Check if we've been able to make surface brightness measurements at larger radii too
            if not np.all(np.isnan(self.possible_to_observe.GAL_MU_R_at_2Re)):

                ax.hist(self.possible_to_observe.GAL_MU_R_at_3Re, color='red', bins=bins, alpha=0.2)
                ax.hist(self.selection_function.GAL_MU_R_at_3Re, color='blue', bins=bins, alpha=0.2)
                ax.hist(self.selection_function_sparsely_sampled.GAL_MU_R_at_3Re, color='magenta', bins=bins, alpha=0.2)

                ax.hist(self.possible_to_observe.GAL_MU_R_at_2Re, color='red', bins=bins, alpha=0.5)
                ax.hist(self.selection_function.GAL_MU_R_at_2Re, color='blue', bins=bins, alpha=0.5)
                ax.hist(self.selection_function_sparsely_sampled.GAL_MU_R_at_2Re, color='magenta', bins=bins, alpha=0.5)
            ax.hist(self.possible_to_observe.GAL_MU_E_R, color='red', bins=bins)
            ax.hist(self.selection_function.GAL_MU_E_R, color='blue', bins=bins)
            ax.hist(self.selection_function_sparsely_sampled.GAL_MU_E_R, color='magenta', bins=bins)

        else:   
            # Plot some fake data otherwise matplotlib complains when I try and    
            # ax.hist(np.random.normal(loc=20, scale=3, size=100), alpha=0.0)      
            ax.set_ylim(0, 250)

        ax.set_xlim(18, 28)
        ax.set_xlabel("Surface brightness at 1Re (filled) \n and 2Re (tranparent)\n and 3Re (most transparent) (mag/arcsec^2)")
        ax.set_ylabel('No. galaxies')

        # Line at the limiting surface brightness of a SAMI galaxy to get S/N ~5 after 4 hours when when binned up 
        # by 24 cores
        # ax.axvline(25, '-', color='green')
        
        return fig, ax

    def plot_percentage_galaxy_sample_imaged_2Re(self, fig, ax):
        """
        Lines 1597 - 1707
        """

        # TotalFracto2Re=((percent100umcores[7,1]+percent100umcores[8,1]) - percentfinalbundledistribution[7,1])+ (percent100umcores[6,1]-(percentfinalbundledistribution[6,1]-(percent100umcores[7,1] - percentfinalbundledistribution[7,1]))) + (percent100umcores[5,1]-(percentfinalbundledistribution[5,1]-((percent100umcores[6,1]-(percentfinalbundledistribution[6,1]-(percent100umcores[7,1] - percentfinalbundledistribution[7,1])))))) + (percent100umcores[4,1]-(percentfinalbundledistribution[4,1]- ((percent100umcores[5,1]-(percentfinalbundledistribution[5,1]-((percent100umcores[6,1]-(percentfinalbundledistribution[6,1]-(percent100umcores[7,1] - percentfinalbundledistribution[7,1])))))))))

        percent100umcores, percent100umcores_zLT0pt1, percent75umcores, cores9, cores13, TotalFracto2Re = self._get_percent_100um_cores()

        # ax = fig3.add_subplot(2,4,8)
        ax.plot(cores9, percent100umcores[:, 1], '-o', color='magenta', label='105um fibres')
        ax.plot(cores9, percent100umcores_zLT0pt1[:, 1], '-o', color='blue', label='105um fibres')
        # ax.plot(cores13,percent75umcores[:,1], '-.', color='magenta',label='75um fibres')
        ax.text(17, 27, "19", color='black', size=8, ha='center')
        ax.plot([19, 19], [2, 20], ':', color='black')
        ax.text(37, 27, "37", color='black', size=8, ha='center')
        ax.plot([37, 37], [2, 20], ':', color='black')
        ax.text(61, 27, "61", color='black', size=8, ha='center')
        ax.plot([61, 61], [2, 20], ':', color='black')
        ax.text(91, 27, "91", color='black', size=8, ha='center')
        ax.plot([91, 91], [2, 20], ':', color='black')
        ax.text(127, 27, "127", color='black', size=8, ha='center')
        ax.plot([127, 127], [2, 20], ':', color='black')
        ax.text(169, 27, "169", color='black', size=8, ha='center')
        ax.plot([169, 169], [2, 20], ':', color='black')
        ax.text(217, 27, "217", color='black', size=8, ha='center')
        ax.plot([217, 217], [2, 20], ':', color='black')
        ax.text(50, 3, "Percentage (magenta)\nimaged to 2Re = " + str(TotalFracto2Re) + "%", color='magenta', size=8, ha='left', bbox=dict(facecolor='white', edgecolor='black', alpha=0.5, boxstyle='round,pad=0.1'))
        ax.set_xlim(0, 300)
        ax.set_ylim(0, 30)
        ax.set_xlabel('No.cores in the bundle that just fits 2Re')
        ax.set_ylabel('% galaxy sample \n imaged to 2Re')

        return fig, ax



    def _get_percent_100um_cores(self):

        bundlediam_2Re = self.selection_function_sparsely_sampled['IFU_diam_2Re']
        bundlediam_2Re_zLT0pt1 = self.selection_function['IFU_diam_2Re']

        cores9 = [1, 7, 19, 37, 61, 91, 127, 169, 170]  # 217, 218]
        cores13 = [1, 7, 19, 37, 61, 91, 127, 169, 217, 271, 331, 397, 469, 470]
        # diamfor100umcores = [1.726476, 5.17942857, 8.63238, 12.085333, 15.5382857, 18.99123809, 22.44419, 25.89714]  # , 28.4952381]  updated for 103um core and max 169 cores
        # diamfor75umcores = [1.219047619, 3.657142857, 6.095238095, 8.533333333, 10.97142857, 13.40952381, 15.84761905, 18.28571429, 20.72380952, 23.16190476, 25.6, 28.03809524, 30.47619048]
        # percentcores = [0.3, 3.5, 9.5, 13.8, 15.6, 13, 11, 9, 6]
        # plt.plot(cores, percentcores) which I quickly plotted for my instrument science talk.
        # Make a temporary array for the number of galaxies for each of these 9 diameters plus an extra one for > 28.5
        percent100umcores = np.array([[1.726476, 1], [5.17942857, 1], [8.63238, 1], [12.085333, 1], [15.5382857, 1], [18.99123809, 1], [22.44419, 1], [25.89714, 1], [28, 1]])  # Note:points at 28 are for everything >25.897
        percent100umcores_zLT0pt1 = np.array([[1.726476, 1], [5.17942857, 1], [8.63238, 1], [12.085333, 1], [15.5382857, 1], [18.99123809, 1], [22.44419, 1], [25.89714, 1], [28, 1]])  # Note:points at 30 are for everything >28.5
        percent75umcores = np.array([[1.219047619, 1], [3.657142857, 1], [6.095238095, 1], [8.533333333, 1], [10.97142857, 1], [13.40952381, 1], [15.84761905, 1], [18.28571429, 1], [20.72380952, 1], [23.16190476, 1], [25.6, 1], [28.03809524, 1], [30.47619048, 1], [35, 1]])  # Note:points at 35 are for everything >30.5
        lenpc100 = len(percent100umcores)
        lenpc75 = len(percent75umcores)

        percentfinalbundledistribution = np.array([[1.726476, 0], [5.17942857, 0], [8.63238, 0], [12.085333, 0], [15.5382857, (14. / 19.) * 100], [18.99123809, (2. / 19.) * 100], [22.44419, (1. / 19.) * 100], [25.89714, (2. / 19.) * 100], [28, 0.]])  # excluding the 2 star bundles

        for i in range(lenpc100):
            if i == 0:
                test1 = float(len(bundlediam_2Re[(bundlediam_2Re < percent100umcores[i, 0])]))
                test3 = ((test1) / (float(len(bundlediam_2Re)))) * 100
                percent100umcores[i, 1] = test3
                test1 = float(len(bundlediam_2Re_zLT0pt1[(bundlediam_2Re_zLT0pt1<percent100umcores[i,0])]))
                test3 = ((test1)/(float(len(bundlediam_2Re_zLT0pt1))))*100
                percent100umcores_zLT0pt1[i,1] = test3
            if i>0:
                test1 = float(len(bundlediam_2Re[(bundlediam_2Re<percent100umcores[i,0])]))
                test2 = float(len(bundlediam_2Re[(bundlediam_2Re<percent100umcores[i-1,0])]))
                test3 = ((test1 - test2)/(float(len(bundlediam_2Re))))*100
                percent100umcores[i,1] = test3
                test1 = float(len(bundlediam_2Re_zLT0pt1[(bundlediam_2Re_zLT0pt1<percent100umcores[i,0])]))
                test2 = float(len(bundlediam_2Re_zLT0pt1[(bundlediam_2Re_zLT0pt1<percent100umcores[i-1,0])]))
                test3 = ((test1 - test2)/(float(len(bundlediam_2Re_zLT0pt1))))*100
                percent100umcores_zLT0pt1[i,1] = test3
            if i==(lenpc100-1):
                test1 = float(len(bundlediam_2Re[(bundlediam_2Re>percent100umcores[i-1,0])]))
                test3 = ((test1)/(float(len(bundlediam_2Re))))*100
                percent100umcores[i,1] = test3
                test1 = float(len(bundlediam_2Re_zLT0pt1[(bundlediam_2Re_zLT0pt1>percent100umcores[i-1,0])]))
                test3 = ((test1)/(float(len(bundlediam_2Re_zLT0pt1))))*100
                percent100umcores_zLT0pt1[i,1] = test3
        #print percent100umcores
        for i in range(lenpc75):
            if i==0:
                test1= float(len(bundlediam_2Re[(bundlediam_2Re<percent75umcores[i,0])]))
                test3 = ((test1)/(float(len(bundlediam_2Re))))*100
                percent75umcores[i,1] = test3
            if i>0:
                test1 = float(len(bundlediam_2Re[(bundlediam_2Re<percent75umcores[i,0])]))
                test2 = float(len(bundlediam_2Re[(bundlediam_2Re<percent75umcores[i-1,0])]))
                test3 = ((test1 - test2)/(float(len(bundlediam_2Re))))*100
                percent75umcores[i,1] = test3
            if i == (lenpc75-1):
                test1 = float(len(bundlediam_2Re[(bundlediam_2Re>percent75umcores[i-1,0])]))
                test3 = ((test1)/(float(len(bundlediam_2Re))))*100
                percent75umcores[i,1] = test3
        #print percent75umcores

        # Now calculate the fraction of all galaxies that are imaged to 2Re if the largest is put in the largest bundle etc, down to the smallest - i.e. not allowing for tiling variations.
        #Total
        if  percent100umcores[8,1]> percentfinalbundledistribution[7,1]:
            numfit169 = 0
            numleft169 = percent100umcores[8,1] - percentfinalbundledistribution[7,1] + percent100umcores[7,1]
        else:
            numfit169 = percentfinalbundledistribution[7,1]-percent100umcores[8,1]
            numleft169 = percent100umcores[7,1] - numfit169 #number in the 169 bin that did not fit
        if  numleft169 > percentfinalbundledistribution[6,1]:
            numfit127 = 0
            numleft127 = numleft169 - percentfinalbundledistribution[6,1] + percent100umcores[6,1]
        else:
            numfit127 = percentfinalbundledistribution[6,1]- numleft169
            numleft127 = numleft169 - numfit127 + percent100umcores[6,1]
        if  numleft127 > percentfinalbundledistribution[5,1]:
            numfit91 = 0
            numleft91 = numleft127 - percentfinalbundledistribution[5,1] + percent100umcores[5,1]
        else:
            numfit91 = percentfinalbundledistribution[5,1]- numleft127
            numleft91 = numleft127 - numfit91 + percent100umcores[5,1]
        if  numleft91 > percentfinalbundledistribution[4,1]:
            numfit61 = 0
            numleft61 = numleft91 - percentfinalbundledistribution[4,1] + percent100umcores[4,1]
        else:
            numfit61 = percentfinalbundledistribution[4,1]- numleft91
            numleft61 = numleft91 - numfit61 + percent100umcores[4,1]
        TotalFracto2Re = np.int(numfit169 + numfit127 + numfit91 + numfit61)

        return percent100umcores, percent100umcores_zLT0pt1, percent75umcores, cores9, cores13, TotalFracto2Re



    def cumulative_percentage_galaxies_imaged_2Re(self, fig, ax):
        """
        Lines 1716- 1798
        """

        percent100umcores, percent100umcores_zLT0pt1, percent75umcores, cores9, cores13, TotalFracto2Re = self._get_percent_100um_cores()

        percent100umcores[4,1] = percent100umcores[0,1] + percent100umcores[1,1] + percent100umcores[2,1] + percent100umcores[3,1] + percent100umcores[4,1]
        percent100umcores[0,1] = 0
        percent100umcores[1,1] = 0
        percent100umcores[2,1] = 0
        percent100umcores[3,1] = 0
        #print "percent100umcores",percent100umcores
        percent75umcores[6,1] = percent75umcores[0,1] + percent75umcores[1,1] + percent75umcores[2,1] + percent75umcores[3,1] + percent75umcores[4,1] + percent75umcores[5,1] + percent75umcores[6,1]
        percent75umcores[0,1] = 0
        percent75umcores[1,1] = 0
        percent75umcores[2,1] = 0
        percent75umcores[3,1] = 0
        percent75umcores[4,1] = 0
        percent75umcores[5,1] = 0
        #***


        #Now do cumulative plot vs cumulative number of cores:
        cum_pc100umcores = np.cumsum(percent100umcores[:,1])
        #print "cum_pc100umcores", cum_pc100umcores
        cum_pc75umcores = np.cumsum(percent75umcores[:,1])
        totalbundles = np.array([100,90,80,70,60,50,20]) #total number of hexabundles
        cores9_sumfibrenumbers_100 = cores9 * (percent100umcores[:,1] /100) * totalbundles[0]
        cumcores9_sumfibrenumbers_100 = np.cumsum(cores9_sumfibrenumbers_100)
        cores9_sumfibrenumbers_90 = cores9 * (percent100umcores[:,1] /100) * totalbundles[1]
        cumcores9_sumfibrenumbers_90 = np.cumsum(cores9_sumfibrenumbers_90)
        cores9_sumfibrenumbers_80 = cores9 * (percent100umcores[:,1] /100) * totalbundles[2]
        cumcores9_sumfibrenumbers_80 = np.cumsum(cores9_sumfibrenumbers_80)
        cores9_sumfibrenumbers_70 = cores9 * (percent100umcores[:,1] /100) * totalbundles[3]
        cumcores9_sumfibrenumbers_70 = np.cumsum(cores9_sumfibrenumbers_70)
        cores9_sumfibrenumbers_60 = cores9 * (percent100umcores[:,1] /100) * totalbundles[4]
        cumcores9_sumfibrenumbers_60 = np.cumsum(cores9_sumfibrenumbers_60)
        cores9_sumfibrenumbers_50 = cores9 * (percent100umcores[:,1] /100) * totalbundles[5]
        cumcores9_sumfibrenumbers_50 = np.cumsum(cores9_sumfibrenumbers_50)
        cores9_sumfibrenumbers_20 = cores9 * (percent100umcores[:,1] /100) * totalbundles[6]
        cumcores9_sumfibrenumbers_20 = np.cumsum(cores9_sumfibrenumbers_20)
        percentintervals=np.array([0,5,10,15,20,25,30,35,40,45,50,55,60,65,70,75,80,85,90,95,100])
        percentintervals_100um_50 = np.interp(percentintervals,cum_pc100umcores, cumcores9_sumfibrenumbers_50)#print the cumulative number of cores at 90% to work out total angular area. Note had to reverse x and y
        percentintervals_100um_100 = np.interp(percentintervals,cum_pc100umcores, cumcores9_sumfibrenumbers_100)
        # print "Number of 105um cores in 50 hexas when", percentintervals[18],"% of galaxies imaged to 2Re is",percentintervals_100um_50[18],"which is a total fibre area of", percentintervals_100um_50[18] * pi * (0.8**2.), "arcsec^2"
        # print "Number of 105um cores in 100 hexas when", percentintervals[18],"% of galaxies imaged to 2Re is",percentintervals_100um_100[18],"which is a total fibre area of", percentintervals_100um_100[18] * pi * (0.8**2.), "arcsec^2"


        cores13_sumfibrenumbers_100 = cores13 * (percent75umcores[:,1] /100) * totalbundles[0]
        cumcores13_sumfibrenumbers_100 = np.cumsum(cores13_sumfibrenumbers_100)
        cores13_sumfibrenumbers_90 = cores13 * (percent75umcores[:,1] /100) * totalbundles[1]
        cumcores13_sumfibrenumbers_90 = np.cumsum(cores13_sumfibrenumbers_90)
        cores13_sumfibrenumbers_80 = cores13 * (percent75umcores[:,1] /100) * totalbundles[2]
        cumcores13_sumfibrenumbers_80 = np.cumsum(cores13_sumfibrenumbers_80)
        cores13_sumfibrenumbers_70 = cores13 * (percent75umcores[:,1] /100) * totalbundles[3]
        cumcores13_sumfibrenumbers_70 = np.cumsum(cores13_sumfibrenumbers_70)
        cores13_sumfibrenumbers_60 = cores13 * (percent75umcores[:,1] /100) * totalbundles[4]
        cumcores13_sumfibrenumbers_60 = np.cumsum(cores13_sumfibrenumbers_60)
        cores13_sumfibrenumbers_50 = cores13 * (percent75umcores[:,1] /100) * totalbundles[5]
        cumcores13_sumfibrenumbers_50 = np.cumsum(cores13_sumfibrenumbers_50)
        cores13_sumfibrenumbers_20 = cores13 * (percent75umcores[:,1] /100) * totalbundles[5]
        cumcores13_sumfibrenumbers_20 = np.cumsum(cores13_sumfibrenumbers_20)
        percentintervals=np.array([0,5,10,15,20,25,30,35,40,45,50,55,60,65,70,75,80,85,90,95,100])
        percentintervals_75um_50 = np.interp(percentintervals,cum_pc75umcores, cumcores13_sumfibrenumbers_50)#print the cumulative number of cores at 90% to work out total angular area. Note had to reverse x and y
        percentintervals_75um_100 = np.interp(percentintervals,cum_pc75umcores, cumcores13_sumfibrenumbers_100)
        # print "Number of 75um cores in 50 hexas when", percentintervals[18],"% of galaxies imaged to 2Re is",percentintervals_75um_50[18],"which is a total fibre area of", percentintervals_75um_50[18] * pi * ((0.75*0.8)**2.), "arcsec^2"
        # print "Number of 75um cores in 100 hexas when", percentintervals[18],"% of galaxies imaged to 2Re is",percentintervals_75um_100[18],"which is a total fibre area of", percentintervals_75um_100[18] * pi * ((0.75*0.8)**2.), "arcsec^2"

        #plt.plot(cumcores9_sumfibrenumbers_100,cum_pc100umcores, '-o', color='green',label='100')
        #plt.plot(cumcores9_sumfibrenumbers_90,cum_pc100umcores, '-o', color='cyan',label='90')
        #plt.plot(cumcores9_sumfibrenumbers_80,cum_pc100umcores, '-o', color='blue',label='80')
        #plt.plot(cumcores9_sumfibrenumbers_70,cum_pc100umcores, '-o', color='red',label='70')
        #plt.plot(cumcores9_sumfibrenumbers_60,cum_pc100umcores, '-o', color='magenta',label='60')
        ax.plot(cumcores9_sumfibrenumbers_50,cum_pc100umcores, '-o', color='orange',label='50')
        ax.plot(cumcores9_sumfibrenumbers_20,cum_pc100umcores, '-o', color='cyan',label='20')
        ax.set_xlim(0,5000)
        ax.set_ylim(0,100)
        ax.set_xlabel('Cumulative number of\n 105 micron fibres')
        ax.set_ylabel('Cumulative % galaxies\n imaged to 2Re')
        ax.text(3500, 60, "Num.\n bundles", color = 'black')
        #ax.text(7000, 50, "100", color = 'green')
        #ax.text(7000, 40, "90", color = 'cyan')
        #ax.text(7000, 30, "80", color = 'blue')
        #ax.text(7000, 20, "70", color = 'red')
        #ax.text(7000, 10, "60", color = 'magenta')
        ax.text(4000, 50, "50", color = 'orange')
        ax.text(4000, 40, "20", color = 'cyan')

        return fig, ax

    def make_Julias_plot(self, fig, axs):
        """ 
        Make an indetical plot to Julia's Hector sim code
        """
        if axs.shape != (3, 6):
            raise ValueError(f"Must pass axes of shape (3, 6)- these have shape {axs.shape}")

        self.plot_selected_sample_MZ_colour_Re(fig, axs[0, 0])
        self.plot_selected_sample_MZ(fig, axs[0, 1])
        self.plot_mass_hist(fig, axs[0, 2])
        self.plot_Re_hist(fig, axs[0, 3])
        self.plot_IFU_needed_for_2Re_hist(fig, axs[0, 4])
        self.plot_IFU_needed_for_2Re_kde(fig, axs[0, 5])

        self.plot_parent_sample(fig, axs[1, 0])
        self.plot_z_hist(fig, axs[1, 1])
        self.plot_ellipticity_hist(fig, axs[1, 2])
        self.plot_percentage_galaxy_sample_imaged_2Re(fig, axs[1, 3])
        self.cumulative_fraction_sample_imaged_to_2Re(fig, axs[1, 4])
        self.plot_SB_hist(fig, axs[1, 5])

        self.plot_SB_Re(fig, axs[2, 0])
        self.plot_gmi_hist(fig, axs[2, 1])
        self.plot_Mstar_z_gi_colour(fig, axs[2, 2])
        self.plot_z_Re(fig, axs[2, 3])
        self.plot_cumulative_fraction_imaged_vs_bundle_diameter(fig, axs[2, 4])
        self.cumulative_percentage_galaxies_imaged_2Re(fig, axs[2, 5])

        return fig, axs

if __name__ == '__main__':

    # get rid of the pandas false positive warning
    pd.options.mode.chained_assignment = None
    # Use the local style file I made
    plt.style.use('./HectorSim.mplstyle')

    import argparse
    parser = argparse.ArgumentParser(description="Simulate Hector Observations using a variety of input catalogues")
    parser.add_argument("BoundaryType", help="Must be Taipan or SAMI (not case sensitive)")
    parser.add_argument("zlimit", type=float, help="Maximum redshift")
    parser.add_argument("MstarMin", type=float, help="Minimum Mstar")
    parser.add_argument("MstarMax", type=float, help="Maximum Mstar")
    parser.add_argument("SparseFunction", type=int, help="Sparse selection function. Must be 1 or 2 (FIXME)")
    parser.add_argument("SourceCat", type=str, help="Short name for input catalogue")
    parser.add_argument("MSparseCut1", type=float, help="Select flat in M* above this mass")
    parser.add_argument("minRe", type=float, help="Minimum Re of galaxies to select (in kpc? arcsec?)")
    parser.add_argument("--quiet", "-q", action="store_true",
        help="Suppress printing to the terminal")
    parser.add_argument("--Dec_Min", type=float, default=np.nan, help="Minimum Dec of the observations. Useful for selecting a strip of targets")
    parser.add_argument("--Dec_Max", type=float, default=np.nan, help="Maximum Dec of the observations. Useful for selecting a strip of targets")



    args = parser.parse_args()


    config = _load_config(args.SourceCat)
    full_catalogue_table = _load_table(config, quiet=args.quiet)
    HectorSim_args = vars(args)


    full_catalogue_table['PRI_SAMI'] = 8
    full_catalogue_table['remaining_observations'] = 1
    low_mass_galaxies_for_repeats = (full_catalogue_table['Mstar'] < 9) & (full_catalogue_table['Mstar'] > 0)
    full_catalogue_table.loc[low_mass_galaxies_for_repeats, 'remaining_observations'] = 3


    HectorSim_args['entire_table'] = full_catalogue_table

    HS = HectorSim(**HectorSim_args)

    fig, axs = plt.subplots(nrows=3, ncols=6, figsize=(18.87, 5.94))
    HS.make_Julias_plot(fig, axs)
    fig.tight_layout()
    fig.subplots_adjust(hspace=0.4, wspace=0.28)

    fig_skyplot, ax_skyplot = plt.subplots(figsize=(18.87, 5.94))
    ax_skyplot.scatter(HS.entire_table.RA, HS.entire_table.DEC, c='k', s=1, alpha=0.5, label='Entire Input Catalogue')
    ax_skyplot.scatter(HS.possible_to_observe.RA, HS.possible_to_observe.DEC, c='red', s=1, alpha=0.5, label='Possible to observe (e.g. big/bright enough)')
    ax_skyplot.scatter(HS.selection_function.RA, HS.selection_function.DEC, c='blue', s=1, alpha=0.5, label='Above selection function')
    ax_skyplot.scatter(HS.selection_function_sparsely_sampled.RA, HS.selection_function_sparsely_sampled.DEC, c='magenta', s=2, label='Final catalogue (sparse sampling)')
    plt.show()
    #plt.savefig('53pc_below_0p6.png')
    #P.save_dataframe_as_FITS_table(HS.selection_function_sparsely_sampled, '/Users/samvaughan/Science/Hector/Tiling/TestingInputCatalogues/Tables/test_GAMA_Hector_Input_catalogue_53pc_below_0p6.fits')


    ## Work out the final source densities
    N_galaxy_targets = len(HS.selection_function_sparsely_sampled)
    N_hexabundles = 19
    final_source_density = N_galaxy_targets / HS.total_area # Galaxies per square degree
    source_density_per_Hector_field = final_source_density * np.pi * 1 * 1

    print(f"\n{len(HS.selection_function)} galaxies satisfy the selection function")
    print(f"Final Catalogue contains {N_galaxy_targets} galaxies")
    print(f"\nSource Density above selection function (blue points): {len(HS.selection_function)/HS.total_area * np.pi:.1f} galaxies per 2DF field")
    print(f"Source Density in final catalogue (magenta points): {source_density_per_Hector_field:.1f} galaxies per 2DF field")
    
    ## Print the fraction of galaxies below z=0.06
    fraction_below_0p6 = (HS.selection_function_sparsely_sampled.z < 0.06).sum()/len(HS.selection_function_sparsely_sampled)
    print(f"Fraction of galaixes below z=0.06: {fraction_below_0p6:.2f}")
    


    # fig, ax = plt.subplots()
    # HS.plot_parent_sample(fig, ax)



