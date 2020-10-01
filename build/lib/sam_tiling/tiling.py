import numpy as np 
import scipy.spatial.distance as dist 
import itertools
from collections import Counter
import pandas as pd
import matplotlib.pyplot as plt 
from tqdm import tqdm

def get_best_tile_centre(targets_df, n_xx_yy=100):
    """
    Given a set of x, y coordinates, find the densest region- the best place to put a tile. We do this by laying down a grid of points then finding the closest grid point to each target. Return the target dataframe, the RA and DEC of the centre.
    Inputs:
        targets_df (dataframe): A dataframe of target galaxies. Must have columns 'RA', 'DEC' and 'TILED'
        n_xx_yy (int, optional): Number of grid points along each direction. The size of the grid is n_xx_yy**2, and needs to fit in memory in one go! So don't make this too large...
    """

    # Only pick targets which haven't been tiled already
    mask = targets_df['TILED'] == False
    RA = targets_df.loc[mask, 'RA']
    Dec = targets_df.loc[mask, 'DEC']

    xx = np.linspace(RA.min(), RA.max(), n_xx_yy)
    yy = np.linspace(Dec.min(), Dec.max(), n_xx_yy)

    # This is a bit sad. array(list(generator)).
    grid_coords = np.array(list(itertools.product(xx, yy)))

    nearest_points = Counter(find_nearest(RA.values, Dec.values, grid_coords))
    densest_point = nearest_points.most_common(1)

    #TO DO: some checks here, in case there's no densest point or something
    index = int(densest_point[0][0])

    return targets_df, grid_coords[index, 0], grid_coords[index, 1]


#@jit(nopython=True, parallel=True)
def find_nearest(x, y, grid):

    """
    Find the nearest grid point to each target (at coordinates (x, y))
    Inputs:
        x (array_like): A list of target x coordinates
        y (array_like): A list of target y coordinates
        grid (2D array): A two dimensional array of grid coordinates to test against. Columns are x (grid[:, 0]) and y (grid[:, 1])
    """

    if len(x) != len(y):
        raise ValueError(f"x and y must be the same length: Currently {len(x)} and {len(y)}")
    if grid.ndim != 2:
        raise ValueError("grid must have two dimensions")

    nearest_points = np.zeros_like(x)
    for i, (target_x, target_y) in enumerate(zip(x, y)):
        nearest_points[i] = np.argmin(np.sqrt((grid[:, 0] - target_x)**2 + (grid[:, 1] - target_y) **2))

    return nearest_points

def check_if_in_fov(df, xcen, ycen, radius):

    """
    Return a binary mask if points are within a circular field of view of radius R
    Inputs: 
        df (dataframe): A dataframe of targets. Must have columns 'RA' and 'DEC'
        xcen (float): x (or RA) coordinate of centre
        ycen (float): y (or DEC) coordinate of centre
        radius (float): radius of circle in degrees
    """

    x = df.RA
    y = df.DEC

    if radius < 0:
        raise ValueError("Radius must be positive")

    return np.sqrt( (x - xcen)**2 + (y - ycen)**2) < radius


def find_clashes(df1, df2, proximity):

    """
    Given some proximity, find objects in two dataframes which will clash 
    Inputs: 
        df1 (dataframe): Dataframe of targets. Must have columns 'RA' and 'DEC'
        df2 (dataframe): Dataframe of targets. Must have columns 'RA' and 'DEC'
        proximity (float): Smallest distance which two objects can come before clashing. Measured in arcseconds

    """

    if proximity<0:
        raise ValueError("Proximity must be positive")

    # Make sure that both the input coordinate lists are 2D
    XA = np.atleast_2d(df1.loc[:, ['RA', 'DEC']].values)
    XB = np.atleast_2d(df2.loc[:, ['RA', 'DEC']].values)

    distance_matrix = dist.cdist(XA, XB)
    clashes = (distance_matrix>0.0) & (distance_matrix < proximity/3600.0)

    return clashes


def noclash(df1, df2, proximity):

    """
    Take two dataframes, find those which are within some proximity of each other and then return the first dataframe as is, the SECOND dataframe without the clashing elements, and the number of elements which clash. Useful for selecting guide stars which don't clash with targets in a tile. 

    Inputs: 
        df1 (dataframe): Dataframe of targets. Must have columns 'RA' and 'DEC'
        df2 (dataframe): Dataframe of targets. Must have columns 'RA' and 'DEC'
        proximity (float): Smallest distance which two objects can come before clashing. Measured in arcseconds
    """

    clashes = find_clashes(df1, df2, proximity)
    ncollide = np.sum(clashes)

    # If a target has a clash with another, there'll be a true in its clashes row. Check for this and only pick things with no clash
    good_indices_1 = clashes.sum(axis=0)==0
    good_indices_2 = clashes.sum(axis=1)==0

    
    if ncollide == 0 :
        return df1, df2, ncollide
    else:
        return df1, df2.iloc[good_indices_1], ncollide


def select_stars_for_tile(star_df, tile_df, proximity, Nsel, star_type):
    """
    Given a tile of targets, select Nsel worth of stars by selecting stars which don't clash with any of our targets. The returned stars are sorted by either their priority (for standard stars) or their R-band magnitude (for guide stars)

    Inputs:
        star_df (dataframe): a dataframe of stars (guide or standard). Must have columns "RA", "DEC" and either "priority" or "R_MAG_AUTO" (see below)
        tile_df (dataframe): a dataframe containing Nsel targets, created from using 'unpick'. Must have columns "RA" amd "DEC"
        proximity (float): distance between two adjacent bundles, in arcseconds. 
        Nsel (int): Return this many stars for each tile. 
        star_type (string): One of either "standards" or "guides". Selects whether we sort the star dataframe by "priority" (for standards) or "R_MAG_AUTO" (for guides)
    """     
    if star_type == 'standards':
        star_df = star_df.sort_values(by='priority', ascending=False)
    elif star_type == 'guides':
        star_df = star_df.sort_values(by='R_MAG_AUTO', ascending=True)
    else:
        raise NameError(f"star_type must be either 'standards' or 'guides'; currently {star_type}")

    tile_df, non_clashing_stars, _ = noclash(tile_df, star_df, proximity)

    if Nsel > len(non_clashing_stars):
        Nsel = len(non_clashing_stars)

    return non_clashing_stars.iloc[:Nsel]




def unpick(df, proximity, Nsel):
    """
    To match Aaron's 'unpick' function. Not sure why it's called that?
    """

    tile_df = pd.DataFrame(columns=df.columns, dtype=float)

    # Add the first target, which we don't need to check for clashes
    df = df.sort_values(by='PRI_SAMI', ascending=False)
    new_target = df.loc[df['TILED']==False].iloc[0].to_frame().T
    tile_df = tile_df.append(new_target)
    df.loc[df.index==new_target.index[0], 'TILED']=True
    #df.loc[df.index[0], 'TILED']=True

    N_remaining_targets = df.loc[df['TILED']==False].shape[0]

    #df, _, ncollide = noclash(df, df, proximity)
    while (len(tile_df) < Nsel) & (len(tile_df) < N_remaining_targets) & (N_remaining_targets>0):
        
        # Now sort by PRI_SAMI and pick the top element
        # Append this to our 'tile' dataframe, tile_df
        # First check if it clashes with any of the exisiting tile members
        # If it does, we'll shuffle the dataframe and try again. 
        # TODO: check this never gets into an infinite loop

        clashes = np.array([True])
        counter = 0 

        while (np.sum(clashes) != 0):


            df = df.sort_values(by='PRI_SAMI', ascending=False)
            
            # the .loc selects only things which haven't been tiled and then the iloc selects the row at the top of the list
            # This .T is important!
            new_target = df.loc[df['TILED']==False].iloc[counter].to_frame().T.copy()
            clashes = find_clashes(tile_df, new_target, proximity)

            counter +=1
            #Subtract 1 here- python indexing
            if counter >= N_remaining_targets -1:
                # Sometimes we can't keep tiling
                return df, tile_df

        # Read this 'else' as 'NoBreak', i.e. we exited the while loop normally:      
        else:
            tile_df = tile_df.append(new_target)

            # weird bug where sometimes we'd add the same target to a tile twice. This catches it
            if len(np.unique(list(tile_df.index))) != len(tile_df.index):
                import ipdb; ipdb.set_trace()

            # Flip the 'tiled' flag of this target to true
            df.loc[df.index==new_target.index[0], 'TILED']=True
            N_remaining_targets = df.loc[df['TILED']==False].shape[0]
            # Just check our tile is definitely observable (no clashes)
            if np.sum(find_clashes(tile_df, tile_df, proximity)) != 0:
                import ipdb; ipdb.set_trace()

    return df, tile_df


def plot_tile(tile_df, guide_df, standards_df, catalogue_df, tile_RA, tile_Dec, tile_radius, fig=None, ax=None):

    if fig is None or ax is None:
        fig, ax = plt.subplots()

    ax.scatter(catalogue_df.RA, catalogue_df.DEC, marker=',', s=1, c='0.5')
    ax.scatter(standards_df.RA, standards_df.DEC, marker='o', c='g', s=20, label='Standards')
    ax.scatter(guide_df.RA, guide_df.DEC, marker='s', c='b', s=20, label='Guides')
    ax.scatter(tile_df.RA, tile_df.DEC, marker='x', c='r', s=40, label='Targets')

    ax.set_aspect('equal')
    ax.set_xlim(tile_RA - 1.5*tile_radius, tile_RA + 1.5*tile_radius)
    ax.set_ylim(tile_Dec - 1.5*tile_radius, tile_Dec + 1.5*tile_radius)
    
    circle=plt.Circle((tile_RA, tile_Dec), tile_radius, facecolor='None', edgecolor='k')
    ax.add_artist(circle)

    return fig, ax




if __name__ =='__main__':

    import pandas_tools as P 

    
    df_targets = P.load_FITS_table_in_pandas('../../MAXI/Data/fits_copies_for_topcat/Hector_Clusters_TARGET_catalogue_SPV_MattOwers_cluster_2.fits')
    df_guide_stars = P.load_FITS_table_in_pandas('../../MAXI/Data/fits_copies_for_topcat/Hector_Clusters_GUIDE_STAR_catalogue_SPV_cluster_2.fits')
    df_standard_stars = P.load_FITS_table_in_pandas('../../MAXI/Data/fits_copies_for_topcat/Hector_Clusters_STANDARD_STAR_catalogue_SPV_cluster_2.fits')

    # Shuffle our input target array
    # Since we go through targets in order, this can speed things up by avoiding checking lots of things close to one another in order
    df_targets = df_targets.sample(frac=1)
    df_guide_stars = df_guide_stars.sample(frac=1)
    df_standard_stars = df_standard_stars.sample(frac=1)


    df_targets['TILED'] = False
    df_targets['Tile_number'] = -999
    RA = df_targets['RA']
    Dec = df_targets['DEC']
    Hector_FOV_radius = 1.0
    proximity = 300
    Nsel = 19

    N_tiles = 300


    best_tile_RAs = np.ones(N_tiles)
    best_tile_Decs = np.ones(N_tiles)

    #fig, ax = plt.subplots()
    #plt.scatter(df_targets.RA, df_targets.DEC, c='k', s=1)
    for i in tqdm(range(N_tiles)):

        df_targets, tile_RA, tile_Dec = get_best_tile_centre(df_targets)

        best_tile_RAs[i] = tile_RA
        best_tile_Decs[i] = tile_Dec

        
        # Only select targets, guides and standards within the FoV
        # check_if_in_FOV returns a boolean mask
        inner_targets = df_targets.loc[check_if_in_fov(df_targets, tile_RA, tile_Dec, Hector_FOV_radius), :]
        inner_guides = df_guide_stars.loc[check_if_in_fov(df_guide_stars, tile_RA, tile_Dec, Hector_FOV_radius), :]
        inner_standards = df_standard_stars.loc[check_if_in_fov(df_standard_stars, tile_RA, tile_Dec, Hector_FOV_radius), :]

        # Select which targets to keep
        inner_targets, tile_members = unpick(inner_targets, proximity, Nsel)

        # Now add in the guide stars and standard stars
        # Make sure none clash with any of the targets in that tile
        guide_stars_for_tile = select_stars_for_tile(inner_guides, tile_members, proximity=proximity, Nsel=100, star_type='guides')
        standard_stars_for_tile = select_stars_for_tile(inner_standards, tile_members, proximity=proximity, Nsel=100, star_type='standards')

        # Change the TILED flag for targets we've added to a tile
        df_targets.loc[tile_members.index, 'TILED'] = True
        # And include the tile number for each target
        df_targets.loc[tile_members.index, 'Tile_number'] = i


        #plt.scatter(tile_members.RA, tile_members.DEC)
        #circle=plt.Circle((tile_RA, tile_Dec), Hector_FOV_radius, facecolor='None', edgecolor='k')
        #ax.add_artist(circle)

        fig, ax = plot_tile(tile_members, guide_stars_for_tile, standard_stars_for_tile, df_targets, tile_RA, tile_Dec, Hector_FOV_radius, fig=None, ax=None)
        fig.savefig(f'outputs/tile_{i}.png')
        plt.close('all')

        if df_targets['TILED'].sum() == len(df_targets):
            print("\n\n")
            print("Done!")
            break


    used_tiles = df_targets['Tile_number'].max()
    efficiency = len(df_targets)/ Nsel / used_tiles
    print(f"Efficiency: {100*efficiency:.3f}%")
    #Remove fillers that collide with high priority targets
    # Remove guides which are too close to the centre of the tile 
    #Find targets near besttile