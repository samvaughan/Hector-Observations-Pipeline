import numpy as np
import scipy.spatial.distance as dist
import itertools
import matplotlib.pyplot as plt
import pandas as pd
import os
from operator import itemgetter
import warnings
from ..hexabundle_allocation.hector import constants as hector_constants

# Set up the logging
import logging
logger = logging.getLogger(__name__)

def get_best_tile_centre_greedy(targets_df, outer_FOV_radius, inner_FoV_radius, priorities, n_xx_yy=100):
    """
    Given a set of x, y coordinates, find the position which would cover the most targets if we placed a field of view there.
    Inputs:
        targets_df (dataframe): A dataframe of target galaxies which have not yet been tiled. Must have columns 'RA' and 'DEC'
        n_xx_yy (int, optional): Number of grid points along each direction. The size of the grid is n_xx_yy**2, and needs to fit in memory in one go! So don't make this too large...
        Priorities: A priority value for each galaxy. If none is given, assume equal priorties for all targets
    """

    RA = targets_df.loc[:, 'RA']
    Dec = targets_df.loc[:, 'DEC']

    # Make the grid
    grid_coords = _get_grid(RA, Dec, n_xx_yy)

    # These are True/False masks for each galaxy if a tile was placed at each grid coord. They're shapes (n_grid_coords x number of galaxies left to tile)
    galaxies_in_FoV = np.zeros((len(grid_coords), len(targets_df)), dtype=bool)
    for i, coord in enumerate(grid_coords):
        galaxies_in_FoV[i, :] = check_if_in_fov(targets_df, grid_coords[i, 0], grid_coords[i, 1], inner_FoV_radius, outer_FOV_radius)

    #galaxies_in_outer_FoV = _calc_clashes(grid_coords, np.column_stack((RA, Dec)), proximity=outer_FOV_radius * 3600.0)
    #galaxies_in_inner_FoV = _calc_clashes(grid_coords, np.column_stack((RA, Dec)), proximity=inner_FoV_radius * 3600.0)

    n_targets_in_FOV = (galaxies_in_FoV * np.array(priorities)[None, :]).sum(axis=1)# - (galaxies_in_inner_FoV * np.array(priorities)[None, :]).sum(axis=1)

    # Find the positions where the number of targets in the FoV is at its max
    possible_tile_centres = np.where(n_targets_in_FOV == np.max(n_targets_in_FOV))[0]
    targets_in_best_tiles_mask = (galaxies_in_FoV)[possible_tile_centres]

    # If we have more than one possible centre, choose a random centre
    # However, weight this choice by how far each target in the FoV is from the centre of the tile
    # This is to avoid tiles where every target is right at one edge
    if len(possible_tile_centres) > 1:
        index = _decide_upon_best_tile_centre(n_targets_in_FOV, possible_tile_centres, grid_coords, targets_in_best_tiles_mask, RA, Dec)
    else:
        index = np.argmax(n_targets_in_FOV)

    return grid_coords[index, 0], grid_coords[index, 1]


def _decide_upon_best_tile_centre(n_targets_in_FOV, possible_tile_centres, grid_coords, targets_in_best_tiles_mask, RA, Dec):
    """
    If we have a few choices for which tile to pick, prefer to select the one which places targets near the centre. This is to avoid some of the funny tiles I've seen where every galaxy is right at one edge.
    To do this, I'm making a random choice from all possible tile centres but weighting that choice by the sum of all the squared distances from the tile centre. Not sure that this is the best idea (could use a convex hull area, including the tile centre point, maybe? Not sure) but it's better than just making a totally random selection
    """
    # This is a mask showing which galaxies would be selected in each of the "best" tiles
    # It's shaped as (number_of_possible_tile_centres x number_of_galaxies_left)
    
    distances_from_centre = np.zeros(len(possible_tile_centres))
    for i, (centre, targets_mask) in enumerate(zip(possible_tile_centres, targets_in_best_tiles_mask)):
        distances_from_centre[i] = np.sum(np.sqrt((RA[targets_mask] - grid_coords[centre, 0])**2 + (Dec[targets_mask] - grid_coords[centre, 1])**2), axis=0)

    # Catch any values which are exactly 0 and give them a small buffer value
    distances_from_centre[distances_from_centre == 0.0] = 0.1
    weights = (1./distances_from_centre)
    norm_weights = weights / weights.sum()
    index = np.random.choice(np.where(n_targets_in_FOV == np.max(n_targets_in_FOV))[0], p=norm_weights)

    return index


def get_best_tile_centre_dengreedy(master_df, targets_df, outer_FOV_radius, inner_FoV_radius, n_xx_yy=100):
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

    # See which tile centres have the largest number of clashes __outside the central 0.1*Radius of the field__. Julia says that the optics are poor here.

    galaxies_in_FoV_all = np.zeros(len(grid_coords), len(RA_all), dtype=bool)
    for i, coord in enumerate(grid_coords):
        galaxies_in_FoV_all[i, :] = check_if_in_fov(master_df, grid_coords[i, 0], grid_coords[i, 1], inner_FoV_radius, outer_FOV_radius)
    
    galaxies_in_FoV_untiled = np.zeros(len(grid_coords), len(RA_untiled), dtype=bool)
    for i, coord in enumerate(grid_coords):
        galaxies_in_FoV_untiled[i, :] = check_if_in_fov(targets_df, grid_coords[i, 0], grid_coords[i, 1], inner_FoV_radius, outer_FOV_radius)


    n_targets_in_FOV_all = galaxies_in_FoV_all.sum()
    n_targets_in_FOV_untiled = galaxies_in_FoV_untiled.sum()

    ratio_of_targets = n_targets_in_FOV_untiled / n_targets_in_FOV_all
    ratio_of_targets[np.isnan(ratio_of_targets)] = 0.0

    # If there are many places we can put a tile, randomise the location
    if np.sum(ratio_of_targets == np.max(ratio_of_targets)):
        index = np.random.choice(np.where(ratio_of_targets == np.max(ratio_of_targets))[0])
    else:
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
    xx = np.linspace(RA.min() - 0.5, RA.max() + 0.5, n_xx_yy)
    yy = np.linspace(Dec.min() - 0.5, Dec.max() + 0.5, n_xx_yy)

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


def check_if_in_fov(df, xcen, ycen, inner_radius, outer_radius):
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

    cos_dec_correction = np.cos(np.radians(np.abs(ycen)))

    ## MULTIPLY by the cos(dec) correction here!
    if inner_radius == 0:
        return (np.sqrt( (x - xcen)**2 * cos_dec_correction**2 + (y - ycen)**2 ) < outer_radius).values
    else:
        return ((np.sqrt( (x - xcen)**2 * cos_dec_correction**2 + (y - ycen)**2 ) < outer_radius) & (np.sqrt( (x - xcen)**2 * cos_dec_correction**2 + (y - ycen)**2 ) > inner_radius)).values


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

def find_great_circle_distance(uu, vv):

    """
    The great circle distance between vectors uu=[RA, Dec] and vv=[RA, Dec]
    Note that RA and Dec *must* be in degrees. We then convert them to radians, calcualte the distance and return that distance in degrees. 
    """
    uu = np.radians(uu)
    vv = np.radians(vv)

    if np.all(uu == vv):
        return 0.0
    else:
        theta = np.arccos(np.sin(uu[1])*np.sin(vv[1])+np.cos(uu[1])*np.cos(vv[1])*np.cos(uu[0]-vv[0]))
        return np.degrees(theta)

def _calc_clashes(XA, XB, proximity, tol=1e-5):
    """
    Given two lists of coordinates, return a boolean mask highlighting only those which clash within some minimum proximity. Note that we ignore self clashes by ignoring clashes with a distance = 0.0
    Note that we use the great-circle distance between two points to find their separation, not just a simple Euclidean 
    distance. This is to make sure that things still work at low declination. 

    tol is the minimum distance two objects can be apart before we say that they are clashing. I've added this in because sometimes the find_great_circle_distance function gives a distance of ~1e-6 or ~1e-7 for a galaxy from itself. I assume that this is just a numerical stability issue. 
    """
    distance_matrix = dist.cdist(XA, XB, find_great_circle_distance)
    clashes = (distance_matrix > tol) & (distance_matrix < proximity / 3600.0)

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
        star_df (dataframe): a dataframe of stars (guide or standard). Must have columns "RA", "DEC" and either "priority" or "r_mag" (see below)
        tile_df (dataframe): a dataframe containing Nsel targets, created from using 'unpick'. Must have columns "RA" amd "DEC"
        proximity (float): distance between two adjacent bundles, in arcseconds.
        Nsel (int): Return this many stars for each tile.
        star_type (string): One of either "standards" or "guides". Selects whether we sort the star dataframe by "priority" (for standards) or "r_mag" (for guides)
    """
    if star_type == 'standards':
        star_df = star_df.sort_values(by='priority', ascending=False)
    elif star_type == 'guides':
        star_df = star_df.sort_values(by='r_mag', ascending=True)
    else:
        raise NameError(f"star_type must be either 'standards' or 'guides'; currently {star_type}")

    tile_df, non_clashing_stars, _ = noclash(tile_df, star_df, proximity)

    # Make sure the non clashing stars also don't clash with themselves
    # Don't need to do this, since Caro's code does it for us
    #_, non_clashing_stars, _ = noclash(non_clashing_stars, non_clashing_stars, proximity)
    
    # There's a bug here where noclash doesn't count a tile with identical entries as clashing. So here we drop the duplicated rows to fix this
    non_clashing_stars = non_clashing_stars.drop_duplicates()

    if Nsel > len(non_clashing_stars):
        Nsel = len(non_clashing_stars)

    #assert len(non_clashing_stars) == len(np.unique(non_clashing_stars.ID)), "We seem to have a repeated star?"
    #import ipdb; ipdb.set_trace()
    return non_clashing_stars.iloc[:Nsel]


def select_targets(all_targets_df, proximity, Nsel, priorities, selection_type='most_clashing', fill_spares_with_repeats=False):
    """
    Given a dataframe of targets, select Nsel galaxies to observe. These can't be nearer to each other than 'proximity'. Return a dataframe of just the new tile we've made.
    Inputs:
        df (dataframe): A dataframe of targets. Must have columns "RA", "DEC", "PRIORITY" (i.e. priority), "COMPLETED" (i.e. tiled before this current iteration).
        proximity (float): Smallest distance between two targets (in arcseconds)
        Nsel (int): Number of targets to select in each tile
    """

    # Select things which haven't been already tiled or selected as a target in this iteration
    target_mask = (all_targets_df['COMPLETED'] == False)
    df = all_targets_df.loc[target_mask]
    already_tiled_in_FOV = all_targets_df.loc[~target_mask]

    # Find which of these targets clash with each other
    clashes = find_clashes(df, df, proximity)
    # n_clashes = clashes.sum(axis=0)

    targets = []
    isel_values = []
    untiled_galaxies = np.arange(len(df))  # [sorted_by_pri_then_nclashes]
    # priorities = priorities[sorted_by_pri_then_nclashes]

    # Now go through and add the target which clashes with the most things to the tile.
    # Keep doing this until we have selected 'Nsel' things or we run out of targets
    while (len(targets) < Nsel) and (len(untiled_galaxies) > 0):

        # Find any clashes in the targets
        n_clashes = np.sum(clashes, axis=0)[untiled_galaxies]
        # This is a dictionary of the priority of every target and then the number of things it clashes with
        clash_and_priority_dict = dict(zip(untiled_galaxies, zip(priorities[untiled_galaxies], n_clashes)))

        # Sort by priority then by number of clashes. Important that the tuples in clash_and_priority_dict are in the order (priority, n_clashes)
        galaxies_in_order = sorted(clash_and_priority_dict.items(), key=itemgetter(1), reverse=True)

        # If we have a clash...
        if np.sum(n_clashes) > 0:

            # Now find the target to select
            if selection_type == 'most_clashing':
                selected_target = galaxies_in_order[0][0]
            elif selection_type == 'random':
                random_choice = np.random.randint(low=0, high=len(galaxies_in_order))
                selected_target = galaxies_in_order[random_choice][0]
            else:
                raise NameError('Unknown selection type')
            # And find the targets it clashes with
            things_it_clashes_with = np.where(clashes[selected_target])[0]
            # Tile the most clashing target
            targets.append(selected_target)

            # Add its 'isel' value as its priority plus one, since it's a clashing target
            isel_values.append(priorities[selected_target] + 1)

            # ... and remove the things this target clashes with from the list of targets in this tile
            untiled_galaxies = [u for u in untiled_galaxies if u not in things_it_clashes_with]
            untiled_galaxies.remove(selected_target)

        # Else if there aren't any clashes remaining...
        else:
            # Just add everything left
            N_to_append = min(Nsel - len(targets), len(untiled_galaxies))

            unclashing_targets = [galaxies_in_order[r][0] for r in range(N_to_append)]

            targets.extend(unclashing_targets)
            isel_values.extend(priorities[unclashing_targets])

            # Remove things we've tiled
            untiled_galaxies = [u for u in untiled_galaxies if u not in targets]

    tile_df = df.iloc[targets]

    if fill_spares_with_repeats:
        if len(tile_df) < Nsel:
            # We now see if any things which have already been tiled can be repeated
            # We check to see whether everything in the Field of View clashes with anything which has already been tiled.
            # NOTE that this a quick-fix way of doing things- for example, if two things are in a close pair, using this method they will never be added to the tile as a repeat, since they will always clash with each other. The correct thing to do would be to add in things to repeat one by one, as in the method above
            # Also need to fix things to pick 'high priority' targets to repeat preferentially
            clashes = find_clashes(pd.concat((tile_df, already_tiled_in_FOV)), already_tiled_in_FOV, proximity)
            n_clashes = clashes.sum(axis=0)
            if len(already_tiled_in_FOV) > 0 & (np.any(n_clashes == 0)):
                unclashing_repeats = np.where(n_clashes == 0)[0]
                N_to_append = min(Nsel - len(tile_df), len(unclashing_repeats))
                repeats_to_fill_hexabundles = np.random.choice(unclashing_repeats, N_to_append, replace=False)
                tile_df = pd.concat((tile_df, already_tiled_in_FOV.iloc[repeats_to_fill_hexabundles]))
                tile_df.loc[tile_df.COMPLETED == True, 'isel'] = 1.0
                isel_values.extend([1.0] * len(repeats_to_fill_hexabundles))

                if len(tile_df) < Nsel:
                    message = f"Can only select {len(targets)} new targets for this field. Can only select {N_to_append} repeats. Tile length is only {len(tile_df)}!!!"
                else:
                    message = f"Can only select {len(targets)} new targets for this field. Select {N_to_append} repeats. Tile length is {len(tile_df)}"
            else:
                message = f"Can only select {len(targets)} new targets for this field. Can't select any repeats. Tile length is only {len(tile_df)}!!!"

            
            logger.warning(message)

    # # # If we don't fill a tile, append already observed galaxies till we get to the right number
    # # # These are set an isel value of 1.
    # # However, check there are other things left to tile! Otherwise we add repeats of the same targets which breaks things
    # if (len(tile_df) < Nsel) & (len(all_targets_df) > len(tile_df)):
        
    #     N_missing = Nsel - len(tile_df)
        
    #     gals_added = 0
    #     # Loop through and add targets, checking each time if they clash with the tile
    #     # This is a bit slower than the proper way of doing things above (since we're calling find_clashes many times) but the tile should be small enough that it's not a big deal. 
    #     for i in range(N_missing):

    #         # We're now finding clashes between the tile and *everything* in the field, not just things we haven't observed
    #         targets_not_already_in_tile = all_targets_df[~all_targets_df.index.isin(tile_df.index)]
    #         clashes = find_clashes(targets_not_already_in_tile, tile_df, proximity=proximity)
    #         n_clashes = clashes.sum(axis=1)
    #         if np.any(np.atleast_1d(n_clashes) == 0):
    #             unclashing_index = np.random.choice(np.where(n_clashes == 0)[0])
    #             gals_added += 1
    #             tile_df = tile_df.append(all_targets_df.iloc[unclashing_index])
    #         else:
    #             break

    #     isel_values.extend([1] * gals_added)
    

    if len(tile_df) < Nsel:
        if (len(all_targets_df) > len(tile_df)):
            message = f"Can't select {Nsel} targets for this field. Info: {len(all_targets_df)} in FOV and {len(tile_df)} in tile!"

        else:
            targets_not_already_in_tile = all_targets_df[~all_targets_df.index.isin(tile_df.index)]
            clashes = find_clashes(targets_not_already_in_tile, tile_df, proximity=proximity)
            n_clashes = clashes.sum(axis=1)
            message = f"Can't select {Nsel} targets for this field. Info: {len(all_targets_df)} in FOV; {len(tile_df)} in tile; {len(targets_not_already_in_tile)} are in FOV but not tiled, of which each one clashes {n_clashes} times"    
        
        logger.warning(message)

        #import ipdb; ipdb.set_trace()

    # Check if we clash with ourself- this should never happen
    clashes = find_clashes(tile_df, tile_df, proximity)
    if np.sum(clashes) != 0:
        raise ValueError("Our tile seems to clash with itself... This should never happen!")

    return tile_df, isel_values


def make_best_tile(df_targets, df_guide_stars, df_standard_stars, proximity, tiling_parameters, tiling_type, use_galaxy_priorities=True, selection_type='most_clashing', fill_spares_with_repeats=False, df_skies=None):
    """
    Put all the above functions togther and make a tile. Note that this function __doesn't__ update any tiling flags in the overall database. This should be done afterwards, so that we can integrate things with the Hector configuration code- the 19 best targets we pick might not actually be tile-able, so we don't want to mark things as tiled if the config code needs to select backups.
    Inputs:
        df_targets (dataframe): A dataframe of galaxy targets. Must include columns 'RA', 'DEC', "PRIORITY" and "TILED"
        df_guide_stars (dataframe): A dataframe of galaxy targets. Must include columns 'RA', 'DEC' and "r_mag"
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

    Hector_FOV_outer_radius = tiling_parameters['Hector_FOV_outer_radius']
    Hector_FOV_inner_radius = tiling_parameters['Hector_FOV_inner_radius']
    Nsel = tiling_parameters['Nsel']
    Nsel_guides = tiling_parameters['Nsel_guides']
    Nsel_standards = tiling_parameters['Nsel_standards']

    # Do some basic error checking
    if Hector_FOV_inner_radius < 0:
        raise ValueError(f"FoV Inner radius must be positive! Currently it's {Hector_FOV_inner_radius}")
    if Hector_FOV_outer_radius < 0:
        raise ValueError(f"FoV Outer radius must be positive! Currently it's {Hector_FOV_outer_radius}")
    if Hector_FOV_outer_radius < Hector_FOV_inner_radius:
        raise ValueError(f"Outer radius must be greater than inner radius! Currently O.R. is {Hector_FOV_outer_radius} and I.R. is {Hector_FOV_inner_radius}")

    untiled = df_targets['COMPLETED'] == False # & (df_targets['PRIORITY'] == 8)

    if use_galaxy_priorities:
        priorities = df_targets['priority']
    else:
        priorities = pd.Series(np.ones_like(df_targets['priority']), name='priority')

    # FIXME
    # This will cause an issue if the separation of two grid points is larger than the FOV size
    n_xx_yy = max(min(500, int(untiled.sum() / 10)), 50)

    if tiling_type == 'greedy':
        tile_RA, tile_Dec = get_best_tile_centre_greedy(df_targets.loc[untiled], outer_FOV_radius=Hector_FOV_outer_radius, inner_FoV_radius=Hector_FOV_inner_radius, n_xx_yy=n_xx_yy, priorities=priorities.loc[untiled])
    elif tiling_type == 'dengreedy':
        tile_RA, tile_Dec = get_best_tile_centre_dengreedy(df_targets, df_targets.loc[untiled], outer_FOV_radius=Hector_FOV_outer_radius, inner_FoV_radius=Hector_FOV_inner_radius, n_xx_yy=n_xx_yy)
    else:
        raise NameError("tiling_type must be one of 'greedy' or 'dengreedy'")

    # Only select targets, guides and standards within the FoV
    # check_if_in_FOV returns a boolean mask
    inner_targets = df_targets.loc[check_if_in_fov(df_targets, tile_RA, tile_Dec, outer_radius=Hector_FOV_outer_radius, inner_radius=Hector_FOV_inner_radius), :]
    inner_guides = df_guide_stars.loc[check_if_in_fov(df_guide_stars, tile_RA, tile_Dec, outer_radius=Hector_FOV_outer_radius, inner_radius=Hector_FOV_inner_radius), :]
    inner_standards = df_standard_stars.loc[check_if_in_fov(df_standard_stars, tile_RA, tile_Dec, outer_radius=Hector_FOV_outer_radius, inner_radius=Hector_FOV_inner_radius), :]

    if df_skies is not None:
        inner_skies = df_skies.loc[check_if_in_fov(df_skies, tile_RA, tile_Dec, outer_radius=Hector_FOV_outer_radius, inner_radius=Hector_FOV_inner_radius), :]

    if (len(inner_guides) == 0) or (len(inner_standards) == 0):
        raise ValueError("No stars in the Field of View!")

    # Select which targets to keep
    tile_members, isel_values = select_targets(inner_targets, proximity, Nsel, priorities=priorities, selection_type=selection_type, fill_spares_with_repeats=fill_spares_with_repeats)

    # Add the 'isel' values to the main dataframe
    df_targets.loc[tile_members.index, 'isel'] = isel_values
    tile_members.loc[:, 'isel'] = isel_values

    # Now add in the guide stars and standard stars
    # Make sure none clash with any of the targets in that tile
    guide_stars_for_tile = select_stars_for_tile(inner_guides, tile_members, proximity=proximity, Nsel=Nsel_guides, star_type='guides')
    standard_stars_for_tile = select_stars_for_tile(inner_standards, tile_members, proximity=proximity, Nsel=Nsel_standards, star_type='standards')

    standard_stars_for_tile['isel'] = standard_stars_for_tile['priority'] + 10

    if df_skies is not None:
        skies_for_tile = select_sky_fibres_for_tile

    return df_targets, tile_members, guide_stars_for_tile, standard_stars_for_tile, tile_RA, tile_Dec


def save_tile_outputs(outfolder, df_targets, tile_df, guide_stars_for_tile, standard_stars_for_tile, tile_RA, tile_Dec, tiling_parameters, tile_number, columns_in_order, guide_columns_in_order, plot=True, tile_out_name=None, guide_out_name=None):
    """
    Save the outputs from a single tile. These are:
        * A text file called tile_{i}.fld, which contains things to be observed the Hector science bundles. This has columns ID, RA, DEC, mag, type (where type is 1 for a galaxy target and 0 for a standard star) and isel (which is a bit like the priority they should be targeted in). The targets are sorted by priority!
        * A text file called guide_tile_{i}.fld, which contains guide stars (observed with the Hector guide bundles). This has columns RA, DEC, mag
        * A plot of the field with the Nsel best targets selected by this code. Note that this may not resemble the final tile selected by the configuration code!
    Note that we also make two new columns called "MagnetX_noDC" and "MagnetY_noDC" which correspond to the xy positions of the galaxies on the Hector plate in microns from the centre. These have *NOT* been corrected for the optical distortions- that's done by the "DistortionCorrection" code, which happens after each time has been saved.
    These outputs will be bundled together in a single folder, which contains subfolders 'Tiles' and 'Plots'
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

    if tile_out_name is None:
        tile_out_name = f"tile_{tile_number:03}.fld"
    if guide_out_name is None:
        guide_out_name = f"guide_tile_{tile_number:03}.fld"

    # Add in the MagnetX and MagnetY values, using the Hector plate radius from the constants file
    tile_df['MagnetX_noDC'] = (tile_df['RA'] - tile_RA) * hector_constants.HECTOR_plate_radius * 1e3
    tile_df['MagnetY_noDC'] = (tile_df['DEC'] - tile_Dec) * hector_constants.HECTOR_plate_radius * 1e3

    standard_stars_for_tile['MagnetX_noDC'] = (standard_stars_for_tile['RA'] - tile_RA) * hector_constants.HECTOR_plate_radius * 1e3
    standard_stars_for_tile['MagnetY_noDC'] = (standard_stars_for_tile['DEC'] - tile_Dec) * hector_constants.HECTOR_plate_radius * 1e3

    guide_stars_for_tile['MagnetX_noDC'] = (guide_stars_for_tile['RA'] - tile_RA) * hector_constants.HECTOR_plate_radius * 1e3
    guide_stars_for_tile['MagnetY_noDC'] = (guide_stars_for_tile['DEC'] - tile_Dec) * hector_constants.HECTOR_plate_radius * 1e3

    save_tile_text_file(outfolder, tile_out_name, tile_df, standard_stars_for_tile, tile_RA, tile_Dec, tiling_parameters, columns_in_order)
    save_guide_text_file(outfolder, guide_out_name, guide_stars_for_tile, tile_RA, tile_Dec, tiling_parameters, guide_columns_in_order)

    if plot:
        if not os.path.exists(f'{outfolder}/Plots'):
            os.makedirs(f'{outfolder}/Plots')

        fig, ax = plot_tile(tile_df, guide_stars_for_tile, standard_stars_for_tile, df_targets, tile_RA, tile_Dec, tiling_parameters['Hector_FOV_outer_radius'], tiling_parameters['Hector_FOV_inner_radius'], tile_number, tiling_parameters['proximity'])
        fig.savefig(f'{outfolder}/Plots/tile_{tile_number:03}.png', bbox_inches='tight')
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
        members = df_targets.loc[tiles[tile_number]] # This selects by index, NOT integer position.
        axs[0].scatter(members.RA, members.DEC) 
        tile_footprint = plt.Circle(xy=(tile_RAs[tile_number], tile_DECs[tile_number]), radius=tiling_parameters['Hector_FOV_outer_radius'], facecolor='None', edgecolor='k')
        axs[0].add_artist(tile_footprint)

    
    completeness, completion_fraction_to_calculate, used_tiles_to_get_to_x, minimum_number_of_tiles_for_x, efficiency_xpc, efficiency = calculate_completeness_stats(df_targets, tiling_parameters['N_targets_per_Hector_field'], completion_fraction_to_calculate=completion_fraction_to_calculate, verbose=verbose)

    # Plot the completeness as a function of tile number
    N_galaxy_observations = df_targets['N_observations_to_complete'].sum()
    xx = np.arange(1, len(completeness) + 1)
    axs[1].plot(xx, tiling_parameters['N_targets_per_Hector_field'] / N_galaxy_observations * xx, c='k', linestyle='dashed')
    axs[1].plot(xx, completeness, c='r', zorder=10)
    axs[1].axhline(completion_fraction_to_calculate, c='0.8', linestyle='dashed')
    axs[1].axhline(1, c='0.5')
    axs[1].set_ylim(0, 1.2)
    axs[1].axvline(used_tiles_to_get_to_x, c='0.8', linestyle='dashed')
    axs[1].axvline(minimum_number_of_tiles_for_x, c='0.8', linestyle='dashed')
    axs[1].set_xlabel(r'$N_{\rm{tiles}}$')
    axs[1].set_ylabel('Completeness')
    fig.tight_layout()

    return fig, axs


def _calc_completeness(df_targets):
    """
    Given a tiling dataframe, work out the fractional completeness of the tiling after each tile.
    Inputs:
        df_targets (dataframe): a dataframe with a row for each target. Must have a column 'Tile_number'
    """
    completeness = np.cumsum([np.sum((df_targets['Tile_number'] == i) & df_targets['COMPLETED'] == True) / len(df_targets) for i in np.unique(df_targets['Tile_number'])])

    return completeness


def calculate_completeness_stats(df_targets, N_targets_per_Hector_field, completion_fraction_to_calculate=0.95, verbose=True):
    """
    Given a set of tiles, calculate some stats about the efficiency to get to a given completeness fraction
    Inputs:
        df_targets (dataframe): a dataframe with a row for each target. Must have a column 'Tile_number'
        N_targets_per_Hector_field (dict): The numnber of hexabundles we can place on galaxy targets
        completeness_fraction_to_calculate (float, default=0.95): calculate the efficiency to reach this completeness fraction. This is defined as actual number of tiles used / minimum number of tiles possible) 
        verbose (bool, default=True): print efficiency stats or not.  
    """

    if (completion_fraction_to_calculate > 1) or (completion_fraction_to_calculate < 0):
        raise ValueError(f"completion_fraction_to_calculate must be between 0 and 1: currently {completion_fraction_to_calculate}")

    N_galaxy_observations = df_targets['N_observations_to_complete'].sum()

    # Work out the completeness after each tile
    completeness = _calc_completeness(df_targets)

    used_tiles = df_targets['Tile_number'].max() + 1  # Since we start indexing from 0
    minimum_number_of_tiles = np.ceil(N_galaxy_observations / N_targets_per_Hector_field)
    efficiency = minimum_number_of_tiles / used_tiles

    used_tiles_to_get_to_x = np.where(completeness > completion_fraction_to_calculate)[0][0] + 1
    minimum_number_of_tiles_for_x = np.ceil(completion_fraction_to_calculate * N_galaxy_observations / N_targets_per_Hector_field)
    efficiency_xpc = minimum_number_of_tiles_for_x / used_tiles_to_get_to_x

    if verbose:
        print(f"Efficiency for completion={completion_fraction_to_calculate}: {100*efficiency_xpc:.3f}%")
        print(f"Efficiency for completion=1: {100*efficiency:.3f}%")

    return completeness, completion_fraction_to_calculate, used_tiles_to_get_to_x, minimum_number_of_tiles_for_x, efficiency_xpc, efficiency


def plot_tile(tile_df, guide_df, standards_df, catalogue_df, tile_RA, tile_Dec, tile_outer_radius, tile_inner_radius, tile_number, proximity, fig=None, ax=None):
    """
    Make a plot of an individual tile.
    Inputs:
        tile_df (dataframe): Dataframe of a tile to plot
        guide_df (dataframe): Dataframe of guide stars from this tile
        standards_df (dataframe): Dataframe of standard stars from this tile
        catalogue_df (dataframe): Dataframe of the entire field, to plot as background
        tile_RA (float): RA of the centre of this tile
        tile_Dec (float): Dec of the centre of this tile
        tile_outer_radius (float): Outer Radius of the FoV (in degrees)
        tile_inner_radius (float): Inner Radius of the FoV (in degrees)
        tile_number (int): Number of the tile
        proximity (float): Radius of the tile
        fig, ax (optional): An exisiting Figure/Axis object to plot on
    """
    top_target_mask = catalogue_df['ID'].isin(tile_df['ID']) 

    if fig is None or ax is None:
        fig, ax = plt.subplots(figsize=(14, 10))

    ax.scatter(catalogue_df.RA, catalogue_df.DEC, marker=',', s=1, c='0.5')
    ax.scatter(tile_df.RA, tile_df.DEC, marker='x', c='k', s=30, label='All Targets', zorder=10)
    ax.scatter(catalogue_df.loc[top_target_mask, 'RA'], catalogue_df.loc[top_target_mask, 'DEC'], marker='x', c='r', s=50, label='Top Targets', zorder=10)
    ax.scatter(standards_df.RA, standards_df.DEC, marker='o', c='g', s=20, label='Standards')
    ax.scatter(guide_df.RA, guide_df.DEC, marker='s', c='b', s=20, label='Guides')
    ax.scatter(tile_RA, tile_Dec, marker='+', c='k', s=200, label='Tile Centre')


    for index, row in tile_df.iterrows():

        circle = plt.Circle(xy=(row.RA, row.DEC), radius=proximity / 3600.0, facecolor='None', edgecolor='k', linestyle='dashed')
        ax.add_artist(circle)


    ax.set_aspect('equal')
    ax.set_xlim(tile_RA - 1.1 * tile_outer_radius, tile_RA + 1.1 * tile_outer_radius)
    ax.set_ylim(tile_Dec - 1.1 * tile_outer_radius, tile_Dec + 1.1 * tile_outer_radius)

    circle = plt.Circle((tile_RA, tile_Dec), tile_outer_radius, facecolor='None', edgecolor='r')
    ax.add_artist(circle)
    # Add the total 2DF field of view in black
    circle = plt.Circle((tile_RA, tile_Dec), 1.0, facecolor='None', edgecolor='k')
    ax.add_artist(circle)

    # Add the central region which we're ignoring
    circle = plt.Circle((tile_RA, tile_Dec), tile_inner_radius, facecolor='0.1', edgecolor='0.1', alpha=0.3)
    ax.add_artist(circle)


    legend = ax.legend(bbox_to_anchor=(1, 0.5))

    ax.set_xlabel('Right Ascension')
    ax.set_ylabel('Declination')

    ax.set_title(f'Tile {tile_number}')

    return fig, ax


def save_tile_text_file(outfolder, out_name, tile_df, standard_stars_for_tile, tile_RA, tile_Dec, tiling_parameters, columns_in_order):
    """
    Save a text file of things to be observed with Hector science bundles (i.e. targets and standards). The second line of this text file __must__ be the tile RA and DEC seperated by a space.
    """

    if not os.path.exists(f"{outfolder}/Tiles"):
        os.makedirs(f"{outfolder}/Tiles")

    # Make sure the tile_df is sorted by priority
    tile_df = tile_df.sort_values(by='priority', ascending=False)

    # Rename the column headings so things match
    #targets_renamer = dict(CATAID="ID")
    #tile_df = tile_df.rename(columns=targets_renamer)
    tile_df['type'] = 1

    #standards_renamer = dict(CoADD_ID='ID')
    #standard_stars_for_tile = standard_stars_for_tile.rename(columns=standards_renamer)
    standard_stars_for_tile['type'] = 0

    # combined_stars_targets_df = tile_df[['ID', 'RA', 'DEC', 'mag', 'type', 'isel']].append(standard_stars_for_tile[['ID', 'RA', 'DEC', 'mag', 'type', 'isel']])
    if not 'MagnetX_noDC' in columns_in_order:
        columns_in_order.extend(['priority', 'MagnetX_noDC', 'MagnetY_noDC', 'type'])
    combined_stars_targets_df = pd.concat((tile_df, standard_stars_for_tile), sort=True)[columns_in_order]
    # combined_stars_targets_df = tile_df.append(standard_stars_for_tile, sort=True)[['ID', 'RA', 'DEC', 'Re', 'Mstar', 'z', 'GAL_MAG_G', 'GAL_MAG_I', 'GAL_MU_0_G', 'GAL_MU_0_I', 'GAL_MU_0_R', 'GAL_MU_0_U',
    #    'GAL_MU_0_Z', 'GAL_MU_E_G', 'GAL_MU_E_I', 'GAL_MU_E_R', 'GAL_MU_E_U',
    #    'GAL_MU_E_Z', 'GAL_MU_R_at_2Re', 'GAL_MU_R_at_3Re', 'Dingoflag', 'Ellipticity_r',
    #    'IFU_diam_2Re', 'MassHIpred',  'PRIORITY',
    #    'SersicIndex_r', 'WALLABYflag', 'g_m_i', 'isel', 'mag',
    #    'priority', 'remaining_observations', 'Tile_number', 'COMPLETED', 'type', 'MagnetX_noDC', 'MagnetY_noDC']]

    # Write a CSV file with the header we want
    with open(f"{outfolder}/Tiles/{out_name}", 'w') as f:
        f.write("# Target and Standard Star file from Sam's tiling code\n")
        f.write(f"# {tile_RA} {tile_Dec}\n")
        f.write(f"# Proximity Value: {tiling_parameters['proximity']}\n")
    combined_stars_targets_df.to_csv(f"{outfolder}/Tiles/{out_name}", sep=',', index=False, mode='a')

    return 0


def save_guide_text_file(outfolder, out_name, guide_stars_for_tile, tile_RA, tile_Dec, tiling_parameters, guide_columns_in_order):
    """
    Save a text file of things to be observed with Hector guide bundles (i.e. guides). The second line of this text file __must__ be the tile RA and DEC seperated by a space.
    """

    if not os.path.exists(f"{outfolder}/Tiles"):
        os.makedirs(f"{outfolder}/Tiles")

    # Make sure the tile_df is sorted by R band magnitude
    guide_stars_for_tile = guide_stars_for_tile.sort_values(by='r_mag')

    #
    #guide_stars_for_tile = guide_stars_for_tile.rename(columns=guides_renamer)
    guide_stars_for_tile['type'] = 2

    # Write a CSV file with the header we want
    with open(f"{outfolder}/Tiles/{out_name}", 'w') as f:
        f.write("# Guide Star file from Sam's tiling code\n")
        f.write(f"# {tile_RA} {tile_Dec}\n")
        f.write(f"# Proximity Value: {tiling_parameters['proximity']}\n")

    # Add in ['MagnetX_noDC', 'MagnetY_noDC']
    if not 'MagnetX_noDC' in guide_columns_in_order:
        guide_columns_in_order.extend(['MagnetX_noDC', 'MagnetY_noDC'])
    guide_stars_for_tile[guide_columns_in_order].to_csv(f"{outfolder}/Tiles/{out_name}", sep=' ', index=False, mode='a')

    return 0

############################################################################################################
