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
source(paste(Sys.getenv('HECTOROBSPIPELINE_LOC'), "/configuration/HECTOR_Config_v3.2.R", sep='/'))

library("argparse")
parser = ArgumentParser(description='Configure a Hector tile such that all targets are observable')
parser$add_argument('tile_files', type="character",
                    help='one or more (use wildcards) tile files for us to configure. Should end in .fld')
parser$add_argument('addition_to_fname', type="character",
                    help='a string to add to the filename')
parser$add_argument('--plot', default=FALSE, action="store_true",
                    help='Show plots to the screen? Default is FALSE')
parser$add_argument('--out-dir', default='./',
                    help='Directory to save the output files')
parser$add_argument('--run_local', default=FALSE, action="store_true",
                    help='Are we running locally on my laptop? If so, set the location')
#parser$add_argument('--FoV', default=152.2, help='Size of the FoV. This decides which Config file to load! Bit of a hack...')



#* This is a bit of a hack to get this to run in RStudio using argparse. The vector tmp_args should be a comma-separated list of the
#* arguments you'd give on the command line.
#* To compare to ipython: %run myscript a, b would become tmp_args=c('a', 'b')
#* DON'T FORGET TO COMMENT OUT LINE 51 TO RUN AS A SCRIPT!!!
tmp_args=c('/Users/samvaughan/Science/Hector/full_simulations/Testing_Distortion_Correction/DistortionCorrected/DC_tile_000.fld',
             'GAMA_G12_',
             '--out-dir',
             '/Users/samvaughan/Science/Hector/full_simulations/Testing_Distortion_Correction/Configuration/',
             '--plot', '--run_local')

#args <- parser$parse_args(tmp_args)
args <- parser$parse_args()


#* Get the targets. We'll use Sys.glob in case a wildcard was passed.
SAMIFields_Targets = Sys.glob(args$tile_files)
addition_to_fname = args$addition_to_fname
plot = args$plot
out_dir = args$out_dir
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


i=1
for (f in SAMIFields_Targets){
  print(paste('********************************************'))
  print(paste('Configuring field ',f, sep=''))
  print(paste('********************************************'))
  
  #Reading in the field data (fd):
  #* Added explicit splitting into fieldsfolder, which is the stem of the filename
  #* And f, which the name of the file
  #* Also added a slash here, in the sep argument (was originally sep=' ')
  #* And changed the substr call below, as the name of the files is different to the original format
  fieldsfolder = dirname(SAMIFields_Targets)
  f = basename(SAMIFields_Targets)
  tile_data=read.table(file=paste(fieldsfolder,f, sep='/'), header=TRUE, sep=',', skip=8, check.names = TRUE)
  g=paste('guide_',f,sep='')
  gdata=read.table(file=paste(fieldsfolder,g, sep='/'), header=TRUE, skip=8, sep=',', check.names = TRUE)
  
  gdata[,'type']=2
  
  #Reading in the field centre (ra, dec) from the fld file header (currently second line, i.e n=2):
  fcentre=as.numeric(strsplit(readLines(paste(fieldsfolder,f, sep='/'), n=2)[2], split=' ')[[1]][c(2,3)])
  fcentre=data.frame(ra=fcentre[1],dec=fcentre[2])
  
  #Combining guides and targets:
  #* I've changed these column headings to match the outputs of my tiling code
  fdata=rbind(tile_data[,c('ID','MagnetX','MagnetY','mag','type')],gdata[,c('ID','MagnetX','MagnetY','mag','type')])
  
  # Converting coordinates into mm
  
  fdata[,'x'] = fdata[,'MagnetX'] / 1000.0 # * cos(fcentre$dec*pi/180.)
  fdata[,'y'] = fdata[,'MagnetY'] / 1000.0
  
  fdata[,'r'] = sqrt(fdata$x**2+fdata$y**2)
  
  #Removing everything that is too far from the centre of the field.
  fdata=fdata[fdata[,'r']<fov/2-excl_radius,]
  
  #Converting into input file for the configuration code.
  IDs=as.data.frame(fdata[,c('ID')])
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
  grads=final_config$grads
  angs_azAng=final_config$angs_azAng
  gangs_gazAng=final_config$gangs_gazAng
  azAngs=final_config$azAngs
  gazAngs=final_config$gazAngs
  probes=defineprobe(x=pos[,'x'],y=pos[,'y'],angs=angs)
  conflicts=find_probe_conflicts(probes=probes, pos=pos, angs=angs)
  nconflicts=dim(conflicts)[1]
  print(paste('There were ', as.character(nconflicts), ' unresolvable conflicts in field ', as.character(i)))
  itnconflicts=c(itnconflicts,nconflicts)
  swapsneeded=c(swapsneeded,final_config$swaps)
  
  #* Get the IDs of things which have been tiled, and save these
  #* IDs=IDs[rownames(pos),] <<- This is wrong! 
  IDs=IDs[match(rownames(pos),rownames(fdata)),] 
  selected_objects = tile_data[tile_data$ID %in% IDs,]
  #Writing output files:
  #* I've added the ability to save things to a specified output directory
  #* This is passed via argparse as 'out_dir'
  #write.table(cbind(probe=c(1:length(angs)),IDs,pos,rads,angs,azAngs,angs_azAng),file=paste(out_dir,'HECTORConfig_Hexa_',addition_to_fname, substr(f, start=1,stop=nchar(f)-4), '.txt', sep=''), row.names=FALSE)
  #write.table(cbind(probe=c(1:length(gangs)),gpos,gangs),file=paste(out_dir,'HECTORConfig_Guides_',addition_to_fname, substr(f, start=1,stop=nchar(f)-4), '.txt', sep=''), row.names=FALSE)
  
  galaxy_out_name = paste(out_dir,'HECTORConfig_Hexa_',addition_to_fname, substr(f, start=1,stop=nchar(f)-4), '.txt', sep='')
  guide_out_name = paste(out_dir,'HECTORConfig_Guides_',addition_to_fname, substr(f, start=1,stop=nchar(f)-4), '.txt', sep='')
  print(paste('********************************************'))
  print(paste('Writing outputs to ', galaxy_out_name, sep=''))
  print(paste('********************************************'))
  
  write.table(cbind(probe=c(1:length(angs)),IDs,pos,rads,angs,azAngs,angs_azAng,selected_objects),file=galaxy_out_name, row.names=TRUE)
  write.table(cbind(probe=c(1:length(gangs)),gpos,grads,gangs,gazAngs,gangs_gazAng),file=guide_out_name, row.names=FALSE)
  
  if (visualise) {
    plot_configured_field(pos=pos,angs=angs,gpos=gpos,gangs=gangs, fieldflags=final_config$flags, aspdf=FALSE)
  }
  plot_filename = filename=paste(out_dir,'/HECTORConfig_',addition_to_fname, substr(f, start=1,stop=nchar(f)-4), '.pdf', sep='')
  plot_configured_field(filename=plot_filename,pos=pos,angs=angs,gpos=gpos,gangs=gangs, fieldflags=final_config$flags, aspdf=TRUE)
  i=i+1
}

TestOutcome=cbind(itnconflicts,swapsneeded)

#* Don't save the R image
#* save.image('HECTOR_Config_v1.0_ClusterFieldsTest_width24.img')




