#############################################################################
### Purpose: Configure the position and orientation of HECTOR probes after
### greedy field and target selection. This version uses optimisation to make
### an initial guess of angles for non-overlapping probes.
### Date: 15 December 2018
### Author: Caroline Foster
#############################################################################

#Loading required libraries:
library(astro)
library(astroFns)
library(plotrix)
library(raster)
library(graphics)
library(pracma)
library(DescTools)
library(PBSmapping)
library(R.utils)
library(rgeos)

#Probe shape definition as global variables (currently has a head/tip, a body/probe and a cable zone of avoidance (all should be in degrees):
#plate_scale"/mm scaling
plate_scale <<- 15.008

#Field dimensions and parameters:
fov <<- 236*2 * plate_scale/3600 #2 degree field - a bit for sky fibres.
wceg <<- 2.* 15 * plate_scale/3600 #Width of the cable exit gap (this is the 2*n where n=15mm initially in Julia's notes)
ceg_pos <<- cbind(x=c(0,0.8504,-0.8504),y=c(-0.982,0.49098,0.49098), deg=c(-90,30,150))
mid_ceg_pos<<-cbind(x=c(0,-0.8504,0.8504),y=c(0.982,-0.49098,-0.49098), deg= c(90,210,-30))
E0<<-sqrt((wceg/2.)**2-(9*plate_scale/3600)**2) #E is the line joining the solid ferule end to the centre of the exit point. 
#E0=SQRT((n^2)-81). If E<E0, then need to point to the next least contaminated exit. This step prevents a ferule from 
#blocking an exit such that other cables will not get past.
fbr <<- 45 * plate_scale/3600 #Fibre bend radius

#Galaxy and standard star bundle dimensions:
tip_w <<- 14.5/sqrt(2) * plate_scale/3600 #(instead of 12)
tip_l <<- 14.5/sqrt(2) * plate_scale/3600  
probe_w <<- 14.5 * plate_scale/3600 #(instead of 8.5)
probe_l <<- 45 * plate_scale/3600
cable_w <<- 8.5 * plate_scale/3600
cable_l <<- 38 * plate_scale/3600
excl_radius <<- sqrt(2.*((tip_w/2)^2))
dngalprobes <<- 19
nstdprobes <<- 2
ngprobesmin <<- 3
ngprobesmax <<- 6
ngalprobes <<- dngalprobes
nprobes <<- dngalprobes #Start with the number of galaxy targets. There will be an additional 2 probes for standard stars.

#Guide bundle dimensions:
#Julia: The guide bundles should be the same 14mm width but shorter than the hexabundle ferules by 20mm ~50mm total plus cable bit.
gtip_w <<- 14.5/sqrt(2) * plate_scale/3600
gtip_l <<- 14.5/sqrt(2) * plate_scale/3600
gprobe_w <<- 14.5 * plate_scale/3600
gprobe_l <<- 45 * plate_scale/3600 
gcable_w <<- 8.5 * plate_scale/3600
gcable_l <<- 38 * plate_scale/3600
gexcl_radius <<- sqrt(2.*((gtip_w/2)^2))

#Setting timeout time (in cpu seconds, I think) for how long each attempt lasts before moving on. 
#The code will try for at most 15 times that before starting to swap a probe.
timeout<<-180 #180 seconds x 15 is about half an hour and is about right. Less than about 90 seconds led to problems in highly conflicted field..

#Compute the (horizontal/RA/x-axis) distance to the field edge.
dist2fieldedge <- function(x,y){
  dist=sqrt((fov/2.)**2-y**2)-abs(x)
  return(dist)
}

#Compute range of allowed angles for given probe:
computeAnglesRange <- function(x, y, ceg, delta_ang=20,guide=FALSE){
  
  ang=as.numeric(first_guess_angs(pos=cbind(x,y),cegs=ceg))*180./pi
  
  # From Julia: " fbr is the minimum fibre bend radius, fbr=45mm. If E<2fbr then the maximum configuration 
  #angle that should be set for that ferule is +/- arcsin(E/2fbr).
  #Implemented this limit on the fibre bend radius in the next 2 lines. Thsi will be applied regardless of the allowed delta_ang
  E=pointDistance(p1=ceg_pos[ceg,c('x','y')], p2=c(x,y), lonlat=FALSE) - (probe_l+tip_l/2.) ##E is the line joining the solid ferule end to the centre of the exit point.
  if (E < 2*fbr){
    new_delta_ang=asind(E/(2*fbr))
    if (new_delta_ang < delta_ang) delta_ang=new_delta_ang #Only apply correction if the angle is more restrictive.
  }
  
  minang=ang-delta_ang
  maxang=ang+delta_ang
  
  r0=fov/2
  if (guide){
    r1=gcable_l+gprobe_l+gtip_l
  }else{
    r1=cable_l+probe_l+tip_l
  }
  d=sqrt(x^2 + y^2)
  
  #Measure the angle corresponding to the shortest distance between the target and the edge of the field.
  #Runs between -90 and 270.
  azAng=(pi+atan2(y,x))/pi*180
  if (azAng > 270) azAng=azAng-360
  if (azAng < -90) azAng=azAng+360
  
  #If the probe can touch the edge of the field:
  if (d+r1 >= r0){
    a=(r0^2-r1^2+d^2)/(2*d)
    h=sqrt(r0^2-a^2)
    x1=a*x/d+h*(y)/d      
    y1=a*y/d-h*(x)/d       
    x2=a*x/d-h*(y)/d      
    y2=a*y/d+h*(x)/d
    
    ang1=atan2(y1-y,x1-x)/pi*180-180
    ang2=atan2(y2-y,x2-x)/pi*180-180
    
    ang1=adjust_angle(theta=ang1,fgang=ang)
    ang2=adjust_angle(theta=ang2,fgang=ang)
    
    elminang=min(ang1,ang2) #edge limit minimum angle
    elmaxang=max(ang1,ang2) #edge limit maximum angle
    
    if (guide) {
      dcegth=(gprobe_l+gcable_l+gtip_l/2.)
    } else {
      dcegth=(probe_l+cable_l+tip_l/2.)
    }
    
    #computing the extremes of the relevant cable exig gap.
    dx=as.numeric(wceg/2.* sind(ceg_pos[ceg,'deg']))
    dy=as.numeric(-1.*wceg/2.* cosd(ceg_pos[ceg,'deg']))
    p2=c(ceg_pos[ceg,'x']-dx, ceg_pos[ceg,'y']-dy)
    p3=c(ceg_pos[ceg,'x']+dx, ceg_pos[ceg,'y']+dy)
    
    #Computing distance between target (p1), and p2 and p3.
    d12=pointDistance(p1=c(x,y),p2=p2, lonlat=FALSE)
    d13=pointDistance(p1=c(x,y),p2=p3, lonlat=FALSE)
    
    #We have 2 cases:
    
    #Case 1: probe goes through the cable exit gap:
    #allow for maximal angle change across the gap, unless it is larger than delta_ang.
    #In this case, the angle range should include the initial ang guess.
    if ((d12 < dcegth) | (d13 < dcegth)) {
      
      #Computing angles range within the cable exit gap
      psi2=asind(cable_w/2./d12)
      psi3=asind(cable_w/2./d13)
      
      ang1=atan2d((y-p2[2]),(x-p2[1]))-psi2
      ang2=atan2d((y-p3[2]),(x-p3[1]))+psi3
      
      ang1=adjust_angle(theta=ang1,fgang=ang)
      ang2=adjust_angle(theta=ang2,fgang=ang)
      
      minang=min(ang1,ang2)
      maxang=max(ang1,ang2)
      
      # temp=min(ang1,ang2)
      # ang2=max(ang1,ang2)
      # ang1=temp
      # 
      # if (ang1 > elminang) {
      #   minang=ang1
      # }
      # if (ang2 < elmaxang) {
      #   maxang=ang2
      # }
    #Case 2: the probe does not go through the exit gap but may hit the side of the FoV.
    #Pick the angles range that contains the first guess ang..
    } else{
      if ((ang < elminang) & (ang < elmaxang)) {
        if (maxang > elminang) maxang=elminang
      } else if ((ang > elminang) & (ang > elmaxang)){
        if (minang < elmaxang) minang=elmaxang
      } else if ((ang > minang) & (ang < maxang)){
        if (minang < elminang) {
          minang = elminang
        } else if (maxang > elmaxang) {
          maxang = elmaxang
        } 
      }
    }
  }
  if (minang > maxang) print(paste('Houston, we have a problem... minang=', as.character(minang),' and maxang=',as.character(maxang)))
  return(c(minang,maxang))
}

#Wanna pick a 360 degree range that includes that first-guess angle (fgang).
adjust_angle <- function(theta,fgang){
  if (theta < fgang-180){
    theta=theta+360
  } else if (theta > fgang + 180){
    theta=theta-360
  }
  
  #fgang is the first_guess_ang.
  #Upper left, should use a range that is between -180 and 180.
  # if (fgang<0){
  #   if (theta < -180) theta=theta+360
  #   if (theta > 180) theta=theta-360
  # #Upper right use a range that is between 90 and 450.
  # } else if (fgang > 180){
  #   while (theta <90) theta=theta+360
  #   if (theta > 450) theta=theta-360
  # # Lower right
  # } else if (fgang >90 & fgang<180) {
  #   while (theta <0) theta=theta+360
  #   if (theta > 360) theta=theta-360
  # #Lower left should be between -90 and 270
  # } else {
  #   if (theta < -90) theta=theta+360
  #   if (theta > 270) theta=theta-360
  # }
  return(theta)
}

#############################################################################
#test code: delete when done with finalising the above.
#for (probe in TPs){
#angRange=computeAnglesRange(pos[probe,'x'],pos[probe,'y'], ceg=cegs[probe], ang=angs[probe], delta_ang=90)/180*pi
#ang_ranges[probe,]=angRange
#}
#draw_pos(x=pos[,'x'],y=pos[,'y'])
#defineprobe(xs=pos[,'x'],ys=pos[,'y'], angs=angs, interactive=TRUE)

#defineprobe(xs=pos[,'x'],ys=pos[,'y'], angs=ang_ranges[,1], interactive=TRUE, col='green')
#defineprobe(xs=pos[,'x'],ys=pos[,'y'], angs=ang_ranges[,2], interactive=TRUE, col='red')

#############################################################################
### Randomly allocate nprobes positions on 2 degree field of view for initial 
### simulation and code testing.Will need to test on real data (GAMA G23?) later
### to get clustering right..
#############################################################################
random_allocation <- function (nrandprobes=ngalprobes+6, nstdprobes=20, nguideprobes=20) {
  #nprobes is the number of positions. HECTOR only has nprobes, but also adding 6 spare ones.
  #fov is the field of view in degrees (global variable).
  #excl_radius is the exclusion radius in arcsec for each object (hexabundle head size, global variable).
  found=FALSE
  while (found==FALSE){
    r <- sqrt(runif(n=nrandprobes/2, min=0, max=fov/2.-excl_radius*2))
    r <- c(r,runif(n=nrandprobes/2, min=0, max=fov/4.-excl_radius*2))
    t <- 2*pi*runif(n=nrandprobes)
    x <- r*cos(t) ; y <- r*sin(t)
    test=as.data.frame(spDists(cbind(x,y)))
    if (length(test[test>0 & test<excl_radius*2.])==0) {found=TRUE}
  }
  
  sx=rep(NaN,n=nstdprobes)
  sy=rep(NaN,n=nstdprobes)
  found=FALSE
  i=1
  while (found==FALSE) {
    r <- sqrt(runif(n=1, min=0,max=fov/2.- excl_radius*2))
    t <- 2*pi*runif(n=1)
    sx[i] <- r*cos(t) ; sy[i] <- r*sin(t)
    test=as.data.frame(spDistsN1(pts=cbind(x,y),pt=c(sx[i],sy[i])))
    if (length(test[test<excl_radius*2.])==0) {
      i=i+1
      if (i>nstdprobes) {found=TRUE}
    }
  }
  
  gx=rep(NaN,n=nguideprobes)
  gy=rep(NaN,n=nguideprobes)
  found=FALSE
  i=1
  while (found==FALSE) {
    r <- sqrt(runif(n=1, min=0,max=fov/2.- gexcl_radius*2))
    t <- 2*pi*runif(n=1)
    gx[i] <- r*cos(t) ; gy[i] <- r*sin(t)
    test=as.data.frame(spDistsN1(pts=cbind(x,y),pt=c(gx[i],gy[i])))
    if (length(test[test<gexcl_radius*2.])==0) {
      i=i+1
      if (i>nguideprobes) {found=TRUE}
    }
  }
  
  return(list(pos=cbind(x,y), stdpos=cbind(x=sx,y=sy), guidepos=cbind(x=gx,y=gy)))
}

#Draw buttons around each position to be allocated.
draw_pos <- function(x,y, label_probe=TRUE, overwrite=TRUE){
  if (overwrite){ #set overwrite to true to start the plots from scratch.
    aplot(0,0,pch='x',col="red",xlim=c(-fov/2.,fov/2.),ylim=c(-fov/2.,fov/2.), asp=1, xlab='X (deg)', ylab='Y (deg)')
    draw.circle(0,0,radius=fov/2.,border='red')
    draw.circle(0,0,radius=1.,border='black')
    #Draw the cable exit gaps in cyan
    for (cceg in c(1:length(ceg_pos[,1]))){
      dx=wceg/2.* sind(ceg_pos[cceg,'deg'])
      dy=-1.*wceg/2.* cosd(ceg_pos[cceg,'deg'])
      lines(c(ceg_pos[cceg,'x']-dx,ceg_pos[cceg,'x']+dx),c(ceg_pos[cceg,'y']-dy,ceg_pos[cceg,'y']+dy), lwd=3, col='cyan')
    }
    text(as.character(c(1:3)), x=ceg_pos[,'x'],y=ceg_pos[,'y'])
  }
  for (i in c(1:length(x))){
    if (i<=ngalprobes){
      draw.circle(x[i],y[i],radius=excl_radius, col='lightblue')
    } else{
      draw.circle(x[i],y[i],radius=excl_radius, col='lightblue', lty=3)
    }
    if (label_probe){
      text(x[i],y[i],as.character(i), cex=0.5)
    }
  }
}

#Add buttons around potential standard stars:.
draw_stds <- function(xs,ys){
  for (i in c(1:length(xs))){
    draw.circle(xs[i],ys[i],radius=excl_radius, col='lightpink', lty=3)
    text(xs[i],ys[i],as.character(i), cex=0.5)
  }
}

#Add buttons around potential standard stars:.
draw_guides <- function(xg,yg){
  for (i in c(1:length(xg))){
    draw.circle(xg[i],yg[i],radius=gexcl_radius, col='yellow', lty=3)
  }
}

#Draw all pos:
draw_all <- function(x,y,xs,ys,xg,yg, interactive=TRUE){
  if (interactive){
    draw_pos(x,y)
    draw_guides(xg,yg)
    draw_stds(xs,ys)
  }
}

#This function computes the probe polygon shape defined by a series of data points. This function needs to remain flexible enough 
#to allow for changes in the probe shape as the HECTOR design evolves.
defineprobe <- function(xs,ys,angs,interactive=FALSE, col='black'){
  probes=c()
  for (i in c(1:length(xs))) {
    x=xs[i]
    y=ys[i]
    probe=cbind(c(x+tip_w/2., 
                  x-(tip_l-tip_w/2.),
                  x-(tip_l-tip_w/2.),
                  x-(tip_l-tip_w/2.)-probe_l, 
                  x-(tip_l-tip_w/2.)-probe_l, 
                  x-(tip_l-tip_w/2.)-probe_l-cable_l, 
                  x-(tip_l-tip_w/2.)-probe_l-cable_l, 
                  x-(tip_l-tip_w/2.)-probe_l, 
                  x-(tip_l-tip_w/2.)-probe_l, 
                  x-(tip_l-tip_w/2.), 
                  x-(tip_l-tip_w/2.), 
                  x+tip_w/2.),
                c(y+tip_w/2., 
                  y+tip_w/2., 
                  y+probe_w/2., 
                  y+probe_w/2., 
                  y+cable_w/2.,
                  y+cable_w/2.,
                  y-cable_w/2.,
                  y-cable_w/2.,
                  y-probe_w/2., 
                  y-probe_w/2., 
                  y-tip_w/2., 
                  y-tip_w/2.))
    probetemp=cbind(Rotate(probe,theta=angs[i], mx=x, my=y)$x, Rotate(probe,theta=angs[i], mx=x, my=y)$y)
    probe=probetemp
    colnames(probe)=c(paste('X',as.character(i), sep=''),paste('Y',as.character(i),sep=''))
    if (interactive) {polygon(probe, border=col)}
    assign(paste('probe',as.character(i), sep=''), probe)
    probes=cbind(probes,probe)
  }
  return(as.data.frame(probes))
}

#This function computes the guide probe polygon shape defined by a series of data points.
defineguideprobe <- function(gxs,gys,gangs,interactive=FALSE){
  gprobes=c()
  for (i in c(1:length(gxs))) {
    x=gxs[i]
    y=gys[i]
    gprobe=cbind(c(x+gtip_w/2., 
                   x-(gtip_l-gtip_w/2.),
                   x-(gtip_l-gtip_w/2.),
                   x-(gtip_l-gtip_w/2.)-gprobe_l, 
                   x-(gtip_l-gtip_w/2.)-gprobe_l, 
                   x-(gtip_l-gtip_w/2.)-gprobe_l-gcable_l, 
                   x-(gtip_l-gtip_w/2.)-gprobe_l-gcable_l, 
                   x-(gtip_l-gtip_w/2.)-gprobe_l, 
                   x-(gtip_l-gtip_w/2.)-gprobe_l, 
                   x-(gtip_l-gtip_w/2.), 
                   x-(gtip_l-gtip_w/2.), 
                   x+gtip_w/2.),
                 c(y+gtip_w/2., 
                   y+gtip_w/2., 
                   y+gprobe_w/2., 
                   y+gprobe_w/2., 
                   y+gcable_w/2.,
                   y+gcable_w/2.,
                   y-gcable_w/2.,
                   y-gcable_w/2.,
                   y-gprobe_w/2., 
                   y-gprobe_w/2., 
                   y-gtip_w/2., 
                   y-gtip_w/2.))
    gprobetemp=cbind(Rotate(gprobe,theta=gangs[i], mx=x, my=y)$x, Rotate(gprobe,theta=gangs[i], mx=x, my=y)$y)
    gprobe=gprobetemp
    colnames(gprobe)=c(paste('X',as.character(i), sep=''),paste('Y',as.character(i),sep=''))
    if (interactive) {polygon(gprobe, border='darkgrey')}
    assign(paste('gprobe',as.character(i), sep=''), gprobe)
    gprobes=cbind(gprobes,gprobe)
  }
  return(as.data.frame(gprobes))
}

#Check overlapping probes (This is a bit slow, could it be optimised?):
find_probe_conflicts <- function(probes, pos, angs){
  probe_conflicts=data.frame()
  if (dim(pos)[1] == 1) {
    return(probe_conflicts)
  }
  for (i in c(1:(length(pos[,'x'])-1))) {
    p1 <- data.frame(PID=rep(i, 12), POS=1:12, X=probes[,paste('X',as.character(i), sep='')], Y=probes[,paste('Y',as.character(i), sep='')])
    for (j in c((i+1):length(pos[,'x']))) {
      p2 <- data.frame(PID=rep(j, 12), POS=1:12, X=probes[,paste('X',as.character(j), sep='')], Y=probes[,paste('Y',as.character(j), sep='')])
      if (!is.null(joinPolys(p1,p2))) {
        junction=joinPolys(p1,p2)
        #Compute the percentage area of a probe that overlaps.100% means they overlap perfectly. 0% means no overlap.
        areapoly=round(polyarea(junction$X,junction$Y)/polyarea(p1$X,p1$Y) * 100, digits=1)
        probe_conflicts=rbind(probe_conflicts, c(i,j,areapoly))
      }
    }
  }
  nconflicts=dim(probe_conflicts)[1]
  if (nconflicts>0) {
    colnames(probe_conflicts)=c('Probe_1st','Probe_2nd','PercentArea')
  }
  return(as.data.frame(probe_conflicts))
}

#Looking for guide/target/standard conflicts with the target/standard probes fixed.
find_guide_conflicts <- function(probes, pos, angs, gprobes, gpos, gangs){
  probe_conflicts=data.frame()
  #First identify clashes with target/standard probes:
  for (i in c(1:(length(pos[,'x'])))) {
    p1 <- data.frame(PID=rep(i, 12), POS=1:12, X=probes[,paste('X',as.character(i), sep='')], Y=probes[,paste('Y',as.character(i), sep='')])
    for (j in 1:length(gpos[,'x'])) {
      p2 <- data.frame(PID=rep(j, 12), POS=1:12, X=gprobes[,paste('X',as.character(j), sep='')], Y=gprobes[,paste('Y',as.character(j), sep='')])
      if (!is.null(joinPolys(p1,p2))) {
        junction=joinPolys(p1,p2)
        #Compute the percentage area of a probe that overlaps.100% means they overlap perfectly. 0% means no overlap.
        areapoly=round(polyarea(junction$X,junction$Y)/polyarea(p1$X,p1$Y) * 100, digits=1)
        probe_conflicts=rbind(probe_conflicts, c(i,j,areapoly))
      }
    }
  }
  #Now identify clashes between guide probes.
  if (length(gpos[,'x']) > 1) { #Only if there is more than one guide probe.
    for (i in c(1:(length(gpos[,'x'])))) {
      p1 <- data.frame(PID=rep(i, 12), POS=1:12, X=gprobes[,paste('X',as.character(i), sep='')], Y=gprobes[,paste('Y',as.character(i), sep='')])
      for (j in c(1:length(gpos[,'x']))) { 
        if (j!=i){
          p2 <- data.frame(PID=rep(j, 12), POS=1:12, X=gprobes[,paste('X',as.character(j), sep='')], Y=gprobes[,paste('Y',as.character(j), sep='')])
          if (!is.null(joinPolys(p1,p2))) {
            junction=joinPolys(p1,p2)
            #Compute the percentage area of a probe that overlaps.100% means they overlap perfectly. 0% means no overlap.
            areapoly=round(polyarea(junction$X,junction$Y)/polyarea(p1$X,p1$Y) * 100, digits=1)
            probe_conflicts=rbind(probe_conflicts, c(i,j,areapoly))
          }
        }
      }
    }
  }
  nconflicts=dim(probe_conflicts)[1]
  if (nconflicts>0) {
    colnames(probe_conflicts)=c('Probe_1st','Probe_2nd','PercentArea')
  }
  return(as.data.frame(probe_conflicts))
}

#Determine the appropriate cable exit gap for a position by picking
#the exit from the direction with the fewest positions in front. This is 
#done by dividing the field into three using the position and the central points
#between the cable exit gaps and counting the number of targets within.
choose_cegs <- function(pos) {
  
  npos=length(pos[,1])
  cegs=rep(0,npos)
  
  #Visualising what's going on (may be deleted when done with editing this bit of code):
  #x=pos[1,'x']
  #y=pos[1,'y']
  #draw_pos(x=pos_master$pos[c(1:19),'x'],y=pos_master$pos[c(1:19),'y'])
  
  #lines(x=c(x,mid_ceg_pos[2,'x']), y=c(y,mid_ceg_pos[2,'y']))
  #lines(x=c(x,mid_ceg_pos[1,'x']), y=c(y,mid_ceg_pos[1,'y']))
  #lines(x=c(x,mid_ceg_pos[3,'x']), y=c(y,mid_ceg_pos[3,'y']))
  
  for (j in c(1:npos)){
    #Define area or "pizza slice" for each cable exit gap.
    pizza1 = data.frame(PID=rep(1, 122), POS=1:122, X=c(pos[j,'x'],cosd(c(-30:-150))), Y=c(pos[j,'y'],sind(c(-30:-150))))
    pizza2 = data.frame(PID=rep(2, 122), POS=1:122, X=c(pos[j,'x'],cosd(c(-30:90))), Y=c(pos[j,'y'],sind(c(-30:90))))
    pizza3 = data.frame(PID=rep(3, 122), POS=1:122, X=c(pos[j,'x'],cosd(c(90:210))), Y=c(pos[j,'y'],sind(c(90:210))))
    nceg=c(-1,-1,-1) #Starting at -1 to avoid counting the probe itself.
    for (i in c(1:npos)){
      cx=pos[i,'x']
      cy=pos[i,'y']
      #Approximating each position as a tiny square for this purpose only.
      polpos= data.frame(PID=rep(1, 4), POS=1:4, X=c(cx+excl_radius,cx, cx-excl_radius, cx), Y=c(cy,cy+excl_radius,cy,cy-excl_radius))
      if (!is.null(joinPolys(pizza1,polpos))) {
        nceg[1]=nceg[1]+1
      }
      if(!is.null(joinPolys(pizza2,polpos))){
        nceg[2]=nceg[2]+1
      } 
      if (!is.null(joinPolys(pizza3,polpos))){
        nceg[3]=nceg[3]+1
      }
    }
    #Need to account for when there are two identically good options.
    if (length(which(min(nceg)==nceg))==1){
      cegs[j]=which(min(nceg)==nceg)
    } else { #If two exits are equally good, pick the closest.
      cegs[j]=which(min(nceg)==nceg)[which.min(pointDistance(p1=ceg_pos[which(min(nceg)==nceg),c('x','y')],p2=c(cx,cy), lonlat=FALSE))]
    }
    #If position is too close to the exit, move them to next closest exit.
    E=pointDistance(p1=ceg_pos[cegs[j],c(‘x’,‘y’)], p2=pos[j,c(‘x’,‘y’)], lonlat=FALSE) - (probe_l+cable_l/3+tip_l/2.) ##E is the line joining the solid ferule end to the centre of the exit point.
    if (E < E0){ #E0 is the threshold to avoid blocking a cable exit gap. It is set above.
      cegs[j]=order(pointDistance(p1=ceg_pos[,c(‘x’,‘y’)],p2=pos[j,c(‘x’,‘y’)], lonlat=FALSE))[2]
    }
  }
  return(cegs)
  
}

#Initial guess of the angles. Should point to the appropriate exit.
first_guess_angs <- function(pos, cegs, guide=FALSE){ 
  
  nprobes=length(pos[,'x'])
  
  angs=rep(0,nprobes)
  
  for (probe in c(1:nprobes)){
    angs[probe]=atan2(ceg_pos[cegs[probe],'y'] - pos[probe,'y'], ceg_pos[cegs[probe],'x'] - pos[probe,'x']) + pi
    if (angs[probe] > 2*pi) angs[probe]=angs[probe]-2.*pi
  }
  
  #Determine probes near the edge of the field that cannot have the initial guess angle.
  if (guide){
    mindist=sqrt((gcable_l+gprobe_l+gtip_l-gtip_w/2.)^2 + (gcable_w/2.)^2)
  }else{
    mindist=sqrt((cable_l+probe_l+tip_l-tip_w/2.)^2 + (cable_w/2.)^2)
  }
  
  #Computing the allowed range... this has changed in v2.0
  #for (probe in c(1:nprobes)){
    #dist=(fov/2.)-sqrt(pos[probe,'y']**2+pos[probe,'x']**2)
    #if (dist < mindist){
    #  arange=computeAnglesRange(x=pos[probe,'x'],y=pos[probe,'y'], ceg=cegs[probe], guide=guide)
    #  if (angs[probe]/pi*180 < min(arange)) angs[probe]=min(arange/180*pi)
    #  else if ((angs[probe]/pi*180 > max(arange))) angs[probe]=max(arange/180*pi)
    #  # else do nothing, you're in a cable gap exit, so just keep pointing to the middle.
    #}
  #}
  
  #testing:
  #draw_pos(x=pos[,'x'],y=pos[,'y'])
  #defineprobe(xs=pos[,'x'],ys=pos[,'y'],angs=angs, interactive=T)
  return(angs)
}

MinimizeConflicts <- function(pos=pos, angs=angs, cegs=cegs, delta_ang=20, probe_conflicts=conflicts){
  
  #Find all conflicted probes:
  CPs=unique(c(probe_conflicts[,1],probe_conflicts[,2]))
  #Identify all probes within reach of the conflicted probes as a list of probes to tweak.
  TPs= CPs #List of tweakable probes:
  for (probe in CPs) {
    dists=sqrt( (pos[,'x']- pos[probe,'x'])^2 + (pos[,'y']- pos[probe,'y'])^2)
    newTPs=which(dists < (probe_l/3))
    TPs=c(TPs,newTPs)
  }
  TPs=unique(TPs)
  
  ang_ranges=cbind(angs,angs)
  for (probe in TPs){
    angRange=computeAnglesRange(x=pos[probe,'x'],y=pos[probe,'y'], ceg=cegs[probe], delta_ang=delta_ang)/180*pi
    ang_ranges[probe,]=angRange
  }
  
  draw_all(x=pos[,'x'],y=pos[,'y'],xs=pos_master$stdpos[,'x'],ys=pos_master$stdpos[,'y'],xg=pos_master$guidepos[,'x'],yg=pos_master$guidepos[,'y'], interactive=visualise)
  probes=defineprobe(x=pos[,'x'],y=pos[,'y'],angs=angs, interactive=visualise)
  
  nit=length(TPs)*500
  angs_tries=c()
  for (probe in c(1:nprobes)){
    angs_tries= rbind(angs_tries,runif(n=nit,min=ang_ranges[probe,1],max=ang_ranges[probe,2]))
  }
  
  angs_tries=as.data.frame(angs_tries)
  #View(angs_tries)
  angs_tries[nprobes+1,]=1000
  areamin=1000
  for (i in c(1:nit)){
    angs_tries[nprobes+1,i]=compute_total_conflict_area(angs=angs_tries[c(1:nprobes),i],pos=pos)
    if (angs_tries[nprobes+1,i] < areamin){
      areamin=angs_tries[nprobes+1,i]
      angsmin=angs_tries[c(1:nprobes),i]
      #print(areamin)
      angsmin[angsmin>3*pi/2]=angsmin[angsmin>3*pi/2]-2*pi
      probes=defineprobe(x=pos[,'x'],y=pos[,'y'],angs=angsmin, interactive=FALSE)
      if (areamin<=0) {return(angsmin)}
    }
  }
  angsmin[angsmin>3*pi/2]=angsmin[angsmin>3*pi/2]-2*pi
  angsmin <<- angsmin
  
  draw_all(x=pos[,'x'],y=pos[,'y'],xs=pos_master$stdpos[,'x'],ys=pos_master$stdpos[,'y'],xg=pos_master$guidepos[,'x'],yg=pos_master$guidepos[,'y'], interactive=visualise)
  probes=defineprobe(x=pos[,'x'],y=pos[,'y'],angs=angsmin, interactive=visualise)
  
  #Identify remaining conflicts.
  conflicts=find_probe_conflicts(probes=probes, pos=pos, angs=angsmin)
  nconflicts=dim(conflicts)[1]
  
  #Try expanding the angles of probes still conflicted.
  if (nconflicts>0){
    #Recurrently run but first fix probes that are now no longer conflicted...
    angsmin <<- MinimizeConflicts(pos=pos, cegs=cegs, delta_ang=delta_ang, angs=angsmin, probe_conflicts=conflicts)
  }
  
  draw_all(x=pos[,'x'],y=pos[,'y'],xs=pos_master$stdpos[,'x'],ys=pos_master$stdpos[,'y'],xg=pos_master$guidepos[,'x'],yg=pos_master$guidepos[,'y'], interactive=visualise)
  probes=defineprobe(x=pos[,'x'],y=pos[,'y'],angs=angsmin, interactive=visualise)
  
  angs=angsmin
  return(angs)
}

wrapMinimizeConflicts <- function(pos=pos, angs=angs, cegs=cegs, init_delta_ang=20, probe_conflicts=conflicts){
  print('Wrapping around MinimizeConflicts')
  conflicts=probe_conflicts
  nconflicts=dim(conflicts)[1]
  
  #Look for a solution.
  i=1 
  while ((nconflicts > 0) & (i<4)){ #We will wrap around this look at most 3 times.
    #Start by allowing probes to move by the initial delta_ang.
    
    delta_ang=init_delta_ang
    print(paste('Starting (again) with delta_ang =', as.character(delta_ang)))
    
    newangs=angs 
    withTimeout(assign('newangs',MinimizeConflicts(pos=pos, angs=angs, cegs=cegs, delta_ang=delta_ang, probe_conflicts=conflicts)), timeout=timeout, onTimeout='warning')
    if (FALSE %in% (angs==newangs)){
      angs=newangs
    } else {
      angs=angsmin
    }
    draw_all(x=pos_master$pos[,'x'],y=pos_master$pos[,'y'],xs=pos_master$stdpos[,'x'],ys=pos_master$stdpos[,'y'],xg=pos_master$guidepos[,'x'],yg=pos_master$guidepos[,'y'], interactive=visualise)
    probes=defineprobe(x=pos[,'x'],y=pos[,'y'],angs=angs, interactive=visualise)
    #Check if could configure:
    conflicts=find_probe_conflicts(probes=probes, pos=pos, angs=angs)
    nconflicts=dim(conflicts)[1]

    #Trying swapping the cable exit gap first as this often resolves conflicts and can speed up the process..
    if (nconflicts > 0) {
      #Trying to swap some cable exit gaps first
      nceg=try_next_ceg(pos=pos, angs=angsmin, cegs=cegs, probe_conflicts=conflicts)
      cegs=nceg$cegs
      angs=nceg$angs
      newangs=angs 
      
      draw_all(x=pos_master$pos[,'x'],y=pos_master$pos[,'y'],xs=pos_master$stdpos[,'x'],ys=pos_master$stdpos[,'y'],xg=pos_master$guidepos[,'x'],yg=pos_master$guidepos[,'y'], interactive=visualise)
      probes=defineprobe(x=pos[,'x'],y=pos[,'y'],angs=angs, interactive=visualise)
      
      conflicts=find_probe_conflicts(probes=probes, pos=pos, angs=angs)
      nconflicts=dim(conflicts)[1]
      if (nconflicts >0){
        withTimeout(assign('newangs',MinimizeConflicts(pos=pos, angs=angs, cegs=cegs, delta_ang=delta_ang, probe_conflicts=conflicts)), timeout=timeout, onTimeout='warning')
      }
      if (FALSE %in% (angs==newangs)){
        angs=newangs
      } else {
        angs=angsmin
      }
      draw_all(x=pos_master$pos[,'x'],y=pos_master$pos[,'y'],xs=pos_master$stdpos[,'x'],ys=pos_master$stdpos[,'y'],xg=pos_master$guidepos[,'x'],yg=pos_master$guidepos[,'y'], interactive=visualise)
      probes=defineprobe(x=pos[,'x'],y=pos[,'y'],angs=angs, interactive=visualise)
      #Check if could configure:
      conflicts=find_probe_conflicts(probes=probes, pos=pos, angs=angs)
      nconflicts=dim(conflicts)[1]
    }
    
    if (nconflicts > 0) {
      print('Could not find a solution with delta_ang=20, trying delta_ang=45')
      delta_ang=45
      newangs=angs
      withTimeout(assign('newangs',MinimizeConflicts(pos=pos, angs=angs, cegs=cegs, delta_ang=delta_ang, probe_conflicts=conflicts)), timeout=timeout, onTimeout='warning')
      if (FALSE %in% (angs==newangs)){
        angs=newangs
      } else {
        angs=angsmin #If it timed out, use global angsmin.
      }
      
      draw_all(x=pos_master$pos[,'x'],y=pos_master$pos[,'y'],xs=pos_master$stdpos[,'x'],ys=pos_master$stdpos[,'y'],xg=pos_master$guidepos[,'x'],yg=pos_master$guidepos[,'y'], interactive=visualise)
      probes=defineprobe(x=pos[,'x'],y=pos[,'y'],angs=angs, interactive=visualise)
      #Check if could configure:
      conflicts=find_probe_conflicts(probes=probes, pos=pos, angs=angs)
      nconflicts=dim(conflicts)[1]
    }
    
    if (nconflicts > 0) {
      print('Could not find a solution with delta_ang=45, trying delta_ang=90')
      delta_ang=90
      newangs=angs
      withTimeout(assign('newangs',MinimizeConflicts(pos=pos, angs=angs, cegs=cegs, delta_ang=delta_ang, probe_conflicts=conflicts)), timeout=timeout, onTimeout='warning')
      if (FALSE %in% (angs==newangs)){
        angs=newangs
      } else {
        angs=angsmin
      }
      
      draw_all(x=pos_master$pos[,'x'],y=pos_master$pos[,'y'],xs=pos_master$stdpos[,'x'],ys=pos_master$stdpos[,'y'],xg=pos_master$guidepos[,'x'],yg=pos_master$guidepos[,'y'], interactive=visualise)
      probes=defineprobe(x=pos[,'x'],y=pos[,'y'],angs=angs, interactive=visualise)
      #Check if could configure:
      conflicts=find_probe_conflicts(probes=probes, pos=pos, angs=angs)
      nconflicts=dim(conflicts)[1]
    }
    
    if (nconflicts > 0) {
      print('Could not find a solution with delta_ang=90, trying to swap some cable exit gaps and delta_ang=20')
      nceg=try_next_ceg(pos=pos, angs=angsmin, cegs=cegs, probe_conflicts=conflicts)
      cegs=nceg$cegs
      angs=nceg$angs
      
      draw_all(x=pos_master$pos[,'x'],y=pos_master$pos[,'y'],xs=pos_master$stdpos[,'x'],ys=pos_master$stdpos[,'y'],xg=pos_master$guidepos[,'x'],yg=pos_master$guidepos[,'y'], interactive=visualise)
      probes=defineprobe(x=pos[,'x'],y=pos[,'y'],angs=angs, interactive=visualise)
      
      conflicts=find_probe_conflicts(probes=probes, pos=pos, angs=angs)
      nconflicts=dim(conflicts)[1]
    }
    i=i+1
  }
  return(angs)
}

MinimizeGuideConflicts <- function(pos=pos, angs=angs, cegs=cegs, gpos=gpos, gangs=gangs, gcegs=gcegs, delta_ang=20, guide_conflicts){
  
  #Find conflicted guide probe:
  CGPs=unique(guide_conflicts[,2])
  print('Minimizing conflicts with guide probe(s):')
  print(as.character(CGPs))
  
  gang_ranges=cbind(gangs,gangs)
  for (i in CGPs){
    gang_ranges[i,] = computeAnglesRange(x=gpos[i,'x'], y=gpos[i,'y'], ceg=gcegs[i], delta_ang=delta_ang, guide=TRUE) / 180*pi
  }
  
  draw_all(x=pos[,'x'],y=pos[,'y'],xs=pos_master$stdpos[,'x'],ys=pos_master$stdpos[,'y'],xg=pos_master$guidepos[,'x'],yg=pos_master$guidepos[,'y'], interactive=visualise)
  probes=defineprobe(x=pos[,'x'],y=pos[,'y'],angs=angs, interactive=visualise)
  gprobes=defineguideprobe(gxs=gpos[,'x'],gys=gpos[,'y'],gangs=gangs, interactive=visualise)
  
  nit=length(CGPs)*50
  gangs_tries=c()
  for (probe in c(1:length(gangs))){
    if (probe %in% CGPs){
      gangs_tries = rbind(gangs_tries,runif(n=nit,min=gang_ranges[probe,1],max=gang_ranges[probe,2]))
    }else{
      gangs_tries = rbind(gangs_tries,rep(gangs[probe],times=nit))
    }
  }
  gangs_tries=as.data.frame(gangs_tries)
  
  for (i in c(1:nit)){
    tempgangs=gangs_tries[,i]
    
    #draw_all(x=pos[,'x'],y=pos[,'y'],xs=pos_master$stdpos[,'x'],ys=pos_master$stdpos[,'y'],xg=gpos[,'x'],yg=gpos[,'y'], interactive=visualise)
    probes=defineprobe(x=pos[,'x'],y=pos[,'y'],angs=angs, interactive=FALSE)
    gprobes=defineguideprobe(gxs=gpos[,'x'],gys=gpos[,'y'],gangs=tempgangs, interactive=FALSE)
    
    tempconflicts=find_guide_conflicts(probes=probes, pos=pos,angs=angs,gprobes=gprobes, gpos=gpos, gangs=tempgangs)
    nconflicts=dim(tempconflicts)[1]
    if (nconflicts <=0){
      return(tempgangs)
    }
  }
  
  print('Could not configure this guide probe. Will try the next one in the list.')
  
  return(gangs)
}

compute_total_conflict_area <- function(angs,pos=pos){
  probes=defineprobe(x=pos[,'x'],y=pos[,'y'],angs=angs, interactive=FALSE)
  sprobes=data.frame()
  for (i in c(1:(length(pos[,'x'])))) {
    sprobes <- rbind(sprobes,cbind(PID=rep(i, 12), POS=1:12, X=probes[,paste('X',as.character(i), sep='')], Y=probes[,paste('Y',as.character(i), sep='')]))
  }
  sprobes=as.PolySet(sprobes)
  junction=joinPolys(sprobes,sprobes,operation='INT')
  nconflicts=(length(unique(junction[,'PID']))-nprobes)/2
  
  area=c()
  for (i in unique(junction[,'PID'])){
    area=c(area,polyarea(junction[junction[,'PID']==i,'X'],junction[junction[,'PID']==i,'Y']))
  }
  total_area=sum(area)-nprobes*Mode(area)
  
  return(total_area)
}

#Removing standard stars that are conflicting with a probe within the exclusion radius.
cleanconflictedstds <- function(angs,pos=pos, stdpos=pos_master$stdpos){
  
  draw_all(x=pos_master$pos[,'x'],y=pos_master$pos[,'y'],xs=stdpos[,'x'],ys=stdpos[,'y'],xg=pos_master$guidepos[,'x'],yg=pos_master$guidepos[,'y'], interactive=visualise)
  
  probes=defineprobe(x=pos[,'x'],y=pos[,'y'],angs=angs, interactive=visualise)
  
  #Setting up into sp objects that package rgeos will understand.
  sprobes=data.frame()
  for (i in c(1:ngalprobes)) {
    sprobes <- rbind(sprobes,cbind(PID=rep(i, 12), POS=1:12, X=probes[,paste('X',as.character(i), sep='')], Y=probes[,paste('Y',as.character(i), sep='')]))
  }
  sprobes=as.PolySet(sprobes)
  for (j in c(1:ngalprobes)) {
    assign(paste('cprobe',as.character(j),sep=''),Polygon(coords=as.matrix(sprobes[sprobes[,'PID']==j,c('X','Y')]), hole=FALSE))
  }
  cprobes=Polygons(srl=mget(paste('cprobe',as.character(c(1:ngalprobes)),sep='')), ID='meh')
  pols=SpatialPolygons(Srl=list(cprobes)) #sp object for the probes as spatial polygons.
  spts = SpatialPoints(coords=as.matrix(pos_master$stdpos[,c('x','y')])) #sp objects for the standard stars as simple points.
  mindists=apply(gDistance(spts, pols, byid=TRUE),2,min) #The minimum distance to the nearest probe for each standard.
  
  clstdpos=stdpos[order(mindists, decreasing=TRUE),] #Ordering standards according to their distance from the closest probe.
  mindists=mindists[order(mindists, decreasing=TRUE)]
  clstdpos=clstdpos[mindists > excl_radius,] #Removing standards that overlap with nearest probe.
  
  return(clstdpos)
}

#Removing guide stars that are conflicting with a probe within the exclusion radius.
cleanconflictedguides <- function(angs,pos=pos, guidepos=pos_master$guidepos){
  
  draw_all(x=pos_master$pos[,'x'],y=pos_master$pos[,'y'],xs=pos_master$stdpos[,'x'],ys=pos_master$stdpos[,'y'],xg=pos_master$guidepos[,'x'],yg=pos_master$guidepos[,'y'], interactive=visualise)
  
  probes=defineprobe(x=pos[,'x'],y=pos[,'y'],angs=angs, interactive=visualise)
  
  #Setting up into sp objects that package rgeos will understand.
  sprobes=data.frame()
  for (i in c(1:(ngalprobes+nstdprobes))) {
    sprobes <- rbind(sprobes,cbind(PID=rep(i, 12), POS=1:12, X=probes[,paste('X',as.character(i), sep='')], Y=probes[,paste('Y',as.character(i), sep='')]))
  }
  sprobes=as.PolySet(sprobes)
  for (j in c(1:(ngalprobes+nstdprobes))) {
    assign(paste('cprobe',as.character(j),sep=''),Polygon(coords=as.matrix(sprobes[sprobes[,'PID']==j,c('X','Y')]), hole=FALSE))
  }
  cprobes=Polygons(srl=mget(paste('cprobe',as.character(c(1:(ngalprobes+nstdprobes))),sep='')), ID='meh')
  pols=SpatialPolygons(Srl=list(cprobes)) #sp object for the probes as spatial polygons.
  spts = SpatialPoints(coords=as.matrix(pos_master$guidepos[,c('x','y')])) #sp objects for the guide stars as simple points.
  mindists=apply(gDistance(spts, pols, byid=TRUE),2,min) #The minimum distance to the nearest probe for each guide.
  
  clguidepos=guidepos[order(mindists, decreasing=TRUE),] #Ordering guides according to their distance from the closest probe.
  mindists=mindists[order(mindists, decreasing=TRUE)]
  clguidepos=clguidepos[mindists > gexcl_radius,] #Removing guides that overlap with nearest probe.
  
  #Reordering guides by distance from the centre:
  dist2cen=sqrt(clguidepos[,'x']^2+clguidepos[,'y']^2)
  #removing guide probes that are very close to the edge of the field.
  clguidepos=clguidepos[which(dist2cen < (fov/2. - 1.5* excl_radius)),]
  clguidepos=clguidepos[order(dist2cen, decreasing=TRUE),]
  
  #Dividing into quadrants:
  first_q=clguidepos[clguidepos[,'x']<0 & clguidepos[,'y']<0,]
  print(first_q)
  second_q=clguidepos[clguidepos[,'x']<0 & clguidepos[,'y']>=0,]
  print(second_q)
  third_q=clguidepos[clguidepos[,'x']>=0 & clguidepos[,'y']>=0,]
  print(third_q)
  fourth_q=clguidepos[clguidepos[,'x']>=0 & clguidepos[,'y']<0,]
  print(fourth_q)
  
  oclguidepos=data.frame(x=double(),y=double()) #ordered and cleaned guide positions.
  
  #Cycling through by quadrant to ensure guides from all corners of the plate get prioritised:
  gui=1
  while ((length(oclguidepos[,1]) < length(clguidepos[,1])) & gui < length(clguidepos[,1])){
    if (length(as.matrix(first_q))>2){
      try(assign('oclguidepos',rbind(oclguidepos, first_q[gui,])), TRUE)
    } else{
      try(assign('oclguidepos',rbind(oclguidepos, as.vector(first_q))), TRUE)
    }
    if (length(as.matrix(second_q))>2){
      try(assign('oclguidepos',rbind(oclguidepos, second_q[gui,])), TRUE)
    } else {
      try(assign('oclguidepos',rbind(oclguidepos, as.vector(second_q))), TRUE)
    }
    if (length(as.matrix(third_q))>2){
      try(assign('oclguidepos',rbind(oclguidepos, third_q[gui,])), TRUE)
    } else {
      try(assign('oclguidepos',rbind(oclguidepos, as.vector(third_q))), TRUE)
    }
    if (length(as.matrix(fourth_q))>2){
      try(assign('oclguidepos',rbind(oclguidepos, fourth_q[gui,])), TRUE)
    } else{
      try(assign('oclguidepos',rbind(oclguidepos, as.vector(fourth_q))), TRUE)
    }
    gui=gui+1
  }
  colnames(oclguidepos)=c('x','y')
  
  return(oclguidepos)
}

Mode <- function(x) {
  ux <- unique(x)
  ux[which.max(tabulate(match(x, ux)))]
}

probe_density <- function(pos=pos) {
  nneighbours=rep(0,ngalprobes)
  #Counting the number of neighbours within the total probe length:
  for (probe in c(1:ngalprobes)){
    dists=sqrt( (pos[,'x']- pos[probe,'x'])^2 + (pos[,'y']- pos[probe,'y'])^2)
    nneighbours[probe]=length(which(dists < 3.*excl_radius))
  }
  return(nneighbours)
}

try_next_ceg <- function(pos, cegs, angs, probe_conflicts=conflicts){
  
  print('Trying to swap the cable exit gap for some probes to see if that helps.')
  #Identify clusters of conflicted probes.
  nconflicts=length(probe_conflicts[,1])
  for (i in c(1:nconflicts)) {
    if (i==1){
      CP_clusters=list(as.numeric(probe_conflicts[i,c(1:2)]))
    }else{
      found=FALSE
      ncl=length(CP_clusters)
      for (cl in c(1:ncl)){
        cCPs=as.numeric(probe_conflicts[i,c(1:2)])
        #If any of the conflicted probes in the conflict is in existing cluster, add them to the cluster.
        if ((cCPs[1] %in% CP_clusters[[cl]]) | (cCPs[2] %in% CP_clusters[[cl]])){
          CP_clusters[[cl]]=unique(c(as.numeric(probe_conflicts[i,c(1:2)]), CP_clusters[[cl]]))
          found=TRUE
        }
      }
      if (!found) { # else, create a new cluster.
        CP_clusters[[ncl+1]]=as.numeric(probe_conflicts[i,c(1:2)])
      }
    }
  }
  
  #Identify clusters of conflicted probes pointing to the same exit and change the ceg for the furthest probe
  # if allowed, otherwise try second furthest probe. If probes pointing at different exit gaps, try swap one.
  cegs_new=cegs
  angs_new=angs
  
  ncl=length(CP_clusters)
  for (cl in c(1:ncl)){
    cCPs=CP_clusters[[cl]]
    if (length(cCPs)-length(unique(cegs[cCPs])) > 0) { #if more than 2 in the cluster point to the same exit gap
      cceg=Mode(cegs[cCPs])
      cCPs=cCPs[cegs[cCPs]==cceg] #focusing on probes in common cable exit gap.
      
      d=pointDistance(p1=pos[cCPs,c('x','y')],p2=ceg_pos[cegs[cCPs][1],c('x','y')], lonlat=FALSE)
      
      #identify probe furthest from the common cable exit gap to swap to second closest cable exit gap:
      toswap=cCPs[which(d==max(d))][1]
    } else { #If all conflicted probes in cluster pointing to different exit gaps, move the probe with the most limited ang_ranges
      angRanges=c()
      for (i in cCPs){
        angRanges=c(angRanges, abs(diff(computeAnglesRange(x=pos[i,'x'],y=pos[i,'y'],ceg=cegs[i]))))
      }
      toswap=cCPs[which(angRanges==min(angRanges))][1]
    }
    d2cegs=pointDistance(p1=ceg_pos[,c('x','y')], p2=pos[toswap,c('x','y')], lonlat=FALSE)
    if ((cegs_new[toswap] == order(d2cegs)[2]) & (d2cegs[order(d2cegs)[1]] > (probe_l+tip_l))) {
      cegs_new[toswap]=order(d2cegs)[1]
    } else{
      cegs_new[toswap]=order(d2cegs)[2]
    }
    #angs_new[toswap]=first_guess_angs(pos=pos,cegs=cegs_new)[toswap]
    #Better to reset all other angles.
    angs_new=first_guess_angs(pos=pos,cegs=cegs_new)
  }
  angs_new[angs_new>3*pi/2]=angs_new[angs_new>3*pi/2]-2*pi
  return(list(cegs=cegs_new, angs=angs_new))
}

try_swaps <- function(pos_master=pos_master){
  if (length(pos_master$pos[,1])<=dngalprobes){
    print('WARNING: There is no spare target to swap in this field.')
    #* SAM- added in pos= and angs= here, as otherwise we can't read the output list and fail with a really obscure error
    #* I have no idea why temp$pos gives NA rather than an error if temp has no attribute pos. REALLY HARD TO DEBUG!!!
    return(list(pos=pos_master$pos, angs=first_guess_angs(pos=pos_master$pos, cegs=choose_cegs(pos_master$pos))))
  }
  #Computing the probe density.
  nspare=length(pos_master$pos[,1])-ngalprobes
  pos=pos_master$pos[1:ngalprobes,]
  pd=probe_density(pos=pos)
  if (nspare<=3) {
    pos2swap=order(pd,decreasing=TRUE)[1:3]
  } else{
    pos2swap=order(pd,decreasing=TRUE)[1:nspare]
  }
  for (p in pos2swap) {
    print(paste('Swapping probe ', as.character(p)))
    for (i in c(1:nspare)){
      print(paste('with spare ',as.character(i)))
      cpos=pos
      cpos[p,]=pos_master$pos[ngalprobes+i,]
      cegs=choose_cegs(pos=cpos)
      angs=first_guess_angs(pos=cpos, cegs=cegs)
      probes=defineprobe(x=cpos[,'x'],y=cpos[,'y'],angs=angs, interactive=FALSE)
      
      print('Identifying initial conflicts')
      conflicts=find_probe_conflicts(probes=probes, pos=cpos, angs=angs)
      nconflicts=dim(conflicts)[1]
      if (nconflicts <1) {
        print('Bam! No initial conflicts, this one is done!')
        return(list(pos=cpos,angs=angs))
      }
      print('Minimizing the overlaping area by rotating probes')
      
      #Try top priority targets first and then try swapping most conflicted probe(s) in turn.
      angs_new=angs
      angs_new=wrapMinimizeConflicts(pos=cpos, cegs=cegs, angs=angs, probe_conflicts=conflicts)
      probes=defineprobe(x=cpos[,'x'],y=cpos[,'y'],angs=angs_new, interactive=FALSE)
      
      #Check if could configure:
      conflicts=find_probe_conflicts(probes=probes, pos=cpos, angs=angs_new)
      nconflicts=dim(conflicts)[1]
      
      if (nconflicts >=1) {
      } else{
        print('Woot woot! I managed to find a solution!')
        return(list(pos=cpos,angs=angs_new))
      }
    }
  }
  return(list(pos=cpos,angs=angs_new))
}


#Configuring the standard stars.
configure_stdstars <- function(pos, angs, cegs, stdpos){
  
  print("Adding some standards.")
  
  #Removing the standards that overlap with allocated probes.
  clstdpos=cleanconflictedstds(angs=angs, pos=pos, stdpos=stdpos)
  
  #* Added by Sam- this seems to fix the case where cleanconflictedstds returns a list with NAs in it
  clstdpos=clstdpos[!is.na(clstdpos)[,1],]
  
  if (lengths(clstdpos)[1] < nstdprobes) {
    print(paste('**Seem to only have ', lengths(clstdpos)[1], ' un-clashing standard stars in this field! Exiting and trying again...', sep=''))
    stdflag='StdFail'
    #* Added by Sam so we can catch when we error and when we just can't configure
    quit(status=10)
  }
  
  draw_all(x=pos_master$pos[,'x'],y=pos_master$pos[,'y'],xs=clstdpos[,'x'],ys=clstdpos[,'y'],xg=pos_master$guidepos[,'x'],yg=pos_master$guidepos[,'y'], interactive=visualise)
  probes=defineprobe(x=pos[,'x'],y=pos[,'y'],angs=angs, interactive=visualise)
  
  #Adding two standards to the probes:
  cs=0 #This will keep track of the configured standards so far.
  std=1 #This will cycle through all available standard stars.
  newpos=pos
  newangs=angs
  newcegs=cegs
  stdflag='none'
  
  while (cs < nstdprobes & std <= length(clstdpos[,1])) {
    nprobes <<- ngalprobes+cs+1
    tempos = rbind(newpos,clstdpos[std,])
    newceg=choose_cegs(pos=tempos)[length(tempos[,'x'])]
    tempcegs=c(newcegs,newceg)
    guessang = first_guess_angs(pos=tempos, cegs=tempcegs, guide=FALSE)[length(newangs)+1]
    tempangs = c(newangs,guessang)
    temprobes = defineprobe(x=tempos[,'x'], y=tempos[,'y'], angs=tempangs, interactive=FALSE)
    tempconflicts = find_probe_conflicts(probes=temprobes, pos=tempos, angs=tempangs)
    nconflicts = dim(tempconflicts)[1]
    
    if (nconflicts > 0) {
      print(tempos)
      #print(tempangs)
      print(tempconflicts)
      withTimeout(expr=assign('tempangs',MinimizeConflicts(pos=tempos, angs=tempangs, cegs=tempcegs, probe_conflicts=tempconflicts)), timeout=timeout, onTimeout='warning')

      draw_all(x=pos_master$pos[,'x'],y=pos_master$pos[,'y'],xs=clstdpos[,'x'],ys=clstdpos[,'y'],xg=pos_master$guidepos[,'x'],yg=pos_master$guidepos[,'y'], interactive=visualise)
      temprobes = defineprobe(x=tempos[,'x'],y=tempos[,'y'],angs=tempangs, interactive=visualise)
      
      tempconflicts = find_probe_conflicts(probes=temprobes, pos=tempos, angs=newangs)
      nconflicts = dim(tempconflicts)[1]
      if (nconflicts > 0){
      }else{
        print(paste0('standard ', as.character(std), ' configured successfully.'))
        newpos=tempos
        newangs=tempangs
        newcegs=tempcegs
        cs = cs+1
        nprobes <<- ngalprobes+cs
      }
    } else{
      print(paste0('standard ', as.character(std), ' configured successfully.'))
      newpos=tempos
      newangs=tempangs
      newcegs=tempcegs
      cs = cs+1
      nprobes <<- ngalprobes+cs
    }
    std = std+1
  }
  if (cs < nstdprobes) {
    print(paste('**Could only configure ',as.character(cs), ' standard stars in this field. Boo! :-('), sep='')
    stdflag='StdFail'
  }
  print(list(pos=newpos,angs=newangs, cegs=newcegs, stdflag=stdflag))
  return(list(pos=newpos,angs=newangs,cegs=newcegs, stdflag=stdflag))
}

#Configuring the guide stars.
configure_guidestars <- function(pos, angs, cegs, guidepos){ #In this case, pos must also include the 2 configured standards.
  
  #Removing the standards that overlap with allocated probes.
  clguidepos=cleanconflictedguides(angs=angs, pos=pos, guidepos=guidepos)
  
  #* Added by Sam- this seems to fix the case where cleanconflictedguides returns a list with NAs in it
  clguidepos=clguidepos[!is.na(clguidepos)[,1],]
  if (lengths(clguidepos)[1] < 3) {
    print(paste('**Seem to only have ', lengths(clguidepos)[1], ' un-clashing guide stars in this field! Exiting and trying again...', sep=''))
    stdflag='GuideFail'
    #* Added by Sam so we can catch when we error and when we just can't configure
    quit(status=10)
  }
  
  #Drawing and defining configured hector probes.
  draw_all(x=pos_master$pos[,'x'],y=pos_master$pos[,'y'],xs=pos_master$stdpos[,'x'],ys=pos_master$stdpos[,'y'],xg=clguidepos[,'x'],yg=clguidepos[,'y'], interactive=visualise)
  probes=defineprobe(x=pos[,'x'],y=pos[,'y'],angs=angs, interactive=visualise)
  
  #Adding 3-6 guides probes:
  cg=0 #This will keep track of the number of configured guides so far.
  sel_guides=c() #This keeps track of selectec guide star probe numbers.
  gui=1 #This will cycle through all available guide stars.
  tempgangs=c()
  newgangs=c()
  newgpos=cbind(c(),c())
  ngprobes <<- 1 #Starting with one probe.
  guideflag=''
  
  while (cg < ngprobesmax & gui <= length(clguidepos[,1])) {
    tempgpos=rbind(newgpos,clguidepos[gui,])
    
    #Redraw:
    draw_all(x=pos_master$pos[,'x'],y=pos_master$pos[,'y'],xs=pos_master$stdpos[,'x'],ys=pos_master$stdpos[,'y'],xg=pos_master$guidepos[,'x'],yg=pos_master$guidepos[,'y'], interactive=visualise)
    
    probes=defineprobe(x=pos[,'x'],y=pos[,'y'],angs=angs, interactive=visualise)
    
    gcegs=choose_cegs(pos=tempgpos)
    guessang=first_guess_angs(pos=tempgpos,cegs=gcegs, guide=TRUE)[cg+1]
    tempgangs=c(newgangs,guessang)
    tempgprobes=defineguideprobe(gxs=tempgpos[,'x'],gys=tempgpos[,'y'],gangs=tempgangs, interactive=visualise)
    
    tempconflicts=find_guide_conflicts(probes=probes, pos=pos, angs=angs, gprobes=tempgprobes, gpos=tempgpos, gangs=tempgangs)
    nconflicts=dim(tempconflicts)[1]
    
    if (nconflicts > 0) {
      newgangs=tempgangs
      withTimeout(expr=assign('newgangs',MinimizeGuideConflicts(pos=pos, angs=angs, cegs=cegs, gpos=tempgpos, gangs=tempgangs, gcegs=gcegs, guide_conflicts=tempconflicts)), timeout=timeout, onTimeout='warning')
      tempgprobes=defineguideprobe(gxs=tempgpos[,'x'],gys=tempgpos[,'y'],gangs=newgangs, interactive=visualise)
      tempconflicts=find_guide_conflicts(probes=probes, pos=pos, angs=angs, gprobes=tempgprobes, gpos=tempgpos, gangs=newgangs)
      nconflicts=dim(tempconflicts)[1]
      if (nconflicts > 0){
        newgangs=tempgangs[c(1:length(tempgangs)-1)]
      }else{
        newgpos=tempgpos
        cg = cg+1
        sel_guides=c(sel_guides,gui)
        ngprobes <<- cg + 1
      }
    } else{
      newgpos=tempgpos
      newgangs=tempgangs
      cg = cg+1
      sel_guides=c(sel_guides,gui)
      ngprobes <<- cg+1
    }
    gui = gui+1
  }
  
  if (cg < 3) {
    print(paste('**Could only configure ',as.character(cg), ' guide stars in this field. Boo! :-('), sep='')
    stdflag='GuideFail'
  }
  
  return(list(gpos=newgpos,gangs=newgangs,guidenum=sel_guides))
}

configure_HECTOR <- function(pos_master=pos_master){
  
  ngalprobes<<-dngalprobes
  
  if (length(pos_master$pos[,1]) < dngalprobes) {
    ngalprobes<<-length(pos_master$pos[,1])
  }
  
  fieldflag=''
  #Keeping only targets that do not overlap with higher priority targets:
  tpts = SpatialPoints(coords=as.matrix(pos_master$pos[,c('x','y')])) #sp objects for the targets as simple points.
  tokeep=c(1)
  if (length(tpts)>1){
    for (tpt in c(2:length(tpts))) {
      mindist=apply(gDistance(tpts[tpt], tpts[1:tpt-1], byid=TRUE),2,min) #The minimum distance to the nearest higher priority neighbour for each target.
      if (mindist > excl_radius*2.){
        tokeep=c(tokeep,tpt)
      }
      else{
        print("We seem to have a clash before we've even started... Error in Tiling Code??")
        fieldflag='fieldfail'
        #* Added by Sam so we can catch when we error and when we just can't configure
        quit(status=10)
      }
    }
  }
  pos_master$pos = pos_master$pos[tokeep,c('x','y')]
  
  #Drawing possible target, standard and guide star positions:
  draw_all(x=pos_master$pos[,'x'],y=pos_master$pos[,'y'],xs=pos_master$stdpos[,'x'],ys=pos_master$stdpos[,'y'],xg=pos_master$guidepos[,'x'],yg=pos_master$guidepos[,'y'], interactive=visualise)
  swaps=FALSE
  nprobes<<-ngalprobes
  pos=pos_master$pos[1:ngalprobes,]
  
  cegs=choose_cegs(pos=pos)
  angs=first_guess_angs(pos=pos, cegs=cegs)
  angsmin<<-angs #Initialising global variable angsmin.
  probes=defineprobe(x=pos[,'x'],y=pos[,'y'],angs=angs, interactive=visualise)
  
  print('Identifying initial conflicts')
  conflicts=find_probe_conflicts(probes=probes, pos=pos, angs=angs)
  print(conflicts)
  nconflicts=dim(conflicts)[1]
  if (nconflicts <1) {
    print('Bam! No initial conflicts, this one is done!')
  } else{
    print('Minimizing the overlaping area by rotating probes')
    delta_ang=20 #Start with 20 degrees
    if (nconflicts > 0){
      #Try top priority targets first (for a given time) and then try swapping most conflicted probe(s) in turn.
      #Usually if there is a solution, it is found well before the timer runs out. 
      #This optimal time limit may depend on the density of the field and size of the probes though...?
      
      angs=wrapMinimizeConflicts(pos=pos, angs=angs, cegs=cegs, init_delta_ang=delta_ang, probe_conflicts=conflicts)
      
      draw_all(x=pos_master$pos[,'x'],y=pos_master$pos[,'y'],xs=pos_master$stdpos[,'x'],ys=pos_master$stdpos[,'y'],xg=pos_master$guidepos[,'x'],yg=pos_master$guidepos[,'y'], interactive=visualise)
      probes=defineprobe(x=pos[,'x'],y=pos[,'y'],angs=angs, interactive=visualise)
      #Check if could configure:
      conflicts=find_probe_conflicts(probes=probes, pos=pos, angs=angs)
      nconflicts=dim(conflicts)[1]
    }
  }
  
  
  
  if (nconflicts > 0) {
    print('Could not find a solution, I am going to try swaps. This could take a while, grab a cuppa!')
    swaps=TRUE
    temp=try_swaps(pos_master=pos_master)
    tempos=temp$pos
    angs_new=temp$angs
    print('Looking for remaining conflicts')
    probes=defineprobe(x=tempos[,'x'],y=tempos[,'y'],angs=angs_new, interactive=FALSE)
    conflicts=find_probe_conflicts(probes=probes, pos=tempos, angs=angs_new)
    nconflicts=dim(conflicts)[1]
    if (nconflicts > 0) {
      print('Sorry, I tried my best but really could not configure this field :-(.')
      fieldflag='fieldfail'
      #* Added by Sam so we can catch when we error and when we just can't configure
      quit(status=10)
    }else{
      pos=tempos
      angs=angs_new
    }
  } else{
    print('19 galaxy probes are in! Let\'s put some standard stars probes!')
    swaps=FALSE
  }
  
  #Configuring standard probes:
  withstdstars=configure_stdstars(pos=pos, angs=angs, cegs=cegs, stdpos=pos_master$stdpos)
  
  pos=withstdstars$pos #Including the 2 standards in the list of positions.
  angs=withstdstars$angs
  stdflag=withstdstars$stdflag
  cegs=withstdstars$cegs
  print('19 galaxy probes and 2 standard probes are in! Let\'s put some standard guide probes!')
  
  #Configure 3-6 guide stars.
  guides=configure_guidestars(pos=pos, angs=angs, cegs=cegs, guidepos=pos_master$guidepos)
  if (visualise) {plot_configured_field(pos=pos,angs=angs,gpos=guides$gpos,gangs=guides$gangs, aspdf=FALSE)}
  
  #Saving output to some random global variable for future tweaking in case of a late crash..
  #temp<<-list(pos=pos,angs=angs,gpos=guides$gpos,gangs=guides$gangs, fieldflags=final_config$flags)
  
  #Creating an output file for the robot:
  #Measure the angle corresponding to the shortest distance between the target and the edge of the field.
  azAngs=(pi+atan2(pos[,'y'],pos[,'x']))
  azAngs[azAngs > 3*pi/2]=azAngs[azAngs > 3*pi/2]-2*pi
  azAngs[azAngs < -1*pi/2]=azAngs[azAngs < -1*pi/2]+2*pi
  angs_azAng=angs-azAngs
  angs_azAng[angs_azAng > pi] = angs_azAng[angs_azAng > pi] - 2*pi
  angs_azAng[angs_azAng < 0] = angs_azAng[angs_azAng < 0] + 2*pi
  rads=sqrt(pos[,'x']**2+pos[,'y']**2)
  
  gazAngs=(pi+atan2(guides$gpos[,'y'],guides$gpos[,'x']))
  gazAngs[gazAngs > 3*pi/2]=gazAngs[gazAngs > 3*pi/2]-2*pi
  gazAngs[gazAngs < -1*pi/2]=gazAngs[gazAngs < -1*pi/2]+2*pi
  gangs_gazAng=guides$gangs-gazAngs
  gangs_gazAng[gangs_gazAng > pi] = gangs_gazAng[gangs_gazAng > pi] - 2*pi
  gangs_gazAng[gangs_gazAng < 0] = gangs_gazAng[gangs_gazAng < 0] + 2*pi
  grads=sqrt(guides$gpos[,'x']**2+guides$gpos[,'y']**2)
  
  final_config=list(pos=pos, rads=rads, angs=angs, azAngs=azAngs, angs_azAng=angs_azAng, guidenum=guides$guidenum, gpos=guides$gpos, grads=grads, gangs=guides$gangs,gazAngs=gazAngs, gangs_gazAng=gangs_gazAng, swaps=swaps, flags=paste(stdflag,guides$guideflag,sep=''))
  
  print('Here you go, here\'s your complete configuration!')
  print(final_config)
  
  return (final_config)
}

plot_configured_field <- function(filename='temp.pdf',pos,angs,gpos,gangs, fieldflags='none', aspdf=TRUE){
  if (aspdf) {pdf(file=filename, width=6, height=6)}
  draw_pos(x=pos_master$pos[,'x'],y=pos_master$pos[,'y'], label_probe=FALSE) 
  draw_pos(x=pos[,'x'],y=pos[,'y'],overwrite=FALSE) 
  draw_stds(xs=pos_master$stdpos[,'x'],ys=pos_master$stdpos[,'y']) 
  draw_guides(xg=pos_master$guidepos[,'x'],yg=pos_master$guidepos[,'y'])
  probes=defineprobe(x=pos[,'x'],y=pos[,'y'],angs=angs, interactive=TRUE)
  gprobes=defineguideprobe(gxs=gpos[,'x'],gys=gpos[,'y'],gangs=gangs, interactive=TRUE)
  text(gpos[,'x'],gpos[,'y'],as.character(c(1:length(gpos[,'x']))), cex=0.75)
  mtext(paste('Flags:', fieldflags), cex=1, side=3)
  if (aspdf) {graphics.off()}
}


#CAN WE TURN THIS INTO A GUI OR USER-FRIENDLY APP? POTENTIALLY DRAGGING PROBES?
# manually_tweak(fieldname){
#   
#   ###
#   #Exact read-in format will need to be reviewed
#   hconfig=read.table(file=paste('HECTORConfig_Hexa_',substr(fieldname, start=1,stop=nchar(f)-4), '.txt', sep=''))
#   
#   pos=hconfig$pos
#   angs=hconfig$angs
#   gpos=hconfig$gpos
#   gangs=hconfig$gangs
#   fieldflags=hconfig$fieldflags
#   ###
#   
#   #Need to implement guides vs target/standard option.
#   
#   happy=FALSE
#   while (!happy){
#     plot_configured_field(pos=pos,angs=angs,gpos=gpos,gangs=gangs,fieldflags=fieldflags,aspdf=FALSE)
#     
#     probe=as.numeric(readline(prompt='Probe to tweak '))
#     
#     fixedprobe=FALSE
#     while (!fixedprobe){
#       rad=probe_l+cable_l+tip_l/2.
#       points(x=pos[probe,'x']+1*rad*cosd(seq(from=0,to=360, by=5)),y=pos[probe,'y']+rad*sind(seq(from=0,to=360, by=5)),pch=16,cex=0.2)
#       points(x=pos[probe,'x']+1*rad*cosd(seq(from=0,to=360, by=15)),y=pos[probe,'y']+rad*sind(seq(from=0,to=360, by=15)),pch=16,cex=0.5)
#       text(x=pos[probe,'x']-1*(rad+tip_l)*cosd(seq(from=0,to=359, by=30)),y=pos[probe,'y']-(rad+tip_l)*sind(seq(from=0,to=359, by=30)), labels=as.character(seq(from=0,to=359, by=30)))
#       lines(x=c(pos[probe,'x'], pos[probe,'x']-rad), y=c(pos[probe,'y'], pos[probe,'y']))
#       #User input the new angle.
#       newangs=angs
#       newangs[probe]=as.numeric(readline(prompt=paste('New angle for probe ',as.character(probe),' ')))*pi/180.
#       #redraw:
#       plot_configured_field(pos=pos,angs=newangs,gpos=gpos,gangs=gangs,fieldflags=fieldflags,aspdf=FALSE)
#       fixedprobe=as.logical(readline(paste('I am happy with probe', as.character(probe), 'now (T for true or F for false) ')))
#       angs=newangs
#     }  
#     happy=as.logical(readline(paste('I am done with this configuration, save output (T for true or F for false) ')))
#   }
#   #Overwrite configuration file.
# }
# 
