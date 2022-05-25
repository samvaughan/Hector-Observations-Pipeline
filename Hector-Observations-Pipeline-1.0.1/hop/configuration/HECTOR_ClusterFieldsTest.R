#! /usr/local/bin/Rscript --no-init-file

#* Added (above)

##Test routine:

#Install various R libraries the first time it is run. This only needs to be run once ever.:
#install.packages('astro')
#install.packages('astroFns')
#install.packages('plotrix')
#install.packages('raster')
#install.packages('graphics')
#install.packages('pracma')
#install.packages('DescTools')
#install.packages('PBSmapping')
#install.packages('DEoptim')
#install.packages('R.utils')
#install.packages('rgeos')


#* Edits by Sam. I'll use a #* for my comments. 

#* No longer need to set the working directory
#Setting working directory:
#* setwd('~/Science/Hector/Tiling/MAXI/Cluster_tests/HectorC1_tests_01/')

#* Source the Hector Config file
#source(paste(Sys.getenv('HECTOROBSPIPELINE_LOC'), "/configuration/HECTOR_Config_v3.2.R", sep='/'))
source("/Users/samvaughan/Science/Hector/HectorObservationPipeline/hop/configuration/HECTOR_Config_v3.2.R")

library("argparse")
parser = ArgumentParser(description='Configure a Hector tile such that all targets are observable')
parser$add_argument('tile_filename', type="character",
                    help='tile file for us to configure. Should end in .fld')
parser$add_argument('guide_filename', type="character",
                    help='guide file for us to configure. Should end in .fld')
parser$add_argument('galaxy_out_name', type="character",
                    help='Name of the output file')
parser$add_argument('guide_out_name', type="character",
                    help='Name of the output guide file')
parser$add_argument('--plot', default=FALSE, action="store_true",
                    help='Show plots to the screen? Default is FALSE')
parser$add_argument('--plot_filename',
                    help='Show plots to the screen? Default is FALSE')
parser$add_argument('--run_local', default=FALSE, action="store_true",
                    help='Are we running locally on my laptop? If so, set the location')
#parser$add_argument('--FoV', default=152.2, help='Size of the FoV. This decides which Config file to load! Bit of a hack...')



#* This is a bit of a hack to get this to run in RStudio using argparse. The vector tmp_args should be a comma-separated list of the
#* arguments you'd give on the command line.
#* To compare to ipython: %run myscript a, b would become tmp_args=c('a', 'b')
#* DON'T FORGET TO COMMENT OUT LINE 51 TO RUN AS A SCRIPT!!!
#tmp_args=c('/Users/samvaughan/Science/Hector/Targets/Commissioning/HandMade/results/TilingOutputs/060_m22/DistortionCorrected/DC_tile_060_m22_guides_central.csv',
#           '/Users/samvaughan/Science/Hector/Targets/Commissioning/HandMade/results/TilingOutputs/060_m22/DistortionCorrected/guide_DC_tile_060_m22_guides_central.csv',
#             '/Users/samvaughan/Science/Hector/Targets/Commissioning/HandMade/results/TilingOutputs/060_m22/Configuration/Hexas_tile_060_m22_guides_central_CONFIGURED.csv',
#           '/Users/samvaughan/Science/Hector/Targets/Commissioning/HandMade/results/TilingOutputs/060_m22/Configuration/Guides_tile_060_m22_guides_central_CONFIGURED.csv',
#             '--plot_filename', '/Users/samvaughan/Science/Hector/Targets/Commissioning/HandMade/results/TilingOutputs/060_m22/Plots/060_m22_guides_central.pdf',
#             '--plot', '--run_local')

tmp_args=c('/Users/samvaughan/Science/Hector/Targets/Commissioning/Feb2022/HandMade_Feb2022/results/TilingOutputs/150_m22/DistortionCorrected/DC_150_m22_reference_stars_iteration_3.csv',
           '/Users/samvaughan/Science/Hector/Targets/Commissioning/Feb2022/HandMade_Feb2022/results/TilingOutputs/150_m22/DistortionCorrected/guide_DC_150_m22_reference_stars_iteration_3.csv',
           '/Users/samvaughan/Desktop/testing_ref_stars_3_CONFIGURED_Updated.csv',
           '/Users/samvaughan/Desktop/guide_testing_ref_stars_3_CONFIGURED_Updated.csv',
           '--plot_filename', '/Users/samvaughan/Desktop/testing_ref_stars_3_Updated.pdf',
           '--plot', '--run_local')

args <- parser$parse_args(tmp_args)
#args <- parser$parse_args()


#* Get the targets. 
tile_filename = args$tile_filename
guide_filename = args$guide_filename
addition_to_fname = args$addition_to_fname
plot = args$plot
out_dir = args$out_dir
galaxy_out_name = args$galaxy_out_name
guide_out_name = args$guide_out_name
plot_filename = args$plot_filename
#FoV_size = args$FoV


#Importing all functions (adjust path according to where HECTOR_Config_v?.?.R is located):
#config_string = paste("/suphys/svau7351/Science/Hector/Tiling/Caros_Code/HECTOR_Config_v1.3.R", sep='')
#source(config_string)


#To enable graphic visualisation, set the "visualise" object to TRUE:
visualise <<- plot

itnconflicts=c()
swapsneeded=c()

#* Commented this out as we're now using files from the command line
#Finding all fld files to configure:
#fieldsfolder='Tiles/'
#SAMIFields_Targets=list.files(path=fieldsfolder,pattern='Targets')



print(paste('********************************************'))
print(paste('Configuring field ', tile_filename, sep=''))
print(paste('********************************************'))

#Reading in the field data (fd):
#* Added explicit splitting into fieldsfolder, which is the stem of the filename
#* And f, which the name of the file
#* Also added a slash here, in the sep argument (was originally sep=' ')
#* And changed the substr call below, as the name of the files is different to the original format


tile_data=read.table(file=tile_filename, header=TRUE, sep=',', comment.char="#", check.names = TRUE, stringsAsFactors = FALSE)

gdata=read.table(file=guide_filename, header=TRUE, comment.char="#", sep=',', check.names = TRUE, colClasses = c("ID"="character"))

#gdata[,'type']=2

# #Reading in the field centre (ra, dec) from the fld file header (currently second line, i.e n=2):
# fcentre=as.numeric(strsplit(readLines(paste(fieldsfolder,f, sep='/'), n=2)[2], split=' ')[[1]][c(2,3)])
# fcentre=data.frame(ra=fcentre[1],dec=fcentre[2])

# # Add in the fraction of the plate radius for debugging
# tile_data[, 'fraction_plate_radius_X'] = tile_data['MagnetX']/1000/(fov/2)
# tile_data[, 'fraction_plate_radius_Y'] = tile_data['MagnetY']/1000/(fov/2)

# Peel off the sky fibres
row_numbers = as.numeric(rownames(tile_data))
# Find all rows which have "Sky" in their index
sky_fibre_rows = grep("Sky", tile_data$ID)
sky_fibre_IDs = tile_data[sky_fibre_rows, 'ID']
not_sky_fibre_rows = row_numbers[!(row_numbers %in% sky_fibre_rows)]
sky_fibre_data = tile_data[sky_fibre_rows,]
#Combining guides and targets:
#* I've changed these column headings to match the outputs of my tiling code
fdata=rbind(tile_data[not_sky_fibre_rows,c('ID','MagnetX','MagnetY','r_mag','type')],gdata[,c('ID','MagnetX','MagnetY','r_mag','type')])

# Converting coordinates into mm

fdata[,'x'] = fdata[,'MagnetX'] / 1000.0 # * cos(fcentre$dec*pi/180.)
fdata[,'y'] = fdata[,'MagnetY'] / 1000.0

fdata[,'r'] = sqrt(fdata$x**2+fdata$y**2)

#Removing everything that is too far from the centre of the field.
fdata=fdata[fdata[,'r']<fov/2-excl_radius,]

#Converting into input file for the configuration code.
IDs=as.data.frame(fdata$ID)
pos=as.data.frame(fdata[fdata[,'type']==1,c('x','y')])
stdpos=as.data.frame(fdata[fdata[,'type']==0,c('x','y')])
guidepos=as.data.frame(fdata[fdata[,'type']==2,c('x','y')])
pos_master=list(pos=pos, stdpos=stdpos, guidepos=guidepos)


#Checking top 19 probes.
draw_pos(x=pos_master$pos[c(1:19),'x'],y=pos_master$pos[c(1:19),'y'])
 

#Configuring all HECTOR probes:
final_config=configure_HECTOR(pos_master = pos_master) 
pos=final_config$pos
angs=final_config$angs
gpos=final_config$gpos
gangs=final_config$gangs
rads=final_config$rads
chosen_guides = final_config$guidenum
grads=final_config$grads
angs_azAng=final_config$angs_azAng
gangs_gazAng=final_config$gangs_gazAng
azAngs=final_config$azAngs
gazAngs=final_config$gazAngs
probes=defineprobe(x=pos[,'x'],y=pos[,'y'],angs=angs)
conflicts=find_probe_conflicts(probes=probes, pos=pos, angs=angs)
nconflicts=dim(conflicts)[1]
print(paste('There were ', as.character(nconflicts), ' unresolvable conflicts in the field'))
itnconflicts=c(itnconflicts,nconflicts)
swapsneeded=c(swapsneeded,final_config$swaps)

#* Get the IDs of things which have been tiled, and save these
#* IDs=IDs[rownames(pos),] <<- This is wrong!
ID=IDs[match(rownames(pos),rownames(fdata)),]
# This gets the rest of the columns from the tile too for us to save
subset_of_IDs_for_saving = (tile_data$ID %in% ID)
# Don't keep the ID column as otherwise this gets duplicated
#selected_objects = tile_data[subset_of_IDs_for_saving, -which(names(tile_data) == "ID")]
selected_objects = tile_data[subset_of_IDs_for_saving,]
# Now make the skyfibre rows
sky_fibre_data = tile_data[(tile_data$ID %in% sky_fibre_IDs),]
sky_fibre_IDs = sky_fibre_data$ID
sky_fibre_data_minus_IDs =  sky_fibre_data[, -which(names(sky_fibre_data) == "ID")]
sky_fibre_filler_list = rep(-99, nrow(sky_fibre_data))

# Now make the tables- one for the hexabundles ("target_table") and one for the sky fibres
# I've added NA values for the sky fibre columns which don't make sense (rads, angs, etc)
new_properties = data.frame(cbind(probe=c(1:length(angs)),ID,pos,rads,angs,azAngs,angs_azAng))
target_table = merge(new_properties, selected_objects, by='ID', sort=FALSE)

sky_fibre_table = cbind(probe=sky_fibre_filler_list, 
                        ID=sky_fibre_IDs, 
                        x=sky_fibre_data$MagnetX / 1000.0, 
                        y=sky_fibre_data$MagnetY / 1000.0,
                        rads=sky_fibre_filler_list,
                        angs=sky_fibre_filler_list,
                        azAngs=sky_fibre_filler_list,
                        angs_azAng=sky_fibre_filler_list,
                        sky_fibre_data_minus_IDs)

final_table = rbind(target_table, sky_fibre_table)

#* Get the IDs of guide stars which have been chosen:
guide_IDs = fdata[match(rownames(gpos),rownames(fdata)), 'ID']
#all_guide_data = fdata[chosen_guides,]
#guide_IDs = gdata[chosen_guides, 'ID']
#* And get the columns from the input file
all_guide_data = gdata[match(guide_IDs, gdata$ID), ]
#* And now add on the other rows that we need 
new_guide_properties = data.frame(x=gpos$x, y=gpos$y, rads=grads,angs=gangs,azAngs=gazAngs,angs_azAng=gangs_gazAng)
final_guide_table = cbind(all_guide_data, new_guide_properties)
#final_guide_table = subset(merged_guides,select=-c(Row.names))
#final_guide_table = cbind(all_guide_data,x=gpos$x, y=gpos$y, rads=grads,angs=gangs,azAngs=gazAngs,angs_azAng=gangs_gazAng)

#Writing output files:
#* I've added the ability to save things to a specified output directory
#* This is passed via argparse as 'out_dir'
#write.table(cbind(probe=c(1:length(angs)),IDs,pos,rads,angs,azAngs,angs_azAng),file=paste(out_dir,'HECTORConfig_Hexa_',addition_to_fname, substr(f, start=1,stop=nchar(f)-4), '.txt', sep=''), row.names=FALSE)
#write.table(cbind(probe=c(1:length(gangs)),gpos,gangs),file=paste(out_dir,'HECTORConfig_Guides_',addition_to_fname, substr(f, start=1,stop=nchar(f)-4), '.txt', sep=''), row.names=FALSE)

# galaxy_out_name = paste(out_dir,'HECTORConfig_Hexa_',addition_to_fname, substr(f, start=1,stop=nchar(f)-4), '.txt', sep='')
# guide_out_name = paste(out_dir,'HECTORConfig_Guides_',addition_to_fname, substr(f, start=1,stop=nchar(f)-4), '.txt', sep='')
print(paste('********************************************'))
print(paste('Writing outputs to ', galaxy_out_name, sep=''))
print(paste('********************************************'))

#* Write the final table for the hexabundles and the guides
write.table(final_table,file=galaxy_out_name, row.names=FALSE, sep=',')
write.table(final_guide_table, file=guide_out_name, row.names=FALSE, sep=',')

if (visualise) {
  plot_configured_field(pos=pos,angs=angs,gpos=gpos,gangs=gangs, fieldflags=final_config$flags, aspdf=FALSE)
}
#* plot_filename = filename=paste(out_dir,'/HECTORConfig_',addition_to_fname, substr(f, start=1,stop=nchar(f)-4), '.pdf', sep='')
plot_configured_field(filename=plot_filename,pos=pos,angs=angs,gpos=gpos,gangs=gangs, fieldflags=final_config$flags, aspdf=TRUE)
#i=i+1


#TestOutcome=cbind(itnconflicts,swapsneeded)

#* Don't save the R image
#* save.image('HECTOR_Config_v1.0_ClusterFieldsTest_width24.img')




