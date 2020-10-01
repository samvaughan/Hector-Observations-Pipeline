import numpy as np
import scipy.spatial.distance as dist
import itertools
import matplotlib.pyplot as plt
import os


def get_best_tile_centre_greedy(targets_df, FOV_radius, n_xx_yy=100):
    """
    Given a set of x, y coordinates, find the position which would cover the most targets if we placed a field of view there.
    Inputs:
        targets_df (dataframe): A dataframe of target galaxies which have not yet been tiled. Must have columns 'RA' and 'DEC'
        n_xx_yy (int, optional): Number of grid points along each direction. The size of the grid is n_xx_yy**2, and needs to fit in memory in one go! So don't make this too large...
    """

    RA = targets_df.loc[:, 'RA']
    Dec = targets_df.loc[:, 'DEC']

    # Make the grid
    grid_coords = _get_grid(RA, Dec, n_xx_yy)

    n_targets_in_FOV = _calc_clashes(grid_coords, np.column_stack((RA, Dec)), proximity=FOV_radius * 3600.0).sum(axis=1)

    # This will return the first position in the grid where n_targets_in_FOV is equal to its max value
    index = np.argmax(n_targets_in_FOV)

    return grid_coords[index, 0], grid_coords[index, 1]


def get_best_tile_centre_dengreedy(master_df, targets_df, FOV_radius, n_xx_yy=100):
    """
    Given a set of x, y coordinates, find the position to place the tile according to Aaron's Dengreedy algorithm

    Inputs:
        master_df (dataframe): A dataframe of *all* target galaxies, irrespective of whether they've been tiled alread. Must have columns 'RA', 'DEC'
        targets_df (dataframe): A dataframe of target galaxies which have not yet been tiled. Must have columns 'RA', 'DEC'
        n_xx_yy (int, optional): Number of grid points along each direction. The size of the grid is n_xx_yy**2, and needs to fit in memory in one go! So don't make this too large...
    """

    RA_all = master_df.loc[:, 'RA']
    Dec_all = master_df.loc[:, 'DEC']

    RA_untiled = targets_df.loc[:, 'RA']
    Dec_untiled = targets_df.loc[:, 'DEC']

    grid_coords = _get_grid(RA_all, Dec_all, n_xx_yy)

    n_targets_in_FOV_all = _calc_clashes(grid_coords, np.column_stack((RA_all, Dec_all)), proximity=FOV_radius * 3600.0).sum(axis=1)
    n_targets_in_FOV_untiled = _calc_clashes(grid_coords, np.column_stack((RA_untiled, Dec_untiled)), proximity=FOV_radius * 3600.0).sum(axis=1)

    ratio_of_targets = n_targets_in_FOV_untiled / n_targets_in_FOV_all
    ratio_of_targets[np.isnan(ratio_of_targets)] = 0.0

    index = np.argmax(ratio_of_targets)

    return grid_coords[index, 0], grid_coords[index, 1]

def _get_grid(RA, Dec, n_xx_yy):
    """
    Make a grid of points which encloses a set of given RAs and Decs
    Inputs: 
        RA (array): A list of RA values of targets
        Dec (array): A list of Dec values of targets
        n_xx_yy (int): Number of grid points along **one** axis. Final grid has n_xx_yy^2 points!
    """
    xx = np.linspace(RA.min(), RA.max(), n_xx_yy)
    yy = np.linspace(Dec.min(), Dec.max(), n_xx_yy)

    # This is a bit sad. array(list(generator)).
    grid_coords = np.array(list(itertools.product(xx, yy)))

    return grid_coords


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

    nearest_points = np.zeros(len(x))
    for i, (target_x, target_y) in enumerate(zip(x, y)):
        nearest_points[i] = np.argmin(np.sqrt((grid[:, 0] - target_x)**2 + (grid[:, 1] - target_y)**2))
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

    return np.sqrt((x - xcen)**2 + (y - ycen)**2) < radius


def find_clashes(df1, df2, proximity):
    """
    Given some proximity, find objects in two dataframes which will clash
    Inputs:
        df1 (dataframe): Dataframe of targets. Must have columns 'RA' and 'DEC'
        df2 (dataframe): Dataframe of targets. Must have columns 'RA' and 'DEC'
        proximity (float): Smallest distance which two objects can come before clashing. Measured in arcseconds

    """

    if proximity < 0:
        raise ValueError("Proximity must be positive")

    # Make sure that both the input coordinate lists are 2D
    XA = np.atleast_2d(df1.loc[:, ['RA', 'DEC']].values)
    XB = np.atleast_2d(df2.loc[:, ['RA', 'DEC']].values)

    clashes = _calc_clashes(XA, XB, proximity)

    return clashes


def find_clashes_from_df_and_array(df1, RA, DEC, proximity):
    """
    Find clashes between one dataframe and one position only (an RA and a DEC)
    """

    XA = np.atleast_2d(df1.loc[:, ['RA', 'DEC']].values)
    XB = np.atleast_2d(np.column_stack((RA, DEC)))

    clashes = _calc_clashes(XA, XB, proximity)

    return clashes


def _calc_clashes(XA, XB, proximity):
    """
    Given two lists of coordinates, return a boolean mask highlighting only those which clash within some minimum proximity. Note that we ignore self clashes by ignoring clashes with a distance = 0.0
    """
    distance_matrix = dist.cdist(XA, XB)
    clashes = (distance_matrix > 0.0) & (distance_matrix < proximity / 3600.0)

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
    good_indices_1 = clashes.sum(axis=0) == 0

    if ncollide == 0:
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


def select_targets(df, proximity, Nsel):
    """
    Given a dataframe of targets, select Nsel galaxies to observe. These can't be nearer to each other than 'proximity'. Return a dataframe of just the new tile we've made.
    Inputs:
        df (dataframe): A dataframe of targets. Must have columns "RA", "DEC", "PRI_SAMI" (i.e. priority), "ALREADY_TILED" (i.e. tiled before this current iteration).
        proximity (float): Smallest distance between two targets (in arcseconds)
        Nsel (int): Number of targets to select in each tile
    """

    # Select things which haven't been already tiled or selected as a target in this iteration
    target_mask = (df['ALREADY_TILED'] == False)
    df = df.loc[target_mask]

    # Get the priority of each target
    priorities = df['PRI_SAMI'].values

    # Find which of these targets clash with each other
    clashes = find_clashes(df, df, proximity)
    n_clashes = clashes.sum(axis=0)

    # Sort by SAMI priority and then number of clashes
    # This is the correct order for these arrays in the call to lexsort!
    sorted_by_pri_then_nclashes = np.lexsort((n_clashes, df.PRI_SAMI.values))

    targets = []
    isel_values = []
    untiled_galaxies = np.arange(len(df))[sorted_by_pri_then_nclashes]
    priorities = priorities[sorted_by_pri_then_nclashes]

    # Now go through and add the target which clashes with the most things to the tile.
    # Keep doing this until we have selected 'Nsel' things or we run out of targets
    while (len(targets) < Nsel) and (len(untiled_galaxies) > 0):

        # Find any clashes in the targets
        n_clashes = dict(zip(untiled_galaxies, np.sum(clashes, axis=0)[untiled_galaxies]))

        # If we have a clash...
        if sum(n_clashes.values()) > 0:

            # Now find the target which clashes with the most things
            most_clashing_target = max(n_clashes.keys(), key=(lambda k: n_clashes[k]))

            # And find the targets it clashes with
            things_it_clashes_with = np.where(clashes[most_clashing_target])[0]

            # Tile the most clashing target
            targets.append(most_clashing_target)
            # Add its 'isel' value as its priority plus one, since it's a clashing target
            isel_values.append(priorities[most_clashing_target] + 1)

            # ... and remove the things this target clashes with from the list of targets in this tile
            untiled_galaxies = [u for u in untiled_galaxies if u not in things_it_clashes_with]
            untiled_galaxies.remove(most_clashing_target)

        # Else if there aren't any clashes remaining...
        else:
            # Just add everything left
            N_to_append = min(Nsel - len(targets), len(untiled_galaxies))
            unclashing_targets = untiled_galaxies[:N_to_append]

            targets.extend(unclashing_targets)
            isel_values.extend(priorities[unclashing_targets])

            # Remove things we've tiled
            untiled_galaxies = [u for u in untiled_galaxies if u not in targets]

    tile_df = df.iloc[targets]

    # Check if we clash with ourself- this should never happen
    clashes = find_clashes(tile_df, tile_df, proximity)
    if np.sum(clashes) != 0:
        import ipdb; ipdb.set_trace()

    return df.iloc[targets], isel_values


def make_best_tile(df_targets, df_guide_stars, df_standard_stars, tiling_parameters, tiling_type):
    """
    Put all the above functions togther and make a tile. Note that this function __doesn't__ update any tiling flags in the overall database. This should be done afterwards, so that we can integrate things with the Hector configuration code- the 19 best targets we pick might not actually be tile-able, so we don't want to mark things as tiled if the config code needs to select backups.
    Inputs:
        df_targets (dataframe): A dataframe of galaxy targets. Must include columns 'RA', 'DEC', "PRI_SAMI" and "TILED"
        df_guide_stars (dataframe): A dataframe of galaxy targets. Must include columns 'RA', 'DEC' and "R_MAG_AUTO"
        df_standard_stars (dataframe): A dataframe of standard stars. Must include columns 'RA', 'DEC' and 'priority'
        tiling_parameters (dict): A dictionary containing the various parameters to do with the tiling. So far, necessary keys are 'Hector_FOV_radius', 'proximity', 'Nsel', 'Nsel_guides' and 'Nsel_standards'
    Returns:
        A tuple of:
            * The original df_targets dataframe
            * A dataframe containing the targets for this tile
            * A dataframe containing the guides for this tile
            * A dataframe containing the standard stars for this tile
            * The tile RA
            * The tile Dec
    """

    Hector_FOV_radius = tiling_parameters['Hector_FOV_radius']
    proximity = tiling_parameters['proximity']
    Nsel = tiling_parameters['Nsel']
    Nsel_guides = tiling_parameters['Nsel_guides']
    Nsel_standards = tiling_parameters['Nsel_standards']

    untiled = df_targets['ALREADY_TILED'] == False
    n_xx_yy = max(min(100, int(untiled.sum() / 10)), 10)

    if tiling_type == 'greedy':

        tile_RA, tile_Dec = get_best_tile_centre_greedy(df_targets.loc[untiled], FOV_radius=Hector_FOV_radius, n_xx_yy=n_xx_yy)
    elif tiling_type == 'dengreedy':
        tile_RA, tile_Dec = get_best_tile_centre_dengreedy(df_targets, df_targets.loc[untiled], FOV_radius=Hector_FOV_radius, n_xx_yy=n_xx_yy)
    # Only select targets, guides and standards within the FoV
    # check_if_in_FOV returns a boolean mask
    inner_targets = df_targets.loc[check_if_in_fov(df_targets, tile_RA, tile_Dec, Hector_FOV_radius), :]
    inner_guides = df_guide_stars.loc[check_if_in_fov(df_guide_stars, tile_RA, tile_Dec, Hector_FOV_radius), :]
    inner_standards = df_standard_stars.loc[check_if_in_fov(df_standard_stars, tile_RA, tile_Dec, Hector_FOV_radius), :]

    # Select which targets to keep
    tile_members, isel_values = select_targets(inner_targets, proximity, Nsel)

    # Add the 'isel' values to the main dataframe
    df_targets.loc[tile_members.index, 'isel'] = isel_values
    tile_members.loc[tile_members.index, 'isel'] = isel_values

    # Now add in the guide stars and standard stars
    # Make sure none clash with any of the targets in that tile
    guide_stars_for_tile = select_stars_for_tile(inner_guides, tile_members, proximity=proximity, Nsel=Nsel_guides, star_type='guides')
    standard_stars_for_tile = select_stars_for_tile(inner_standards, tile_members, proximity=proximity, Nsel=Nsel_standards, star_type='standards')

    standard_stars_for_tile['isel'] = standard_stars_for_tile['priority'] + 10

    return df_targets, tile_members, guide_stars_for_tile, standard_stars_for_tile, tile_RA, tile_Dec


def save_tile_outputs(outfolder, df_targets, tile_df, guide_stars_for_tile, standard_stars_for_tile, tile_RA, tile_Dec, tiling_parameters, tile_number, plot=True):
    """
    Save the outputs from a single tile. These are:
        * A text file called tile_{i}.fld, which contains things to be observed the Hector science bundles. This has columns CATID, RA, DEC, mag, type (where type is 1 for a galaxy target and 0 for a standard star) and isel (which is a bit like the priority they should be targeted in). The targets are sorted by priority!
        * A text file called guide_tile_{i}.fld, which contains guide stars (observed with the Hector guide bundles). This has columns RA, DEC, mag
        * A plot of the field with the Nsel best targets selected by this code. Note that this may not resemble the final tile selected by the configuration code!
    These outputs will be bundled together in a single outfolder" folder, which contains subfolders 'Tiles' and 'Plots'
    Inputs:
        outfolder (str): Location for the output files
        df_targets (dataframe): Dataframe of all targets. This is used for plotting
        tile_df (dataframe): Dataframe of targets for a single tile
        guide_stars_for_tile (dataframe): Dataframe of guide stars for a single tile
        standard_stars_for_tile (dataframe): Dataframe of standard stars for a single tile
        tile_RA (float): RA of tile centre
        tile_DEC (float): DEC of tile centre
        tiling parameters (dict): Dictionary of tiling parameters
        tile_number (int): Number of the tile
        plot (bool, optional): Whether to save a plot of the tile. Default is True
    """

    tile_out_name = f"tile_{tile_number}.fld"
    guide_out_name = f"guide_tile_{tile_number}.fld"

    save_tile_text_file(outfolder, tile_out_name, tile_df, standard_stars_for_tile, tile_RA, tile_Dec)
    save_guide_text_file(outfolder, guide_out_name, guide_stars_for_tile, tile_RA, tile_Dec)

    if plot:
        if not os.path.exists(f'{outfolder}/Plots'):
            os.makedirs(f'{outfolder}/Plots')

        fig, ax = plot_tile(tile_df, guide_stars_for_tile, standard_stars_for_tile, df_targets, tile_RA, tile_Dec, tiling_parameters['Hector_FOV_radius'], tile_number, tiling_parameters['proximity'])
        fig.savefig(f'{outfolder}/Plots/tile_{tile_number}.png')
        plt.close('all')

    return 0


def plot_survey_completeness_and_tile_positions(tile_positions, df_targets, tiling_parameters, fig=None, ax=None, completion_fraction_to_calculate=0.95, verbose=True):
    """
    Make an overall plot of the survey completeness and the positions of each tile.
    Inputs:
        tile_positions (list): A two component list. First element is a list of RA values of the tile centres, second component is a list of Dec values of the tile centres.
        tiling_parameters (dict): A dictionary of tiling parameters. Must have  a 'Hector_FOV_radius' key
    """

    tile_RAs, tile_DECs = tile_positions

    fig, axs = plt.subplots(nrows=1, ncols=2, figsize=(17, 5))
    axs[0].axis('equal')
    
    tiles = df_targets.groupby('Tile_number').groups

    for tile_number in tiles:
        members = df_targets.loc[df_targets.index[tiles[tile_number]]] 
        axs[0].scatter(members.RA, members.DEC) 
        tile_footprint = plt.Circle(xy=(tile_RAs[tile_number], tile_DECs[tile_number]), radius=tiling_parameters['Hector_FOV_radius'], facecolor='None', edgecolor='k')
        axs[0].add_artist(tile_footprint)

    
    completeness, comp_fraction_x, used_tiles_to_get_to_x, min_number_tiles_for_x = calculate_completeness_stats(df_targets, tiling_parameters, completion_fraction_to_calculate=completion_fraction_to_calculate, verbose=verbose)

    # Plot the completeness as a function of tile number
    xx = np.arange(1, len(completeness) + 1)
    axs[1].plot(xx, Nsel_targets_only / len(df_targets) * xx, c='k', linestyle='dashed')
    axs[1].plot(xx, completeness, c='r', zorder=10)
    axs[1].axhline(completion_fraction_to_calculate, c='0.8', linestyle='dashed')
    axs[1].axhline(1, c='0.5')
    axs[1].set_ylim(0, 1.2)
    axs[1].axvline(used_tiles_to_get_to_x, c='0.8', linestyle='dashed')
    axs[1].axvline(min_number_tiles_for_x, c='0.8', linestyle='dashed')
    axs[1].set_xlabel(r'$N_{\rm{tiles}}$')
    axs[1].set_ylabel('Completeness')

    return fig, axs


def _calc_completeness(df_targets):
    """
    Given a tiling dataframe, work out the fractional completeness of the tiling after each tile.
    Inputs:
        df_targets (dataframe): a dataframe with a row for each target. Must have a column 'Tile_number'
    """

    completeness = np.cumsum([np.sum(df_targets['Tile_number'] == i) / len(df_targets) for i in np.unique(df_targets['Tile_number'])])

    return completeness


def calculate_completeness_stats(df_targets, tiling_parameters, completion_fraction_to_calculate=0.95, verbose=True):
    """
    Given a set of tiles, calculate some stats about the efficiency to get to a given completeness fraction
    Inputs:
        df_targets (dataframe): a dataframe with a row for each target. Must have a column 'Tile_number'
        tiling_parameters (dict): a dictionary of parameters used for the tiling. Must have key 'Nsel_targets_only'
        completeness_fraction_to_calculate (float, default=0.95): calculate the efficiency to reach this completeness fraction. This is defined as actual number of tiles used / minimum number of tiles possible) 
        verbose (bool, default=True): print efficiency stats or not.  
    """

    if (completion_fraction_to_calculate > 1) or (completion_fraction_to_calculate < 0):
        raise ValueError(f"completion_fraction_to_calculate must be between 0 and 1: currently {completion_fraction_to_calculate}")

    Nsel_targets_only = tiling_parameters['Nsel_targets_only']

    # Work out the completeness after each tile
    completeness = _calc_completeness(df_targets)

    used_tiles = df_targets['Tile_number'].max() + 1  # Since we start indexing from 0
    minimum_number_of_tiles = np.ceil(len(df_targets) / Nsel_targets_only)
    efficiency = minimum_number_of_tiles / used_tiles

    used_tiles_to_get_to_x = np.where(completeness > completion_fraction_to_calculate)[0][0] + 1
    minimum_number_of_tiles_for_x = np.ceil(completion_fraction_to_calculate * len(df_targets) / Nsel_targets_only)
    efficiency_xpc = minimum_number_of_tiles_for_x / used_tiles_to_get_to_x

    if verbose:
        print(f"Efficiency for completion={completion_fraction_to_calculate}: {100*efficiency_xpc:.3f}%")
        print(f"Efficiency for completion=1: {100*efficiency:.3f}%")

    return completeness, completion_fraction_to_calculate, used_tiles_to_get_to_x, minimum_number_of_tiles_for_x


def plot_tile(tile_df, guide_df, standards_df, catalogue_df, tile_RA, tile_Dec, tile_radius, tile_number, proximity, fig=None, ax=None):
    """
    Make a plot of an individual tile.
    Inputs:
        tile_df (dataframe): Dataframe of a tile to plot
        guide_df (dataframe): Dataframe of guide stars from this tile
        standards_df (dataframe): Dataframe of standard stars from this tile
        catalogue_df (dataframe): Dataframe of the entire field, to plot as background
        tile_RA (float): RA of the centre of this tile
        tile_Dec (float): Dec of the centre of this tile
        tile_radius (float): Radius of the FoV (in degrees)
        tile_number (int): Number of the tile
        proximity (float): Radius of the tile
        fig, ax (optional): An exisiting Figure/Axis object to plot on
    """
    top_target_mask = catalogue_df['Tile_number'] == tile_number

    if fig is None or ax is None:
        fig, ax = plt.subplots(figsize=(14, 10))

    ax.scatter(catalogue_df.RA, catalogue_df.DEC, marker=',', s=1, c='0.5')
    ax.scatter(tile_df.RA, tile_df.DEC, marker='x', c='k', s=30, label='All Targets', zorder=10)
    ax.scatter(catalogue_df.loc[top_target_mask, 'RA'], catalogue_df.loc[top_target_mask, 'DEC'], marker='x', c='r', s=50, label='Top Targets', zorder=10)
    ax.scatter(standards_df.RA, standards_df.DEC, marker='o', c='g', s=20, label='Standards')
    ax.scatter(guide_df.RA, guide_df.DEC, marker='s', c='b', s=20, label='Guides')

    for index, row in catalogue_df.loc[top_target_mask].iterrows():
        circle = plt.Circle(xy=(row.RA, row.DEC), radius=proximity / 3600.0, facecolor='None', edgecolor='k', linestyle='dashed')
        ax.add_artist(circle)

    ax.set_aspect('equal')
    ax.set_xlim(tile_RA - 1.1 * tile_radius, tile_RA + 1.1 * tile_radius)
    ax.set_ylim(tile_Dec - 1.1 * tile_radius, tile_Dec + 1.1 * tile_radius)

    circle = plt.Circle((tile_RA, tile_Dec), tile_radius, facecolor='None', edgecolor='k')
    ax.add_artist(circle)

    ax.legend(bbox_to_anchor=(1, 0.5))

    ax.set_xlabel('Right Ascension')
    ax.set_ylabel('Declination')

    ax.set_title(f'Tile {tile_number}')

    return fig, ax


def save_tile_text_file(outfolder, out_name, tile_df, standard_stars_for_tile, tile_RA, tile_Dec):
    """
    Save a text file of things to be observed with Hector science bundles (i.e. targets and standards). The second line of this text file __must__ be the tile RA and DEC seperated by a space.
    """

    if not os.path.exists(f"{outfolder}/Tiles"):
        os.makedirs(f"{outfolder}/Tiles")

    # Make sure the tile_df is sorted by priority
    tile_df = tile_df.sort_values(by='PRI_SAMI', ascending=False)

    # Rename the column headings so things match
    targets_renamer = dict(CATAID="ID", r_mag='mag')
    tile_df = tile_df.rename(columns=targets_renamer)
    tile_df['type'] = 1

    standards_renamer = dict(CoADD_ID='ID', R_MAG_AUTO='mag')
    standard_stars_for_tile = standard_stars_for_tile.rename(columns=standards_renamer)
    standard_stars_for_tile['type'] = 0

    combined_stars_targets_df = tile_df[['ID', 'RA', 'DEC', 'mag', 'type', 'isel']].append(standard_stars_for_tile[['ID', 'RA', 'DEC', 'mag', 'type', 'isel']])

    # Write a CSV file with the header we want
    with open(f"{outfolder}/Tiles/{out_name}", 'w') as f:
        f.write("# Target and Standard Star file from Sam's tiling code\n")
        f.write(f"# {tile_RA} {tile_Dec}\n")
    combined_stars_targets_df.to_csv(f"{outfolder}/Tiles/{out_name}", sep=' ', mode='a', index=False)

    return 0


def save_guide_text_file(outfolder, out_name, guide_stars_for_tile, tile_RA, tile_Dec):
    """
    Save a text file of things to be observed with Hector guide bundles (i.e. guides). The second line of this text file __must__ be the tile RA and DEC seperated by a space.
    """

    if not os.path.exists(f"{outfolder}/Tiles"):
        os.makedirs(f"{outfolder}/Tiles")

    # Make sure the tile_df is sorted by priority
    guide_stars_for_tile = guide_stars_for_tile.sort_values(by='R_MAG_AUTO')

    guides_renamer = dict(CoADD_ID='ID', R_MAG_AUTO='mag')
    guide_stars_for_tile = guide_stars_for_tile.rename(columns=guides_renamer)
    guide_stars_for_tile['type'] = 2

    # Write a CSV file with the header we want
    with open(f"{outfolder}/Tiles/{out_name}", 'w') as f:
        f.write("# Target and Standard Star file from Sam's tiling code\n")
        f.write(f"# {tile_RA} {tile_Dec}\n")
    guide_stars_for_tile[['ID', 'RA', 'DEC', 'mag', 'type']].to_csv(f"{outfolder}/Tiles/{out_name}", sep=' ', mode='a', index=False)

    return 0

############################################################################################################


if __name__ == '__main__':
    """
    Example Usage
    """
    import pandas_tools as P

    cluster_number = 2

    df_targets = P.load_FITS_table_in_pandas(f'/Users/samvaughan/Science/Hector/Tiling/SAMI_clusters_tests/Data/SAMI_Clusters_TARGET_catalogue_SPV_cluster_{cluster_number}.fits')
    df_guide_stars = P.load_FITS_table_in_pandas(f'/Users/samvaughan/Science/Hector/Tiling/SAMI_clusters_tests/Data/SAMI_Clusters_GUIDE_STAR_catalogue_SPV_cluster_{cluster_number}.fits')
    df_standard_stars = P.load_FITS_table_in_pandas(f'/Users/samvaughan/Science/Hector/Tiling/SAMI_clusters_tests/Data/SAMI_Clusters_STANDARD_STAR_catalogue_SPV_cluster_{cluster_number}.fits')

    df_targets = df_targets.rename(columns=dict(r_Petro='r_mag'))
    df_guide_stars = df_guide_stars.rename(columns=dict(ROWID='CoADD_ID'))
    df_standard_stars = df_standard_stars.rename(columns=dict(ROWID='CoADD_ID'))

    # Add the empty columns which we'll update
    df_targets['ALREADY_TILED'] = False
    df_targets['Tile_number'] = -999
    df_targets['isel'] = -999

    tiling_type = 'greedy'
    Hector_FOV_radius = 1.0
    proximity = 400
    Nsel = 30
    Nsel_targets_only = 19
    Nsel_guides = 100
    Nsel_standards = 100

    tiling_parameters = dict(Hector_FOV_radius=Hector_FOV_radius, proximity=proximity, Nsel=Nsel, Nsel_guides=Nsel_guides, Nsel_standards=Nsel_standards, Nsel_targets_only=Nsel_targets_only)

    N_tiles = 100
    best_tile_RAs = []
    best_tile_Decs = []

    print(f"Total Targets in field: {len(df_targets[df_targets['ALREADY_TILED']==False])}")
    for i in range(N_tiles):

        df_targets, tile_df, guide_stars_for_tile, standard_stars_for_tile, tile_RA, tile_Dec = make_best_tile(df_targets, df_guide_stars, df_standard_stars, tiling_parameters=tiling_parameters, tiling_type=tiling_type)

        top_targets = tile_df.iloc[:min(Nsel_targets_only, len(tile_df))]
        # Change the TILED flag for targets we've added to a tile
        df_targets.loc[top_targets.index, 'ALREADY_TILED'] = True
        # And include the tile number for each target
        df_targets.loc[top_targets.index, 'Tile_number'] = i

        save_tile_outputs('SAMI_cluster_outputs', df_targets, tile_df, guide_stars_for_tile, standard_stars_for_tile, tile_RA, tile_Dec, tiling_parameters, tile_number=i, plot=True)

        best_tile_RAs.append(tile_RA)
        best_tile_Decs.append(tile_Dec)

        print(f"Tile {i}: Tiled targets: {df_targets['ALREADY_TILED'].sum()}. Selected for this tile: {len(top_targets)}. Remaining targets in field: {len(df_targets[df_targets['ALREADY_TILED']==False])}. ")

        if df_targets['ALREADY_TILED'].sum() == len(df_targets):
            print("\n\n")
            print("Done!")
            break

    tile_positions = [best_tile_RAs, best_tile_Decs]

    fig, ax = plot_survey_completeness_and_tile_positions(tile_positions, df_targets, tiling_parameters, fig=None, ax=None, completion_fraction_to_calculate=0.95, verbose=True)
    fig.savefig('SAMI_cluster_outputs/survey_plot.pdf', bbox_inches='tight')
    plt.show()
