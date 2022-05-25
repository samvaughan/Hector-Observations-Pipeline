import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
from ..hexabundle_allocation.hector import constants as hector_constants

def plot_distortion_correction_before_after(DC_tile_filename, title_text=None):

    """
    Take a tile which has had the distortion correction applied. Plot a before/after image to see the effects
    """

    font_size = 12

    # # Get the RA and DEC of the tile
    # with open(DC_tile_filename, 'r') as f:
    #     lines = f.readlines()
    # RA, DEC = lines[1].split()[1:]
    # RA = float(RA)
    # DEC = float(DEC)

    df = pd.read_csv(DC_tile_filename, comment='#')

    # Select only galaxies in the tile
    mask = df.type == 1

    # Rename things for brevity
    x_noDC = df.loc[mask, 'MagnetX_noDC']/1000
    y_noDC = df.loc[mask, 'MagnetY_noDC']/1000

    x_DC = df.loc[mask, 'MagnetX']/1000
    y_DC = df.loc[mask, 'MagnetY']/1000

    # Find the offsets for plotting
    dxs = np.row_stack([x_noDC, x_DC])
    dys = np.row_stack([y_noDC, y_DC])

    # Do the plotting
    fig, ax = plt.subplots(figsize=(8, 8))

    ax.plot(dxs, dys, c='k', zorder=1)
    ax.scatter(x_noDC, y_noDC, label='Before correction', c='b', s=40, zorder=10)
    ax.scatter(x_DC, y_DC, label='After correction', c='r', s=40, zorder=10)

    Hector_FOV = plt.Circle((0, 0), radius=hector_constants.HECTOR_plate_radius, linestyle='dashed', color='k', fill=False)

    ax.add_patch(Hector_FOV)
    ax.set_xlim(-1.2 * hector_constants.HECTOR_plate_radius, 1.2 * hector_constants.HECTOR_plate_radius)
    ax.set_ylim(-1.2 * hector_constants.HECTOR_plate_radius, 1.2 * hector_constants.HECTOR_plate_radius)

    ax.scatter(0, 0, marker='+', c='k')
    ax.legend(fontsize=font_size, loc='lower left')
    #ax.set_box_aspect(1)

    ax.set_xlabel("X (mm)")
    ax.set_ylabel("Y (mm)")

    ax.set_title(title_text, fontsize=font_size)

    box_props = dict(boxstyle='round', facecolor='0.9', alpha=0.5)
    #ax.text(0.5, 0.95, s=f"Tile Centre is ({RA:.3f}, {DEC:.3f})", transform=ax.transAxes, bbox=box_props, fontsize=font_size, ha='center', va='bottom')

    return fig, ax

