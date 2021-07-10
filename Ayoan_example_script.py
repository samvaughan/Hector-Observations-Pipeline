# #!/usr/bin/env python3

"""
A pipeline for Hector observations 
"""

from hop import pipeline 


HP = pipeline.HectorPipe(config_filename="ConfigFiles/Ayoan_example_config_Cluster2.yaml")

for i in range(22):
    # HP.allocate_hexabundles_for_single_tile(tile_number=i,plot=True)

    # For running the show sky fibre changes function, the guidefile and proxy hexa and guide file need to be created at first
    for j in range(i,22):
        if i != j:
            HP.show_sky_fibre_changes(i,j)


