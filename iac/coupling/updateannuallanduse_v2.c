/*-------------------
 
 September 2019 update (adv):
 
	This has been updated to work with new LUH2 data (which has been converted to LUH1 format), with 2015 the start of the future run
 	The standalone code is in #ifdef blocks, with the value #define STANDALONE 1 needing to be uncommented to compile the standalone program
 	For iESM, the standalone code will not be compiled
 	Unused functions have been commented out
 	This update was formed by merging the standalone code (see below) with the original iESM file
 	I removed timestamps from file names to be consistent with iESM
 	I added a future/historical flag to the main function to automatically set the LUH read-in lengths for the standalone code
 	The harvest data are normalized to the beginning of the model year area, as was done in iESM - the original standalone code normalized harvest to end of year area
 
 	Note that land use areas are for the beginning of the labelled output year, and as such facilitate transitions during the model year, which is the previous year
 		and that the harvest areas stored in the output-year labelled files are also for the model year, which is the previous year
 		for example, 2014 harvest data are stored in the 2015 labelled file because these data are actually applied to the 2014 model year
 
 this is the compile line on my machine:
 gcc -g -lnetcdf -L/usr/local/lib -I/usr/local/include -lm updateannuallanduse_v2.c -o ualu_v2
 
 -L and -I need to be changed to reflect the locations of the NetCDF library and header files, respectively.
 
 Some other details on the standalone version from July 2019 (adv):
 
 these are modifications to full_updateannuallanduse_louise_cleanedup.c, which louise gave me
 
 takes about 30 minutes to run on my desktop
 
 added writing the dynamic files in order to get the final year for use as initial files for the future
 - the readhurtt functions fill a global glmo array directly with the input data
 - louise's 2015 init pft file is close (6% difference from mine), and it has only the four variables, so different than the original dynamic files
 - louise's 2014 init land type file looks like it is not for 2014
 
 automated time stamps within files and for file names
 
 moved constants in the main annual function to the beginning of the function
 
 moved the dynamic file name variables to the global constants
 
 moved the length of the luh input data time series to the global constants
 - code was crashing because the last year of the harvest file (2015) was corrupted in some way
 - the harvest 2015 data are not needed for historical; they should be in the future harvest file
 - so the length of the time series is different for the crop/pasture and the harvest files
 
 future runs are set up to use the previous year as the reference
 
 the years to run are still hardcoded in main(), which is at the very end of the file
 
 cleaned up some code - added some "breaks"; deleted some stuff
 
 
 Code Description:
	
	Modified Peter Lawrence code for facilitating transient land cover change
	Set crop and shrub/grass PFT grid cell percentages and harvested fractions of forest area based on GLM output
	The new PFT values are calculated with respect to a base year, which is now the previous year
	Changes between the output-year GLM crop fraction and the base-year PFT crop are calculated first,
		then applied to the base-year pft data (the crop pft percent is set equal to the GLM crop output-year percent)
	Changes between the output-year GLM pasture and the base-year GLM pasture are calculated second, adjusted for pft limits,
		then applied to the crop-adjusted base-year pft data
	The code output-year harvested areas are normalized by the the amount of previous-year primary and secondary land area
		the harvest output year is actually the active model year, which is one year prior to the pft output year
 		so, for example, 2014 harvest data are stored in the 2015 labelled file because these data are actually applied to the 2014 model year
 		this is because the 2015 land use area data are for the beginning of 2015 and are used to generate the transitions during 2014
	The glm input year is the output year of this code, which is the year after the actual model year
		This is because the output year sets the land distribution for the first day of the output year
		So the model needs to first interpolate from the first day of the model year to the first day of this output year
		The harvest amounts, however, are applied directly to the model year, and they are actually for year prior to the outupt year
	Year information refers to the beginning of that year for land use; the harvest data are the totals for year-1
	Input and output are currently at 0.5x0.5 degree resolution
	
 
	Original version
	The base year was 2000
		glmo land use categories were incorrectly assumed to be fraction of vegetated land unit
		PFTS were added based on the input potential vegetation fractions, regardless of which vegetation had been cleared
		crops are set directly to the glmo fraction, so there is an initial shift in 2005 from initial CLM to 2006 GLM
		pasture has a series of unusual adjustments
		Bare ground can only be removed via crop addition, and added via crop removal if not enough other potential PFTs exist in cell
		historical years were also calculated from the 2000 base year (this is not reflected in the code below!)
			so historical crop/pasture additions followed the logic for future removals and vice versa
 
	The new version
		The base year is now 2000 for years <=2000, and the previous year for years >2000
		glmo land use categories are normalized to the PFT vegetated land unit fraction
			this directly preserves the GLM fraction of grid cell
		PFTs are added based on the availabel potential vegetation area, rather than the input potential vegetation fracions
		REVIEW: if the base maps are equal crops should be based on changes from base year,
			but keep it as in the original for now to best preserve the spatial distribution passed from GCAM through GLM
		REVIEW: leave bare ground as is because little to none has been removed due to crop addition since 1850,
			and we have yet to implement appropriate degradations to bare soil
		There are two different sets of assumptions for land conversion based on the time period:
			1850 through 2014
				Use the original land conversion assumptions, but with normalized glmo data and available potential veg constraints
			2015 through 2100
				only trees are added when crops and pasture are removed
				trees are preferentially removed when crop and pasture are added
					(because the previously added trees are either on the best crop/pasture land or on land that doesn't necessarily support trees)
				normalized glmo data and available potential veg constraints
		Note: Do not switch from the future assumptions to the original assumptions,
			some of the pft additions upon crop/pasture removal will be unconstrained and could fail due to how the available potential pfts are calculated
 
	Note:
		mksurfdat has to reduce the pfts to accommodate non-zero urban area!
		this is because urban info does not exist in either input file!
		urban is assumed to be zero for this processing
 
	Input:
	
	From glmo (the GLM output):
	Fraction of grid cell in Primary (GOTHR), Secondary (GSECD), Crop (GCROP), and Pasture (GPAST) land
	Fraction of grid cell harvested from each of:
		primary forest (vh1)
		primary non-forest (vh2)
		secondary mature forest (sh1)
		secondary young forest (sh2)
		secondary non-forest (sh3)
	
	There are two static files that are specific to the initial start year
		If branching these files need to be created from the dynamic pl files
		These two files are the basis for the dynamic files
 
	There are two year 2000 static files that are used as reference for historical changes (<=2000)
 
	There are two dynamic pl files that are created in the initial year as copies of the intial static files
		each new year is added as the model runs
		These files include all of the info in the static files, but with an added time dimension for the year of the stored data
		These are the files used as the previous year reference for years >2000
			pasture is used as reference for change; crop is not used
 
	From static files:
	"./iESM_Ref_CropPast.nc" -- start-year GLM crop and pasture, either year 1850 or 2005 data are copied into this generic file name
		fraction of total grid cell
		global crop area is 15,358,126 km^2 (year 2000 from iESM_Expt1_C_S2_CropPast_Ref.nc)
		surfdata_360x720_mcrop2000.nc has different crop fraction distribution than iESM_Expt1_C_S2_CropPast_Ref.nc
	"surfdata_360x720_mcrop.nc" -- start-year PFTs, either year 1850 or 2005 data are copied into this generic file name
		PFTs are percent of vegetated land unit
		Glacier, wetland, lake, urban are percents of land frac per grid cell
			so with vegland unit they sum to 100 for all cells with landfrac != 0, and this is the percent of the landfrac area
		Wetland = 100 where landfrac = 0
		Urban = nodata value, so assume it = 0 here, but it is not zero after mksurfdat!
		so veg land unit = 100-glacier-lake-urban-wetland, and these represent the land portion of the grid cell
		LandFrac is fraction of grid cell that is not ocean
		Area is km^2 and is the entire grid cell
		global crop area is 14,773,299 km^2 (year 2000 from surfdata_360x720_mcrop2000.nc)
	
	"surfdata_360x720_mcrop2000.nc"
		This is the year 2000 data set that is used as the pft reference for the historical period (<=2000)
		
	"iESM_Ref_CropPast2000.nc"
		This is the year 2000 data set that is used as the land use reference for the historical perion (<=2000)
			pasture is used as reference for change; crop is not used
 
	Another static file that is used:
	"surfdata_360x720_potveg.nc" -- potential veg (no historical land use) map for ~ year 2000;
		used to assign PFTs when crop or pasture decreases
		units are the same as for surfdata_360x720_mcrop.nc
	
	Output (plodata, which is filled from the outhurtt### arrays):
	
	Percent of vegetated land unit for each of the 16 PFTS listed below
	Fraction of total primary and secondary land that is harvested from each of vh1, vh2, sh1, sh2, sh3
	Fraction of output year herbaceous (shrub/grass) land that is GLM pasture
	
  REVISION/MODIFICATION HISTORY:
       03/23/2011 initial version for IESM by Tony Craig. Original code by Peter Lawrence (NCAR)
       05/07/2013 modified by Alan Di Vittorio (lbl), Jiafu Mau (ornl), Louise Parsons Chini (umd), and Benjamin Bond-Lamberty (pnnl)
	   jan 2014 modified by Alan Di Vittorio (lbl) to reference previous year and use original assumptions prior to 2005
	   march-april 2014 modified by Alan Di Vittorio (lbl) to reference year 2000 as base year for output year <= 2000
			and also to normalize harvest fractions to the correct year of primary and secondar fractions
 
	End code description -adv
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <netcdf.h>

// use this to compile for standalone operation
//#define STANDALONE 1

// use this to output extra stuff to the terminal or to the log file (via printf statements)
// in iESM, these statements are captured in a log file
// for standalone, these statements print to the terminal, so can be captured to a file via a pipe
//#define DEBUG 1

#define MAXPFT 16
#define MAXMONTH 12
#define MAXSOILCOLOR 20
#define MAXSOILLAYERS 10

#define BPFT 0
#define NEMPFT 1
#define NEBPFT 2
#define NDBPFT 3
#define BETPFT 4
#define BEMPFT 5
#define BDTPFT 6
#define BDMPFT 7
#define BDBPFT 8
#define SEMPFT 9
#define SDMPFT 10
#define SDBPFT 11
#define GA3PFT 12
#define GC3PFT 13
#define GC4PFT 14
#define CPFT 15

#define PFTVALUES 30
#define PFTIDINDEX 0
#define PFTPCTINDEX 1
#define PFTGROUP1INDEX 2
#define PFTGROUP2INDEX 3
#define PFTLAIINDEX 4
#define PFTSAIINDEX 16
#define PFTBOTINDEX 28
#define PFTTOPINDEX 29

#define CLMLAIVAR 23

#define GLMONFLDS 9
#define PLONFLDS 23
#define MAXOUTPIX 720
#define MAXOUTLIN 360
#define OUTPIXWIDTH 0.5
#define OUTPIXHEIGHT 0.5
#define OUTLLX 0.0
#define OUTLLY -89.75

// for standalone:
// the last year of the harvest file isn't needed, and it causes the code to crash if it is read in
// so set the number of time records here, and automatically select in the code via a command line argument
// historical
#define INHISTHARVTIME 165
#define INHISTLUTIME 166
// future
#define INFUTUREHARVTIME 85
#define INFUTURELUTIME 86

#define MAXINPIX 7200
#define MAXINLIN 3600
#define INPIXSIZE 0.05
#define INLLX -180.0
#define INLLY -90.0

#define SEARCHMIN 2
#define SEARCHMAX 1024
#define BOXLIMIT 360

#define DAYSINYEAR 365.0
#define F0 1375.0
#define TAU0 0.7
#define PI 4.0*atan(1.0)
#define SLIMIT 10

/* these 1-d arrays of the grid have a cell boundary origin at the lower left corner of: lon= -180, lat = -90)-adv */
/* the index increases with increasing longitude then with increaseing latitude -adv */
/* i.e. row-by-row from left to right, starting at the bottom -adv */

#ifdef STANDALONE
// this is for writing the dynamic crop/pasture array
float glmo[MAXOUTPIX * MAXOUTLIN][GLMONFLDS];
#endif

float inmask[MAXOUTPIX * MAXOUTLIN];
float inland[MAXOUTPIX * MAXOUTLIN];
float inlake[MAXOUTPIX * MAXOUTLIN];
float inwetland[MAXOUTPIX * MAXOUTLIN];
float inice[MAXOUTPIX * MAXOUTLIN];
float invegbare[MAXOUTPIX * MAXOUTLIN];

float insand[MAXSOILLAYERS][MAXOUTPIX * MAXOUTLIN];
float inclay[MAXSOILLAYERS][MAXOUTPIX * MAXOUTLIN];
float insoilslope[MAXOUTPIX * MAXOUTLIN];

float incurrentpftid[MAXPFT][MAXOUTPIX * MAXOUTLIN];
float incurrentpftval[MAXPFT][MAXOUTPIX * MAXOUTLIN];		// this now contains the reference year data, which can be the previous year or ref year instead of the base year
float incurrentlaival[MAXMONTH][MAXPFT][MAXOUTPIX * MAXOUTLIN];
float incurrentsaival[MAXMONTH][MAXPFT][MAXOUTPIX * MAXOUTLIN];
float incurrentsoilcolor[MAXOUTPIX * MAXOUTLIN];

float inpotvegpftid[MAXPFT][MAXOUTPIX * MAXOUTLIN];
float inpotvegpftval[MAXPFT][MAXOUTPIX * MAXOUTLIN];

float inhurttbasecrop[MAXOUTPIX * MAXOUTLIN];		// this now contains the reference year data, which can be the previous year or ref year instead of the base year
float inhurttbasepasture[MAXOUTPIX * MAXOUTLIN];	// this now contains the reference year data, which can be the previous year or ref instead of the base year
float inhurttcrop[MAXOUTPIX * MAXOUTLIN];
float inhurttpasture[MAXOUTPIX * MAXOUTLIN];

float inhurttprimary[MAXOUTPIX * MAXOUTLIN];
float inhurttsecondary[MAXOUTPIX * MAXOUTLIN];

float prevprimary[MAXOUTPIX * MAXOUTLIN];
float prevsecondary[MAXOUTPIX * MAXOUTLIN];

float inhurttvh1[MAXOUTPIX * MAXOUTLIN];
float inhurttvh2[MAXOUTPIX * MAXOUTLIN];
float inhurttsh1[MAXOUTPIX * MAXOUTLIN];
float inhurttsh2[MAXOUTPIX * MAXOUTLIN];
float inhurttsh3[MAXOUTPIX * MAXOUTLIN];

float outhurttpftid[MAXPFT][MAXOUTPIX * MAXOUTLIN];
float outhurttpftval[MAXPFT][MAXOUTPIX * MAXOUTLIN];
float outhurttlaival[MAXMONTH][MAXPFT][MAXOUTPIX * MAXOUTLIN];
float outhurttsaival[MAXMONTH][MAXPFT][MAXOUTPIX * MAXOUTLIN];
float outhurttsoilcolor[MAXOUTPIX * MAXOUTLIN];

float outhurttvh1[MAXOUTPIX * MAXOUTLIN];
float outhurttvh2[MAXOUTPIX * MAXOUTLIN];
float outhurttsh1[MAXOUTPIX * MAXOUTLIN];
float outhurttsh2[MAXOUTPIX * MAXOUTLIN];
float outhurttsh3[MAXOUTPIX * MAXOUTLIN];
float outhurttgrazing[MAXOUTPIX * MAXOUTLIN];

/* !!! these arrays keep track of whether there is enough tree potential veg to match the removal of all crop or pasture -adv */
int cropavailpotvegtreepftval[MAXOUTPIX * MAXOUTLIN];
int pastureavailpotvegtreepftval[MAXOUTPIX * MAXOUTLIN];
FILE *tempfile;

// dynamic file names to store the luh and pft data each year - the name is created only once, but used for each year
// for iESM do not include the date because then it won't be found upon restart
char dyn_luh_file[250];
char dyn_pft_file[250];

char *monthname[MAXMONTH] = {"jan","feb","mar","apr","may","jun","jul","aug","sep","oct","nov","dec"};
float monthday[12] = {15,46,74,105,135,166,196,227,258,288,319,349};

char *pftnameinfilestr[MAXPFT];
char *pftname[MAXPFT];
float pftLAIMax[MAXPFT];
float pftHeightTop[MAXPFT];
float pftHeightBot[MAXPFT];
float pftGroup1[MAXPFT];
float pftGroup2[MAXPFT];

/* NetCDF Variables */

int  innetcdfid;
int  innetcdfstat;

/* dimension variables */
int  ndimsp, ndimsp2;
int  ndimspcnt;
char dimname[128], dimname2[128];
size_t dimlen, dimlen2;

/* variable variables */
int  nvarsp;
int  nvarspcnt;
int  isvardim;
char varname[256];
nc_type vartype;
int  vardimsp;
int  vardimidsp;
int  varattsp;
int  varlayers, varlayers2;
int  selectedvarcnt;
int  selectedvarnum = -1;
int  selectedvarids[256];

int  layercnt;
int  latcnt;
int  latdim = -1;
int  latlen = -1;
int  loncnt;
int  londim = -1;
int  lonlen = -1;
long varindex;

/* attribute variables */
int  nattsp;
int  nattspcnt;
char attname[128];
nc_type atttype;
size_t attlen;

/* unlimited dimension variables */
int  unlimdimidp;

char *nc_typename[7] = {"no type","signed 1 byte integer",
    "ISO/ASCII character",
    "signed 2 byte integer",
    "signed 4 byte integer",
    "single precision floating point number",
    "float precision floating point number"};

/* NOTE: roundit not currently being used by the code - lpc */
float
roundit(float innumber, int inplaces) {
    
    float newnumber;
    float multiply;
    long roundval;
    long nextroundval;
    
    multiply = pow(10.0,inplaces);
    
    roundval = innumber * multiply;
    nextroundval = innumber * multiply * 10.0;
    nextroundval =  nextroundval - roundval * 10;
    if (nextroundval >= 5) {
        roundval = roundval + 1;
    }
    if (nextroundval <= -5) {
        roundval = roundval - 1;
    }
    newnumber = (float) roundval;
    newnumber = newnumber / multiply;
    
    return newnumber;
    
}

/* this is not used by the code at all - adv
void
readpftparamfile(char *filenamestr) {
    
    FILE *pftparamfile;
    int inpft;
    char infilename[50], inpftname[50];
    float inlaimax, inheightbot, inheighttop, ingroup1, ingroup2;
    
    printf("Reading %s\n",filenamestr);
    pftparamfile = fopen(filenamestr,"r");
    
    for (inpft = 0; inpft < MAXPFT; inpft++) {
        fscanf(pftparamfile,"%s%s%f%f%f%f%f",infilename,inpftname,&inlaimax,&inheightbot,&inheighttop,&ingroup1,&ingroup2);
        pftnameinfilestr[inpft] = strdup(infilename);
        pftname[inpft] = strdup(inpftname);
        pftLAIMax[inpft] = inlaimax;
        pftHeightTop[inpft] = inheighttop;
        pftHeightBot[inpft] = inheightbot;
        pftGroup1[inpft] = ingroup1;
        pftGroup2[inpft] = ingroup2;
    }
    
}
--- */

int
opennetcdf(char *filenamestr) {
    
    int openstatus = 1;
    
    /* open and check the netcdf file */
    
    innetcdfstat = nc_open(filenamestr, NC_WRITE, &innetcdfid);
    if (innetcdfstat != NC_NOERR) {
        printf("Error no such file %s\n", filenamestr);
        openstatus = 0;
        
        
        /* find the longitude and latitude dimensions */
    }
    else {
        
        nc_inq(innetcdfid, &ndimsp, &nvarsp, &nattsp, &unlimdimidp);
        
        if (ndimsp != -1)
            for (ndimspcnt = 0; ndimspcnt < ndimsp; ndimspcnt ++) {
                nc_inq_dim(innetcdfid, ndimspcnt, dimname, &dimlen);
                
                if (strcmp(dimname,"lsmlon") == 0 || strcmp(dimname,"longitude") == 0 || strcmp(dimname,"lon") == 0 || strcmp(dimname,"LON") == 0) {
                    londim = ndimspcnt;
                    lonlen = dimlen;
                }
                
                if (strcmp(dimname,"lsmlat") == 0 || strcmp(dimname,"latitude") == 0 || strcmp(dimname,"lat") == 0 || strcmp(dimname,"LAT") == 0) {
                    latdim = ndimspcnt;
                    latlen = dimlen;
                }
                
            }
        
        if (londim == -1) {
            printf("Error Longitude dimension Not Found\n");
            openstatus = 0;
        }
        
        if (latdim == -1) {
            printf("Error Latitude dimension Not Found\n");
            openstatus = 0;
        }
        
        if (lonlen != MAXOUTPIX) {
            printf("Error Longitude dimension Wrong Size %d Expected %d\n",londim,MAXOUTPIX);
            openstatus = 0;
        }
        
        if (latlen != MAXOUTLIN) {
            printf("Error Latitude dimension Wrong Size %d Expected %d\n",latdim,MAXOUTLIN);
            openstatus = 0;
        }
    }
    
    return openstatus;
    
}


int
closenetcdf(char *filenamestr) {
    
    int closestatus = 1;
    
    innetcdfstat = nc_close(innetcdfid);
    if (innetcdfstat != NC_NOERR) {
        printf("Error closing file %s\n", filenamestr);
        closestatus = 0;
    }
    
    return closestatus;
    
}

////////////////////////////////////////////////////////////////////////////////////
/* these updatehurtt functions are not used in iESM, but are used in standalone mode -adv */
/* skip to ~1250 to see the readhurtt functions, which are not used, but still useful in understanding the glmo data -adv */
/* skip to ~1900 to get to functions that are actually used -adv */

#ifdef STANDALONE

/* NOTE: updatehurttlakefrac not currently being used by the code - adv */
void
updatehurttlandfrac() {
    
    float *landfracvalues;
    float *landmaskvalues;
    int outgrid;
    
    selectedvarcnt = 0;
    
    nc_inq(innetcdfid, &ndimsp, &nvarsp, &nattsp, &unlimdimidp);
    
    for (nvarspcnt = 0; nvarspcnt < nvarsp; nvarspcnt ++) {
        nc_inq_varname(innetcdfid, nvarspcnt, varname);
        /*      nc_inq_var(innetcdfid, nvarspcnt, varname, &vartype, &vardimsp, &vardimidsp, &varattsp); */
        if (strcmp(varname,"LANDFRAC") == 0) {
            selectedvarids[0] = nvarspcnt;
            selectedvarcnt++;
            printf("Updating variable: %d %s \n",nvarspcnt,varname);
        }
        
        if (strcmp(varname,"LANDMASK") == 0) {
            selectedvarids[1] = nvarspcnt;
            selectedvarcnt++;
            printf("Updating variable: %d %s \n",nvarspcnt,varname);
        }
        
    }
    
    /*  nc_inq_var(innetcdfid, selectedvarids[0], varname, &vartype, &vardimsp, &vardimidsp, &varattsp); */
    varlayers = 1;
    varlayers2 = 1;
    landfracvalues = malloc(sizeof(float) * lonlen * latlen * varlayers * varlayers2);
    nc_get_var_float(innetcdfid,selectedvarids[0],landfracvalues);
    
    /*  nc_inq_var(innetcdfid, selectedvarids[1], varname, &vartype, &vardimsp, &vardimidsp, &varattsp); */
    varlayers = 1;
    varlayers2 = 1;
    landmaskvalues = malloc(sizeof(float) * lonlen * latlen * varlayers * varlayers2);
    nc_get_var_float(innetcdfid,selectedvarids[1],landmaskvalues);
    
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        landmaskvalues[outgrid] = inmask[outgrid] / 100.0;
        landfracvalues[outgrid] = inland[outgrid];
    }
    
    /*  nc_put_var_float(innetcdfid,selectedvarids[0],landfracvalues);
     nc_put_var_float(innetcdfid,selectedvarids[1],landmaskvalues);  */
    free(landfracvalues);
    free(landmaskvalues);
    
}

/* NOTE: updatehurttlakefrac not currently being used by the code - lpc */
void
updatehurttlakefrac() {
    
    float *lakefracvalues;
    int outgrid;
    
    selectedvarcnt = 0;
    
    nc_inq(innetcdfid, &ndimsp, &nvarsp, &nattsp, &unlimdimidp);
    
    for (nvarspcnt = 0; nvarspcnt < nvarsp; nvarspcnt ++) {
        nc_inq_varname(innetcdfid, nvarspcnt, varname);
        /*      nc_inq_var(innetcdfid, nvarspcnt, varname, &vartype, &vardimsp, &vardimidsp, &varattsp); */
        if (strcmp(varname,"PCT_LAKE") == 0) {
            selectedvarids[0] = nvarspcnt;
            selectedvarcnt++;
            printf("Updating variable: %d %s \n",nvarspcnt,varname);
        }
        
    }
    
    /*  nc_inq_var(innetcdfid, selectedvarids[0], varname, &vartype, &vardimsp, &vardimidsp, &varattsp); */
    varlayers = 1;
    varlayers2 = 1;
    lakefracvalues = malloc(sizeof(float) * lonlen * latlen * varlayers * varlayers2);
    nc_get_var_float(innetcdfid,selectedvarids[0],lakefracvalues);
    
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        lakefracvalues[outgrid] = inlake[outgrid];
    }
    
    /*  nc_put_var_float(innetcdfid,selectedvarids[0],lakefracvalues); */
    free(lakefracvalues);
    
}

/* NOTE: updatehurttwetlandfrac not currently being used by the code - lpc */
void
updatehurttwetlandfrac() {
    
    float *wetlandfracvalues;
    int outgrid;
    
    selectedvarcnt = 0;
    
    nc_inq(innetcdfid, &ndimsp, &nvarsp, &nattsp, &unlimdimidp);
    
    for (nvarspcnt = 0; nvarspcnt < nvarsp; nvarspcnt ++) {
        nc_inq_varname(innetcdfid, nvarspcnt, varname);
        /*      nc_inq_var(innetcdfid, nvarspcnt, varname, &vartype, &vardimsp, &vardimidsp, &varattsp); */
        if (strcmp(varname,"PCT_WETLAND") == 0) {
            selectedvarids[0] = nvarspcnt;
            selectedvarcnt++;
            printf("Updating variable: %d %s \n",nvarspcnt,varname);
        }
        
    }
    
    /*  nc_inq_var(innetcdfid, selectedvarids[0], varname, &vartype, &vardimsp, &vardimidsp, &varattsp); */
    varlayers = 1;
    varlayers2 = 1;
    wetlandfracvalues = malloc(sizeof(float) * lonlen * latlen * varlayers * varlayers2);
    nc_get_var_float(innetcdfid,selectedvarids[0],wetlandfracvalues);
    
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        wetlandfracvalues[outgrid] = inwetland[outgrid];
    }
    
    /*  nc_put_var_float(innetcdfid,selectedvarids[0],wetlandfracvalues); */
    free(wetlandfracvalues);
    
}

/* NOTE: updatehurtticefrac not currently being used by the code - lpc */
void
updatehurtticefrac() {
    
    float *icefracvalues;
    int outgrid;
    
    selectedvarcnt = 0;
    
    nc_inq(innetcdfid, &ndimsp, &nvarsp, &nattsp, &unlimdimidp);
    
    for (nvarspcnt = 0; nvarspcnt < nvarsp; nvarspcnt ++) {
        nc_inq_varname(innetcdfid, nvarspcnt, varname);
        /*      nc_inq_var(innetcdfid, nvarspcnt, varname, &vartype, &vardimsp, &vardimidsp, &varattsp); */
        if (strcmp(varname,"PCT_GLACIER") == 0) {
            selectedvarids[0] = nvarspcnt;
            selectedvarcnt++;
            printf("Updating variable: %d %s \n",nvarspcnt,varname);
        }
        
    }
    
    /*  nc_inq_var(innetcdfid, selectedvarids[0], varname, &vartype, &vardimsp, &vardimidsp, &varattsp); */
    varlayers = 1;
    varlayers2 = 1;
    icefracvalues = malloc(sizeof(float) * lonlen * latlen * varlayers * varlayers2);
    nc_get_var_float(innetcdfid,selectedvarids[0],icefracvalues);
    
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        icefracvalues[outgrid] = inice[outgrid];
    }
    
    /*  nc_put_var_float(innetcdfid,selectedvarids[0],icefracvalues); */
    free(icefracvalues);
    
}

/* NOTE: updatehurttsand not currently being used by the code - lpc */
void
updatehurttsand() {
    
    float *sandvalues;
    float vegetatedcount, soilcount;
    int outgrid, offsetgrid, layer;
    
    selectedvarcnt = 0;
    
    nc_inq(innetcdfid, &ndimsp, &nvarsp, &nattsp, &unlimdimidp);
    
    for (nvarspcnt = 0; nvarspcnt < nvarsp; nvarspcnt ++) {
        nc_inq_varname(innetcdfid, nvarspcnt, varname);
        /*      nc_inq_var(innetcdfid, nvarspcnt, varname, &vartype, &vardimsp, &vardimidsp, &varattsp); */
        if (strcmp(varname,"PCT_SAND") == 0) {
            selectedvarids[0] = nvarspcnt;
            selectedvarcnt++;
            printf("Updating variable: %d %s \n",nvarspcnt,varname);
        }
        
    }
    
    /*  nc_inq_var(innetcdfid, selectedvarids[0], varname, &vartype, &vardimsp, &vardimidsp, &varattsp); */
    varlayers = MAXSOILLAYERS;
    varlayers2 = 1;
    sandvalues = malloc(sizeof(float) * lonlen * latlen * varlayers * varlayers2);
    nc_get_var_float(innetcdfid,selectedvarids[0],sandvalues);
    
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        for (layer = 0; layer < MAXSOILLAYERS; layer++) {
            offsetgrid = layer * MAXOUTPIX * MAXOUTLIN + outgrid;
            sandvalues[offsetgrid] = insand[layer][outgrid];
        }
    }
    
    /*  nc_put_var_float(innetcdfid,selectedvarids[0],sandvalues); */
    free(sandvalues);
    
}

/* NOTE: updatehurttclay not currently being used by the code - lpc */
void
updatehurttclay() {
    
    float *clayvalues;
    float vegetatedcount, soilcount;
    int outgrid, offsetgrid, layer;
    
    selectedvarcnt = 0;
    
    nc_inq(innetcdfid, &ndimsp, &nvarsp, &nattsp, &unlimdimidp);
    
    for (nvarspcnt = 0; nvarspcnt < nvarsp; nvarspcnt ++) {
        nc_inq_varname(innetcdfid, nvarspcnt, varname);
        /*      nc_inq_var(innetcdfid, nvarspcnt, varname, &vartype, &vardimsp, &vardimidsp, &varattsp); */
        if (strcmp(varname,"PCT_CLAY") == 0) {
            selectedvarids[0] = nvarspcnt;
            selectedvarcnt++;
            printf("Updating variable: %d %s \n",nvarspcnt,varname);
        }
        
    }
    
    /*  nc_inq_var(innetcdfid, selectedvarids[0], varname, &vartype, &vardimsp, &vardimidsp, &varattsp); */
    varlayers = MAXSOILLAYERS;
    varlayers2 = 1;
    clayvalues = malloc(sizeof(float) * lonlen * latlen * varlayers * varlayers2);
    nc_get_var_float(innetcdfid,selectedvarids[0],clayvalues);
    
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        for (layer = 0; layer < MAXSOILLAYERS; layer++) {
            offsetgrid = layer * MAXOUTPIX * MAXOUTLIN + outgrid;
            clayvalues[offsetgrid] = inclay[layer][outgrid];
        }
    }
    
    /*  nc_put_var_float(innetcdfid,selectedvarids[0],clayvalues); */
    free(clayvalues);
    
}

/* NOTE: updatehurttsoilslope not currently being used by the code - lpc */
void
updatehurttsoilslope() {
    
    float *soilslopevalues;
    float slopecount, soilslopetopvalue, soilslopebotvalue;
    int outgrid, offsetgrid, layer;
    
    selectedvarcnt = 0;
    
    nc_inq(innetcdfid, &ndimsp, &nvarsp, &nattsp, &unlimdimidp);
    
    for (nvarspcnt = 0; nvarspcnt < nvarsp; nvarspcnt ++) {
        nc_inq_varname(innetcdfid, nvarspcnt, varname);
        /*      nc_inq_var(innetcdfid, nvarspcnt, varname, &vartype, &vardimsp, &vardimidsp, &varattsp); */
        if (strcmp(varname,"SOIL_SLOPE") == 0) {
            selectedvarids[0] = nvarspcnt;
            selectedvarcnt++;
            printf("Updating variable: %d %s \n",nvarspcnt,varname);
        }
        
    }
    
    /*  nc_inq_var(innetcdfid, selectedvarids[0], varname, &vartype, &vardimsp, &vardimidsp, &varattsp); */
    varlayers = 1;
    varlayers2 = 1;
    soilslopevalues = malloc(sizeof(float) * lonlen * latlen * varlayers * varlayers2);
    nc_get_var_float(innetcdfid,selectedvarids[0],soilslopevalues);
    
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        soilslopevalues[outgrid] = insoilslope[outgrid];
    }
    
    /*  nc_put_var_float(innetcdfid,selectedvarids[0],soilslopevalues); */
    free(soilslopevalues);
    
}

/* NOTE: updatehurttsoilcolor not currently being used by the code - lpc */
void
updatehurttsoilcolor() {
    
    float *soilcolorvalues;
    float colorcount, soilcolortopvalue, soilcolorbotvalue;
    int outgrid, offsetgrid, layer;
    
    selectedvarcnt = 0;
    
    nc_inq(innetcdfid, &ndimsp, &nvarsp, &nattsp, &unlimdimidp);
    
    for (nvarspcnt = 0; nvarspcnt < nvarsp; nvarspcnt ++) {
        nc_inq_varname(innetcdfid, nvarspcnt, varname);
        /*      nc_inq_var(innetcdfid, nvarspcnt, varname, &vartype, &vardimsp, &vardimidsp, &varattsp); */
        if (strcmp(varname,"SOIL_COLOR") == 0) {
            selectedvarids[0] = nvarspcnt;
            selectedvarcnt++;
            printf("Updating variable: %d %s \n",nvarspcnt,varname);
        }
        
    }
    
    /*  nc_inq_var(innetcdfid, selectedvarids[0], varname, &vartype, &vardimsp, &vardimidsp, &varattsp); */
    varlayers = 1;
    varlayers2 = 1;
    soilcolorvalues = malloc(sizeof(float) * lonlen * latlen * varlayers * varlayers2);
    nc_get_var_float(innetcdfid,selectedvarids[0],soilcolorvalues);
    
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        soilcolorvalues[outgrid] = outhurttsoilcolor[outgrid];
    }
    
    /*  nc_put_var_float(innetcdfid,selectedvarids[0],soilcolorvalues); */
    free(soilcolorvalues);
    
}

/* NOTE: updatehurttpftpct not currently being used by the code - lpc */
void
updatehurttpftpct() {
    
    float *pftpctvalues;
    int outgrid, offsetgrid, inpft, inmonth;
    
    selectedvarcnt = 0;
    
    nc_inq(innetcdfid, &ndimsp, &nvarsp, &nattsp, &unlimdimidp);
    
    for (nvarspcnt = 0; nvarspcnt < nvarsp; nvarspcnt ++) {
        nc_inq_varname(innetcdfid, nvarspcnt, varname);
        /*      nc_inq_var(innetcdfid, nvarspcnt, varname, &vartype, &vardimsp, &vardimidsp, &varattsp); */
        if (strcmp(varname,"PCT_PFT") == 0) {
            selectedvarids[0] = nvarspcnt;
            selectedvarcnt++;
            printf("Updating variable: %d %s \n",nvarspcnt,varname);
        }
    }
    
    varlayers = MAXPFT+1;
    varlayers2 = 1;
    pftpctvalues = malloc(sizeof(float) * lonlen * latlen * varlayers * varlayers2);
    nc_get_var_float(innetcdfid,selectedvarids[0],pftpctvalues);
    
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        for (inpft = 0; inpft < MAXPFT; inpft++) {
            offsetgrid = inpft * MAXOUTPIX * MAXOUTLIN + outgrid;
            pftpctvalues[offsetgrid] = outhurttpftval[inpft][outgrid];
        }
        inpft = MAXPFT;
        offsetgrid = inpft * MAXOUTPIX * MAXOUTLIN + outgrid;
        pftpctvalues[offsetgrid] = 0.0;
    }
    
      nc_put_var_float(innetcdfid,selectedvarids[0],pftpctvalues); 
    
    free(pftpctvalues);
    
}

/* NOTE: updatehurttpftlai not currently being used by the code - lpc */
void
updatehurttpftlai() {
    
    float *pftlaivalues;
    int outgrid, offsetgrid, inpft, inmonth;
    
    selectedvarcnt = 0;
    
    nc_inq(innetcdfid, &ndimsp, &nvarsp, &nattsp, &unlimdimidp);
    
    for (nvarspcnt = 0; nvarspcnt < nvarsp; nvarspcnt ++) {
        nc_inq_varname(innetcdfid, nvarspcnt, varname);
        nc_inq_var(innetcdfid, nvarspcnt, varname, &vartype, &vardimsp, &vardimidsp, &varattsp);
        if (nvarspcnt == CLMLAIVAR) {
            selectedvarids[0] = nvarspcnt;
            selectedvarcnt++;
            sprintf(varname,"MONTHLY_LAI");
            printf("Updating variable: %d %s \n",nvarspcnt,varname);
        }
    }
    
    varlayers = 12;
    varlayers2 = MAXPFT+1;
    pftlaivalues = malloc(sizeof(float) * lonlen * latlen * varlayers * varlayers2);
    nc_get_var_float(innetcdfid,selectedvarids[0],pftlaivalues);
    
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        for (inpft = 0; inpft < MAXPFT; inpft++) {
            for (inmonth = 0; inmonth < MAXMONTH; inmonth++ ) {
                offsetgrid = inmonth * (MAXPFT+1) * MAXOUTPIX * MAXOUTLIN + inpft * MAXOUTPIX * MAXOUTLIN + outgrid;
                if (outhurttpftval[inpft][outgrid] > 0.0) {
                    pftlaivalues[offsetgrid] = outhurttlaival[inmonth][inpft][outgrid];
                }
                else {
                    pftlaivalues[offsetgrid] = 0.0;
                }
            }
        }
        inpft = MAXPFT;
        for (inmonth = 0; inmonth < MAXMONTH; inmonth++ ) {
            offsetgrid = inmonth * (MAXPFT+1) * MAXOUTPIX * MAXOUTLIN + inpft * MAXOUTPIX * MAXOUTLIN + outgrid;
            pftlaivalues[offsetgrid] = 0.0;
        }
    }
    
    nc_put_var_float(innetcdfid,selectedvarids[0],pftlaivalues);
    
    free(pftlaivalues);
    
}

/* NOTE: updatehurttpftsai not currently being used by the code - lpc */
void
updatehurttpftsai() {
    
    float *pftsaivalues;
    int outgrid, offsetgrid, inpft, inmonth;
    
    selectedvarcnt = 0;
    
    nc_inq(innetcdfid, &ndimsp, &nvarsp, &nattsp, &unlimdimidp);
    
    for (nvarspcnt = 0; nvarspcnt < nvarsp; nvarspcnt ++) {
        nc_inq_varname(innetcdfid, nvarspcnt, varname);
        nc_inq_var(innetcdfid, nvarspcnt, varname, &vartype, &vardimsp, &vardimidsp, &varattsp);
        if (nvarspcnt == CLMLAIVAR + 1) {
            sprintf(varname,"MONTHLY_SAI");
            selectedvarids[0] = nvarspcnt;
            selectedvarcnt++;
            printf("Updating variable: %d %s \n",nvarspcnt,varname);
        }
    }
    
    varlayers = 12;
    varlayers2 = MAXPFT+1;
    pftsaivalues = malloc(sizeof(float) * lonlen * latlen * varlayers * varlayers2);
    nc_get_var_float(innetcdfid,selectedvarids[0],pftsaivalues);
    
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        for (inpft = 0; inpft < MAXPFT; inpft++) {
            for (inmonth = 0; inmonth < MAXMONTH; inmonth++ ) {
                offsetgrid = inmonth * (MAXPFT+1) * MAXOUTPIX * MAXOUTLIN + inpft * MAXOUTPIX * MAXOUTLIN + outgrid;
                if (outhurttpftval[inpft][outgrid] > 0.0) {
                    pftsaivalues[offsetgrid] = outhurttsaival[inmonth][inpft][outgrid];
                }
                else {
                    pftsaivalues[offsetgrid] = 0.0;
                }
            }
        }
        inpft = MAXPFT;
        for (inmonth = 0; inmonth < MAXMONTH; inmonth++ ) {
            offsetgrid = inmonth * (MAXPFT+1) * MAXOUTPIX * MAXOUTLIN + inpft * MAXOUTPIX * MAXOUTLIN + outgrid;
            pftsaivalues[offsetgrid] = 0.0;
        }
    }
    
    nc_put_var_float(innetcdfid,selectedvarids[0],pftsaivalues);
    
    free(pftsaivalues);
    
}

/* NOTE: updatehurttpfttop not currently being used by the code - lpc */
/* this is not present in the code at all - adv
void
updatehurttpfttop() {
    
    float *pfttopvalues;
    int outgrid, offsetgrid, inpft, inmonth;
    
    selectedvarcnt = 0;
    
    nc_inq(innetcdfid, &ndimsp, &nvarsp, &nattsp, &unlimdimidp);
    
    for (nvarspcnt = 0; nvarspcnt < nvarsp; nvarspcnt ++) {
        nc_inq_varname(innetcdfid, nvarspcnt, varname);
        nc_inq_var(innetcdfid, nvarspcnt, varname, &vartype, &vardimsp, &vardimidsp, &varattsp);
        if (nvarspcnt == CLMLAIVAR + 2) {
            sprintf(varname,"MONTHLY_HEIGHT_TOP");
            selectedvarids[0] = nvarspcnt;
            selectedvarcnt++;
            printf("Updating variable: %d %s \n",nvarspcnt,varname);
        }
    }
    
    varlayers = 12;
    varlayers2 = MAXPFT+1;
    pfttopvalues = malloc(sizeof(float) * lonlen * latlen * varlayers * varlayers2);
    nc_get_var_float(innetcdfid,selectedvarids[0],pfttopvalues);
    
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        for (inpft = 1; inpft <= MAXPFT; inpft++) {
            for (inmonth = 0; inmonth < MAXMONTH; inmonth++ ) {
                offsetgrid = inmonth * (MAXPFT+1) * MAXOUTPIX * MAXOUTLIN + inpft * MAXOUTPIX * MAXOUTLIN + outgrid;
                pfttopvalues[offsetgrid] = pftHeightTop[inpft-1];
            }
        }
        inpft = 0;
        for (inmonth = 0; inmonth < MAXMONTH; inmonth++ ) {
            offsetgrid = inmonth * (MAXPFT+1) * MAXOUTPIX * MAXOUTLIN + inpft * MAXOUTPIX * MAXOUTLIN + outgrid;
            pfttopvalues[offsetgrid] = 0.0;
        }
    }
    
    nc_put_var_float(innetcdfid,selectedvarids[0],pfttopvalues);
    
    free(pfttopvalues);
    
}
 --- */

/* NOTE: updatehurttpftbot not currently being used by the code - lpc */
/* this is not present in the code at all - adv
void
updatehurttpftbot() {
    
    float *pftbotvalues;
    int outgrid, offsetgrid, inpft, inmonth;
    
    selectedvarcnt = 0;
    
    nc_inq(innetcdfid, &ndimsp, &nvarsp, &nattsp, &unlimdimidp);
    
    for (nvarspcnt = 0; nvarspcnt < nvarsp; nvarspcnt ++) {
        nc_inq_varname(innetcdfid, nvarspcnt, varname);
        nc_inq_var(innetcdfid, nvarspcnt, varname, &vartype, &vardimsp, &vardimidsp, &varattsp);
        if (nvarspcnt == CLMLAIVAR + 3) {
            sprintf(varname,"MONTHLY_HEIGHT_BOT");
            selectedvarids[0] = nvarspcnt;
            selectedvarcnt++;
            printf("Updating variable: %d %s \n",nvarspcnt,varname);
        }
    }
    
    varlayers = 12;
    varlayers2 = MAXPFT+1;
    pftbotvalues = malloc(sizeof(float) * lonlen * latlen * varlayers * varlayers2);
    nc_get_var_float(innetcdfid,selectedvarids[0],pftbotvalues);
    
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        for (inpft = 1; inpft <= MAXPFT; inpft++) {
            for (inmonth = 0; inmonth < MAXMONTH; inmonth++ ) {
                offsetgrid = inmonth * (MAXPFT+1) * MAXOUTPIX * MAXOUTLIN + inpft * MAXOUTPIX * MAXOUTLIN + outgrid;
                pftbotvalues[offsetgrid] = pftHeightBot[inpft-1];
            }
        }
        inpft = 0;
        for (inmonth = 0; inmonth < MAXMONTH; inmonth++ ) {
            offsetgrid = inmonth * (MAXPFT+1) * MAXOUTPIX * MAXOUTLIN + inpft * MAXOUTPIX * MAXOUTLIN + outgrid;
            pftbotvalues[offsetgrid] = 0.0;
        }
    }
    
    nc_put_var_float(innetcdfid,selectedvarids[0],pftbotvalues);
    
    free(pftbotvalues);
    
}
--- */

/* NOTE: updatehurttvh1 not currently being used by the code - lpc */
void
updatehurttvh1() {
    
    float *vh1values;
    int outgrid;
    
    selectedvarcnt = 0;
    
    nc_inq(innetcdfid, &ndimsp, &nvarsp, &nattsp, &unlimdimidp);
    
    for (nvarspcnt = 0; nvarspcnt < nvarsp; nvarspcnt ++) {
        nc_inq_varname(innetcdfid, nvarspcnt, varname);
            nc_inq_var(innetcdfid, nvarspcnt, varname, &vartype, &vardimsp, &vardimidsp, &varattsp); 
        if (strcmp(varname,"HARVEST_VH1") == 0) {
            selectedvarids[0] = nvarspcnt;
            selectedvarcnt++;
            printf("Updating variable: %d %s \n",nvarspcnt,varname);
        }
        
    }
    
     nc_inq_var(innetcdfid, selectedvarids[0], varname, &vartype, &vardimsp, &vardimidsp, &varattsp); 
    varlayers = 1;
    varlayers2 = 1;
    vh1values = malloc(sizeof(float) * lonlen * latlen * varlayers * varlayers2);
    nc_get_var_float(innetcdfid,selectedvarids[0],vh1values);
    
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        vh1values[outgrid] = outhurttvh1[outgrid];
    }
    
    nc_put_var_float(innetcdfid,selectedvarids[0],vh1values); 
    free(vh1values);
    
}

/* NOTE: updatehurttvh2 not currently being used by the code - lpc */
void
updatehurttvh2() {
    
    float *vh2values;
    int outgrid;
    
    selectedvarcnt = 0;
    
    nc_inq(innetcdfid, &ndimsp, &nvarsp, &nattsp, &unlimdimidp);
    
    for (nvarspcnt = 0; nvarspcnt < nvarsp; nvarspcnt ++) {
        nc_inq_varname(innetcdfid, nvarspcnt, varname);
        nc_inq_var(innetcdfid, nvarspcnt, varname, &vartype, &vardimsp, &vardimidsp, &varattsp);
        if (strcmp(varname,"HARVEST_VH2") == 0) {
            selectedvarids[0] = nvarspcnt;
            selectedvarcnt++;
            printf("Updating variable: %d %s \n",nvarspcnt,varname);
        }
        
    }
    
    nc_inq_var(innetcdfid, selectedvarids[0], varname, &vartype, &vardimsp, &vardimidsp, &varattsp);
    varlayers = 1;
    varlayers2 = 1;
    vh2values = malloc(sizeof(float) * lonlen * latlen * varlayers * varlayers2);
    nc_get_var_float(innetcdfid,selectedvarids[0],vh2values);
    
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        vh2values[outgrid] = outhurttvh2[outgrid];
    }
    
    nc_put_var_float(innetcdfid,selectedvarids[0],vh2values); 
    free(vh2values);
    
}

/* NOTE: updatehurttsh1 not currently being used by the code - lpc */
void
updatehurttsh1() {
    
    float *sh1values;
    int outgrid;
    
    selectedvarcnt = 0;
    
    nc_inq(innetcdfid, &ndimsp, &nvarsp, &nattsp, &unlimdimidp);
    
    for (nvarspcnt = 0; nvarspcnt < nvarsp; nvarspcnt ++) {
        nc_inq_varname(innetcdfid, nvarspcnt, varname);
        nc_inq_var(innetcdfid, nvarspcnt, varname, &vartype, &vardimsp, &vardimidsp, &varattsp);
        if (strcmp(varname,"HARVEST_SH1") == 0) {
            selectedvarids[0] = nvarspcnt;
            selectedvarcnt++;
            printf("Updating variable: %d %s \n",nvarspcnt,varname);
        }
        
    }
    
    nc_inq_var(innetcdfid, selectedvarids[0], varname, &vartype, &vardimsp, &vardimidsp, &varattsp);
    varlayers = 1;
    varlayers2 = 1;
    sh1values = malloc(sizeof(float) * lonlen * latlen * varlayers * varlayers2);
    nc_get_var_float(innetcdfid,selectedvarids[0],sh1values);
    
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        sh1values[outgrid] = outhurttsh1[outgrid];
    }
    
    nc_put_var_float(innetcdfid,selectedvarids[0],sh1values); 
    free(sh1values);
    
}

/* NOTE: updatehurttsh2 not currently being used by the code - lpc */
void
updatehurttsh2() {
    
    float *sh2values;
    int outgrid;
    
    selectedvarcnt = 0;
    
    nc_inq(innetcdfid, &ndimsp, &nvarsp, &nattsp, &unlimdimidp);
    
    for (nvarspcnt = 0; nvarspcnt < nvarsp; nvarspcnt ++) {
        nc_inq_varname(innetcdfid, nvarspcnt, varname);
        nc_inq_var(innetcdfid, nvarspcnt, varname, &vartype, &vardimsp, &vardimidsp, &varattsp);
        if (strcmp(varname,"HARVEST_SH2") == 0) {
            selectedvarids[0] = nvarspcnt;
            selectedvarcnt++;
            printf("Updating variable: %d %s \n",nvarspcnt,varname);
        }
        
    }
    
    nc_inq_var(innetcdfid, selectedvarids[0], varname, &vartype, &vardimsp, &vardimidsp, &varattsp);
    varlayers = 1;
    varlayers2 = 1;
    sh2values = malloc(sizeof(float) * lonlen * latlen * varlayers * varlayers2);
    nc_get_var_float(innetcdfid,selectedvarids[0],sh2values);
    
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        sh2values[outgrid] = outhurttsh2[outgrid];
    }
    
    nc_put_var_float(innetcdfid,selectedvarids[0],sh2values); 
    free(sh2values);
    
}

/* NOTE: updatehurttsh3 not currently being used by the code - lpc */
void
updatehurttsh3() {
    
    float *sh3values;
    int outgrid;
    
    selectedvarcnt = 0;
    
    nc_inq(innetcdfid, &ndimsp, &nvarsp, &nattsp, &unlimdimidp);
    
    for (nvarspcnt = 0; nvarspcnt < nvarsp; nvarspcnt ++) {
        nc_inq_varname(innetcdfid, nvarspcnt, varname);
        nc_inq_var(innetcdfid, nvarspcnt, varname, &vartype, &vardimsp, &vardimidsp, &varattsp);
        if (strcmp(varname,"HARVEST_SH3") == 0) {
            selectedvarids[0] = nvarspcnt;
            selectedvarcnt++;
            printf("Updating variable: %d %s \n",nvarspcnt,varname);
        }
        
    }
    
    nc_inq_var(innetcdfid, selectedvarids[0], varname, &vartype, &vardimsp, &vardimidsp, &varattsp);
    varlayers = 1;
    varlayers2 = 1;
    sh3values = malloc(sizeof(float) * lonlen * latlen * varlayers * varlayers2);
    nc_get_var_float(innetcdfid,selectedvarids[0],sh3values);
    
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        sh3values[outgrid] = outhurttsh3[outgrid];
    }
    
     nc_put_var_float(innetcdfid,selectedvarids[0],sh3values);
    free(sh3values);
    
}

/* NOTE: updatehurttgrazing not currently being used by the code - lpc */
void
updatehurttgrazing() {
    
    float *grazingvalues;
    int outgrid;
    
    selectedvarcnt = 0;
    
    nc_inq(innetcdfid, &ndimsp, &nvarsp, &nattsp, &unlimdimidp);
    
    for (nvarspcnt = 0; nvarspcnt < nvarsp; nvarspcnt ++) {
        nc_inq_varname(innetcdfid, nvarspcnt, varname);
        nc_inq_var(innetcdfid, nvarspcnt, varname, &vartype, &vardimsp, &vardimidsp, &varattsp);
        if (strcmp(varname,"GRAZING") == 0) {
            selectedvarids[0] = nvarspcnt;
            selectedvarcnt++;
            printf("Updating variable: %d %s \n",nvarspcnt,varname);
        }
        
    }
    
    nc_inq_var(innetcdfid, selectedvarids[0], varname, &vartype, &vardimsp, &vardimidsp, &varattsp);
    varlayers = 1;
    varlayers2 = 1;
    grazingvalues = malloc(sizeof(float) * lonlen * latlen * varlayers * varlayers2);
    nc_get_var_float(innetcdfid,selectedvarids[0],grazingvalues);
    
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        grazingvalues[outgrid] = outhurttgrazing[outgrid];
    }
    
    nc_put_var_float(innetcdfid,selectedvarids[0],grazingvalues); 
    free(grazingvalues);
    
}

/* these readhurtt functions are not used, but still helpful in understanding the glmo data -adv */
// the exact number of valid records have to be allocated and read in, and these are different for historical and future
void
readhurttprimary(long hurttbaseyear, long hurttyear, int ISFUTURE) {
    // hurttbaseyear is not used
    
    float *primaryvalues;
    float inprimaryvalue;
    long outgrid;
    long offsetgrid;
    
    selectedvarcnt = 0;
    
    // ncid: NetCDF ID, from a previous call to nc_open or nc_create. 
    // ndimsp: Pointer to location for returned number of dimensions defined for this netCDF dataset. 
    // nvarsp: Pointer to location for returned number of variables defined for this netCDF dataset. 
    // ngattsp: Pointer to location for returned number of global attributes defined for this netCDF dataset. 
    // unlimdimidp: Pointer to location for returned ID of the unlimited dimension, if there is one for this netCDF dataset. If no unlimited length dimension has been defined, -1 is returned. 
    // formatp: Pointer to location for returned format version, one of NC_FORMAT_CLASSIC, NC_FORMAT_64BIT, NC_FORMAT_NETCDF4, NC_FORMAT_NETCDF4_CLASSIC.
    // Members of the nc_inq family of functions return information about an open netCDF dataset, given its netCDF ID
    nc_inq(innetcdfid, &ndimsp, &nvarsp, &nattsp, &unlimdimidp);

    for (nvarspcnt = 0; nvarspcnt < nvarsp; nvarspcnt ++) { // nvarspcnt loops over variables in netCDF file
        nc_inq_varname(innetcdfid, nvarspcnt, varname);  // Get name of variable indicated by nvarspcnt
        // nc_inq_var(innetcdfid, nvarspcnt, varname, &vartype, &vardimsp, &vardimidsp, &varattsp); // nc_inq_var returns all the information about a netCDF variable, given its ID
        if (strcmp(varname,"GOTHR") == 0) {
            selectedvarids[0] = nvarspcnt;
            selectedvarcnt++;
            printf("Reading variable: %d %s \n",nvarspcnt, varname);
			break;
        }        
    }
    
    // nc_inq_var(innetcdfid, selectedvarids[0], varname, &vartype, &vardimsp, &vardimidsp, &varattsp);
	if(ISFUTURE){
		varlayers = INFUTURELUTIME;
	} else {
		varlayers = INHISTLUTIME;
	}
    varlayers2 = 1;
    primaryvalues = malloc(sizeof(float) * lonlen * latlen * varlayers * varlayers2); // lonlen=720, latlen=360

    nc_get_var_float(innetcdfid, selectedvarids[0], primaryvalues);
    
    if (hurttyear >= 0) {
        for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {  // MAXOUTPIX 720; MAXOUTLIN 360
            offsetgrid = hurttyear * MAXOUTPIX * MAXOUTLIN + outgrid;
            inprimaryvalue = primaryvalues[offsetgrid];
			// put input directly into glmo array
            // this is needed only when running standalone
			glmo[outgrid][2] = inprimaryvalue;
            if (inprimaryvalue >= 0.0 && inprimaryvalue <= 1.1) {
                inhurttprimary[outgrid] = round(inprimaryvalue * 100.0);
                if (inhurttprimary[outgrid] > 100.0) {
                    inhurttprimary[outgrid] = 100.0;
                }
            }
            else {
                inhurttprimary[outgrid] = 0.0;
            }
        }
    }

    free(primaryvalues);    
}

/* NOTE: readhurttsecondary not currently being used by the code - lpc */
// the exact number of valid records have to be allocated and read in, and these are different for historical and future
void
readhurttsecondary(long hurttbaseyear, long hurttyear, int ISFUTURE) {
	// hurttbaseyear is not used
    
    float *secondaryvalues;
    float insecondaryvalue;
    long outgrid;
    long offsetgrid;
    
    selectedvarcnt = 0;
    
    nc_inq(innetcdfid, &ndimsp, &nvarsp, &nattsp, &unlimdimidp);
    
    for (nvarspcnt = 0; nvarspcnt < nvarsp; nvarspcnt ++) {
        nc_inq_varname(innetcdfid, nvarspcnt, varname);
        nc_inq_var(innetcdfid, nvarspcnt, varname, &vartype, &vardimsp, &vardimidsp, &varattsp);
        if (strcmp(varname,"GSECD") == 0) {
            selectedvarids[0] = nvarspcnt;
            selectedvarcnt++;
            printf("Reading variable: %d %s \n",nvarspcnt,varname);
			break;
        }
        
    }
    
    nc_inq_var(innetcdfid, selectedvarids[0], varname, &vartype, &vardimsp, &vardimidsp, &varattsp);
	if(ISFUTURE){
		varlayers = INFUTURELUTIME;
	} else {
		varlayers = INHISTLUTIME;
	}
    varlayers2 = 1;
    secondaryvalues = malloc(sizeof(float) * lonlen * latlen * varlayers * varlayers2);
    nc_get_var_float(innetcdfid,selectedvarids[0],secondaryvalues);
    
    if (hurttyear >= 0) {
        for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
            offsetgrid = hurttyear * MAXOUTPIX * MAXOUTLIN + outgrid;
            insecondaryvalue = secondaryvalues[offsetgrid];
			// put input directly into glmo array
            // this is only used for standalone
			glmo[outgrid][3] = insecondaryvalue;
            if (insecondaryvalue >= 0.0 && insecondaryvalue <= 1.1) {
                inhurttsecondary[outgrid] = round(insecondaryvalue * 100.0);
                if (inhurttsecondary[outgrid] > 100.0) {
                    inhurttsecondary[outgrid] = 100.0;
                }
            }
            else {
                inhurttsecondary[outgrid] = 0.0;
            }
        }
    }
    
    free(secondaryvalues);
    
}

/* NOTE: readhurttcrop not currently being used by the code - lpc */
// the exact number of valid records have to be allocated and read in, and these are different for historical and future
void
readhurttcrop(long hurttbaseyear, long hurttyear, int ISFUTURE) {
    
    float *cropvalues;
    float incropvalue;
    long outgrid;
    long offsetgrid;
    
    selectedvarcnt = 0;
    
    nc_inq(innetcdfid, &ndimsp, &nvarsp, &nattsp, &unlimdimidp);
    
    for (nvarspcnt = 0; nvarspcnt < nvarsp; nvarspcnt ++) {
        nc_inq_varname(innetcdfid, nvarspcnt, varname);
        /*      nc_inq_var(innetcdfid, nvarspcnt, varname, &vartype, &vardimsp, &vardimidsp, &varattsp); */
        if (strcmp(varname,"GCROP") == 0) {
            selectedvarids[0] = nvarspcnt;
            selectedvarcnt++;
            printf("Reading variable: %d %s \n",nvarspcnt,varname);
			break;
        }
        
    }
    
    /*  nc_inq_var(innetcdfid, selectedvarids[0], varname, &vartype, &vardimsp, &vardimidsp, &varattsp); */
	if(ISFUTURE){
		varlayers = INFUTURELUTIME;
	} else {
		varlayers = INHISTLUTIME;
	}
    varlayers2 = 1;
    cropvalues = malloc(sizeof(float) * lonlen * latlen * varlayers * varlayers2);
    nc_get_var_float(innetcdfid,selectedvarids[0],cropvalues);
    
    //printf("hurttbaseyear: %li \n",hurttbaseyear);
    //printf("hurttyear: %li \n",hurttyear);
    
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        // hurttbaseyear is set to 0. This means that it is reading values for year 1850
		// this is not being used for the reference; this is overwritten by values from the dynamic file
        offsetgrid = hurttbaseyear * MAXOUTPIX * MAXOUTLIN + outgrid;
        incropvalue = cropvalues[offsetgrid];
        if (incropvalue >= 0.0 && incropvalue <= 1.1) {
            inhurttbasecrop[outgrid] = round(incropvalue * 100.0);
            if (inhurttbasecrop[outgrid] > 100.0) {
                inhurttbasecrop[outgrid] = 100.0;
            }
        }
        else {
            inhurttbasecrop[outgrid] = 0.0;
        }
    }
    
    if (hurttyear >= 0) {
        for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
            offsetgrid = hurttyear * MAXOUTPIX * MAXOUTLIN + outgrid;
            incropvalue = cropvalues[offsetgrid];
			// put input directly into glmo array
            // this is for standalone runds only
			glmo[outgrid][0] = incropvalue;
            if (incropvalue >= 0.0 && incropvalue <= 1.1) {
                inhurttcrop[outgrid] = round(incropvalue * 100.0);
                if (inhurttcrop[outgrid] > 100.0) {
                    inhurttcrop[outgrid] = 100.0;
                }
            }
            else {
                inhurttcrop[outgrid] = 0.0;
            }
        }
    }
    
    free(cropvalues);
    
}

/* NOTE: readhurttpasture not currently being used by the code - lpc */
// the exact number of valid records have to be allocated and read in, and these are different for historical and future
void
readhurttpasture(long hurttbaseyear, long hurttyear, int ISFUTURE) {
    
    float *pasturevalues;
    float inpasturevalue;
    long outgrid;
    long offsetgrid;
    
    selectedvarcnt = 0;
    
    nc_inq(innetcdfid, &ndimsp, &nvarsp, &nattsp, &unlimdimidp);
    
    for (nvarspcnt = 0; nvarspcnt < nvarsp; nvarspcnt ++) {
        nc_inq_varname(innetcdfid, nvarspcnt, varname);
        /*      nc_inq_var(innetcdfid, nvarspcnt, varname, &vartype, &vardimsp, &vardimidsp, &varattsp); */
        if (strcmp(varname,"GPAST") == 0) {
            selectedvarids[0] = nvarspcnt;
            selectedvarcnt++;
            printf("Reading variable: %d %s \n",nvarspcnt,varname);
			break;
        }
        
    }
    /*  nc_inq_var(innetcdfid, selectedvarids[0], varname, &vartype, &vardimsp, &vardimidsp, &varattsp); */
	if(ISFUTURE){
		varlayers = INFUTURELUTIME;
	} else {
		varlayers = INHISTLUTIME;
	}
    varlayers2 = 1;
    pasturevalues = malloc(sizeof(float) * lonlen * latlen * varlayers * varlayers2);
    nc_get_var_float(innetcdfid,selectedvarids[0],pasturevalues);
    
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        offsetgrid = hurttbaseyear * MAXOUTPIX * MAXOUTLIN + outgrid;
        inpasturevalue = pasturevalues[offsetgrid];
        if (inpasturevalue >= 0.0 && inpasturevalue <= 1.1) {
            inhurttbasepasture[outgrid] = round(inpasturevalue * 100.0);
            if (inhurttbasepasture[outgrid] > 100.0) {
                inhurttbasepasture[outgrid] = 100.0;
            }
        }
        else {
            inhurttbasepasture[outgrid] = 0.0;
        }
    }
    
    if (hurttyear >= 0) {
        for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
            offsetgrid = hurttyear * MAXOUTPIX * MAXOUTLIN + outgrid;
            inpasturevalue = pasturevalues[offsetgrid];
			// put input directly into glmo array
            // this is for standalone only
			glmo[outgrid][1] = inpasturevalue;
            /* Bugfix. Changed from 1.0 to 1.1 to be consistent with rest of code. -adv */
            if (inpasturevalue >= 0.0 && inpasturevalue <= 1.1) {
                inhurttpasture[outgrid] = round(inpasturevalue * 100.0);
                if (inhurttpasture[outgrid] > 100.0) {
                    inhurttpasture[outgrid] = 100.0;
                }
            }
            else {
                inhurttpasture[outgrid] = 0.0;
            }
        }
    }
    
    free(pasturevalues);
    
}

/* NOTE: readhurttvh1 not currently being used by the code - lpc */
// the exact number of valid records have to be allocated and read in, and these are different for historical and future
void
readhurttvh1(long hurttbaseyear, long hurttyear, int ISFUTURE) {
	// hurttbaseyear is not used
    
    float *vh1values;
    float invh1value;
    long outgrid;
    long offsetgrid;
	const size_t start[] = {1,1,1};
	int udimid;
	size_t udimlen;
	
	if(ISFUTURE){
		varlayers = INFUTUREHARVTIME;
	} else {
		varlayers = INHISTHARVTIME;
	}
	
	const size_t count[] ={varlayers,MAXOUTLIN,MAXOUTPIX};
    
    selectedvarcnt = 0;
    
    nc_inq(innetcdfid, &ndimsp, &nvarsp, &nattsp, &unlimdimidp);
	
	nc_inq_dimid(innetcdfid, "TIME", &udimid);
	nc_inq_dimlen(innetcdfid, udimid, &udimlen);
	
    for (nvarspcnt = 0; nvarspcnt < nvarsp; nvarspcnt ++) {
        nc_inq_varname(innetcdfid, nvarspcnt, varname);
        //nc_inq_var(innetcdfid, nvarspcnt, varname, &vartype, &vardimsp, &vardimidsp, &varattsp);
        if (strcmp(varname,"GFVH1") == 0) {
            selectedvarids[0] = nvarspcnt;
            selectedvarcnt++;
            printf("Reading variable: %d %s \n",nvarspcnt,varname);
			break;
        }
        
    }
    nc_inq_var(innetcdfid, selectedvarids[0], varname, &vartype, &vardimsp, &vardimidsp, &varattsp);
    varlayers2 = 1;
    vh1values = malloc(sizeof(float) * lonlen * latlen * varlayers * varlayers2);
	// vara doesn't work on the new file either, but var does
    //nc_get_vara_float(innetcdfid,selectedvarids[0],start,count,vh1values);
	nc_get_var_float(innetcdfid,selectedvarids[0],vh1values);
	//printf("varlayers=%i\tcount0=%zu\tcount1=%zu\tcount2=%zu\tvarid=%i\n", varlayers, count[0], count[1], count[2], selectedvarids[0]);

    if (hurttyear >= 0) {
        for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
            offsetgrid = hurttyear * MAXOUTPIX * MAXOUTLIN + outgrid;
            invh1value = vh1values[offsetgrid];
            if (invh1value >= 0.0 && invh1value <= 1.1) {
                inhurttvh1[outgrid] = (invh1value * 100.0);
                if (inhurttvh1[outgrid] > 100.0) {
                    inhurttvh1[outgrid] = 100.0;
                }
            }
            else {
                inhurttvh1[outgrid] = 0.0;
            }
			//printf("outgrid=%li\toffsetgrid=%li\tinval=%f\tihvh1=%f\tvh1val=%f\n", outgrid, offsetgrid, invh1value, inhurttvh1[outgrid], vh1values[offsetgrid]);
        }
    }
    
    free(vh1values);
    
}

/* NOTE: readhurttvh2 not currently being used by the code - lpc */
// the exact number of valid records have to be allocated and read in, and these are different for historical and future
void
readhurttvh2(long hurttbaseyear, long hurttyear, int ISFUTURE) {
	// hurttbaseyear is not used
    
    float *vh2values;
    float invh2value;
    long outgrid;
    long offsetgrid;
	const size_t start[] = {1,1,1};
	
	if(ISFUTURE){
		varlayers = INFUTUREHARVTIME;
	} else {
		varlayers = INHISTHARVTIME;
	}
	
	const size_t count[] ={varlayers,MAXOUTLIN,MAXOUTPIX};
    
    selectedvarcnt = 0;
    
    nc_inq(innetcdfid, &ndimsp, &nvarsp, &nattsp, &unlimdimidp);
    
    for (nvarspcnt = 0; nvarspcnt < nvarsp; nvarspcnt ++) {
        nc_inq_varname(innetcdfid, nvarspcnt, varname);
        /*      nc_inq_var(innetcdfid, nvarspcnt, varname, &vartype, &vardimsp, &vardimidsp, &varattsp); */
        if (strcmp(varname,"GFVH2") == 0) {
            selectedvarids[0] = nvarspcnt;
            selectedvarcnt++;
            printf("Reading variable: %d %s \n",nvarspcnt,varname);
			break;
        }
        
    }
	
    /*  nc_inq_var(innetcdfid, selectedvarids[0], varname, &vartype, &vardimsp, &vardimidsp, &varattsp); */
    varlayers2 = 1;
    vh2values = malloc(sizeof(float) * lonlen * latlen * varlayers * varlayers2);
	// vara doesn't work on the new file either, but var does
	nc_get_var_float(innetcdfid,selectedvarids[0],vh2values);
	//nc_get_vara_float(innetcdfid,selectedvarids[0],start,count,vh2values);
    
    if (hurttyear >= 0) {
        for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
            offsetgrid = hurttyear * MAXOUTPIX * MAXOUTLIN + outgrid;
            invh2value = vh2values[offsetgrid];
            if (invh2value >= 0.0 && invh2value <= 1.1) {
                inhurttvh2[outgrid] = (invh2value * 100.0);
                if (inhurttvh2[outgrid] > 100.0) {
                    inhurttvh2[outgrid] = 100.0;
                }
            }
            else {
                inhurttvh2[outgrid] = 0.0;
            }
        }
    }
    
    free(vh2values);
    
}

/* NOTE: readhurttsh1 not currently being used by the code - lpc */
// the exact number of valid records have to be allocated and read in, and these are different for historical and future
void
readhurttsh1(long hurttbaseyear, long hurttyear, int ISFUTURE) {
	// hurttbaseyear is not used
    
    float *sh1values;
    float insh1value;
    long outgrid;
    long offsetgrid;
	const size_t start[] = {1,1,1};
	
	if(ISFUTURE){
		varlayers = INFUTUREHARVTIME;
	} else {
		varlayers = INHISTHARVTIME;
	}
	
	const size_t count[] ={varlayers,MAXOUTLIN,MAXOUTPIX};
    
    selectedvarcnt = 0;
    
    nc_inq(innetcdfid, &ndimsp, &nvarsp, &nattsp, &unlimdimidp);
    
    for (nvarspcnt = 0; nvarspcnt < nvarsp; nvarspcnt ++) {
        nc_inq_varname(innetcdfid, nvarspcnt, varname);
        /*      nc_inq_var(innetcdfid, nvarspcnt, varname, &vartype, &vardimsp, &vardimidsp, &varattsp); */
        if (strcmp(varname,"GFSH1") == 0) {
            selectedvarids[0] = nvarspcnt;
            selectedvarcnt++;
            printf("Reading variable: %d %s \n",nvarspcnt,varname);
			break;
        }
        
    }
    
    /*  nc_inq_var(innetcdfid, selectedvarids[0], varname, &vartype, &vardimsp, &vardimidsp, &varattsp); */
    varlayers2 = 1;
    sh1values = malloc(sizeof(float) * lonlen * latlen * varlayers * varlayers2);
	// vara doesn't work on the new file either, but var does
	nc_get_var_float(innetcdfid,selectedvarids[0],sh1values);
    //nc_get_vara_float(innetcdfid,selectedvarids[0],start,count,sh1values);
    
    if (hurttyear >= 0) {
        for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
            offsetgrid = hurttyear * MAXOUTPIX * MAXOUTLIN + outgrid;
            insh1value = sh1values[offsetgrid];
            if (insh1value >= 0.0 && insh1value <= 1.1) {
                inhurttsh1[outgrid] = (insh1value * 100.0);
                if (inhurttsh1[outgrid] > 100.0) {
                    inhurttsh1[outgrid] = 100.0;
                }
            }
            else {
                inhurttsh1[outgrid] = 0.0;
            }
        }
    }
    
    free(sh1values);
    
}

/* NOTE: readhurttsh2 not currently being used by the code - lpc */
// the exact number of valid records have to be allocated and read in, and these are different for historical and future
void
readhurttsh2(long hurttbaseyear, long hurttyear, int ISFUTURE) {
	// hurttbaseyear is not used
    
    float *sh2values;
    float insh2value;
    long outgrid;
    long offsetgrid;
	const size_t start[] = {1,1,1};
	
	if(ISFUTURE){
		varlayers = INFUTUREHARVTIME;
	} else {
		varlayers = INHISTHARVTIME;
	}
	
	const size_t count[] ={varlayers,MAXOUTLIN,MAXOUTPIX};
    
    selectedvarcnt = 0;
    
    nc_inq(innetcdfid, &ndimsp, &nvarsp, &nattsp, &unlimdimidp);
    
    for (nvarspcnt = 0; nvarspcnt < nvarsp; nvarspcnt ++) {
        nc_inq_varname(innetcdfid, nvarspcnt, varname);
        /*      nc_inq_var(innetcdfid, nvarspcnt, varname, &vartype, &vardimsp, &vardimidsp, &varattsp); */
        if (strcmp(varname,"GFSH2") == 0) {
            selectedvarids[0] = nvarspcnt;
            selectedvarcnt++;
            printf("Reading variable: %d %s \n",nvarspcnt,varname);
			break;
        }
        
    }
    
    /*  nc_inq_var(innetcdfid, selectedvarids[0], varname, &vartype, &vardimsp, &vardimidsp, &varattsp); */
    varlayers2 = 1;
    sh2values = malloc(sizeof(float) * lonlen * latlen * varlayers * varlayers2);
	// vara doesn't work on the new file either, but var does
	nc_get_var_float(innetcdfid,selectedvarids[0],sh2values);
    //nc_get_vara_float(innetcdfid,selectedvarids[0],start,count,sh2values);
    
    if (hurttyear >= 0) {
        for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
            offsetgrid = hurttyear * MAXOUTPIX * MAXOUTLIN + outgrid;
            insh2value = sh2values[offsetgrid];
            if (insh2value >= 0.0 && insh2value <= 1.1) {
                inhurttsh2[outgrid] = (insh2value * 100.0);
                if (inhurttsh2[outgrid] > 100.0) {
                    inhurttsh2[outgrid] = 100.0;
                }
            }
            else {
                inhurttsh2[outgrid] = 0.0;
            }
        }
    }
    
    free(sh2values);
    
}

/* NOTE: readhurttsh3 not currently being used by the code - lpc */
// the exact number of valid records have to be allocated and read in, and these are different for historical and future
void
readhurttsh3(long hurttbaseyear, long hurttyear, int ISFUTURE) {
	// hurttbaseyear is not used
    
    float *sh3values;
    float insh3value;
    long outgrid;
    long offsetgrid;
	const size_t start[] = {1,1,1};
	
	if(ISFUTURE){
		varlayers = INFUTUREHARVTIME;
	} else {
		varlayers = INHISTHARVTIME;
	}
	
	const size_t count[] ={varlayers,MAXOUTLIN,MAXOUTPIX};
    
    selectedvarcnt = 0;
    
    nc_inq(innetcdfid, &ndimsp, &nvarsp, &nattsp, &unlimdimidp);
    
    for (nvarspcnt = 0; nvarspcnt < nvarsp; nvarspcnt ++) {
        nc_inq_varname(innetcdfid, nvarspcnt, varname);
        /*      nc_inq_var(innetcdfid, nvarspcnt, varname, &vartype, &vardimsp, &vardimidsp, &varattsp); */
        if (strcmp(varname,"GFSH3") == 0) {
            selectedvarids[0] = nvarspcnt;
            selectedvarcnt++;
            printf("Reading variable: %d %s \n",nvarspcnt,varname);
			break;
        }
        
    }
    
    /*  nc_inq_var(innetcdfid, selectedvarids[0], varname, &vartype, &vardimsp, &vardimidsp, &varattsp); */
    varlayers2 = 1;
    sh3values = malloc(sizeof(float) * lonlen * latlen * varlayers * varlayers2);
	// vara doesn't work on the new file either, but var does
	nc_get_var_float(innetcdfid,selectedvarids[0],sh3values);
    //nc_get_vara_float(innetcdfid,selectedvarids[0],start,count,sh3values);
    
    if (hurttyear >= 0) {
        for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
            offsetgrid = hurttyear * MAXOUTPIX * MAXOUTLIN + outgrid;
            insh3value = sh3values[offsetgrid];
            if (insh3value >= 0.0 && insh3value <= 1.1) {
                inhurttsh3[outgrid] = (insh3value * 100.0);
                if (inhurttsh3[outgrid] > 100.0) {
                    inhurttsh3[outgrid] = 100.0;
                }
            }
            else {
                inhurttsh3[outgrid] = 0.0;
            }
        }
    }
    
    free(sh3values);
    
}

/* the output (inhurttbasecrop) of this function is not used;
 instead the glmo crop data is used directly for the pfts
 this has been modified to not have any arguments because there is only one record
 Note: this function is no longer needed due to the dynamic hurtt pl file
 this isn't used by standalone either
 -adv

void
readhurttbasecrop() {
    
    float *cropvalues;
    float incropvalue;
    long outgrid;
    long offsetgrid;
    
    selectedvarcnt = 0;
    
    nc_inq(innetcdfid, &ndimsp, &nvarsp, &nattsp, &unlimdimidp);
    
    for (nvarspcnt = 0; nvarspcnt < nvarsp; nvarspcnt ++) {
        nc_inq_varname(innetcdfid, nvarspcnt, varname);
        //nc_inq_var(innetcdfid, nvarspcnt, varname, &vartype, &vardimsp, &vardimidsp, &varattsp);
        if (strcmp(varname,"GCROP") == 0) {
            selectedvarids[0] = nvarspcnt;
            selectedvarcnt++;
            printf("Reading variable: %d %s \n",nvarspcnt,varname);
			break;
        }
        
    }
    
    //nc_inq_var(innetcdfid, selectedvarids[0], varname, &vartype, &vardimsp, &vardimidsp, &varattsp);
    //varlayers = 506;
    varlayers = 1;
    varlayers2 = 1;
    cropvalues = malloc(sizeof(float) * lonlen * latlen * varlayers * varlayers2);
    nc_get_var_float(innetcdfid,selectedvarids[0],cropvalues);
    
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        //offsetgrid = hurttbaseyear * MAXOUTPIX * MAXOUTLIN + outgrid;
		offsetgrid = outgrid;	// hurttbaseyear used to be an argument that was an index set == 0 for this application
        incropvalue = cropvalues[offsetgrid];
        if (incropvalue >= 0.0 && incropvalue <= 1.1) {
            inhurttbasecrop[outgrid] = round(incropvalue * 100.0);
            if (inhurttbasecrop[outgrid] > 100.0) {
                inhurttbasecrop[outgrid] = 100.0;
            }
        }
        else {
            inhurttbasecrop[outgrid] = 0.0;
        }
    }
    
    free(cropvalues);
    
}
--- */

/* this has been modified to not have any arguments because there is only one record
 Note: this function is no longer needed due to the dynamic hurtt pl file
 this isn't used by standalone either
 -adv

void
readhurttbasepasture() {
    
    float *pasturevalues;
    float inpasturevalue;
    long outgrid;
    long offsetgrid;
    
    selectedvarcnt = 0;
    
    nc_inq(innetcdfid, &ndimsp, &nvarsp, &nattsp, &unlimdimidp);
    
    for (nvarspcnt = 0; nvarspcnt < nvarsp; nvarspcnt ++) {
        nc_inq_varname(innetcdfid, nvarspcnt, varname);
        //nc_inq_var(innetcdfid, nvarspcnt, varname, &vartype, &vardimsp, &vardimidsp, &varattsp);
        if (strcmp(varname,"GPAST") == 0) {
            selectedvarids[0] = nvarspcnt;
            selectedvarcnt++;
            printf("Reading variable: %d %s \n",nvarspcnt,varname);
			break;
        }
        
    }
    
    //nc_inq_var(innetcdfid, selectedvarids[0], varname, &vartype, &vardimsp, &vardimidsp, &varattsp);
    //varlayers = 506;
    varlayers = 1;
    varlayers2 = 1;
    pasturevalues = malloc(sizeof(float) * lonlen * latlen * varlayers * varlayers2);
    nc_get_var_float(innetcdfid,selectedvarids[0],pasturevalues);
    
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        //offsetgrid = hurttbaseyear * MAXOUTPIX * MAXOUTLIN + outgrid;
		offsetgrid = outgrid;	// hurttbaseyear used to be an argument that was an index set == 0 for this application
        inpasturevalue = pasturevalues[offsetgrid];
        if (inpasturevalue >= 0.0 && inpasturevalue <= 1.1) {
            inhurttbasepasture[outgrid] = round(inpasturevalue * 100.0);
            if (inhurttbasepasture[outgrid] > 100.0) {
                inhurttbasepasture[outgrid] = 100.0;
            }
        }
        else {
            inhurttbasepasture[outgrid] = 0.0;
        }
    }
    
    free(pasturevalues);
    
}

--- */

#endif

/////////////////////////////////////////////////////////////////////
/* here start the useful functions in this code -adv */

/* new function to read the first year value in the luh file data */

void
getinithurttyear(long *year) {
	long year_ind;
	int time_dimid;
	int year_varid;
	float *temp_float;
	size_t numrecs;
	
	nc_inq_dimid(innetcdfid, "TIME", &time_dimid);
	nc_inq_dimlen(innetcdfid, time_dimid, &numrecs);
	temp_float = malloc(sizeof(float) * numrecs);
	nc_inq_varid(innetcdfid, "TIME", &year_varid);
	nc_get_var_float(innetcdfid, year_varid, temp_float);
	*year = (long) temp_float[0];
	free(temp_float);
}

/* new function for reading crop from dynamic hurtt pl file */
/* the output (inhurttbasecrop) of this function is not used;
	instead the glmo crop data is used directly for the pfts -adv */
/* but set it up for reading anyway under the new reference year system;
	modyear is the reference year, which is the year prior to outyear and happens to be the actual cesm model year -adv */

void
readhurttdyncrop(long modyear) {
    
    float *cropvalues;
    float incropvalue;
    long outgrid;
	
    long year_ind;
	int year_varid;
	size_t numrecs, *start, *count;
	float *years;
	
    selectedvarcnt = 0;
    
    nc_inq(innetcdfid, &ndimsp, &nvarsp, &nattsp, &unlimdimidp);
    /* get the index of the crop variable */
    for (nvarspcnt = 0; nvarspcnt < nvarsp; nvarspcnt ++) {
        nc_inq_varname(innetcdfid, nvarspcnt, varname);
        /*      nc_inq_var(innetcdfid, nvarspcnt, varname, &vartype, &vardimsp, &vardimidsp, &varattsp); */
        if (strcmp(varname,"GCROP") == 0) {
            selectedvarids[0] = nvarspcnt;
            selectedvarcnt++;
            printf("Reading variable: %d %s \n",nvarspcnt,varname);
			break;
        }
        
    }
	
	/* get the index of modyear */
	nc_inq_dimlen(innetcdfid, unlimdimidp, &numrecs);
	years = malloc(sizeof(float) * numrecs);
	nc_inq_varid(innetcdfid, "TIME", &year_varid);
	nc_get_var_float(innetcdfid, year_varid, years);
	for(year_ind = 0; year_ind < ((long) numrecs); year_ind++) {
		if (((long) years[year_ind]) == modyear) {
			break;
		}
	}
	if(year_ind == ((long) numrecs)) {
		printf("Error reading reference year glm crop data %li from invalid index %li\n",modyear,year_ind);
	}
	
	start = calloc(ndimsp, sizeof(size_t));
	count = calloc(ndimsp, sizeof(size_t));
	start[0] = year_ind;
	start[1] = 0;
	start[2] = 0;
	count[0] = 1;
	count[1] = latlen;
	count[2] = lonlen;
    
    varlayers = 1;
    varlayers2 = 1;
    cropvalues = malloc(sizeof(float) * lonlen * latlen * varlayers * varlayers2);
    nc_get_vara_float(innetcdfid,selectedvarids[0], start, count, cropvalues);
    
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        incropvalue = cropvalues[outgrid];
        if (incropvalue >= 0.0 && incropvalue <= 1.1) {
            inhurttbasecrop[outgrid] = round(incropvalue * 100.0);
            if (inhurttbasecrop[outgrid] > 100.0) {
                inhurttbasecrop[outgrid] = 100.0;
            }
        }
        else {
            inhurttbasecrop[outgrid] = 0.0;
        }
    }
    
    free(cropvalues);
	free(years);
	free(start);
	free(count);
    
}

/* new function for reading pasture from dynamic hurtt pl file */
/* modyear is the reference year, which is the year prior to outyear and happens to be the actual cesm model year -adv */

void
readhurttdynpasture(long modyear) {
    
    float *pasturevalues;
    float inpasturevalue;
    long outgrid;
    
    long year_ind;
	int year_varid;
	size_t numrecs, *start, *count;
	float *years;
	
    selectedvarcnt = 0;
    
    nc_inq(innetcdfid, &ndimsp, &nvarsp, &nattsp, &unlimdimidp);
    
    for (nvarspcnt = 0; nvarspcnt < nvarsp; nvarspcnt ++) {
        nc_inq_varname(innetcdfid, nvarspcnt, varname);
        /*      nc_inq_var(innetcdfid, nvarspcnt, varname, &vartype, &vardimsp, &vardimidsp, &varattsp); */
        if (strcmp(varname,"GPAST") == 0) {
            selectedvarids[0] = nvarspcnt;
            selectedvarcnt++;
            printf("Reading variable: %d %s \n",nvarspcnt,varname);
			break;
        }
        
    }
	
	/* get the index of modyear */
	nc_inq_dimlen(innetcdfid, unlimdimidp, &numrecs);
	years = malloc(sizeof(float) * numrecs);
	nc_inq_varid(innetcdfid, "TIME", &year_varid);
	nc_get_var_float(innetcdfid, year_varid, years);
	for(year_ind = 0; year_ind < ((long) numrecs); year_ind++) {
		if (((long) years[year_ind]) == modyear) {
			break;
		}
	}
	if(year_ind == ((long) numrecs)) {
		printf("Error reading reference year glm pasture data %li from invalid index %li\n",modyear,year_ind);
	}
	
	start = calloc(ndimsp, sizeof(size_t));
	count = calloc(ndimsp, sizeof(size_t));
	start[0] = year_ind;
	start[1] = 0;
	start[2] = 0;
	count[0] = 1;
	count[1] = latlen;
	count[2] = lonlen;
    
    varlayers = 1;
    varlayers2 = 1;
    pasturevalues = malloc(sizeof(float) * lonlen * latlen * varlayers * varlayers2);
    nc_get_vara_float(innetcdfid,selectedvarids[0], start, count, pasturevalues);
    
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        inpasturevalue = pasturevalues[outgrid];
        if (inpasturevalue >= 0.0 && inpasturevalue <= 1.1) {
            inhurttbasepasture[outgrid] = round(inpasturevalue * 100.0);
            if (inhurttbasepasture[outgrid] > 100.0) {
                inhurttbasepasture[outgrid] = 100.0;
            }
        }
        else {
            inhurttbasepasture[outgrid] = 0.0;
        }
    }
    
    free(pasturevalues);
	free(years);
	free(start);
	free(count);
    
}

/* new function for reading primary land from dynamic hurtt pl file */
/* modyear is the reference year, which is the year prior to outyear and happens to be the actual cesm model year -adv */

void
readhurttdynprimary(long modyear) {
    
    float *primvalues;
    float inprimvalue;
    long outgrid;
    
    long year_ind;
	int year_varid;
	size_t numrecs, *start, *count;
	float *years;
	
    selectedvarcnt = 0;
    
    nc_inq(innetcdfid, &ndimsp, &nvarsp, &nattsp, &unlimdimidp);
    
    for (nvarspcnt = 0; nvarspcnt < nvarsp; nvarspcnt ++) {
        nc_inq_varname(innetcdfid, nvarspcnt, varname);
        nc_inq_var(innetcdfid, nvarspcnt, varname, &vartype, &vardimsp, &vardimidsp, &varattsp);
        if (strcmp(varname,"GOTHR") == 0) {
            selectedvarids[0] = nvarspcnt;
            selectedvarcnt++;
            printf("Reading variable: %d %s \n",nvarspcnt,varname);
			break;
        }
        
    }
	
	/* get the index of modyear */
	nc_inq_dimlen(innetcdfid, unlimdimidp, &numrecs);
	years = malloc(sizeof(float) * numrecs);
	nc_inq_varid(innetcdfid, "TIME", &year_varid);
	nc_get_var_float(innetcdfid, year_varid, years);
	for(year_ind = 0; year_ind < ((long) numrecs); year_ind++) {
            // printf("%d years: %f %d\n", year_ind, years[year_ind], modyear);
		if (((long) years[year_ind]) == modyear) {
			break;
		}
	}
	if(year_ind == ((long) numrecs)) {
		printf("Error reading harvest year glm primary data %li from invalid index %li\n",modyear,year_ind);
	}
	
	start = calloc(ndimsp, sizeof(size_t));
	count = calloc(ndimsp, sizeof(size_t));
	start[0] = year_ind;
	start[1] = 0;
	start[2] = 0;
	count[0] = 1;
	count[1] = latlen;
	count[2] = lonlen;
    
    varlayers = 1;
    varlayers2 = 1;
    primvalues = malloc(sizeof(float) * lonlen * latlen * varlayers * varlayers2);
    nc_get_vara_float(innetcdfid,selectedvarids[0], start, count, primvalues);
    
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        inprimvalue = primvalues[outgrid];
        if (inprimvalue >= 0.0 && inprimvalue <= 1.1) {
            prevprimary[outgrid] = round(inprimvalue * 100.0);
            if (prevprimary[outgrid] > 100.0) {
                prevprimary[outgrid] = 100.0;
            }
        }
        else {
            prevprimary[outgrid] = 0.0;
        }
    }
    
    free(primvalues);
	free(years);
	free(start);
	free(count);
    
}

/* new function for reading secondary land from dynamic hurtt pl file */
/* modyear is the reference year, which is the year prior to outyear and happens to be the actual cesm model year -adv */

void
readhurttdynsecondary(long modyear) {
    
    float *secvalues;
    float insecvalue;
    long outgrid;
    
    long year_ind;
	int year_varid;
	size_t numrecs, *start, *count;
	float *years;
	
    selectedvarcnt = 0;
    
    nc_inq(innetcdfid, &ndimsp, &nvarsp, &nattsp, &unlimdimidp);
    
    for (nvarspcnt = 0; nvarspcnt < nvarsp; nvarspcnt ++) {
        nc_inq_varname(innetcdfid, nvarspcnt, varname);
        nc_inq_var(innetcdfid, nvarspcnt, varname, &vartype, &vardimsp, &vardimidsp, &varattsp);
        if (strcmp(varname,"GSECD") == 0) {
            selectedvarids[0] = nvarspcnt;
            selectedvarcnt++;
            printf("Reading variable: %d %s \n",nvarspcnt,varname);
			break;
        }
        
    }
	
	/* get the index of modyear */
	nc_inq_dimlen(innetcdfid, unlimdimidp, &numrecs);
	years = malloc(sizeof(float) * numrecs);
	nc_inq_varid(innetcdfid, "TIME", &year_varid);
	nc_get_var_float(innetcdfid, year_varid, years);
	for(year_ind = 0; year_ind < ((long) numrecs); year_ind++) {
		if (((long) years[year_ind]) == modyear) {
			break;
		}
	}
	if(year_ind == ((long) numrecs)) {
		printf("Error reading harvest year glm secondary data %li from invalid index %li\n",modyear,year_ind);
	}
	
	start = calloc(ndimsp, sizeof(size_t));
	count = calloc(ndimsp, sizeof(size_t));
	start[0] = year_ind;
	start[1] = 0;
	start[2] = 0;
	count[0] = 1;
	count[1] = latlen;
	count[2] = lonlen;
    
    varlayers = 1;
    varlayers2 = 1;
    secvalues = malloc(sizeof(float) * lonlen * latlen * varlayers * varlayers2);
    nc_get_vara_float(innetcdfid,selectedvarids[0], start, count, secvalues);
    
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        insecvalue = secvalues[outgrid];
        if (insecvalue >= 0.0 && insecvalue <= 1.1) {
            prevsecondary[outgrid] = round(insecvalue * 100.0);
            if (prevsecondary[outgrid] > 100.0) {
                prevsecondary[outgrid] = 100.0;
            }
        }
        else {
            prevsecondary[outgrid] = 0.0;
        }
    }
    
    free(secvalues);
	free(years);
	free(start);
	free(count);
    
}

/* new function to add crop, pasture, primary, and secondary data record to the hurtt reference file - adv
	outyear is the year of glm land use data operated on
	glmo[][GLMONFLDS] is the imput glmo data
		####_index is the glmo land use type index in the GLMONFLDS dimension (0=crop, 1=pasture, 2=primary, 3=secondary)
 */

// make sure this will run with the standalone version where glmo is global and is not an argument here
#ifdef STANDALONE
void
writehurttdynfile(long outyear) {
#else
void
writehurttdynfile(long outyear, float glmo[][GLMONFLDS]) {
#endif
	
	float *values;
    long outgrid;
	int crop_index = 0;
	int past_index = 1;
	int prim_index = 2;
	int secd_index = 3;
	int year_varid;
	size_t numrecs, *start, *count;
	float writeyear;
	size_t countone = 1;
	
	writeyear = (float) outyear;
    
    nc_inq(innetcdfid, &ndimsp, &nvarsp, &nattsp, &unlimdimidp);
	
	/* get the number of current records - only need this once */
	/* this works here because there is only one call to this function per year */
	nc_inq_dimlen(innetcdfid, unlimdimidp, &numrecs);
	//printf("\n\n\n %%%%%%%\n\n\n\n%%%% writehurttdynfile %d\n", numrecs);
	/* write the year info */
	nc_inq_varid(innetcdfid, "TIME", &year_varid);
	nc_put_vara_float(innetcdfid, year_varid, &numrecs, &countone, &writeyear);
	
	start = calloc(ndimsp, sizeof(size_t));
	count = calloc(ndimsp, sizeof(size_t));
	start[0] = numrecs;
	start[1] = 0;
	start[2] = 0;
	count[0] = 1;
	count[1] = latlen;
	count[2] = lonlen;
    
	/* get the indices of the variables */
	selectedvarcnt = 0;
    for (nvarspcnt = 0; nvarspcnt < nvarsp; nvarspcnt ++) {
        nc_inq_varname(innetcdfid, nvarspcnt, varname);
        if (strcmp(varname,"GCROP") == 0) {
            selectedvarids[crop_index] = nvarspcnt;
            selectedvarcnt++;
            printf("Reading variable: %d %s \n",nvarspcnt,varname);
        }
		else if (strcmp(varname,"GPAST") == 0) {
            selectedvarids[past_index] = nvarspcnt;
            selectedvarcnt++;
            printf("Reading variable: %d %s \n",nvarspcnt,varname);
        }
		else if (strcmp(varname,"GOTHR") == 0) {
            selectedvarids[prim_index] = nvarspcnt;
            selectedvarcnt++;
            printf("Reading variable: %d %s \n",nvarspcnt,varname);
        }
		else if (strcmp(varname,"GSECD") == 0) {
            selectedvarids[secd_index] = nvarspcnt;
            selectedvarcnt++;
            printf("Reading variable: %d %s \n",nvarspcnt,varname);
        }
    }
	
	values = calloc(lonlen * latlen, sizeof(float));
	
	/* write crop */
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        values[outgrid] = glmo[outgrid][crop_index];
    }
    nc_put_vara_float(innetcdfid,selectedvarids[crop_index], start, count, values);
    
	/* write pasture */
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        values[outgrid] = glmo[outgrid][past_index];
    }
    nc_put_vara_float(innetcdfid,selectedvarids[past_index], start, count, values);
	
	/* write primary */
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        values[outgrid] = glmo[outgrid][prim_index];
    }
    nc_put_vara_float(innetcdfid,selectedvarids[prim_index], start, count, values);
	
	/* write secondary */
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        values[outgrid] = glmo[outgrid][secd_index];
    }
    nc_put_vara_float(innetcdfid,selectedvarids[secd_index], start, count, values);
    // nc_inq_dimlen(innetcdfid, unlimdimidp, &numrecs);
    // printf("\n\n\n ^^^^^^^^^^\n\n\n\n%%%% writehurttdynfile %d\n", numrecs);
	free(start);
	free(count);
	free(values);
}

void
copyarray(float array[MAXOUTPIX * MAXOUTLIN], int index, float arraypft[][MAXOUTPIX * MAXOUTLIN]) {
    
    float value;
    int outgrid;
    
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        array[outgrid] = arraypft[index][outgrid];
    }
}

void
copyplo(float array[MAXOUTPIX * MAXOUTLIN], int index, float plodata[][PLONFLDS]) {
    
    float value;
    int outgrid;
    
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        array[outgrid] = plodata[outgrid][index];
    }
}

// the standalone version needs 'float** glmo' to compile, but I don't know if this will work for iESM
// so only compile this function for the iESM function because it is not used in standalone
#ifndef STANDALONE
copyglmo(float array[MAXOUTPIX * MAXOUTLIN], int index, float glmo[][GLMONFLDS]) {
	
    float value;
    int outgrid;
    
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        value = glmo[outgrid][index];
        if (value >= 0.0 && value <= 1.1) {
            array[outgrid] = (value * 100.0);
            if (index <= 3) {array[outgrid] = round(array[outgrid]);}
            if (array[outgrid] > 100.0) {
                array[outgrid] = 100.0;
            }
        }
        else {
            array[outgrid] = 0.0;
        }
    }
}
#endif

/* !!! this is a new function to normalize the glmo data to the vegetated land unit -adv */
/* it is called after copyglmo() and after the base year pft data are read in -adv */
/* it also applies to the base year hurtt data -adv */
void
normglmo(float array[MAXOUTPIX * MAXOUTLIN]) {
    
    float value;
    int outgrid;
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        if (invegbare[outgrid] > 0.0 && invegbare[outgrid] <= 100.0) {
			// mystery line that keeps my version from crashing
			//printf("");
            value = array[outgrid] * 100.0 / inland[outgrid] / invegbare[outgrid];
			array[outgrid] = round(value * 100.0);
            if (array[outgrid] > 100.0) {
                array[outgrid] = 100.0;
            }
        }
        else {
            array[outgrid] = 0.0;
        }
    }
}

void
copy2plodata(float plodata[][PLONFLDS]) {
    
    float value;
    int outgrid;
    int inpft;
    
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        for (inpft = 0; inpft < MAXPFT; inpft++) {
            plodata[outgrid][inpft] = outhurttpftval[inpft][outgrid];
        }
        inpft = MAXPFT;
        plodata[outgrid][inpft] = 0.0;
        inpft = MAXPFT+1;
        plodata[outgrid][inpft] = outhurttvh1[outgrid];
        inpft = MAXPFT+2;
        plodata[outgrid][inpft] = outhurttvh2[outgrid];
        inpft = MAXPFT+3;
        plodata[outgrid][inpft] = outhurttsh1[outgrid];
        inpft = MAXPFT+4;
        plodata[outgrid][inpft] = outhurttsh2[outgrid];
        inpft = MAXPFT+5;
        plodata[outgrid][inpft] = outhurttsh3[outgrid];
        inpft = MAXPFT+6;
        plodata[outgrid][inpft] = outhurttgrazing[outgrid];
    }
}

void
writearray(float array[MAXOUTPIX * MAXOUTLIN], const char *tstring) {
    
    float value;
    float minval;
    float maxval;
    float sum;
    long outgrid;
    
    minval = array[1];
    maxval = array[1];
    sum = 0.0;
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        value = array[outgrid];
        sum = sum + value;
        if (value < minval) minval = value;
        if (value > maxval) maxval = value;
    }
    printf("writearray %s = %f %f %f \n", tstring, minval, maxval, sum);
    
}

void
writeinhurtt() {
    
    char tstring[32];
    
    strcpy(tstring, "inhurttbasecrop\0");
    writearray(inhurttbasecrop, tstring);
    
    strcpy(tstring, "inhurttbasepasture\0");
    writearray(inhurttbasepasture, tstring);
    
    strcpy(tstring, "inhurttcrop\0");
    writearray(inhurttcrop, tstring);
    
    strcpy(tstring, "inhurttpasture\0");
    writearray(inhurttpasture, tstring);
    
    strcpy(tstring, "inhurttprimary\0");
    writearray(inhurttprimary, tstring);
    
    strcpy(tstring, "inhurttsecondary\0");
    writearray(inhurttsecondary, tstring);
    
    strcpy(tstring, "inhurttvh1\0");
    writearray(inhurttvh1, tstring);
    
    strcpy(tstring, "inhurttvh2\0");
    writearray(inhurttvh2, tstring);
    
    strcpy(tstring, "inhurttsh1\0");
    writearray(inhurttsh1, tstring);
    
    strcpy(tstring, "inhurttsh2\0");
    writearray(inhurttsh2, tstring);
    
    strcpy(tstring, "inhurttsh3\0");
    writearray(inhurttsh3, tstring);
    
}

void
writeplodata(float plodata[][PLONFLDS]) {
    
    char tstring[32];
    float array[MAXOUTPIX * MAXOUTLIN];
    
    copyarray(array,0,outhurttpftval);
    strcpy(tstring,"outhurttpftval0\0");
    writearray(array,tstring);
    
    copyplo(array,0,plodata);
    strcpy(tstring,"plodata0\0");
    writearray(array,tstring);
    
    copyarray(array,1,outhurttpftval);
    strcpy(tstring,"outhurttpftval1\0");
    writearray(array,tstring);
    
    copyplo(array,1,plodata);
    strcpy(tstring,"plodata1\0");
    writearray(array,tstring);
    
    copyarray(array,2,outhurttpftval);
    strcpy(tstring,"outhurttpftval2\0");
    writearray(array,tstring);
    
    copyplo(array,2,plodata);
    strcpy(tstring,"plodata2\0");
    writearray(array,tstring);
    
    copyarray(array,3,outhurttpftval);
    strcpy(tstring,"outhurttpftval3\0");
    writearray(array,tstring);
    
    copyplo(array,3,plodata);
    strcpy(tstring,"plodata3\0");
    writearray(array,tstring);
    
    copyarray(array,4,outhurttpftval);
    strcpy(tstring,"outhurttpftval4\0");
    writearray(array,tstring);
    
    copyplo(array,4,plodata);
    strcpy(tstring,"plodata4\0");
    writearray(array,tstring);
    
    copyarray(array,5,outhurttpftval);
    strcpy(tstring,"outhurttpftval5\0");
    writearray(array,tstring);
    
    copyplo(array,5,plodata);
    strcpy(tstring,"plodata5\0");
    writearray(array,tstring);
    
    copyarray(array,6,outhurttpftval);
    strcpy(tstring,"outhurttpftval6\0");
    writearray(array,tstring);
    
    copyplo(array,6,plodata);
    strcpy(tstring,"plodata6\0");
    writearray(array,tstring);
    
    copyarray(array,7,outhurttpftval);
    strcpy(tstring,"outhurttpftval7\0");
    writearray(array,tstring);
    
    copyplo(array,7,plodata);
    strcpy(tstring,"plodata7\0");
    writearray(array,tstring);
    
    copyarray(array,8,outhurttpftval);
    strcpy(tstring,"outhurttpftval8\0");
    writearray(array,tstring);
    
    copyplo(array,8,plodata);
    strcpy(tstring,"plodata8\0");
    writearray(array,tstring);
    
    copyarray(array,9,outhurttpftval);
    strcpy(tstring,"outhurttpftval9\0");
    writearray(array,tstring);
    
    copyplo(array,9,plodata);
    strcpy(tstring,"plodata9\0");
    writearray(array,tstring);
    
    copyarray(array,10,outhurttpftval);
    strcpy(tstring,"outhurttpftval10\0");
    writearray(array,tstring);
    
    copyplo(array,10,plodata);
    strcpy(tstring,"plodata10\0");
    writearray(array,tstring);
    
    copyarray(array,11,outhurttpftval);
    strcpy(tstring,"outhurttpftval11\0");
    writearray(array,tstring);
    
    copyplo(array,11,plodata);
    strcpy(tstring,"plodata11\0");
    writearray(array,tstring);
    
    copyarray(array,12,outhurttpftval);
    strcpy(tstring,"outhurttpftval12\0");
    writearray(array,tstring);
    
    copyplo(array,12,plodata);
    strcpy(tstring,"plodata12\0");
    writearray(array,tstring);
    
    copyarray(array,13,outhurttpftval);
    strcpy(tstring,"outhurttpftval13\0");
    writearray(array,tstring);
    
    copyplo(array,13,plodata);
    strcpy(tstring,"plodata13\0");
    writearray(array,tstring);
    
    copyarray(array,14,outhurttpftval);
    strcpy(tstring,"outhurttpftval14\0");
    writearray(array,tstring);
    
    copyplo(array,14,plodata);
    strcpy(tstring,"plodata14\0");
    writearray(array,tstring);
    
    copyarray(array,15,outhurttpftval);
    strcpy(tstring,"outhurttpftval15\0");
    writearray(array,tstring);
    
    copyplo(array,15,plodata);
    strcpy(tstring,"plodata15\0");
    writearray(array,tstring);
    
	/* NOTE: the 17th pft is a placeholder that is not currently used by CLM unless irrigation is enabled; its pct value is 0 -adv */
    
    copyplo(array,16,plodata);
    strcpy(tstring,"plodata16\0");
    writearray(array,tstring);

    strcpy(tstring,"outhurttvh1\0");
    writearray(outhurttvh1,tstring);

    copyplo(array,17,plodata);
    strcpy(tstring,"plodata17\0");
    writearray(array,tstring);
    
    strcpy(tstring,"outhurttvh2\0");
    writearray(outhurttvh2,tstring);
    
    copyplo(array,18,plodata);
    strcpy(tstring,"plodata18\0");
    writearray(array,tstring);
    
    strcpy(tstring,"outhurttsh1\0");
    writearray(outhurttsh1,tstring);
    
    copyplo(array,19,plodata);
    strcpy(tstring,"plodata19\0");
    writearray(array,tstring);
    
    strcpy(tstring,"outhurttsh2\0");
    writearray(outhurttsh2,tstring);
    
    copyplo(array,20,plodata);
    strcpy(tstring,"plodata20\0");
    writearray(array,tstring);
    
    strcpy(tstring,"outhurttsh3\0");
    writearray(outhurttsh3,tstring);
    
    copyplo(array,21,plodata);
    strcpy(tstring,"plodata21\0");
    writearray(array,tstring);
    
    strcpy(tstring,"outhurttgrazing\0");
    writearray(outhurttgrazing,tstring);
    
    copyplo(array,22,plodata);
    strcpy(tstring,"plodata22\0");
    writearray(array,tstring);
    
}

void
readlandmask() {
    
    float *landmaskvalues;
    int outgrid;
    
    selectedvarcnt = 0;
    
    nc_inq(innetcdfid, &ndimsp, &nvarsp, &nattsp, &unlimdimidp);
    
    for (nvarspcnt = 0; nvarspcnt < nvarsp; nvarspcnt ++) {
        nc_inq_varname(innetcdfid, nvarspcnt, varname);
        /*      nc_inq_var(innetcdfid, nvarspcnt, varname, &vartype, &vardimsp, &vardimidsp, &varattsp); */
        if (strcmp(varname,"LANDMASK") == 0) {
            selectedvarids[0] = nvarspcnt;
            selectedvarcnt++;
            printf("Reading variable: %d %s \n",nvarspcnt,varname);
			break;
        }
        
    }
    
    /*  nc_inq_var(innetcdfid, selectedvarids[0], varname, &vartype, &vardimsp, &vardimidsp, &varattsp); */
    varlayers = 1;
    varlayers2 = 1;
    landmaskvalues = malloc(sizeof(float) * lonlen * latlen * varlayers * varlayers2);
    nc_get_var_float(innetcdfid,selectedvarids[0],landmaskvalues);
    
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        inmask[outgrid] = landmaskvalues[outgrid] * 100.0;
    }
    
    free(landmaskvalues);
    
}

void
readlandfrac() {
    
    float *landfracvalues;
    int outgrid;
    
    selectedvarcnt = 0;
    
    nc_inq(innetcdfid, &ndimsp, &nvarsp, &nattsp, &unlimdimidp);
    
    for (nvarspcnt = 0; nvarspcnt < nvarsp; nvarspcnt ++) {
        nc_inq_varname(innetcdfid, nvarspcnt, varname);
        /*      nc_inq_var(innetcdfid, nvarspcnt, varname, &vartype, &vardimsp, &vardimidsp, &varattsp); */
        if (strcmp(varname,"LANDFRAC") == 0) {
            selectedvarids[0] = nvarspcnt;
            selectedvarcnt++;
            printf("Reading variable: %d %s \n",nvarspcnt,varname);
			break;
        }
        
    }
    
    /*  nc_inq_var(innetcdfid, selectedvarids[0], varname, &vartype, &vardimsp, &vardimidsp, &varattsp); */
    varlayers = 1;
    varlayers2 = 1;
    landfracvalues = malloc(sizeof(float) * lonlen * latlen * varlayers * varlayers2);
    nc_get_var_float(innetcdfid,selectedvarids[0],landfracvalues);
    
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        inland[outgrid] = landfracvalues[outgrid] * 100.0;
    }
    
    free(landfracvalues);
    
}

void
readlakefrac() {
    
    float *lakefracvalues;
    int outgrid;
    
    selectedvarcnt = 0;
    
    nc_inq(innetcdfid, &ndimsp, &nvarsp, &nattsp, &unlimdimidp);
    
    for (nvarspcnt = 0; nvarspcnt < nvarsp; nvarspcnt ++) {
        nc_inq_varname(innetcdfid, nvarspcnt, varname);
        /*      nc_inq_var(innetcdfid, nvarspcnt, varname, &vartype, &vardimsp, &vardimidsp, &varattsp); */
        if (strcmp(varname,"PCT_LAKE") == 0) {
            selectedvarids[0] = nvarspcnt;
            selectedvarcnt++;
            printf("Reading variable: %d %s \n",nvarspcnt,varname);
			break;
        }
        
    }
    
    /*  nc_inq_var(innetcdfid, selectedvarids[0], varname, &vartype, &vardimsp, &vardimidsp, &varattsp); */
    varlayers = 1;
    varlayers2 = 1;
    lakefracvalues = malloc(sizeof(float) * lonlen * latlen * varlayers * varlayers2);
    nc_get_var_float(innetcdfid,selectedvarids[0],lakefracvalues);
    
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        inlake[outgrid] = lakefracvalues[outgrid];
    }
    
    free(lakefracvalues);
    
}


void
readwetlandfrac() {
    
    float *wetlandfracvalues;
    int outgrid;
    
    selectedvarcnt = 0;
    
    nc_inq(innetcdfid, &ndimsp, &nvarsp, &nattsp, &unlimdimidp);
    
    for (nvarspcnt = 0; nvarspcnt < nvarsp; nvarspcnt ++) {
        nc_inq_varname(innetcdfid, nvarspcnt, varname);
        /*      nc_inq_var(innetcdfid, nvarspcnt, varname, &vartype, &vardimsp, &vardimidsp, &varattsp); */
        if (strcmp(varname,"PCT_WETLAND") == 0) {
            selectedvarids[0] = nvarspcnt;
            selectedvarcnt++;
            printf("Reading variable: %d %s \n",nvarspcnt,varname);
			break;
        }
        
    }
    
    /*  nc_inq_var(innetcdfid, selectedvarids[0], varname, &vartype, &vardimsp, &vardimidsp, &varattsp); */
    varlayers = 1;
    varlayers2 = 1;
    wetlandfracvalues = malloc(sizeof(float) * lonlen * latlen * varlayers * varlayers2);
    nc_get_var_float(innetcdfid,selectedvarids[0],wetlandfracvalues);
    
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        inwetland[outgrid] = wetlandfracvalues[outgrid];
    }
    
    free(wetlandfracvalues);
    
}


void
readicefrac() {
    
    float *icefracvalues;
    int outgrid;
    
    selectedvarcnt = 0;
    
    nc_inq(innetcdfid, &ndimsp, &nvarsp, &nattsp, &unlimdimidp);
    
    for (nvarspcnt = 0; nvarspcnt < nvarsp; nvarspcnt ++) {
        nc_inq_varname(innetcdfid, nvarspcnt, varname);
        /*      nc_inq_var(innetcdfid, nvarspcnt, varname, &vartype, &vardimsp, &vardimidsp, &varattsp); */
        if (strcmp(varname,"PCT_GLACIER") == 0) {
            selectedvarids[0] = nvarspcnt;
            selectedvarcnt++;
            printf("Reading variable: %d %s \n",nvarspcnt,varname);
			break;
        }
        
    }
    
    /*  nc_inq_var(innetcdfid, selectedvarids[0], varname, &vartype, &vardimsp, &vardimidsp, &varattsp); */
    varlayers = 1;
    varlayers2 = 1;
    icefracvalues = malloc(sizeof(float) * lonlen * latlen * varlayers * varlayers2);
    nc_get_var_float(innetcdfid,selectedvarids[0],icefracvalues);
    
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        inice[outgrid] = icefracvalues[outgrid];
    }
    
    free(icefracvalues);
    
}

/* NOTE: readsand not currently being used by the code - lpc */
/* not even present in the code - adv ----
void
readsand() {
    
    float *sandvalues;
    int outgrid, layer;
    long offsetgrid;
    
    selectedvarcnt = 0;
    
    nc_inq(innetcdfid, &ndimsp, &nvarsp, &nattsp, &unlimdimidp);
    
    for (nvarspcnt = 0; nvarspcnt < nvarsp; nvarspcnt ++) {
        nc_inq_varname(innetcdfid, nvarspcnt, varname);
        //      nc_inq_var(innetcdfid, nvarspcnt, varname, &vartype, &vardimsp, &vardimidsp, &varattsp);
        if (strcmp(varname,"PCT_SAND") == 0) {
            selectedvarids[0] = nvarspcnt;
            selectedvarcnt++;
            printf("Reading variable: %d %s \n",nvarspcnt,varname);
			break;
        }
        
    }
    
    //  nc_inq_var(innetcdfid, selectedvarids[0], varname, &vartype, &vardimsp, &vardimidsp, &varattsp);
    varlayers = MAXSOILLAYERS;
    varlayers2 = 1;
    sandvalues = malloc(sizeof(float) * lonlen * latlen * varlayers * varlayers2);
    nc_get_var_float(innetcdfid,selectedvarids[0],sandvalues);
    
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        for (layer = 0; layer < MAXSOILLAYERS; layer++) {
            offsetgrid = layer * MAXOUTPIX * MAXOUTLIN + outgrid;
            insand[layer][outgrid] = sandvalues[offsetgrid];
        }
    }
    
    free(sandvalues);
    
}

----*/

/* NOTE: readclay not currently being used by the code - lpc */
/* not even present in the code - adv ---
void
readclay() {
    
    float *clayvalues;
    int outgrid, layer;
    long offsetgrid;
    
    selectedvarcnt = 0;
    
    nc_inq(innetcdfid, &ndimsp, &nvarsp, &nattsp, &unlimdimidp);
    
    for (nvarspcnt = 0; nvarspcnt < nvarsp; nvarspcnt ++) {
        nc_inq_varname(innetcdfid, nvarspcnt, varname);
        //      nc_inq_var(innetcdfid, nvarspcnt, varname, &vartype, &vardimsp, &vardimidsp, &varattsp);
        if (strcmp(varname,"PCT_CLAY") == 0) {
            selectedvarids[0] = nvarspcnt;
            selectedvarcnt++;
            printf("Reading variable: %d %s \n",nvarspcnt,varname);
			break;
        }
        
    }
    
    //  nc_inq_var(innetcdfid, selectedvarids[0], varname, &vartype, &vardimsp, &vardimidsp, &varattsp);
    varlayers = MAXSOILLAYERS;
    varlayers2 = 1;
    clayvalues = malloc(sizeof(float) * lonlen * latlen * varlayers * varlayers2);
    nc_get_var_float(innetcdfid,selectedvarids[0],clayvalues);
    
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        for (layer = 0; layer < MAXSOILLAYERS; layer++) {
            offsetgrid = layer * MAXOUTPIX * MAXOUTLIN + outgrid;
            inclay[layer][outgrid] = clayvalues[offsetgrid];
        }
    }
    
    free(clayvalues);
    
}

--- */

/* NOTE: readsoilslope not currently being used by the code - lpc */
/* not even present in the code - adv ---
void
readsoilslope() {
    
    float *soilslopevalues;
    int outgrid;
    
    selectedvarcnt = 0;
    
    nc_inq(innetcdfid, &ndimsp, &nvarsp, &nattsp, &unlimdimidp);
    
    for (nvarspcnt = 0; nvarspcnt < nvarsp; nvarspcnt ++) {
        nc_inq_varname(innetcdfid, nvarspcnt, varname);
        //      nc_inq_var(innetcdfid, nvarspcnt, varname, &vartype, &vardimsp, &vardimidsp, &varattsp);
        if (strcmp(varname,"SOIL_SLOPE") == 0) {
            selectedvarids[0] = nvarspcnt;
            selectedvarcnt++;
            printf("Reading variable: %d %s \n",nvarspcnt,varname);
			break;
        }
        
    }
    
    //  nc_inq_var(innetcdfid, selectedvarids[0], varname, &vartype, &vardimsp, &vardimidsp, &varattsp);
    varlayers = 1;
    varlayers2 = 1;
    soilslopevalues = malloc(sizeof(float) * lonlen * latlen * varlayers * varlayers2);
    nc_get_var_float(innetcdfid,selectedvarids[0],soilslopevalues);
    
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        insoilslope[outgrid] = soilslopevalues[outgrid];
    }
    
    free(soilslopevalues);
    
}

--- */

/* not present in code -adv ---
// new function to read the year of the initial pft file data

void
getpftyear(long *year) {
	long year_ind;
	int year_varid;
	float temp_float;
	
	nc_inq_varid(innetcdfid, "TIME", &year_varid);
	nc_get_var_float(innetcdfid, year_varid, &temp_float);
	*year = temp_float;
}
 --- */

void
readcurrentpft() {
    
    float outpftid;
    int outgrid, offsetgrid, inpft, inmonth, barefound;
    
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        for (inpft = 0; inpft < MAXPFT; inpft++) {
            incurrentpftid[inpft][outgrid] = inpft;
        }
    }
    
}

/* modified to read the dynamic pft pf file -adv
 */

void
readcurrentpftpct(long modyear) {
    
     float *pftpctvalues;
    float pfttotal;
    int outgrid, offsetgrid, inpft, outpftid;
    
	long numdims = 4;
	long year_ind;
	int year_varid;
	size_t numrecs, *start, *count;
	float *years;
    
    selectedvarcnt = 0;
    
    nc_inq(innetcdfid, &ndimsp, &nvarsp, &nattsp, &unlimdimidp);
    
    for (nvarspcnt = 0; nvarspcnt < nvarsp; nvarspcnt ++) {
        nc_inq_varname(innetcdfid, nvarspcnt, varname);
        /*      nc_inq_var(innetcdfid, nvarspcnt, varname, &vartype, &vardimsp, &vardimidsp, &varattsp); */
        if (strcmp(varname,"PCT_PFT") == 0) {
            selectedvarids[0] = nvarspcnt;
            selectedvarcnt++;
            printf("Reading variable: %d %s \n",nvarspcnt,varname);
			break;
        }
    }
    
	/* get the index of modyear */
	nc_inq_dimlen(innetcdfid, unlimdimidp, &numrecs);
	years = malloc(sizeof(float) * numrecs);
	nc_inq_varid(innetcdfid, "TIME", &year_varid);
	nc_get_var_float(innetcdfid, year_varid, years);
	for(year_ind = 0; year_ind < ((long) numrecs); year_ind++) {
		if (((long) years[year_ind]) == modyear) {
			//printf("Selected year: %f %li\n", years[year_ind], modyear);
			break;
		}
	}
	if(year_ind == ((long) numrecs)) {
		printf("Error reading reference year data %li from invalid index %li\n",modyear,year_ind);
	}
	
	start = calloc(numdims, sizeof(size_t));
	count = calloc(numdims, sizeof(size_t));
	start[0] = year_ind;
	start[1] = 0;
	start[2] = 0;
	start[3] = 0;
	count[0] = 1;
	count[1] = MAXPFT+1;
	count[2] = latlen;
	count[3] = lonlen;
	
    varlayers = MAXPFT+1;
    varlayers2 = 1;
    pftpctvalues = malloc(sizeof(float) * lonlen * latlen * varlayers * varlayers2);
    nc_get_vara_float(innetcdfid,selectedvarids[0], start, count, pftpctvalues);
    
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        pfttotal = 0.0;
        for (inpft = 0; inpft < MAXPFT; inpft++) {
            offsetgrid = inpft * MAXOUTPIX * MAXOUTLIN + outgrid;
            outpftid = incurrentpftid[inpft][outgrid];
            incurrentpftval[outpftid][outgrid] = pftpctvalues[offsetgrid];
            pfttotal = pfttotal + pftpctvalues[offsetgrid];
        }
    }
    
    free(pftpctvalues);
	free(years);
	free(start);
	free(count);
    
}

/* new function to store the outyear values in the pft reference surface file - adv
 outyear is the year being operated on, so it is the one appended to the file
 */

void
writepftdynfile(long outyear) {
    
	float *values;
    long outgrid;
	float writeyear;
    int offsetgrid, inpft;
	
	int year_varid;
	size_t numrecs, *start, *count;
	size_t countone = 1;
    
	writeyear = (float) outyear;
	
    selectedvarcnt = 0;
    
    nc_inq(innetcdfid, &ndimsp, &nvarsp, &nattsp, &unlimdimidp);
    
    for (nvarspcnt = 0; nvarspcnt < nvarsp; nvarspcnt ++) {
        nc_inq_varname(innetcdfid, nvarspcnt, varname);
        /*      nc_inq_var(innetcdfid, nvarspcnt, varname, &vartype, &vardimsp, &vardimidsp, &varattsp); */
        if (strcmp(varname,"PCT_PFT") == 0) {
            selectedvarids[0] = nvarspcnt;
            selectedvarcnt++;
            printf("Writing variable: %d %s \n",nvarspcnt,varname);
			break;
        }
    }
	
	/* get the number of current records */
	/* this works here because there is only one call to this function per year */
	nc_inq_dimlen(innetcdfid, unlimdimidp, &numrecs);
	
	/* write the year info */
	nc_inq_varid(innetcdfid, "TIME", &year_varid);
	nc_put_vara_float(innetcdfid, year_varid, &numrecs, &countone, &writeyear);
	
	start = calloc(ndimsp, sizeof(size_t));
	count = calloc(ndimsp, sizeof(size_t));
	start[0] = numrecs;
	start[1] = 0;
	start[2] = 0;
	start[3] = 0;
	count[0] = 1;
	count[1] = MAXPFT+1;
	count[2] = latlen;
	count[3] = lonlen;
	
    varlayers = MAXPFT+1;
    varlayers2 = 1;
    values = calloc(lonlen * latlen * varlayers * varlayers2, sizeof(float));
    
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        for (inpft = 0; inpft < MAXPFT; inpft++) {
            offsetgrid = inpft * MAXOUTPIX * MAXOUTLIN + outgrid;
			values[offsetgrid] = outhurttpftval[inpft][outgrid];
        }
		/* need to add the zero placeholder for the 17th pft (unused) */
		 offsetgrid = inpft * MAXOUTPIX * MAXOUTLIN + outgrid;
		 values[offsetgrid] = 0;
    }
    
	nc_put_vara_float(innetcdfid,selectedvarids[0], start, count, values);
	
    free(values);
	free(start);
	free(count);
    
}

/* NOTE: readcurrentpftlai not currently being used by the code - lpc */
void
readcurrentpftlai() {
    
    float *pftlaivalues;
    int outgrid, offsetgrid, inpft, inmonth, outpftid;
    
    selectedvarcnt = 0;
    
    nc_inq(innetcdfid, &ndimsp, &nvarsp, &nattsp, &unlimdimidp);
    
    for (nvarspcnt = 0; nvarspcnt < nvarsp; nvarspcnt ++) {
        nc_inq_varname(innetcdfid, nvarspcnt, varname);
        /*      nc_inq_var(innetcdfid, nvarspcnt, varname, &vartype, &vardimsp, &vardimidsp, &varattsp); */
        if (nvarspcnt == CLMLAIVAR) {
            selectedvarids[0] = nvarspcnt;
            selectedvarcnt++;
            sprintf(varname,"MONTHLY_LAI");
            printf("Reading variable: %d %s \n",nvarspcnt,varname);
			break;
        }
    }
    
    varlayers = 12;
    varlayers2 = MAXPFT+1;
    pftlaivalues = malloc(sizeof(float) * lonlen * latlen * varlayers * varlayers2);
    nc_get_var_float(innetcdfid,selectedvarids[0],pftlaivalues);
    
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        for (inpft = 0; inpft < MAXPFT; inpft++) {
            outpftid = inpft;
            for (inmonth = 0; inmonth < MAXMONTH; inmonth++ ) {
                offsetgrid = inmonth * (MAXPFT+1) * MAXOUTPIX * MAXOUTLIN + inpft * MAXOUTPIX * MAXOUTLIN + outgrid;
                incurrentlaival[inmonth][outpftid][outgrid] = pftlaivalues[offsetgrid];
            }
        }
    }
    
    free(pftlaivalues);
    
}

/* NOTE: readcurrentpftsai not currently being used by the code - lpc */
void
readcurrentpftsai() {
    
    float *pftsaivalues;
    int outgrid, offsetgrid, inpft, inmonth, outpftid;
    
    selectedvarcnt = 0;
    
    nc_inq(innetcdfid, &ndimsp, &nvarsp, &nattsp, &unlimdimidp);
    
    for (nvarspcnt = 0; nvarspcnt < nvarsp; nvarspcnt ++) {
        nc_inq_varname(innetcdfid, nvarspcnt, varname);
        /*      nc_inq_var(innetcdfid, nvarspcnt, varname, &vartype, &vardimsp, &vardimidsp, &varattsp); */
        if (nvarspcnt == CLMLAIVAR + 1) {
            sprintf(varname,"MONTHLY_SAI");
            selectedvarids[0] = nvarspcnt;
            selectedvarcnt++;
            printf("Reading variable: %d %s \n",nvarspcnt,varname);
			break;
        }
    }
    
    varlayers = 12;
    varlayers2 = MAXPFT+1;
    pftsaivalues = malloc(sizeof(float) * lonlen * latlen * varlayers * varlayers2);
    nc_get_var_float(innetcdfid,selectedvarids[0],pftsaivalues);
    
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        for (inpft = 0; inpft < MAXPFT; inpft++) {
            outpftid = inpft;
            for (inmonth = 0; inmonth < MAXMONTH; inmonth++ ) {
                offsetgrid = inmonth * (MAXPFT+1) * MAXOUTPIX * MAXOUTLIN + inpft * MAXOUTPIX * MAXOUTLIN + outgrid;
                incurrentsaival[inmonth][outpftid][outgrid] = pftsaivalues[offsetgrid];
            }
        }
    }
    
    free(pftsaivalues);
    
}

/* NOTE: readcurrentsoilcolor not currently being used by the code - lpc */
void
readcurrentsoilcolor() {
    
    float *soilcolorvalues;
    int outgrid;
    
    selectedvarcnt = 0;
    
    nc_inq(innetcdfid, &ndimsp, &nvarsp, &nattsp, &unlimdimidp);
    
    for (nvarspcnt = 0; nvarspcnt < nvarsp; nvarspcnt ++) {
        nc_inq_varname(innetcdfid, nvarspcnt, varname);
        /*      nc_inq_var(innetcdfid, nvarspcnt, varname, &vartype, &vardimsp, &vardimidsp, &varattsp); */
        if (strcmp(varname,"SOIL_COLOR") == 0) {
            selectedvarids[0] = nvarspcnt;
            selectedvarcnt++;
            printf("Reading variable: %d %s \n",nvarspcnt,varname);
			break;
        }
        
    }
    
    /*  nc_inq_var(innetcdfid, selectedvarids[0], varname, &vartype, &vardimsp, &vardimidsp, &varattsp); */
    varlayers = 1;
    varlayers2 = 1;
    soilcolorvalues = malloc(sizeof(float) * lonlen * latlen * varlayers * varlayers2);
    nc_get_var_float(innetcdfid,selectedvarids[0],soilcolorvalues);
    
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        incurrentsoilcolor[outgrid] = soilcolorvalues[outgrid];
    }
    
    free(soilcolorvalues);
    
}


void
readpotvegpft() {
    
    float outpftid;
    int outgrid, offsetgrid, inpft, inmonth, barefound;
    
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        for (inpft = 0; inpft < MAXPFT; inpft++) {
            inpotvegpftid[inpft][outgrid] = inpft;
        }
    }
    
}

void
readpotvegpftpct() {
    
    float *pftpctvalues;
    float pfttotal;
    int outgrid, offsetgrid, inpft, inmonth, outpftid;
    
    selectedvarcnt = 0;
    
    nc_inq(innetcdfid, &ndimsp, &nvarsp, &nattsp, &unlimdimidp);
    
    for (nvarspcnt = 0; nvarspcnt < nvarsp; nvarspcnt ++) {
        nc_inq_varname(innetcdfid, nvarspcnt, varname);
        /*      nc_inq_var(innetcdfid, nvarspcnt, varname, &vartype, &vardimsp, &vardimidsp, &varattsp); */
        if (strcmp(varname,"PCT_PFT") == 0) {
            selectedvarids[0] = nvarspcnt;
            selectedvarcnt++;
            printf("Reading potential veg variable: %d %s \n",nvarspcnt,varname);
			break;
        }
    }
    
    varlayers = MAXPFT+1;
    varlayers2 = 1;
    pftpctvalues = malloc(sizeof(float) * lonlen * latlen * varlayers * varlayers2);
    nc_get_var_float(innetcdfid,selectedvarids[0],pftpctvalues);
    
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        pfttotal = 0.0;
        for (inpft = 0; inpft < MAXPFT; inpft++) {
            offsetgrid = inpft * MAXOUTPIX * MAXOUTLIN + outgrid;
            outpftid = inpotvegpftid[inpft][outgrid];
            inpotvegpftval[outpftid][outgrid] = pftpctvalues[offsetgrid];
            pfttotal = pfttotal + pftpctvalues[offsetgrid];
        }
    }
    
    free(pftpctvalues);
    
}


void
setvegbarefrac() {
    
    int outgrid;
    
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        if (inmask[outgrid] == 100.0) {
            invegbare[outgrid] = 100.0 - inice[outgrid] - inlake[outgrid] - inwetland[outgrid];
        }
        else {
            invegbare[outgrid] = 0.0;
        }
    }
    
}

/* NOTE: findcurrentpftidgrid not currently being used by the code - lpc */
/* not even present in the code - adv ---
long
findcurrentpftidgrid(long outgrid, int pftid) {
    
    long searchlength, searchgrid, foundgrid;
    int pftnotfound;
    
    if (incurrentpftval[pftid][outgrid] > 0.0) {
        foundgrid = outgrid;
        pftnotfound = 0;
    }
    else {
        pftnotfound = 1;
    }
    
    searchlength = 1;
    while (pftnotfound == 1) {
        searchgrid = outgrid + searchlength;
        if (searchgrid >= 0 && searchgrid < MAXOUTLIN * MAXOUTPIX) {
            if (incurrentpftval[pftid][searchgrid] > 0.0) {
                foundgrid = searchgrid;
                pftnotfound = 0;
            }
        }
        searchgrid = outgrid - searchlength;
        if (searchgrid >= 0 && searchgrid < MAXOUTLIN * MAXOUTPIX) {
            if (incurrentpftval[pftid][searchgrid] > 0.0) {
                foundgrid = searchgrid;
                pftnotfound = 0;
            }
        }
        searchgrid = outgrid + searchlength * MAXOUTPIX;
        if (searchgrid >= 0 && searchgrid < MAXOUTLIN * MAXOUTPIX) {
            if (incurrentpftval[pftid][searchgrid] > 0.0) {
                foundgrid = searchgrid;
                pftnotfound = 0;
            }
        }
        searchgrid = outgrid - searchlength * MAXOUTPIX;
        if (searchgrid >= 0 && searchgrid < MAXOUTLIN * MAXOUTPIX) {
            if (incurrentpftval[pftid][searchgrid] > 0.0) {
                foundgrid = searchgrid;
                pftnotfound = 0;
            }
        }
        searchlength = searchlength + 1;
    }
    
    return foundgrid;
    
}

 --- */
 
int
iscurrentpasturegrid(long outgrid) {
    
    int outpft;
    float pftsum;
    
    pftsum = 0;
    for (outpft = GA3PFT;outpft <= GC4PFT; outpft++) {
        pftsum = pftsum + incurrentpftval[outpft][outgrid];
    }
    
    if (pftsum > 0.0) {
        return 1;
    }
    else {
        return 0;
    }
    
}

long
findcurrentpasturegrid(long outgrid) {
    
    long searchlength, searchgrid, foundgrid;
    int pftnotfound;
    
    if (iscurrentpasturegrid(outgrid)) {
        foundgrid = outgrid;
        pftnotfound = 0;
    }
    else {
        pftnotfound = 1;
    }
    
    searchlength = 1;
    while (pftnotfound == 1) {
        searchgrid = outgrid + searchlength;
        if (searchgrid >= 0 && searchgrid < MAXOUTLIN * MAXOUTPIX) {
            if (iscurrentpasturegrid(searchgrid)) {
                foundgrid = searchgrid;
                pftnotfound = 0;
            }
        }
        searchgrid = outgrid - searchlength;
        if (searchgrid >= 0 && searchgrid < MAXOUTLIN * MAXOUTPIX) {
            if (iscurrentpasturegrid(searchgrid)) {
                foundgrid = searchgrid;
                pftnotfound = 0;
            }
        }
        searchgrid = outgrid + searchlength * MAXOUTPIX;
        if (searchgrid >= 0 && searchgrid < MAXOUTLIN * MAXOUTPIX) {
            if (iscurrentpasturegrid(searchgrid)) {
                foundgrid = searchgrid;
                pftnotfound = 0;
            }
        }
        searchgrid = outgrid - searchlength * MAXOUTPIX;
        if (searchgrid >= 0 && searchgrid < MAXOUTLIN * MAXOUTPIX) {
            if (iscurrentpasturegrid(searchgrid)) {
                foundgrid = searchgrid;
                pftnotfound = 0;
            }
        }
        searchlength = searchlength + 1;
    }
    
    return foundgrid;
    
}


/*------
	sethurttcurrent()
	outgrid - grid cell index
	initialize one cell of the output pft array with the 'current' pfts
	current pfts are defined by what is in the incurrentpft arrays
	-adv
------*/
void
sethurttcurrent(int outgrid) {
    
    int outpft, outmonth;
    
    for (outpft = 0;outpft < MAXPFT;outpft++) {
        outhurttpftid[outpft][outgrid] = incurrentpftid[outpft][outgrid];
        outhurttpftval[outpft][outgrid] = incurrentpftval[outpft][outgrid];  // incurrentpftval contains reference PFT values (from initial ref file or from dynamic file)
        for (outmonth = 0;outmonth < MAXMONTH;outmonth++) {
            outhurttlaival[outmonth][outpft][outgrid] = incurrentlaival[outmonth][outpft][outgrid];
            outhurttsaival[outmonth][outpft][outgrid] = incurrentsaival[outmonth][outpft][outgrid];
        }
    }
    outhurttsoilcolor[outgrid] = incurrentsoilcolor[outgrid];
    
}


/*------
	sethurttcrop()
	outgrid - grid cell index
	modyear - the model year, which is the year prior to the output year
	calcyear - the output year for pft values
	set the output crop pft value in the grid cell indexed by outgrid, using the difference between the output and base years for pft conversion
		the output crop pft value is actually set to equal the input hurtt crop value
	output crop value cannot exceed the available vegetated land unit amount
	bare ground is removed (if not enough veg pft available) or added (if not enough pot veg available) as needed to accommodate crop amount
	the removal and addition of pfts is controlled by the land conversion assumption variables
		which are set specifically to match the orignial historical assumptions up to 2015 and to increase afforestation from 2015 forward
	-adv
------*/
void sethurttcrop(int outgrid, int modyear, int calcyear) {
    
    int maxpftid, outpft, outmonth, pftgrid, temppftid;
    float newcropval, noncroppftsum, treepftsum;
    float potvegpftsum, potvegtreepftsum, potvegherbaceouspftsum;
    float maxpftval;
    /* float addpftsum, addtreepftsum, addherbaceouspftsum, removepftsum, updatedpftsum; commented out by -adv */
	float addpftsum, removepftsum, updatedpftsum;
	/* these are new variables for preferential pft manipulation -adv */
	float herbaceouspftsum, availpotvegtreepftsum, availpotvegherbpftsum, availpotvegtreeherbpftsum;
	float availpotveggrasspftsum, availpotvegshrubpftsum, grasspftsum, shrubpftsum;
	float maxherbaceousfracremain, propherbaceousfracremain, minherbaceousfracremain, herbaceousfracremain, treefracremain;
	float maxavailtreefracremain, propavailtreefracremain, minavailtreefracremain, availtreefracremain, availherbfracremain;
	float outtreepftsum, outherbaceouspftsum, outavailpotvegtreepftsum, outavailpotvegherbpftsum;
	float potveggrasspftsum, potvegshrubpftsum;
	float removeavailpotvegherb, removeavailpotveggrass, removeavailpotvegshrub;
	
	// land conversion assumption variables
	/* !!! these are the new variables that control the preferential pft removal/addition -adv
	 the values range from:
	 2 = maximize herbaceous pfts (i.e. minimize forest pfts)
	 1 = proportional based on relevant pft distribution
	 0 = minimize herbaceous pfts (i.e. maximize forest pfts)
	 */
	
	int ADDTREEONLY;			// For crop removal: 1=addtreeonly; 0=add pfts proportionally based on setavailtreefracrem
	float setherbfracrem;		// crop addition
	float setavailtreefracrem;	// crop removal
	
	// afforestation policy begins affecting land use at the beginning of 2015
	// the original assumptions appply before 2015
	if (modyear >= 2015) {
		//	only trees replace crop removals (setavailtreefracrem not used)
		//	trees are preferentially removed upon crop addition to compensate for indiscriminant tree addition upon crop removal
		//		this also follows from the idea that trees added to unfavoravble areas might be the first to go due to higher value crops
		ADDTREEONLY = 1;	// For crop removal: 1=addtreeonly; 0=add pfts proportionally based on setavailtreefracrem
		setherbfracrem = 2.0;	// crop addition
		setavailtreefracrem = 0.0;	// crop removal
    } else {
		// Historically, crop addition and removal cause pft changes proportional to respective pft distributions
		// this is the original assumption, but available potential vegetation has been used in place of potential vegetation for crop removal
		ADDTREEONLY = 0;	// For crop removal: 1=addtreeonly; 0=add pfts proportionally based on setavailtreefracrem
		setherbfracrem = 1.0;	// crop addition
		setavailtreefracrem = 1.0;	// crop removal
	}

	/* REVIEW:
	 This could be based on the difference between the inhurttcrop values and the inhurttbasecrop
		because inhurttbasecrop (current day values, representing the year 2000)
		has different grid cell crop fractions than the year 2000 pft values used for calculating the pft changes
	 The inhurttcrop values likely follow a trajectory from the inhurttbasecrop values, rather than the year 2000 pfts
	 Currently this causes an initial shift in plo crop area during the first year that is not due to estimated land use change
	 This forces the future CLM PFTs to follow the GLM spatial pattern determined from GCAM
	*/
	
	/* !!! this first block sets the relative change in crop area from the base year glm data to the glmo data, rather than the the glmo crop are directly; start new code block -adv
	// actually, don't do this because we want to preserve the spatial crop distribution passed from GCAM through GLM -adv
	// calculate relative change from a base year glm map to the glmo data, then apply this change to the base year pft map
	// this keeps the clm crop area consistent with its history
	// it definitely changes the crop trajectory
	if(inhurttbasecrop[outgrid] > 0.0 && incurrentpftval[CPFT][outgrid] > 0.0) {
		newcropval = round(incurrentpftval[CPFT][outgrid] * inhurttcrop[outgrid] / inhurttbasecrop[outgrid]);
	}
	else {
		newcropval = inhurttcrop[outgrid];
	}
	 
	// the next line below that sets newcropval needs to be commented out -adv
	 
	end of new code block -adv */
	
	/* set the clm crop pft equal to the GLM output year crop -adv */
	/* remember that newcropval is the actual percent for output, not the change in percent -adv */
    newcropval = inhurttcrop[outgrid];
    if (outhurttpftval[CPFT][outgrid] > newcropval) {
        addpftsum = outhurttpftval[CPFT][outgrid] - newcropval;
        removepftsum = 0.0;
    }
    else {
        addpftsum = 0.0;
        removepftsum = newcropval - outhurttpftval[CPFT][outgrid];
    }
    
    noncroppftsum = 0.0;	/* does not include bare soil pft -adv */
    for (outpft = NEMPFT;outpft <= GC4PFT;outpft++) {
        noncroppftsum = noncroppftsum + outhurttpftval[outpft][outgrid];
    }
    
    treepftsum = 0.0;
    /* Bugfix. Loop below was terminating incorrectly at GC4PFT. -bbl */
	/* NOTE: treepftsum is used in the new crop addition code below -adv */
    for (outpft = NEMPFT;outpft <= BDBPFT;outpft++) {
        treepftsum = treepftsum + outhurttpftval[outpft][outgrid];
    }
    
    potvegpftsum = 0.0;	/* does not include bare soil pft -adv */
    for (outpft = NEMPFT;outpft <= GC4PFT;outpft++) {
        // inpotvegpftval comes from surfdat_360x720_potveg.nc i.e. potential vegetation
        potvegpftsum = potvegpftsum + inpotvegpftval[outpft][outgrid];
    }
    
    potvegtreepftsum = 0.0;
    for (outpft = NEMPFT;outpft <= BDBPFT;outpft++) {
        potvegtreepftsum = potvegtreepftsum + inpotvegpftval[outpft][outgrid];
    }
    
    potvegherbaceouspftsum = potvegpftsum - potvegtreepftsum;

    // /* !!! determine the herbaceous and grass and shrub sums for preferential removal; start new code block -adv
    potveggrasspftsum = 0.0;
    for (outpft = GA3PFT;outpft <= GC4PFT;outpft++) {
        potveggrasspftsum = potveggrasspftsum + inpotvegpftval[outpft][outgrid];
    }
	
    potvegshrubpftsum = 0.0;
    for (outpft = SEMPFT;outpft <= SDBPFT;outpft++) {
        potvegshrubpftsum = potvegshrubpftsum + inpotvegpftval[outpft][outgrid];
    }
	
    herbaceouspftsum = 0.0;
    for (outpft = SEMPFT;outpft <= GC4PFT;outpft++) {
        herbaceouspftsum = herbaceouspftsum + outhurttpftval[outpft][outgrid];
    }

    grasspftsum = 0.0;
    for (outpft = GA3PFT;outpft <= GC4PFT;outpft++) {
        grasspftsum = grasspftsum + outhurttpftval[outpft][outgrid];
    }
	
    shrubpftsum = 0.0;
    for (outpft = SEMPFT;outpft <= SDBPFT;outpft++) {
        shrubpftsum = shrubpftsum + outhurttpftval[outpft][outgrid];
    }
	
    // get the available percents of tree and herbaceous and tree/herbaceous, and grass and shrub potential veg
    availpotvegtreepftsum = potvegtreepftsum - treepftsum;
    if( availpotvegtreepftsum < 0.0 ) { availpotvegtreepftsum = 0.0; }
    availpotvegherbpftsum = potvegherbaceouspftsum - herbaceouspftsum;
    if ( availpotvegherbpftsum < 0.0 ) { availpotvegherbpftsum = 0.0; }
    availpotvegtreeherbpftsum = availpotvegtreepftsum + availpotvegherbpftsum;
    availpotveggrasspftsum = potveggrasspftsum - grasspftsum;
    if ( availpotveggrasspftsum < 0.0 ) { availpotveggrasspftsum = 0.0; }
    availpotvegshrubpftsum = potvegshrubpftsum - shrubpftsum;
    if ( availpotvegshrubpftsum < 0.0 ) { availpotvegshrubpftsum = 0.0; }
    
    // end new code block -adv */
	
    if (removepftsum > 0.0) {		/* crops being added, other PFTs removed */
#ifdef DEBUG
		printf("\naddcrop\n");
		printf("newcropval: %f\n", newcropval);
		printf("noncroppftsum: %f\n", noncroppftsum);
		printf("removepftsum: %f\n", removepftsum);
		printf("treepftsum: %f\n", treepftsum);
		printf("herbaceouspftsum: %f\n", herbaceouspftsum);
#endif            
        if (noncroppftsum < removepftsum) {	/* not enough veg pfts to accommodate crops -adv */
            if (outhurttpftval[BPFT][outgrid] > (removepftsum - noncroppftsum)) {	/* there is enough bare soil to make up the difference, so remove it -adv */
#ifdef DEBUG                
				printf("adding crops, removing bare\n");
				printf("bare: %f\n", outhurttpftval[BPFT][outgrid]);
#endif                                
                outhurttpftval[BPFT][outgrid] = outhurttpftval[BPFT][outgrid] - (removepftsum - noncroppftsum);
#ifdef DEBUG                
				printf("adjusted removepftsum: %f\n", removepftsum);
#endif                                
                removepftsum = noncroppftsum;
            }
            else {	/* not enough vegetated land unit, so cap the crop amount to the entire veg land unit -adv */
#ifdef DEBUG                
                printf("adding crops, removing bare, and reducing newcropval\n");                
				printf("bare: %f\n", outhurttpftval[BPFT][outgrid]);
#endif                                
				newcropval = newcropval - (removepftsum - noncroppftsum - outhurttpftval[BPFT][outgrid]);
				/* Bugfix: removepftsum refers to the non-bare soil pfts only -adv */
                // wrong code: removepftsum = noncroppftsum + outhurttpftval[BPFT][outgrid];
				removepftsum = noncroppftsum;
#ifdef DEBUG                                
				printf("adjsuted removepftsum: %f\n", removepftsum);
				printf("adjusted newcropval: %f\n", newcropval);
#endif                                
                outhurttpftval[BPFT][outgrid] = 0.0;
            }
        }
        
        if (removepftsum < 0.0 || removepftsum > 100.0) {
            printf("Error in removepftsum %f\n",removepftsum);
        }
        
        if (removepftsum > 0.0) {
            if (noncroppftsum == 0.0 ) {	/* no pfts to remeove -adv */
                printf("Error in noncroppftsum noncroppftsum%f\n",noncroppftsum);
                printf("Error in noncroppftsum removepftsum%f\n",removepftsum);
                for (outpft = NEMPFT;outpft <= GC4PFT;outpft++) {
                    outhurttpftval[outpft][outgrid] = round( (outhurttpftval[outpft][outgrid]));
                }
            }
            else { /* remove equal amounts of each non-bare-soil pft -adv */
				/* !!! this is the original code commented by -adv
				for (outpft = NEMPFT;outpft <= GC4PFT;outpft++) {
				 outhurttpftval[outpft][outgrid] = round( (outhurttpftval[outpft][outgrid] * (noncroppftsum - removepftsum) / noncroppftsum));
                }
				original code commented out by -adv */
				
				// /* !!! preferentially remove non-forest pfts; start new code block -adv
				// this code replaces the for loop immediately prior
				 
				// remove a greater proportion of herbaceous by reducing the remaining fraction of herbaceous
				// if enough herbaceous, trees do not have to be removed at all
				// this is derived from noncroppftsum - removepftsum = herbaceousfracremain * herbaceouspftsum + treefracremain * treepftsum
				//
				// herbaceousfracremain can be set to provide different degrees of reduction
				//
				// for proportional removal:
				// the proportional herbaceousfracremain is based on the original method of removing equal proportions of all pfts:
				//		(noncroppftsum - removepftsum) / noncroppftsum
				//
				// for maximizing herbaceous removal:
				// if removepftsum < herbaceouspftsum then the minimum herbaceousfracremain = 1 - removepftsum/herbaceouspftsum
				//		with max treefracremain = 1
				// if removepftsum >= herbaceouspftsum then the minimum herbaceousfracremain = 0,
				//		with treefracremain = (noncroppftsum - removepftsum) / treepftsum
				//
				// for minimizing herbaceous removal:
				// if removepftsum <= treepftsum then the maximum herbaceousfracremain = 1
				//		with max treefracremain = 1 - removepftsum / treepftsum
				// if removepftsum > treepftsum then the maximum herbaceousfracremain = (noncroppftsum - removepftsum) / herbaceouspftsum,
				//		with treefracremain = 0
				
				// using this else value will remove pfts proportionally to their base year distribution
				if(noncroppftsum > 0.0) {
					propherbaceousfracremain = (noncroppftsum - removepftsum) / noncroppftsum;
				}
				else {
					propherbaceousfracremain = 1.0;
				}

				 // reduce the remaining herbaceous fraction to preferentially remove it
				 // this is the minimum, and if this is negative it needs to be set to zero
				if (herbaceouspftsum > 0.0) {
					minherbaceousfracremain = 1.0 - removepftsum / herbaceouspftsum;
					maxherbaceousfracremain = (noncroppftsum - removepftsum) / herbaceouspftsum;
				}
				else {
					minherbaceousfracremain = 0.0;
					maxherbaceousfracremain = 1.0;
				}

				if(minherbaceousfracremain <= 0.0) {
					minherbaceousfracremain = 0.0;
				}
				
				if (maxherbaceousfracremain > 1.0) {
					maxherbaceousfracremain = 1.0;
				}
				 
				// NOTE: setherbfracrem is the variable to adjust above!
				//		Ranges from 0 to 1 for maximizing forest (minimzing herbaceous)
				//			setherbfracrem = 1 is proportional removal
				//			setherbfracrem = 0 is remove herbaceous first (maximizes forest)
				//			this can be renormalized to include minimized forest
				//		Ranges from 1 to 2 for minimizing forest (maximizing herbaceous)
				//			setherbfracrem = 1 is proportional removal
				//			setherbfracrem = 2 is remove tree first (minimizes forest)
				if (setherbfracrem >= 0.0 && setherbfracrem <= 1.0) {
					herbaceousfracremain = minherbaceousfracremain + setherbfracrem * (propherbaceousfracremain - minherbaceousfracremain);
				} else if (setherbfracrem <= 2.0) {
					setherbfracrem = setherbfracrem - 1.0;
					herbaceousfracremain = propherbaceousfracremain + setherbfracrem * (maxherbaceousfracremain - propherbaceousfracremain);
				} else {
					printf("Error: setherbfracrem %f not within input range of 0 to 2 in sethurttcrop()\n", setherbfracrem);
				}

				if (treepftsum > 0.0) {
					treefracremain = (1.0 - herbaceousfracremain) * herbaceouspftsum / treepftsum - (removepftsum / treepftsum) + 1.0;
				}
				else {
					treefracremain = 1.0;
				}

				// ensure that the fractions are between 0.0 and 1.0
				if (herbaceousfracremain < 0.0) { herbaceousfracremain = 0.0; }
				if (herbaceousfracremain > 1.0) { herbaceousfracremain = 1.0; }
				if (treefracremain < 0.0) { treefracremain = 0.0; }
				if (treefracremain > 1.0) { treefracremain = 1.0; }
#ifdef DEBUG				
				printf("herbaceousfracremain: %f\n", herbaceousfracremain);
				printf("treefracremain: %f\n", treefracremain);
#endif                                
				
				// remove the herbaceous
				outherbaceouspftsum = 0.0;
				for (outpft = SEMPFT;outpft <= GC4PFT;outpft++) {
					outhurttpftval[outpft][outgrid] = round(outhurttpftval[outpft][outgrid] * herbaceousfracremain);
					outherbaceouspftsum = outherbaceouspftsum + outhurttpftval[outpft][outgrid];
				}
				outtreepftsum = 0.0;
				// remove the trees
				for (outpft = NEMPFT;outpft <= BDBPFT;outpft++) {
					outhurttpftval[outpft][outgrid] = round(outhurttpftval[outpft][outgrid] * treefracremain);
					outtreepftsum = outtreepftsum + outhurttpftval[outpft][outgrid];
				}
#ifdef DEBUG				
				printf("outtreepftsum: %f\n", outtreepftsum);
				printf("outherbaceouspftsum: %f\n", outherbaceouspftsum);
#endif                                
				// check for forest maximization
				// this check takes into account rounding error up to 1 unit (percent) of veg land unit
				if (setherbfracrem == 0.0 && removepftsum >= herbaceouspftsum &&
					(outherbaceouspftsum < -1.0 || outherbaceouspftsum > 1.0)) {
					printf("notreemax when adding crops and when all herbs and some trees need to be removed\n");
				}
				if (setherbfracrem == 0.0 && removepftsum < herbaceouspftsum && outtreepftsum != treepftsum) {
					printf("notreemax when adding crops and when only herbs need to be removed\n");
				}
				
				// check for forest minimization
				// this check takes into account rounding error up to 1 unit (percent) of veg land unit
				if (setherbfracrem == 2.0 && removepftsum >= treepftsum &&
					(outtreepftsum < -1.0 || outtreepftsum > 1.0)) {
					printf("notreemin when adding crops and when all trees need to be removed\n");
				}
				if (setherbfracrem == 2.0 && removepftsum < treepftsum && outherbaceouspftsum != herbaceouspftsum) {
					printf("notreemin when adding crops and when only trees need to be removed\n");
				}
				
				// end of new code block -adv */
            }	// end if noncroppftsum == 0.0 else otherwise
        }	// end if second check removepftsum > 0.0
    }	// end if first check removepftsum > 0.0
    else {
        if (addpftsum > 0.0) {		/* crops being removed, other PFTs added */
#ifdef DEBUG            
			printf("\nremovecrop\n");
			printf("newcropval: %f\n", newcropval);
			printf("noncroppftsum: %f\n", noncroppftsum);
			printf("addpftsum: %f\n", addpftsum);
			printf("availpotvegtreepftsum: %f\n", availpotvegtreepftsum);
			printf("availpotvegherbpftsum: %f\n", availpotvegherbpftsum);
#endif                        
            if (noncroppftsum + addpftsum + newcropval + outhurttpftval[BPFT][outgrid] > 100.0) {	/* cap the addition of pfts to the veg land unit -adv */
#ifdef DEBUG                
                printf("removing crops, capping pft addition\n");
				printf("bare: %f\n", outhurttpftval[BPFT][outgrid]);
#endif                                
				addpftsum = 100.0 - (noncroppftsum + newcropval + outhurttpftval[BPFT][outgrid]);
#ifdef DEBUG                                
				printf("adjusted addpftsum: %f\n", addpftsum);
#endif                                
            }
            
			// !!! this code is used to check where potential trees can replace all removed crops -adv
			if (availpotvegtreepftsum >= addpftsum) {
				cropavailpotvegtreepftval[outgrid] = 1;
#ifdef DEBUG                                
				printf("trees can replace all removed crops\n");
#endif                                
			}
			else {
				cropavailpotvegtreepftval[outgrid] = 0;
			}

			/* !!! just add trees -adv */
			if (ADDTREEONLY) {
				if (addpftsum < 0.0 || addpftsum > 100.0) {
					printf("Error in addpftsum %f\n",addpftsum);
				}
				
				if (addpftsum > 0.0) {
					if (potvegtreepftsum > 0.0) {
						for (outpft = NEMPFT;outpft <= BDBPFT;outpft++) {
							outhurttpftval[outpft][outgrid] =
								round(outhurttpftval[outpft][outgrid] + inpotvegpftval[outpft][outgrid] * addpftsum / potvegtreepftsum);
						}
					}
					else if (treepftsum > 0.0) {
						for (outpft = NEMPFT;outpft <= BDBPFT;outpft++) {
							outhurttpftval[outpft][outgrid] =
								round(outhurttpftval[outpft][outgrid] + outhurttpftval[outpft][outgrid] * addpftsum / treepftsum);
						}
					}
					else {
						for (outpft = NEMPFT;outpft <= BDBPFT;outpft++) {
							outhurttpftval[outpft][outgrid] =
								round(outhurttpftval[outpft][outgrid] + 1.0 * addpftsum / 8.0);
						}
					}
				}
			}
			else {
				/* add pfts based on potential vegetation -adv */
				/* this zero potveg catch isn't necessary, but it avoids going through the calculations below -adv */
				if (potvegpftsum == 0.0) {	/* add bare soil if no potential pfts reside in this cell -adv */
#ifdef DEBUG                                    
					printf("removing crops, adding only bare\n");
					printf("bare: %f\n", outhurttpftval[BPFT][outgrid]);
					printf("addpftsum: %f\n", addpftsum);
#endif                                        
					outhurttpftval[BPFT][outgrid] = outhurttpftval[BPFT][outgrid] + addpftsum;
					addpftsum = 0.0;
#ifdef DEBUG                                        
					printf("adjusted addpftsum: %f\n", addpftsum);
#endif                                        
				}
            
				if (addpftsum < 0.0 || addpftsum > 100.0) {
					printf("Error in addpftsum %f\n",addpftsum);
				}
            
				/* !!! original code commented out by -adv
				 if (potvegpftsum != 0.) {addtreepftsum = addpftsum * potvegtreepftsum / potvegpftsum;}
				 // addtreepftsum = addpftsum * potvegtreepftsum / potvegpftsum;
				 addherbaceouspftsum = addpftsum - addtreepftsum;
				 original code commented out by -adv */
            
				/* NOTE: this is where secondary land is created start new code block -adv */
				/* add potential veg pfts in proportion to their available potential distribution -adv */
				if (addpftsum > 0.0) {
					/* original code commented out by -adv
					for (outpft = NEMPFT;outpft <= GC4PFT;outpft++) {
					if (outpft <= BDBPFT && potvegtreepftsum > 0.0) {
					 outhurttpftval[outpft][outgrid] = round( (outhurttpftval[outpft][outgrid] + inpotvegpftval[outpft][outgrid] * addtreepftsum / potvegtreepftsum));
					}
					if (outpft >= SEMPFT && potvegherbaceouspftsum > 0.0) {
					 outhurttpftval[outpft][outgrid] = round( (outhurttpftval[outpft][outgrid] + inpotvegpftval[outpft][outgrid] *	addherbaceouspftsum / potvegherbaceouspftsum));
					}
					}
					original code commented out by -adv */
				
					// /* !!! preferentially add tree or non-tree pfts -adv
				 
					// add potential vegetation tree and herbaceous pfts
					// REVIEW: if not enough available potential veg make up the difference with bare soil
					//	only crop addition removes bare soil if other pfts not available, so bare soil should be added first upon crop removal
					if(addpftsum > availpotvegtreeherbpftsum) {
#ifdef DEBUG                                            
						printf("removing crops, adding bare because not enough available potential veg\n");
						printf("bare: %f\n", outhurttpftval[BPFT][outgrid]);
						printf("availpotvegtreeherbpftsum: %f\n", availpotvegtreeherbpftsum);
#endif                                                
						outhurttpftval[BPFT][outgrid] = outhurttpftval[BPFT][outgrid] + addpftsum - availpotvegtreeherbpftsum;
						addpftsum = availpotvegtreeherbpftsum;
#ifdef DEBUG                                                
						printf("adjusted addpftsum: %f\n", addpftsum);
#endif                                                
					}
				 
					// preferentially add forest pfts over herbaceous pfts, or vice versa
					// use the same logic as above, but reduce the available potential veg
					// the available potential veg is how much can be added until the potential veg is reached
					// range of availtreefracremain goes from add all trees first to proportional addition of available potential pft percents
					// to add non-tree first to minimize forest addition
				
					// using this else value will add herbaceous and tree pfts proportionally to their available potential percents
					if(availpotvegtreeherbpftsum > 0.0) {
						propavailtreefracremain = (availpotvegtreeherbpftsum - addpftsum) / availpotvegtreeherbpftsum;
					}
					else {
						propavailtreefracremain = 1.0;
					}

					// calculate the remaining available tree fraction to preferentially add trees
					// this is the minimum, and if this is negative it needs to be set to zero
					// also calculate the maximum remaining available tree fraction, which has a max of 1
					if(availpotvegtreepftsum > 0.0) {
						minavailtreefracremain = 1.0 - addpftsum / availpotvegtreepftsum;
						maxavailtreefracremain = (availpotvegtreeherbpftsum - addpftsum) / availpotvegtreepftsum;
					}
					else {
						minavailtreefracremain = 0.0;
						maxavailtreefracremain = 1.0;
					}

					if(minavailtreefracremain <= 0.0) {
						minavailtreefracremain = 0.0;
					}
				
					if(maxavailtreefracremain > 1.0) {
						maxavailtreefracremain = 1.0;
					}
				 
					// NOTE: setavailtreefracrem is the variable to adjust!
					//		Ranges from 0 to 1 for maximizing forest (minimzing herbaceous)
					//			setavailtreefracrem = 1 is proportional removal to available potential
					//			setavailtreefracrem = 0 is add trees first (maximizes forest)
					//		Ranges from 1 to 2 for minimizing forest (maximizing herbaceous)
					//			setavailtreefracrem = 1 is proportional removal to available potential
					//			setavailtreefracrem = 2 is add herb first (minimizes forest)
					if (setavailtreefracrem >= 0.0 && setavailtreefracrem <= 1.0) {
						availtreefracremain = minavailtreefracremain + setavailtreefracrem * (propavailtreefracremain - minavailtreefracremain);
					} else if (setavailtreefracrem <= 2.0) {
						setavailtreefracrem = setavailtreefracrem - 1.0;
						availtreefracremain = propavailtreefracremain + setavailtreefracrem * (maxavailtreefracremain - propavailtreefracremain);
					} else {
						printf("Error: setavailtreefracrem %f not within input range of 0 to 2 in sethurttcrop()\n", setavailtreefracrem);
					}

					if(availpotvegherbpftsum > 0.0) {
						availherbfracremain =
							(1.0 - availtreefracremain) * availpotvegtreepftsum / availpotvegherbpftsum -
							(addpftsum / availpotvegherbpftsum) + 1.0;
					}
					else {
						availherbfracremain = 1.0;
					}
					
					// ensure that the fractions are between 0.0 and 1.0
					if (availherbfracremain < 0.0) { availherbfracremain = 0.0; }
					if (availherbfracremain > 1.0) { availherbfracremain = 1.0; }
					if (availtreefracremain < 0.0) { availtreefracremain = 0.0; }
					if (availtreefracremain > 1.0) { availtreefracremain = 1.0; }
#ifdef DEBUG					
					printf("availtreefracremain: %f\n", availtreefracremain);
					printf("availherbfracremain: %f\n", availherbfracremain);
#endif                                        
				
					// add tree pfts by available potential proportions
					// if there is no potential tree veg then these outhurttpftvals do not change
					outavailpotvegtreepftsum = availpotvegtreepftsum;
					if(potvegtreepftsum > 0.0) {
						for (outpft = NEMPFT;outpft <= BDBPFT;outpft++) {
							outhurttpftval[outpft][outgrid] =
								round(outhurttpftval[outpft][outgrid] + inpotvegpftval[outpft][outgrid] *
									  (availpotvegtreepftsum * (1.0 - availtreefracremain)) / potvegtreepftsum);
							outavailpotvegtreepftsum = outavailpotvegtreepftsum - 
							round(inpotvegpftval[outpft][outgrid] *
								  (availpotvegtreepftsum * (1.0 - availtreefracremain)) / potvegtreepftsum);
						}
					}
					
					// add each grass and shrub by potential proportions, constrained by available potential grass and shrub
					// if there is no potential herb veg then these outhurttpftvals do not change change here
					// the bare soil is changed separately above if necessary
					outavailpotvegherbpftsum = availpotvegherbpftsum;
					removeavailpotvegherb = availpotvegherbpftsum * (1.0 - availherbfracremain);
					removeavailpotveggrass = availpotveggrasspftsum * (1.0 - availherbfracremain);
					removeavailpotvegshrub = removeavailpotvegherb - removeavailpotveggrass;
					if(potveggrasspftsum > 0.0) {
						for (outpft = GA3PFT;outpft <= GC4PFT;outpft++) {
							outhurttpftval[outpft][outgrid] = round(outhurttpftval[outpft][outgrid] + inpotvegpftval[outpft][outgrid] *
																	removeavailpotveggrass / potveggrasspftsum);
							outavailpotvegherbpftsum = outavailpotvegherbpftsum -
							round(inpotvegpftval[outpft][outgrid] * removeavailpotveggrass / potveggrasspftsum);
						}
					}
					if(potvegshrubpftsum > 0.0) {
						for (outpft = SEMPFT;outpft <= SDBPFT;outpft++) {
							outhurttpftval[outpft][outgrid] = round(outhurttpftval[outpft][outgrid] + inpotvegpftval[outpft][outgrid] *
																	removeavailpotvegshrub / potvegshrubpftsum);
							outavailpotvegherbpftsum = outavailpotvegherbpftsum -
								round(inpotvegpftval[outpft][outgrid] * removeavailpotvegshrub / potvegshrubpftsum);
						}
					}
#ifdef DEBUG				 
					printf("outavailpotvegtreepftsum: %f\n", outavailpotvegtreepftsum);
					printf("outavailpotvegherbpftsum: %f\n", outavailpotvegherbpftsum);
#endif                                        
					// check forest maximization
					// this check takes into account rounding error up to 1 unit (percent) of veg land unit
					if (setavailtreefracrem == 0.0 && addpftsum >= availpotvegtreepftsum &&
						(outavailpotvegtreepftsum < -1.0 || outavailpotvegtreepftsum > 1.0)) {
							printf("notreemax when removing crops and when all avail trees and some herb need to be added\n");
					}
					if (setavailtreefracrem == 0.0 && addpftsum < availpotvegtreepftsum &&
						outavailpotvegherbpftsum != availpotvegherbpftsum) {
							printf("notreemax when removing crops and when only avail trees need to be added\n");
					}
					
					// check forest minimization
					// this check takes into account rounding error up to 1 unit (percent) of veg land unit
					if (setavailtreefracrem == 2.0 && addpftsum >= availpotvegherbpftsum &&
						(outavailpotvegherbpftsum < -1.0 || outavailpotvegherbpftsum > 1.0)) {
						printf("notreemin when removing crops and when all avail herb and some trees need to be added\n");
					}
					if (setavailtreefracrem == 2.0 && addpftsum < availpotvegherbpftsum &&
						outavailpotvegtreepftsum != availpotvegtreepftsum) {
						printf("notreemin when removing crops and when only avail herb need to be added\n");
					}
					
					// end new code block -adv */
				
				}	// end if second check for addpftsum > 0.0
			}	// end if just add trees else base pft addition on potential vegetation
        }	// end if first check for addpftsum > 0.0
    }	// end else remove crop
    
    maxpftid = 0;
    maxpftval = 0.0;
    updatedpftsum = 0.0;
    
    for (outpft = BPFT;outpft <= GC4PFT;outpft++) {
        if (outhurttpftval[outpft][outgrid] < 0.0) {
            outhurttpftval[outpft][outgrid] = 0.0;
        }
        if (outpft > BPFT && maxpftval < outhurttpftval[outpft][outgrid]) {
            maxpftid = outpft;
            maxpftval = outhurttpftval[outpft][outgrid];
        }
        updatedpftsum = updatedpftsum + outhurttpftval[outpft][outgrid];
    }
    
    if ((updatedpftsum + newcropval) < 100.0) {
        addpftsum = 100.0 - (updatedpftsum + newcropval);
        if (addpftsum > 1.0) {
#ifdef DEBUG            
            printf("Crop Addsum = %f ",addpftsum);
#endif            
            for (temppftid = 0; temppftid < MAXPFT-1; temppftid++) {
#ifdef DEBUG                
                printf("%f ",outhurttpftval[temppftid][outgrid]);
#endif                
            }
			/* Bugfix: newcropval is the new crop pft sum, so don't add it to the outhurttpftval[CPFT] -adv */
#ifdef DEBUG            
            printf("%f ",newcropval);
            printf("\n");
#endif            
        }
        outhurttpftval[maxpftid][outgrid] = outhurttpftval[maxpftid][outgrid] + addpftsum;
        updatedpftsum = updatedpftsum + addpftsum;
    }
    
    if ((updatedpftsum + newcropval) > 100.0) {
        removepftsum = (updatedpftsum + newcropval - 100.0);
        if (removepftsum > 1.0) {
#ifdef DEBUG            
            printf("Crop Removesum = %f ",removepftsum);
#endif            
            for (temppftid = 0; temppftid < MAXPFT-1; temppftid++) {
#ifdef DEBUG                
                printf("%f ",outhurttpftval[temppftid][outgrid]);
#endif                
            }
			/* Bugfix: newcropval is the new crop pft sum, so don't add it to the outhurttpftval[CPFT] -adv */
#ifdef DEBUG            
            printf("%f ",newcropval);
            printf("\n");
#endif            
        }
        outhurttpftval[maxpftid][outgrid] = outhurttpftval[maxpftid][outgrid] - removepftsum;
        if (outhurttpftval[maxpftid][outgrid] < 0.0) {
#ifdef DEBUG            
			printf("balance pft sum in sethurttcrop, subtracting bare\n");
			printf("bare: %f\n", outhurttpftval[BPFT][outgrid]);
			printf("outhurttpftval[maxpftid][outgrid]: %f\n", outhurttpftval[maxpftid][outgrid]);

#endif                        
            outhurttpftval[BPFT][outgrid] = outhurttpftval[BPFT][outgrid] + outhurttpftval[maxpftid][outgrid];
            outhurttpftval[maxpftid][outgrid] = 0.0;
#ifdef DEBUG            
			printf("adjusted bare: %f\n",outhurttpftval[BPFT][outgrid]);
			printf("adjusted outhurttpftval[maxpftid][outgrid]: %f\n", outhurttpftval[maxpftid][outgrid]);
#endif                        
        }
    }
    
    /* NOTE: the crop fraction is finally set to be the same value like that of GLM. All the above algorithms in sethurttcrop() are actually not be used. -jfm */
	/* this is still set to match glmo (capped by the vegetated land unit), so actually the above algorithms are still needed to adjust the pfts;
	 the beginning lines can be changed to set newcropval as input crop fraction + the relative change in crop fraction, which would change this last line -adv */
    outhurttpftval[CPFT][outgrid] = newcropval;
    
}


/*------
	sethurttpasture()
	outgrid - grid cell index
	modyear - the model year, which is the year prior to the output year
	calcyear - the output year for pft values
	set the output pft values in the grid cell indexed by outgrid, based on the difference between the output and base years
	the max amount of pasture that can be removed is the amount of base year shrubs and grasses,
		and historically, further capped by the potential tree pft amount because pasture addition removes only tree pfts
	pasture addition is capped by the amount of base year tree pfts
	the removal and addition of pfts is controlled by the land conversion assumption variables
		which are set specifically to match the orignial historical assumptions up to 2015 and to increase afforestation from 2015 forward
	-adv
------*/
void sethurttpasture(int outgrid, int modyear, int calcyear) {
    
    int maxpftid, outpft, outmonth, pasturegrid, temppftid;
    float maxpftval, treepftsum, potvegpftsum, potvegtreepftsum;
    /* float addpftsum, addtreepftsum, addherbaceouspftsum, removepftsum, updatedpftsum; commented out by -adv */
	float addpftsum, removepftsum, updatedpftsum;
    float herbaceouspftsum, potvegherbaceouspftsum, newpasturepftsum, grasspftsum;
	/* new variables for preferential pft removal/addition -adv */
	float shrubpftsum, treeshrubpftsum, availablepasturesum, potveggrasspftsum, potvegshrubpftsum;
	float availpotvegtreepftsum, availpotvegherbpftsum, availpotvegtreeherbpftsum, availpotveggrasspftsum, availpotvegshrubpftsum;
	float basegrasspftsum, outpasturesum;
	float maxshrubfracremain, propshrubfracremain, minshrubfracremain, shrubfracremain, treefracremain;
	float maxavailtreefracremain, propavailtreefracremain, minavailtreefracremain, availtreefracremain, availherbfracremain;
	float outtreepftsum, outshrubpftsum, outavailpotvegtreepftsum, outavailpotvegherbpftsum;
	float removeavailpotvegherb, removeavailpotveggrass, removeavailpotvegshrub;

	// land conversion assumption variables
	/* !!! these are the new variables that control the preferential pft removal/addition -adv
	 the values range from:
	 2 = maximize herbaceous pfts (i.e. minimize forest pfts)
	 1 = proportional based on relevant pft distribution
	 0 = minimize herbaceous pfts (i.e. maximize forest pfts)
	 */
	
	int ADDTREEONLY;			// For pasture removal: 1=addtreeonly; 0=add pfts proportionally based on setavailtreefracrem
	int INCLUDEBARE;			// 1=include bare soil as available for pasture and trees; 0=do not make bare soil available
	int GRASSPASTURE;			// 1=match clm grass to glmo pasture; 0=adjust grass and shrub based on differences from basepasture
	float setshrubfracrem;		// pasture addition; this includes bare soil when INCLUDEBARE=1=ADDTREEONLY
	float setavailtreefracrem;	// pasture removal
	
	// afforestation policy begins affecting land use at the beginning of 2015
	// the original assumptions appply before 2015
	// INCLUDEBARE = 1 is valid only when ADDTREEONLY = 1
	// INCLUDEBARE and GRASSPASTURE should always equal 0 because they cause strange land use behavior otherwise
	if (modyear >= 2015) {
		//	only trees replace pasture removals (setavailtreefracrem not used)
		//	trees are preferentially removed upon pasture addition to compensate for indiscriminant tree addition upon pasture removal
		//		this also follows from the idea that trees added to unfavoravble areas might be the first to go due to higher value pasture
		ADDTREEONLY = 1;	// For pasture removal: 1=addtreeonly; 0=add pfts proportionally based on setavailtreefracrem
		INCLUDEBARE = 0;	// 1=include bare soil as available for pasture and trees; 0=do not make bare soil available
		GRASSPASTURE = 0;	// 1=match clm grass to glmo pasture; 0=adjust grass and shrub based on differences from basepasture
		setshrubfracrem = 2.0;	// pasture addition; this includes bare soil when INCLUDEBARE=1=ADDTREEONLY
		setavailtreefracrem = 0.0;	// pasture removal
    } else {
		// Historically, pasture addition removes only trees, and pasture removal subtracts herbaceous pfts and adds pfts in proportion to available potential veg
		// this is the original assumption, but available potential vegetation has been used in place of potential vegetation for pasture removal
		ADDTREEONLY = 0;	// For pasture removal: 1=addtreeonly; 0=add pfts proportionally based on setavailtreefracrem
		INCLUDEBARE = 0;	// 1=include bare soil as available for pasture and trees; 0=do not make bare soil available
		GRASSPASTURE = 0;	// 1=match clm grass to glmo pasture; 0=adjust grass and shrub based on differences from basepasture
		setshrubfracrem = 2.0;	// pasture addition; this includes bare soil when INCLUDEBARE=1=ADDTREEONLY
		setavailtreefracrem = 1.0;	// pasture removal
	}
    
	// calculate pft states
	
    herbaceouspftsum = 0.0;
    for (outpft = SEMPFT;outpft <= GC4PFT;outpft++) {
        herbaceouspftsum = herbaceouspftsum + outhurttpftval[outpft][outgrid];
    }
	
    treepftsum = 0.0;
    for (outpft = NEMPFT;outpft <= BDBPFT;outpft++) {
        treepftsum = treepftsum + outhurttpftval[outpft][outgrid];
    }
    
	/* this potential veg sum does not include bare soil -adv */
    potvegpftsum = 0.0;
    for (outpft = NEMPFT;outpft <= GC4PFT;outpft++) {
        potvegpftsum = potvegpftsum + inpotvegpftval[outpft][outgrid];
    }
    
    potvegtreepftsum = 0.0;
    for (outpft = NEMPFT;outpft <= BDBPFT;outpft++) {
        potvegtreepftsum = potvegtreepftsum + inpotvegpftval[outpft][outgrid];
    }
    
    potvegherbaceouspftsum = 0.0;
    for (outpft = SEMPFT;outpft <= GC4PFT;outpft++) {
        potvegherbaceouspftsum = potvegherbaceouspftsum + inpotvegpftval[outpft][outgrid];
    }
    
	// /* !!! set this up to use inhurttpasture[outgrid] as the new clm grass percent; start of new code block -adv
	// this will cause an initial shift in pfts, like the original crop code,
	//		but the clm grass changes will attempt to track glm pasture changes over time
	//		clm grass will more or less match the glm pasture by the end of 2005
	// some cells will have more grass pft than pasture,
	//  where there is more potential grass pft than pasture
	// some cells will have less grass pft than pasture,
	//  where not enough tree, shrub, and grass pfts exist (post-setcrop output state) to meet the pasture demand for the output year
	// as in the original code:
	// bare soil has been excluded from pft removal (pasture addition) because we assume that most pasture is based on 'natural' vegeatation here,
	//		and bare soil would require agricultural improvement (which has actually happened, but not yet in the model)
	// bare soil is also excluded from pft addition because we have not implemented proper degradation algorithms
	// an option has been added to not restrict added pasture (and trees upon pasture removal) to non-bare soil if the pasture removal option is ADDTREEONLY
	//		so pasture addition can go anywhere
	//		for pasture removal with only trees added the trees replace the pasture
	//		pasture removal based on potential vegetation cannot use this option to include bare soil in either addition or removal
	 
	grasspftsum = 0.0;
	for (outpft = GA3PFT;outpft <= GC4PFT;outpft++) {
		grasspftsum = grasspftsum + outhurttpftval[outpft][outgrid];
	}
	 
	shrubpftsum = 0.0;
	for (outpft = SEMPFT;outpft <= SDBPFT;outpft++) {
		shrubpftsum = shrubpftsum + outhurttpftval[outpft][outgrid];
	}
	
	potveggrasspftsum = 0.0;
    for (outpft = GA3PFT;outpft <= GC4PFT;outpft++) {
        potveggrasspftsum = potveggrasspftsum + inpotvegpftval[outpft][outgrid];
    }
	
	potvegshrubpftsum = 0.0;
    for (outpft = SEMPFT;outpft <= SDBPFT;outpft++) {
        potvegshrubpftsum = potvegshrubpftsum + inpotvegpftval[outpft][outgrid];
    }
	
	// the total available pasture land (post-sethurttcrop output state)
	// add the bare pft to the shrub pft if required - should not be used (INCLUDEBARE = 0)
	if (INCLUDEBARE && ADDTREEONLY) {
		shrubpftsum = shrubpftsum + outhurttpftval[BPFT][outgrid];
		availablepasturesum = herbaceouspftsum + treepftsum + outhurttpftval[BPFT][outgrid];
	} else {
		availablepasturesum = herbaceouspftsum + treepftsum;
	}
	
	treeshrubpftsum = treepftsum + shrubpftsum;
	
	// find the percent of grass in the nearest base year cell with grass in it
	// this is used to calculate the proportions of each grass pft within total grass
	pasturegrid = findcurrentpasturegrid(outgrid);
	basegrasspftsum = 0.0;
	for (outpft = GA3PFT;outpft <= GC4PFT;outpft++) {
		basegrasspftsum = basegrasspftsum + incurrentpftval[outpft][pasturegrid];
	}
	
	// option to match grass to pasture (this causes dramatic shifts in PFTs)
	// so do not use this option (GRASSPASTURE should be set to 0)
	//  try making changes similar to the original pl code, by differences from basepasture
	//  add grass, but remove tree/shrub pfts based on settings (instead of just removing trees)
	//  remove herbaceous, but add pfts either as tree only or by available potential veg (instead of by potential veg)
	//  this should keep the PFTs in line with history 
	if (GRASSPASTURE) {
		// the added pasture is what is needed to make the grass pft match the out pasture fraction
		// output pasture cannot exceed available area for pasture,
		//  which generally is only the existing state of tree and shrub pfts
		//  so clm grass can be limited by bare soil and crops in relation to glmo pasture
		outpasturesum = inhurttpasture[outgrid];
		if(outpasturesum > availablepasturesum) {
			outpasturesum = availablepasturesum;
		}
		newpasturepftsum = outpasturesum - grasspftsum;
	} else {
		// make changes based on difference from base pasture
		newpasturepftsum = inhurttpasture[outgrid] - inhurttbasepasture[outgrid];
	}
	
    if (newpasturepftsum > 0.0) {		// pasture (grass) being added, PFT removal limited to existing tree amount
        addpftsum = 0.0;
		if (!GRASSPASTURE && newpasturepftsum > treepftsum) {
			newpasturepftsum = treepftsum;		// new pasture can replace only existing trees
		}
        removepftsum = newpasturepftsum;
		outpasturesum = grasspftsum + newpasturepftsum;		// this is just the output grass in this case
    }
    else {								// pasture (herbaceous) being removed, herbaceous/tree PFTs added 
		removepftsum = 0.0;
        addpftsum = -newpasturepftsum;
		// remove all required grass/pasture then add tree and shrub and grass in proportion to their new available potential
		//  grass tracks glmo pasture fairly well, but doesn't eliminate all grass when pasture is less than potential grass
		// this is limited by how much grass or herbaceous there is to remove
		if (GRASSPASTURE && addpftsum > grasspftsum) {
			addpftsum = grasspftsum;
		} else if (!GRASSPASTURE) {
			if (addpftsum > herbaceouspftsum) {
				addpftsum = herbaceouspftsum;	// remove only herbaceous PFTs as pasture
			}
			// REVIEW: to coincide with orginal assumptions, further constrain pasture removal to the available potential tree amount
			// i think this was included because only trees are removed on pasture addition
			// but it forces a redistribution unconstrained by other potential veg to ensure that forest doesn't get too big
			//	and doesn't account for additional removal of trees by crop addition
			if (!ADDTREEONLY) {
				if (addpftsum + treepftsum > potvegtreepftsum) {
					if (treepftsum < potvegtreepftsum) {
						addpftsum = potvegtreepftsum - treepftsum;
					}
					else {
						addpftsum = 0.0;
					}
				}
			}
			outpasturesum = herbaceouspftsum - addpftsum;	// this is remaining herbaceous before trees or potential veg are added
		}
		
		// these are needed only for pasture removal
		// get the available percents of tree and grass and shrub and total potential veg
		// the available herbaceous amount depends on how much pasture is removed
		// the available grass and shrub amounts depend on the ratio between existing amounts and the amount of pasture removed
		// the math works out such that the available values max out at the potential values
		availpotvegtreepftsum = potvegtreepftsum - treepftsum;
		if( availpotvegtreepftsum < 0.0 ) { availpotvegtreepftsum = 0.0; }
		availpotvegherbpftsum = potvegherbaceouspftsum - herbaceouspftsum + addpftsum;
		if( availpotvegherbpftsum < 0.0 ) { availpotvegherbpftsum = 0.0; }
		availpotvegtreeherbpftsum = availpotvegtreepftsum + availpotvegherbpftsum;
		if (herbaceouspftsum <= 0) {
			availpotveggrasspftsum = potveggrasspftsum - grasspftsum;
			availpotvegshrubpftsum = potvegshrubpftsum - shrubpftsum;
		} else {
			availpotveggrasspftsum = potveggrasspftsum - grasspftsum + grasspftsum / herbaceouspftsum * addpftsum;
			availpotvegshrubpftsum = potvegshrubpftsum - shrubpftsum + shrubpftsum / herbaceouspftsum * addpftsum;
		}
		if( availpotveggrasspftsum < 0.0 ) { availpotveggrasspftsum = 0.0; }
		if( availpotvegshrubpftsum < 0.0 ) { availpotvegshrubpftsum = 0.0; }
    }	// end else remove pasture
	 
	if (removepftsum > 0.0) {		// pasture (grass) being added, PFT removal limited to existing tree amount
#ifdef DEBUG	
		printf("\naddpasture\n");
		printf("availablepasturesum: %f\n",availablepasturesum);
		printf("outpasturesum: %f\n",outpasturesum);
		printf("removepftsum: %f\n", removepftsum);
		printf("treepftsum: %f\n", treepftsum);
		printf("shrubpftsum: %f\n", shrubpftsum);
		printf("grasspftsum: %f\n", grasspftsum);
#endif		 
		// !!! preferentially add pasture to non-forest PFTs, or to forest PFTs -adv
			
		// given that GCAM treats all pasture as grass, regardless of the base year maps,
		// and only CLM grass pfts contribute to the pasture scalers
		// and GLM also considers pasture as grass
		// all pasture fraction should be converted to grass whenever possible
		// preferentially remove shrubs or trees, based on a remaining fraction of shrub, analogous to the new crop addition code
		// for original assumptions, preferentially remove trees (the amount has been limited trees above)
	 
		// range of shrubfracremain can be:
		//	max removal of (treeshrubpftsum - removepftsum) / shrubpftsum or 1.0
		//	proporitonal removal of (treeshrubpftsum - removepftsum) / treeshrubpftsum
		//  min removal of 1 - removepftsum/shrubpftsum or 0.0
	 
		// using this else value will remove shrub and tree pfts proportionally to their base year state
		 if(treeshrubpftsum > 0.0) {
			 propshrubfracremain = (treeshrubpftsum - removepftsum) / treeshrubpftsum;
		 }
		 else {
			 propshrubfracremain = 1.0;
		 }

		// calculate the remaining shrub fraction to preferentially remove it
		// this is the minimum, and if this is negative it needs to be set to zero
		 // also calculate the maximum remaining shrub fraction, which has a max of 1
		if (shrubpftsum > 0.0) {
			minshrubfracremain = 1.0 - removepftsum / shrubpftsum;
			maxshrubfracremain = (treeshrubpftsum - removepftsum) / shrubpftsum;
		}
		else {
			minshrubfracremain = 0.0;
			maxshrubfracremain = 1.0;
		}

		if(minshrubfracremain <= 0.0) {
			minshrubfracremain = 0.0;
		}
	 
		if (maxshrubfracremain > 1.0) {
			maxshrubfracremain = 1.0;
		}
		 
		// NOTE: setshrubfracrem is the variable to adjust above!
		//		Ranges from 0 to 1 for maximizing forest (minimzing shrub)
		//			setshrubfracrem = 1 is proportional removal
		//			setshrubfracrem = 0 is remove shrub first (maximizes forest)
		//		Ranges from 1 to 2 for minimizing forest (maximizing shrub)
		//			setshrubfracrem = 1 is proportional removal
		//			setshrubfracrem = 2 is remove tree first (minimizes forest)
		if (setshrubfracrem >= 0.0 && setshrubfracrem <= 1.0) {
			shrubfracremain = minshrubfracremain + setshrubfracrem * (propshrubfracremain - minshrubfracremain);
		} else if (setshrubfracrem <= 2.0) {
			setshrubfracrem = setshrubfracrem - 1.0;
			shrubfracremain = propshrubfracremain + setshrubfracrem * (maxshrubfracremain - propshrubfracremain);
		} else {
			printf("Error: setshrubfracrem %f not within input range of 0 to 2 in sethurttpasture()\n", setshrubfracrem);
		}
		
		if (treepftsum > 0.0) {
			treefracremain = (1.0 - shrubfracremain) * shrubpftsum / treepftsum - (removepftsum / treepftsum) + 1.0;
		}
		else {
			treefracremain = 1.0;
		}

		// ensure that the fractions are between 0.0 and 1.0
		if (shrubfracremain < 0.0) { shrubfracremain = 0.0; }
		if (shrubfracremain > 1.0) { shrubfracremain = 1.0; }
		if (treefracremain < 0.0) { treefracremain = 0.0; }
		if (treefracremain > 1.0) { treefracremain = 1.0; }
#ifdef DEBUG		
		printf("shrubfracremain: %f\n", shrubfracremain);
		printf("treefracremain: %f\n", treefracremain);
#endif                
		 
		// remove the shrubs
		outshrubpftsum = 0.0;
		for (outpft = SEMPFT;outpft <= SDBPFT;outpft++) {
			outhurttpftval[outpft][outgrid] = round(outhurttpftval[outpft][outgrid] * shrubfracremain);
			outshrubpftsum = outshrubpftsum + outhurttpftval[outpft][outgrid];
		}
		// remove the bare soil if required
		if (INCLUDEBARE && ADDTREEONLY) {
			outhurttpftval[BPFT][outgrid] = round(outhurttpftval[BPFT][outgrid] * shrubfracremain);
			outshrubpftsum = outshrubpftsum + outhurttpftval[BPFT][outgrid];
		}
		// remove the trees
		outtreepftsum = 0.0;
		for (outpft = NEMPFT;outpft <= BDBPFT;outpft++) {
			outhurttpftval[outpft][outgrid] = round(outhurttpftval[outpft][outgrid] * treefracremain);
			outtreepftsum = outtreepftsum + outhurttpftval[outpft][outgrid];
		}
	 
		// add the pasture grass
		if (basegrasspftsum > 0.0) {
			// add the grass using the proportions of the nearest base year grid cell with grass in it
			for (outpft = GA3PFT;outpft <= GC4PFT;outpft++) {
				outhurttpftval[outpft][outgrid] =
					round(outhurttpftval[outpft][outgrid] +  newpasturepftsum * incurrentpftval[outpft][pasturegrid] / basegrasspftsum);
			}
		}
		else if(grasspftsum > 0.0) {
			// add the grass using the proportions of this output year grid cell
			for (outpft = GA3PFT;outpft <= GC4PFT;outpft++) {
				outhurttpftval[outpft][outgrid] =
					round(outhurttpftval[outpft][outgrid] +  newpasturepftsum * outhurttpftval[outpft][outgrid] / grasspftsum);
			}
		}
		else {
			// add the grass based on latitude
			// need to account for arctic vs non-arctic
			// the linear array starts at lower left corner and goes up latitude line-by-line
			// base on the order of values in the lon and lat arrays in the current day pft file
			// so each row is 0.5 deg so:
			// > 48N = outgrid 198721 to 259200
			// < -42N = outgrid 1 to 34560
			// > 55N = outgrid 234001 to 259200
			// < -55 = outgrid 1 to 25200
			// the grass rules are bioclimate-based (gdd and t and p),but just do this here:
			// no c4 if lat > 48 or < -42
			// use arctic grass for > 55 or < -55
			if (outgrid > 234000 || outgrid <= 25200) {		// arctic c3 only
				outhurttpftval[GA3PFT][outgrid] = round(outhurttpftval[outpft][outgrid] +  newpasturepftsum);
				outhurttpftval[GC3PFT][outgrid] = 0.0;
				outhurttpftval[GC4PFT][outgrid] = 0.0;
			}
			else if(outgrid > 198720 || outgrid <= 34560) {	// c3 only
				outhurttpftval[GA3PFT][outgrid] = 0.0;
				outhurttpftval[GC3PFT][outgrid] = round(outhurttpftval[outpft][outgrid] +  newpasturepftsum);
				outhurttpftval[GC4PFT][outgrid] = 0.0;
			}
			else {												// else split evenly between c3 and c4
				for (outpft = GC3PFT;outpft <= GC4PFT;outpft++) {
					outhurttpftval[outpft][outgrid] =
						round(outhurttpftval[outpft][outgrid] +  newpasturepftsum * 1 / 2);
				}
			}	
			 
		}	// end if-else for adding pasture grass
#ifdef DEBUG                
		printf("outtreepftsum: %f\n", outtreepftsum);
		printf("outshrubpftsum: %f\n", outshrubpftsum);
#endif                
		// check forest maximization
		// this check takes into account rounding error up to 1 unit (percent) of veg land unit
		if (setshrubfracrem == 0.0 && removepftsum >= shrubpftsum &&
			(outshrubpftsum < -1.0 || outshrubpftsum > 1.0)) {
			printf("notreemax when adding pasture when all shrubs and some trees need to be removed\n");
		}
		if (setshrubfracrem == 0.0 && removepftsum < shrubpftsum && outtreepftsum != treepftsum) {
			printf("notreemax when adding pasture when no trees need to be removed\n");
		}
		
		// check forest minimization
		// this check takes into account rounding error up to 1 unit (percent) of veg land unit
		if (setshrubfracrem == 2.0 && removepftsum >= treepftsum &&
			(outtreepftsum < -1.0 || outtreepftsum > 1.0)) {
			printf("notreemin when adding pasture when all trees and some shrubs need to be removed\n");
		}
		if (setshrubfracrem == 2.0 && removepftsum < treepftsum && outshrubpftsum != shrubpftsum) {
			printf("notreemin when adding pasture when no shrubs need to be removed\n");
		}
		
	}		// end if add pasture
	 else {						// pasture being removed, hrebaceous/tree PFTs added
	 
		 if (addpftsum > 0.0) {
#ifdef DEBUG
			 printf("\nremovepasture\n");
			 printf("availablepasturesum: %f\n",availablepasturesum);
			 printf("outpasturesum: %f\n",outpasturesum);
			 printf("addpftsum: %f\n", addpftsum);
			 printf("availpotvegtreepftsum: %f\n", availpotvegtreepftsum);
			 printf("availpotvegherbpftsum: %f\n", availpotvegherbpftsum);
#endif                         
			 
			 if (GRASSPASTURE) {
				 // remove grass pfts
				 if (grasspftsum <= 0.0) {
					 printf("Error: grasspftsum <= 0 when addpftsum > 0\n");
					 for (outpft = GA3PFT;outpft <= GC4PFT;outpft++) {
						 outhurttpftval[outpft][outgrid] = 0.0;
					 }
				 }
				 else {
					 for (outpft = GA3PFT;outpft <= GC4PFT;outpft++) {
						 outhurttpftval[outpft][outgrid] =
							round(outhurttpftval[outpft][outgrid] * (grasspftsum - addpftsum) / grasspftsum);
					 }
				 }
			 } else {	// end if GRASSPASTURE
				 // remove herbaceous pfts
				 if (herbaceouspftsum <= 0.0) {
					 printf("Error: herbaceouspftsum <= 0 when addpftsum > 0\n");
					 for (outpft = SEMPFT;outpft <= GC4PFT;outpft++) {
						 outhurttpftval[outpft][outgrid] = 0.0;
					 }
				 }
				 else {
					 for (outpft = SEMPFT;outpft <= GC4PFT;outpft++) {
						 outhurttpftval[outpft][outgrid] =
							round(outhurttpftval[outpft][outgrid] * (herbaceouspftsum - addpftsum) / herbaceouspftsum);
					 }
				 }
			 }	// end else !GRASSPASTURE

			 // !!! this code is used to check where potential trees can replace all removed pasture -adv
			 if (availpotvegtreepftsum >= addpftsum) {
				 pastureavailpotvegtreepftval[outgrid] = 1;
#ifdef DEBUG                                 
				 printf("trees can replace all removed pasture\n");
#endif                                 
			 }
			 else {
				 pastureavailpotvegtreepftval[outgrid] = 0;
			 }
			 
			 /* just add trees -adv */
			 if (ADDTREEONLY) {
				if (potvegtreepftsum > 0.0) {
					for (outpft = NEMPFT;outpft <= BDBPFT;outpft++) {
						outhurttpftval[outpft][outgrid] =
							round(outhurttpftval[outpft][outgrid] + inpotvegpftval[outpft][outgrid] * addpftsum / potvegtreepftsum);
					}
				}
				else if (treepftsum > 0.0) {
					for (outpft = NEMPFT;outpft <= BDBPFT;outpft++) {
						outhurttpftval[outpft][outgrid] =
							round(outhurttpftval[outpft][outgrid] + outhurttpftval[outpft][outgrid] * addpftsum / treepftsum);
					}
				}
				else {
					for (outpft = NEMPFT;outpft <= BDBPFT;outpft++) {
						outhurttpftval[outpft][outgrid] =
							round(outhurttpftval[outpft][outgrid] + 1.0 * addpftsum / 8.0);
					}
				}
			 }
			 else {
				 /* add pfts based on available potential vegetation -adv */
			 
				 // add potential vegetation tree and herbaceous pfts
			 
				 // preferentially add forest or herbaceous pfts based on setavailtreefracrem
				 // use the same logic as above, but reduce the available potential veg
				 // range of availtreefracremain ranges from add all trees first, to proportional addition of available potential pft percents,
				 //  to add all herbaceous pfts first
	 
				 // using this else value will add herbaceous and tree pfts proportionally to their available potential percents
				 if (availpotvegtreeherbpftsum > 0.0) {
					 propavailtreefracremain = (availpotvegtreeherbpftsum - addpftsum) / availpotvegtreeherbpftsum;
				 }
				 else {
					 propavailtreefracremain = 1.0;
				 }

				 // calclate the remaining available tree fraction to preferentially add trees
				 // this is the minimum, and if this is negative it needs to be set to zero
				 // also calculate the maximum remaining avail tree fraction, which has a max of 1
				 if (availpotvegtreepftsum > 0.0) {
					 minavailtreefracremain = 1.0 - addpftsum / availpotvegtreepftsum;
					 maxavailtreefracremain = (availpotvegtreeherbpftsum - addpftsum) / availpotvegtreepftsum;
				 }
				 else {
					 minavailtreefracremain = 0.0;
					 maxavailtreefracremain = 1.0;
				 }

				 if(minavailtreefracremain <= 0.0) {
					 minavailtreefracremain = 0.0;
				 }
				 
				if(maxavailtreefracremain > 1.0) {
					maxavailtreefracremain = 1.0;
				}
				 				 
				 // NOTE: setavailtreefracrem is the variable to adjust!
				 //		Ranges from 0 to 1 for maximizing forest (minimzing herbaceous+bare)
				 //			setavailtreefracrem = 1 is proportional removal to available potential
				 //			setavailtreefracrem = 0 is add trees first (maximizes forest)
				 //		Ranges from 1 to 2 for minimizing forest (maximizing herbaceous+bare)
				 //			setavailtreefracrem = 1 is proportional removal to available potential
				 //			setavailtreefracrem = 2 is add herb+bare first (minimizes forest)
				 if (setavailtreefracrem >= 0.0 && setavailtreefracrem <= 1.0) {
					 availtreefracremain = minavailtreefracremain + setavailtreefracrem * (propavailtreefracremain - minavailtreefracremain);
				 } else if (setavailtreefracrem <= 2.0) {
					 setavailtreefracrem = setavailtreefracrem - 1.0;
					 availtreefracremain = propavailtreefracremain + setavailtreefracrem * (maxavailtreefracremain - propavailtreefracremain);
				 } else {
					 printf("Error: setavailtreefracrem %f not within input range of 0 to 2 in sethurttpasture()\n", setavailtreefracrem);
				 }

				 if (availpotvegherbpftsum > 0.0) {
					 availherbfracremain =
					 (1.0 - availtreefracremain) * availpotvegtreepftsum / availpotvegherbpftsum -
					 (addpftsum / availpotvegherbpftsum) + 1.0;
				 }
				 else {
					 availherbfracremain = 1.0;
				 }

				 // ensure that the fractions are between 0.0 and 1.0
				 if (availherbfracremain < 0.0) { availherbfracremain = 0.0; }
				 if (availherbfracremain > 1.0) { availherbfracremain = 1.0; }
				 if (availtreefracremain < 0.0) { availtreefracremain = 0.0; }
				 if (availtreefracremain > 1.0) { availtreefracremain = 1.0; }
#ifdef DEBUG				 
				 printf("availtreefracremain: %f\n", availtreefracremain);
				 printf("availherbfracremain: %f\n", availherbfracremain);
#endif                                 
			 
				 // add tree pfts by potential proportion
				 // if there is no potential tree veg then these outhurttpftvals do not change
				 outavailpotvegtreepftsum = availpotvegtreepftsum;
				 if(potvegtreepftsum > 0.0) {
					 for (outpft = NEMPFT;outpft <= BDBPFT;outpft++) {
						 outhurttpftval[outpft][outgrid] =
							round(outhurttpftval[outpft][outgrid] + inpotvegpftval[outpft][outgrid] *
							(availpotvegtreepftsum * (1.0 - availtreefracremain)) / potvegtreepftsum);
						 outavailpotvegtreepftsum = outavailpotvegtreepftsum - round(inpotvegpftval[outpft][outgrid] *
						 (availpotvegtreepftsum * (1.0 - availtreefracremain)) / potvegtreepftsum);
					 }
				 }
	 
				 // add each grass and shrub by potential proportions, constrained by available potential grass and shrub
				 // if there is no potential herbaceous veg then these outhurttpftvals do not change change here
				 // no provision for adding back bare soil here because the INCLUDEBARE option is only used when ADDTREEONLY = 1
				 outavailpotvegherbpftsum = availpotvegherbpftsum;
				 removeavailpotvegherb = availpotvegherbpftsum * (1.0 - availherbfracremain);
				 removeavailpotveggrass = availpotveggrasspftsum * (1.0 - availherbfracremain);
				 removeavailpotvegshrub = removeavailpotvegherb - removeavailpotveggrass;
				 if(potveggrasspftsum > 0.0) {
					 for (outpft = GA3PFT;outpft <= GC4PFT;outpft++) {
						 outhurttpftval[outpft][outgrid] = round(outhurttpftval[outpft][outgrid] + inpotvegpftval[outpft][outgrid] *
																 removeavailpotveggrass / potveggrasspftsum);
						 outavailpotvegherbpftsum = outavailpotvegherbpftsum -
						 round(inpotvegpftval[outpft][outgrid] * removeavailpotveggrass / potveggrasspftsum);
					 }
				 }
				 if(potvegshrubpftsum > 0.0) {
					 for (outpft = SEMPFT;outpft <= SDBPFT;outpft++) {
						 outhurttpftval[outpft][outgrid] = round(outhurttpftval[outpft][outgrid] + inpotvegpftval[outpft][outgrid] *
																 removeavailpotvegshrub / potvegshrubpftsum);
						 outavailpotvegherbpftsum = outavailpotvegherbpftsum -
						 round(inpotvegpftval[outpft][outgrid] * removeavailpotvegshrub / potvegshrubpftsum);
					 }
				 }
#ifdef DEBUG			
				 printf("outavailpotvegtreepftsum: %f\n", outavailpotvegtreepftsum);
				 printf("outavailpotvegherbpftsum: %f\n", outavailpotvegherbpftsum);
#endif                                 
				 // check forest maximization
				 // this check takes into account rounding error up to 1 unit (percent) of veg land unit
				 if (setavailtreefracrem == 0.0 && addpftsum >= availpotvegtreepftsum &&
					 (outavailpotvegtreepftsum < -1.0 || outavailpotvegtreepftsum > 1.0)) {
					 printf("notreemax when removing pasture and all avail trees and some herbaceous are added\n");
				 }
				 if (setavailtreefracrem == 0.0 && addpftsum < availpotvegtreepftsum &&
					 outavailpotvegherbpftsum != availpotvegherbpftsum) {
					 printf("notreemax when removing pasture and only avail trees should be added\n");
				 }
				 
				 // check forest minimization
				 // this check takes into account rounding error up to 1 unit (percent) of veg land unit
				 if (setavailtreefracrem == 2.0 && addpftsum >= availpotvegherbpftsum &&
					 (outavailpotvegherbpftsum < -1.0 || outavailpotvegherbpftsum > 1.0)) {
					 printf("notreemin when removing pasture and all avail herbaceous and some trees are added\n");
				 }
				 if (setavailtreefracrem == 2.0 && addpftsum < availpotvegherbpftsum &&
					 outavailpotvegtreepftsum != availpotvegtreepftsum) {
					 printf("notreemin when removing pasture and only avail herbaceous should be added\n");
				 }
				 
			} // end else potential veg based additoin of pfts
		}	// end if addpftsum > 0.0
	 
	 }		// end else remove pasture
	 
	 //  the code below needs to be commented out down to the end of the seond else statement for removing pasture -adv
	 //  ( just prior to maxpftid = 0; )
	 
	 // end of new code block -adv */
	
	/* !!! original code commented out by -adv
	
    // the code calculates adjusted changes from a base year glm map to the glmo data, then applies these changes to the base year pft map -adv
    newpasturepftsum = inhurttpasture[outgrid] - inhurttbasepasture[outgrid];
    
    if (newpasturepftsum > 0.0) {		// pasture being added, other non-bare PFTs removed
        addpftsum = 0.0;
        // why is check below performed? -bbl
        // NOTE: pasture can be added only to former tree pfts;
        //	this is because no information is available to determine which shrub and grass pfts are pasture -adv 
		// !!! comment out this if statement for the new pasture code -adv
        if (newpasturepftsum > treepftsum) {
            newpasturepftsum = treepftsum;
        }
		
        removepftsum = newpasturepftsum;
    }
    else {								// pasture being removed, other non-barePFTs added
        removepftsum = 0.0;
        // NOTE: this seems like a reasonable cap, given that clm doesn't track pasture;
        //	but this could be inconsistent with the addition of grass-only pasture if the same asymmetry is in the historical calculations also -adv 
        if (-newpasturepftsum > herbaceouspftsum) {
            newpasturepftsum = -herbaceouspftsum;
        }
        // REVIEW: trees are never allowed to increase more than their potential value?
        //		This seems like a place where we could change logic to enable afforestation. -bbl 
        // REVIEW: the trees cannot reach the potential tree value because this limits total pasture removal;
        //	and when pasture is removed below all potential pfts are added proportionally -adv 
		// /!! comment out this if-else statement for the new pasture code -adv
        if (treepftsum - newpasturepftsum > potvegtreepftsum) {
            if (treepftsum < potvegtreepftsum) {
                newpasturepftsum = round(treepftsum - potvegtreepftsum);
            }
            else {
                newpasturepftsum = 0.0;
            }
        }
		
        addpftsum = -newpasturepftsum;
        
        //jt      addtreepftsum = addpftsum * potvegtreepftsum / potvegpftsum;
        if (potvegpftsum != 0.) { addtreepftsum = addpftsum * potvegtreepftsum / potvegpftsum;}
        addherbaceouspftsum = addpftsum - addtreepftsum;
        
    }
    
    if (removepftsum > 0.0) {		// pasture being added, other PFTs removed
		
        if (treepftsum > 0.0) {	// add pasture only if it can replace trees; and add only grasses -adv
            pasturegrid = findcurrentpasturegrid(outgrid);
            grasspftsum = 0.0;
            for (outpft = GA3PFT;outpft <= GC4PFT;outpft++) {
                grasspftsum = grasspftsum + incurrentpftval[outpft][pasturegrid];
            }
            for (outpft = NEMPFT;outpft <= GC4PFT;outpft++) {	// distribute the pasture grass pfts based on the nearest 'current' year (base in this case) pasture-containing grid cell -adv 
                if (outpft >= GA3PFT) {
                    outhurttpftval[outpft][outgrid] = round(outhurttpftval[outpft][outgrid] + incurrentpftval[outpft][pasturegrid] * newpasturepftsum / grasspftsum);
                }
                if (outpft <= BDBPFT) {
                    outhurttpftval[outpft][outgrid] = round(outhurttpftval[outpft][outgrid] * (treepftsum - removepftsum) / treepftsum);
                }
            }
            
        }
		
    }
    else {
        if (addpftsum > 0.0) {		// pasture being removed, other PFTs added
        // REVIEW: this code currently increases all other PFTs as pasture is removed,
        //		proportionate to their potential amounts. Seems like a place where
        //		we could change logic to prioritize forests. -bbl 
            if (herbaceouspftsum > 0.0 && potvegtreepftsum > 0.0) {
                for (outpft = NEMPFT;outpft <= GC4PFT;outpft++) {
                    if (outpft >= SEMPFT) { // remove current day herbaceous
                        outhurttpftval[outpft][outgrid] = round(outhurttpftval[outpft][outgrid] * (herbaceouspftsum + newpasturepftsum) / herbaceouspftsum);
                    }
                    if (outpft <= BDBPFT && potvegtreepftsum > 0.0) { // add potveg tree
                        outhurttpftval[outpft][outgrid] = round( (outhurttpftval[outpft][outgrid] + inpotvegpftval[outpft][outgrid] * addtreepftsum / potvegtreepftsum));
                    }
                    if (outpft >= SEMPFT && potvegherbaceouspftsum > 0.0) { // add potveg herbaceous
                        outhurttpftval[outpft][outgrid] = round( (outhurttpftval[outpft][outgrid] + inpotvegpftval[outpft][outgrid] * addherbaceouspftsum / potvegherbaceouspftsum));
                    }
                }
            }
        }
    }
    original code commented out by -adv */
	
    maxpftid = 0;
    maxpftval = 0.0;
    updatedpftsum = 0.0;
    
    for (outpft = BPFT;outpft <= CPFT;outpft++) {
        if (outhurttpftval[outpft][outgrid] < 0.0) {
            outhurttpftval[outpft][outgrid] = 0.0;
        }
        if (outpft > BPFT && maxpftval < outhurttpftval[outpft][outgrid]) {
            maxpftid = outpft;
            maxpftval = outhurttpftval[outpft][outgrid];
        }
        updatedpftsum = updatedpftsum + outhurttpftval[outpft][outgrid];
    }
    
    if (updatedpftsum < 100.0) {
        addpftsum = 100.0 - updatedpftsum;
        if (addpftsum > 1.0) {
#ifdef DEBUG            
            printf("Pasture Addsum = %f ",addpftsum);
#endif            
            //for (temppftid = 0; temppftid < MAXPFT; temppftid++) {
            //    printf("%f ",outhurttpftval[temppftid][outgrid]);
            //}
            //printf("\n");
        }
        outhurttpftval[maxpftid][outgrid] = outhurttpftval[maxpftid][outgrid] + addpftsum;
        updatedpftsum = updatedpftsum + addpftsum;
    }
    
    if (updatedpftsum > 100.0) {
        removepftsum = updatedpftsum - 100.0;
        if (removepftsum > 1.0) {
#ifdef DEBUG            
            printf("Pasture Removesum = %f ",removepftsum);
#endif            
            //for (temppftid = 0; temppftid < MAXPFT; temppftid++) {
            //    printf("%f ",outhurttpftval[temppftid][outgrid]);
            //}
            //printf("\n");
        }
        outhurttpftval[maxpftid][outgrid] = outhurttpftval[maxpftid][outgrid] - removepftsum;
        if (outhurttpftval[maxpftid][outgrid] < 0.0) {
#ifdef DEBUG            
			printf("balance pft sum in sethurttpasture, subtracting bare\n");
			printf("bare: %f\n\n", outhurttpftval[BPFT][outgrid]);
			printf("outhurttpftval[maxpftid][outgrid]: %f\n", outhurttpftval[maxpftid][outgrid]);
#endif                        
            outhurttpftval[BPFT][outgrid] = outhurttpftval[BPFT][outgrid] + outhurttpftval[maxpftid][outgrid];
            outhurttpftval[maxpftid][outgrid] = 0.0;
			printf("adjusted bare: %f\n\n", outhurttpftval[BPFT][outgrid]);
			printf("adjusted outhurttpftval[maxpftid][outgrid]: %f\n", outhurttpftval[maxpftid][outgrid]);
        }
    }
    
}


/* Note: Lawrence (2012) says that "primary PFTs are assigned from potential vegetation PFTs
		 scaled by the GLM primary land unit...secondary PFTs are assigned from the GLM secondary
		  land unit based on the year 2000 secondary PFT composition."

		Where in the code does this happen?!? -jm
		
		I don't think CLM can use the primary and secondary info from GLM because CLM doesn't
		keep track of these. Instead it uses the potential vegetation map to determine how to
		fill abandoned crop and pasture, which generates secondary vegetation, and the current
		pfts are reduced to make new crop and pasture, which reduces primary and/or secondary 
		vegetation. -adv
		
		Actually, the primary and secondary land fractions are used to scale the fraction of harvested area -adv
		
		Primary land in clm is such only if it is still potential veg and has always been only potential veg -adv
*/

/*-----
	sethurttlanduse()
	outgrid - grid cell index
	calcyear - the output year for pft values, the harvest data is actually for calcyear - 1
	normalize the harvested fractions to the amount of available harvested area
	output the fraction of primary+secondary land harvested by each glm harvest type in the model year
	output the fraction of output year herbaceous (grass/shrub) land that is grazed - this is not used by CLM
	-adv
-----*/
void sethurttlanduse(int outgrid) {
    
    float herbaceouspftsum, harvestsum;
    int outpft;
	int harvestyear;
	
    /* converts fractions of gridcell being harvested, to fractions of total natural vegetation being harvested */
    /* NOTE: this treats all 5 harvest classes the same, and then applies them all to forested PFTs later on */
    /* this will cause inconsistencies between the models. Information from GLM on biomass being harvested, */
    /* biomass density of land being selected for harvest, and harvest on forest vs. non-forest could be used */
    /* here to improve methods                                       -lpc    */

	//printf("outgrid=%i\tpp=%f\tps=%f\tihvh1=%f\n", outgrid, prevprimary[outgrid], prevsecondary[outgrid], inhurttvh1[outgrid]);
	
    if ((prevprimary[outgrid] + prevsecondary[outgrid]) > 0.0) {
        outhurttvh1[outgrid] = inhurttvh1[outgrid] / (prevprimary[outgrid] + prevsecondary[outgrid]);
        outhurttvh2[outgrid] = inhurttvh2[outgrid] / (prevprimary[outgrid] + prevsecondary[outgrid]);
        outhurttsh1[outgrid] = inhurttsh1[outgrid] / (prevprimary[outgrid] + prevsecondary[outgrid]);
        outhurttsh2[outgrid] = inhurttsh2[outgrid] / (prevprimary[outgrid] + prevsecondary[outgrid]);
        outhurttsh3[outgrid] = inhurttsh3[outgrid] / (prevprimary[outgrid] + prevsecondary[outgrid]);
    }
    else {
        outhurttvh1[outgrid] = 0.0;
        outhurttvh2[outgrid] = 0.0;
        outhurttsh1[outgrid] = 0.0;
        outhurttsh2[outgrid] = 0.0;
        outhurttsh3[outgrid] = 0.0;
    }
    
    /* checks for overflow and underflow - lpc */
    if (outhurttvh1[outgrid] < 0.0) {
        outhurttvh1[outgrid] = 0.0;
    }
    harvestsum = outhurttvh1[outgrid];
    if (harvestsum > 1.0) {
        outhurttvh1[outgrid] = 1.0;
        harvestsum = 1.0;
    }
    
    if (outhurttvh2[outgrid] < 0.0) {
        outhurttvh2[outgrid] = 0.0;
    }
    harvestsum = harvestsum + outhurttvh2[outgrid];
    if (harvestsum > 1.0) {
        outhurttvh2[outgrid] = outhurttvh2[outgrid] - (harvestsum - 1.0);
        harvestsum = 1.0;
    }
    
    if (outhurttsh1[outgrid] < 0.0) {
        outhurttsh1[outgrid] = 0.0;
    }
    harvestsum = harvestsum + outhurttsh1[outgrid];
    if (harvestsum > 1.0) {
        outhurttsh1[outgrid] = outhurttsh1[outgrid] - (harvestsum - 1.0);
        harvestsum = 1.0;
    }
    
    if (outhurttsh2[outgrid] < 0.0) {
        outhurttsh2[outgrid] = 0.0;
    }
    harvestsum = harvestsum + outhurttsh2[outgrid];
    if (harvestsum > 1.0) {
        outhurttsh2[outgrid] = outhurttsh2[outgrid] - (harvestsum - 1.0);
        harvestsum = 1.0;
    }
    
    if (outhurttsh3[outgrid] < 0.0) {
        outhurttsh3[outgrid] = 0.0;
    }
    harvestsum = harvestsum + outhurttsh3[outgrid];
    if (harvestsum > 1.0) {
        outhurttsh3[outgrid] = outhurttsh3[outgrid] - (harvestsum - 1.0);
        harvestsum = 1.0;
    }
	
    /* compute the total herbaceous area over all herbaceous PFTs - lpc */
    herbaceouspftsum = 0.0;
    for (outpft = SEMPFT;outpft <= GC4PFT;outpft++) {
        herbaceouspftsum = herbaceouspftsum + outhurttpftval[outpft][outgrid];
    }
    
    /* fraction of herbaceous area that is grazed is the ratio of GLM pasture area to CLM herbaceous area - lpc */
    if (herbaceouspftsum > 0.0) {
        outhurttgrazing[outgrid] = inhurttpasture[outgrid] / herbaceouspftsum;
    }
    else {
        outhurttgrazing[outgrid] = 0.0;
    }
    
    if (outhurttgrazing[outgrid] < 0.0) {
        outhurttgrazing[outgrid] = 0.0;
    }
    if (outhurttgrazing[outgrid] > 1.0) {
        outhurttgrazing[outgrid] = 1.0;
    }
    
}


/*-----
	calchurtt()
	modyear - the model year, which is prior to the output year (unless calcyear<=2015)
	calcyear - the output year for pft values 
	-adv
-----*/
void
calchurtt(int modyear, int calcyear) {
    
    int outgrid,outpft;
    
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
		/* initalize two pft mask arrays for each grid -adv */
		cropavailpotvegtreepftval[outgrid] = 0;
		pastureavailpotvegtreepftval[outgrid] = 0;
		
    	/* put the base year pfts into the output pft array -adv */
        sethurttcurrent(outgrid);
        if (invegbare[outgrid] > 0.0) {	/* calc new pfts only if the grid cell has a non-zero vegetated land unit -adv */
            sethurttcrop(outgrid,modyear,calcyear);	/* add or remove crops from the output pft array -adv */
            sethurttpasture(outgrid,modyear,calcyear);	/* add or remove pasture from the output pft array -adv */
            sethurttlanduse(outgrid);	/* calculate the normalized harvest and grazing fractions -adv */
        } 
    }
    
}

/*-----
	sethurttpotveg()
	set the output year glm land use to 100% primary and no harvest
	this function is no longer present becuase the model year is currently restricted between 1850 and 2100 by the case configuration
	-adv

void
sethurttpotveg() {
    
    int outgrid;
    
    for (outgrid = 0; outgrid < MAXOUTPIX * MAXOUTLIN; outgrid++) {
        inhurttprimary[outgrid] = 100.0;
        inhurttsecondary[outgrid] = 0.0;
        inhurttcrop[outgrid] = 0.0;
        inhurttpasture[outgrid] = 0.0;
        inhurttvh1[outgrid] = 0.0;
        inhurttvh2[outgrid] = 0.0;
        inhurttsh1[outgrid] = 0.0;
        inhurttsh2[outgrid] = 0.0;
        inhurttsh3[outgrid] = 0.0;
    }
    
}
 -----*/

/*-----
	updateannuallanduse_main()
	glmo - ouput from glm2iac glm call
	plodata - array for storing the output of this function
	inyear - the output year (which is the year of the input glm land use data); this is actually the current model year plus one
			- the glmo harvest data is for the model year
 	ISFUTURE is a flag to tell how many values (constants are defined at top) to read from the LUH files; 1=future, 0=historical
	calulate the output year pfts from changes in the base year pfts, based on base year (pasture only) and output year glmo data
	-adv
-----*/

	// standalone does not need the array arguments:
#ifdef STANDALONE
void
updateannuallanduse(int *inyear, int ISFUTURE, char *out_dir) {
#else
void
updateannuallanduse_main(float glmo[][GLMONFLDS], float plodata[][PLONFLDS], int *inyear) {
#endif
	fprintf(stderr, "\ninyear %i started in updateannuallanduse\n", *inyear);
	int i;
	char filenamestr[250];
	long hurttyear;		// this is an index for the luh files, starts at 0 for the first data year; assumes that the first luh data year coincides with either 1850 or 2015 model start years
	char fout[250];
	long outyear;	/* the output year, which is the year of the glmo land use data (inyear); this used to be myear, but I changed the name for consistency - adv */
	long modyear;	/* the actual cesm model year, which is the inyear - 1; this is the reference year for the previous year method - adv */
	long hurttinityear;	/* this is the initial year of the luh data and is used to check for year consistency with model start; it is also used as a dummy index for luh read functions - adv */
    
	char buf[250];
	char msg[1000];
	
	time_t t;
	struct tm* tm;
	
	t = time(NULL);
	tm = localtime(&t);
	
	// some values that determine limits - so that changing them is done here
	int min_year = 1850; 			// can't run with outyear prior to this value
	int max_year = 2100;			// can't run with outyear after this values
	// note that these two initial years assume that the luh data start in these same years in the respective files - this is checked below
	int initial_hist_year = 1850;		// this is the initial model year for historical runs - used to get set up dynamic lut files and to calculate index for hurtt data
	int initial_future_year = 2015;		// this is the initial model year for future runs - used to get set up dynamic lut files and to calculate index for hurtt data
	int hist_ref_year = 2000;			// this is the historical run reference year for calculating crop and pft changes
	// future run reference year is currently modyear (the previous year) in the code below
	// so to change it to a constant reference year do it in the code below - but also have to specify a new reference file
	
	outyear = *inyear;
	modyear = outyear - 1;
	printf("outyear in updateannuallanduse = %li, and modyear = %li \n", outyear, modyear);
	
	if (outyear < min_year || outyear > max_year) {
		printf("Invalid Year %li not in range %i - %i\n", outyear, min_year, max_year);
		exit(0);
	}
	
	// the following is for standalone mode only
#ifdef STANDALONE
	
	// the paths below need to match those on the machine this is being run on, and can be changed by the user
	// note that the paths are relative to the folder from which the program is being run

	// create the output path
	strcpy(msg, "mkdir -p ");
	strcat(msg, out_dir);
	system(msg);
	
	// output file base names for updated LUH-PFT data for mksrfdat - the output year and the creation date are appended
	const char out_hist_land_filebase[] = "LUT_LUH2_historical_test";
	const char out_future_land_filebase[] = "LUT_LUH2_SSP5_RCP85_test";
	
	// initial historic dynamic lut file names - so that changing them is done here
	const char initial_hist_dyn_luh_file[] = "./historical_files/iESM_Ref_CropPast1850_c04022014.nc";
	const char initial_hist_dyn_pft_file[] = "./historical_files/surfdata_360x720_mcrop1850_c04022014.nc";
	// initial future dynamic lut file names
	const char initial_future_dyn_luh_file[] = "./future_files/iESM_Ref_CropPast2015_c07252019.nc";
	const char initial_future_dyn_pft_file[] = "./future_files/surfdata_360x720_mcrop2015_c07252019.nc";
	
	// input luh data file names
	const char luh_hist_file[] = "./historical_files/iESM_Expt_rs_Ref_gfrac.nc";		// this is luh2 1850-2015 in luh format
	const char luh_future_file[] = "./future_files/LUH2_SSP5_RCP85_LUH1_format.nc";		// this is luh2 2015-2100 in luh format
	const char luh_harvest_hist_file[] = "./historical_files/iESM_Expt_rs_Ref_harvest_updated.nc";		// this is luh2 harvest 1850-2014 in luh format
	const char luh_harvest_future_file[] = "./future_files/LUH2_SSP5_RCP85_LUH1_format_harvest_updated.nc";		// this is luh2 harvest 2015-2099 in luh format
	// template for writing output files
	const char out_land_template_file[] = "./mksrf_landuse_template.nc";

	// reference files for historic dynamic calculations
	// the dynamic files are used for future calculations with the previous year (modyear) as a reference
	//    but the reference year and file can be changed in the code
	const char luh_hist_ref_file[] = "./historical_files/iESM_Ref_CropPast2000_c03282014.nc";
	const char pft_hist_ref_file[] = "./historical_files/surfdata_360x720_mcrop2000_c03062014.nc";
	// useful files
	const char pot_veg_file[] = "./surfdata_360x720_potveg.nc";
	
	// create the dynamic crop/pasture file and the dynamic pft file for 1850 start
	// label the file with the date
	if (modyear == initial_hist_year) {
		printf("***************\n");
		t = time(NULL);
		tm = localtime(&t);
		strftime(buf,250, "c%m%d%Y", tm);
		// initial dynamic crop/pasture file
		sprintf(dyn_luh_file, "%s/iESM_Dyn_CropPast_%s.nc", out_dir, buf);
		sprintf(msg, "cp -f %s %s", initial_hist_dyn_luh_file, dyn_luh_file);
		system(msg);
		sprintf(msg, "chmod 666 %s", dyn_luh_file);
		system(msg);
		// initial dynamic pft file
		sprintf(dyn_pft_file, "%s/surfdata_360x720_mcrop_dyn_%s.nc", out_dir, buf);
		sprintf(msg, "cp -f %s %s", initial_hist_dyn_pft_file, dyn_pft_file);
		system(msg);
		sprintf(msg, "chmod 666 %s", dyn_pft_file);
		system(msg);
		printf("***************\n");
	}
	// create the dynamic crop/pasture file and the dynamic pft file for 2015 start
	// label the file with the date
	if (modyear == initial_future_year) {
		printf("***************\n");
		t = time(NULL);
		tm = localtime(&t);
		strftime(buf,250, "c%m%d%Y", tm);
		// initial dynamic crop/pasture file
		sprintf(dyn_luh_file, "%s/iESM_Dyn_CropPast_%s.nc", out_dir, buf);
		sprintf(msg, "cp -f %s %s", initial_future_dyn_luh_file, dyn_luh_file);
		system(msg);
		sprintf(msg, "chmod 666 %s", dyn_luh_file);
		system(msg);
		// initial dynamic pft file
		sprintf(dyn_pft_file, "%s/surfdata_360x720_mcrop_dyn_%s.nc", out_dir, buf);
		sprintf(msg, "cp -f %s %s", initial_future_dyn_pft_file, dyn_pft_file);
		system(msg);
		sprintf(msg, "chmod 666 %s", dyn_pft_file);
		system(msg);
		printf("***************\n");
	}
	
	if (modyear < initial_future_year){
		hurttyear = outyear - initial_hist_year;	// use this line for historical simulations
	}
	else {
		hurttyear = outyear - initial_future_year;  // use this line for future simulations
	}
	
	printf("***************\n");
	printf("hurttyear index, modyear: %li %li", hurttyear, modyear);
	printf("\n***************\n");
	
	// contains GOTHR GSECD GCROP GPAST GURBN LANDMASK ...
	if (modyear < initial_future_year){
		strcpy(filenamestr, luh_hist_file);  // LUH2 data in LUH1 format (1850 - 2015) - use for historical simulations
		if (opennetcdf(filenamestr) == 0) {
			printf("LUH file %s is not available; current modyear = %li\n", filenamestr, modyear);
			exit(0);
		}
		// check that the initial luh year matches the respective start year
		getinithurttyear(&hurttinityear);
		if((int) hurttinityear != initial_hist_year) {
			printf("LUH data initial year %li does not match model start year %i\n", hurttinityear, initial_hist_year);
			exit(0);
		}
	}
	else {
		strcpy(filenamestr, luh_future_file);  //LUH2 future scenario in LUH1 format - for future simulations (2015-2100)
		if (opennetcdf(filenamestr) == 0) {
			printf("LUH file %s is not available; current modyear = %li\n", filenamestr, modyear);
			exit(0);
		}
		// check that the initial luh year matches the respective start year
		getinithurttyear(&hurttinityear);
		if((int) hurttinityear != initial_future_year) {
			printf("LUH data initial year %li does not match model start year %i\n", hurttinityear, initial_future_year);
			exit(0);
		}
	}
	
	printf("reading in land use data\n");
	// just set the first argument to the first year of data
	// these functions read the data of the first argument into the inhurttbase arrays
	//   these inhurttbase arrays are overwritten below by the reference year data
	//   readhurttprimary and readhurttsecondary do not even use the first argument
	// the second argument is the outyear of input data to read into the inhurtt arrays
	// these also now put the hurttyear data directly into the glmo array for standalone mode
	hurttinityear = 0;
	readhurttprimary(hurttinityear, hurttyear, ISFUTURE); // inhurttprimary
	readhurttsecondary(hurttinityear, hurttyear, ISFUTURE); // inhurttsecondary
	readhurttcrop(hurttinityear, hurttyear, ISFUTURE); // hurttinityear is set to 0 here, inhurttbasecrop is read using hurttinityear==0, inhurttcrop
	readhurttpasture(hurttinityear, hurttyear, ISFUTURE); // hurttinityear is set to 0 here, inhurttbasepasture is read using hurttinityear==0, inhurttpasture
	
	// write the glmo array to dynamic crop/pasture file
	printf("writing in land use data to dynamic crop/pasture file\n");
	strcpy(filenamestr,dyn_luh_file);
	if (opennetcdf(filenamestr) == 0) {
		printf("Dynamic hurtt pl file %s has not been created; current modyear = %li\n", filenamestr, modyear);
		exit(0);
	}
	
	if (modyear == initial_hist_year || initial_future_year) {
		// set the creation date if this is the first model year
		t = time(NULL);
		tm = localtime(&t);
		nc_put_att_text(innetcdfid, NC_GLOBAL, "creation_date",strlen(asctime(tm))-1, asctime(tm));
	}
	
	/* now write the output year glm crop and pasture and primary and secondary data to the dynamic pl hurtt file */
	/* this function adds a record to the time dimension */
	writehurttdynfile(outyear);
	
	if (closenetcdf(filenamestr) == 0) {
		exit(0);
	}
	
	// contains GFVH1 GFVH2 GFSH1 GFSH2 GFSH3
	if (modyear < initial_future_year){
		strcpy(filenamestr, luh_harvest_hist_file); // LUH2 wood harvest data in LUH1 format (1850 - 2014) - use for historical simulations
		if (opennetcdf(filenamestr) == 0) {
			printf("LUH harvest file %s is not available; current modyear = %li\n", filenamestr, modyear);
			exit(0);
		}
		// check that the initial luh harvest year matches the respective start year
		getinithurttyear(&hurttinityear);
		if((int) hurttinityear != initial_hist_year) {
			printf("LUH harvest data initial year %li does not match model start year %i\n", hurttinityear, initial_hist_year);
			exit(0);
		}
	}
	else {
		strcpy(filenamestr, luh_harvest_future_file);  //LUH2 future wood harvest scenario in LUH1 format - use for future simulations (2015-2099)
		if (opennetcdf(filenamestr) == 0) {
			printf("LUH harvest file %s is not available; current modyear = %li\n", filenamestr, modyear);
			exit(0);
		}
		// check that the initial luh harvest year matches the respective start year
		getinithurttyear(&hurttinityear);
		if((int) hurttinityear != initial_future_year) {
			printf("LUH harvest data initial year %li does not match model start year %i\n", hurttinityear, initial_hist_year);
			exit(0);
		}
	}
	
	// Harvest data is used for model year i.e. hurttyear - 1
	// hurttyear is year of input GLM data
	printf("reading in harvest data\n");
	hurttinityear = 0;
	readhurttvh1(hurttinityear, hurttyear-1, ISFUTURE); // hurttinityear is not used here, inhurttvh1
	readhurttvh2(hurttinityear, hurttyear-1, ISFUTURE); // hurttinityear is not used here, inhurttvh2
	readhurttsh1(hurttinityear, hurttyear-1, ISFUTURE); // hurttinityear is not used here, inhurttsh1
	readhurttsh2(hurttinityear, hurttyear-1, ISFUTURE); // hurttinityear is not used here, inhurttsh2
	readhurttsh3(hurttinityear, hurttyear-1, ISFUTURE); // hurttinityear is not used here, inhurttsh3
	
	if (closenetcdf(filenamestr) == 0) {
		exit(0);
	}
	
#else
	// these names are for iESM
	
	// create the dynamic file names without the time stamp to be consistent with the already created files
	// the initial dynamic files are copies of the initial data files, this is done in the build process; see clm.buildnml.csh

	// initial dynamic crop/pasture file
	sprintf(dyn_luh_file, "./iESM_Dyn_CropPast.nc");
	// initial dynamic pft file
	sprintf(dyn_pft_file, "./surfdata_360x720_mcrop_dyn.nc");
	
	// reference files for historic dynamic calculations
	//    these are the same files as listed above, but with the time stamp removed from the name
	// the dynamic files are used for future calculations with the previous year (modyear) as a reference
	//    but the reference year and file can be changed in the code
	// see notes below about these file names
	const char luh_hist_ref_file[] = "./iESM_Ref_CropPast2000.nc";
	const char pft_hist_ref_file[] = "./surfdata_360x720_mcrop2000.nc";
	// useful files
	const char pot_veg_file[] = "./surfdata_360x720_potveg.nc";
	
	// do this here because it doesn't depend on any processing
	/* now write the output year glm crop and pasture and primary and secondary data to the dynamic pl hurtt file */
	/* this function add a record to the time dimension */
	sprintf(filenamestr, dyn_luh_file);
	if (opennetcdf(filenamestr) == 0) {
		printf("Dynamic hurtt pl file %s has not been created; current modyear = %li\n", filenamestr, modyear);
		exit(0);
	}

	if (modyear == initial_hist_year || initial_future_year) {
		// set the creation date if this is the first model year
		t = time(NULL);
		tm = localtime(&t);
		nc_put_att_text(innetcdfid, NC_GLOBAL, "creation_date",strlen(asctime(tm))-1, asctime(tm));
	}
	
	writehurttdynfile(outyear, glmo);
	
	if (closenetcdf(filenamestr) == 0) {
		exit(0);
	}
	
	/* put the output year glmo land use data into separate arrays, and convert to percent -adv */
	copyglmo(inhurttcrop,0,glmo);
	copyglmo(inhurttpasture,1,glmo);
	copyglmo(inhurttprimary,2,glmo);
	copyglmo(inhurttsecondary,3,glmo);
	/* put the output year glm harvest data into separate arrays, and convert to percent */
	copyglmo(inhurttvh1,4,glmo);
	copyglmo(inhurttvh2,5,glmo);
	copyglmo(inhurttsh1,6,glmo);
	copyglmo(inhurttsh2,7,glmo);
	copyglmo(inhurttsh3,8,glmo);
	
#endif

	// continue for iESM and standalone
	
	/* -adv
	 // read in the glm reference year data
	 //  see clm.buildnml.csh
	 //	only the crop and pasture variables change with time
	 // these data are based on actual glm output
	 // the original file name for 2000 is iESM_Expt1_C_S2_CropPast_Ref.nc - it has been copied to iESM_Ref_CropPast2000_c03282014.nc
	 // and for iESM this file was copied to ./iESM_Ref_CropPast2000.nc
	 // the original file name for 1850 is iESM_Ref_CropPast1850_c08202013.nc, converted to new format: iESM_Ref_CropPast1850_c01302014.nc
	 // the 1850 file was updated again: iESM_Ref_CropPast1850_c04022014.nc
	 // for the fixed reference year 2000 the files above are set in clm.buildnml.csh, so they need to match here
	 // the original file name is changed for the dynamic file by clm.buildnml.csh to remove time stamp so that this code doesn't have to change if the initial file changes
	 */

	// the historical period (model year <2015) needs to use year 2000 as the reference to be consistent with archived runs
	// the future is set up here to use the previous year, starting in model year 2015
	// Note that the output iESM_Dyn_CropPast.nc for previous LUH1 runs has same 2000 data as iESM_Ref_CropPast2000_c03282014.nc
	
	printf("reading reference year land use data\n");
	if (modyear < initial_future_year){
		strcpy(filenamestr, luh_hist_ref_file); // use this line for historical simulations
		
		if (opennetcdf(filenamestr) == 0) {
			printf("Reference hurtt pl file %s is not available; current modyear = %li\n", filenamestr, modyear);
			exit(0);
		}
		readhurttdynpasture(hist_ref_year);	// the reference (model) year pasture - original year is 2000
		readhurttdyncrop(hist_ref_year);		// get the reference (model) year crop amount; not used - original year is 2000
		
		if (closenetcdf(filenamestr) == 0) {
			exit(0);
		}
	}
	else {
		strcpy(filenamestr, dyn_luh_file);  //use this line for future simulations
		if (opennetcdf(filenamestr) == 0) {
			printf("Dynamic hurtt pl file %s has not been created; current modyear = %li\n", filenamestr, modyear);
			exit(0);
		}
		readhurttdynpasture(modyear);	// the reference (model) year pasture
		readhurttdyncrop(modyear);		// get the reference (model) year crop amount
		
		if (closenetcdf(filenamestr) == 0) {
			exit(0);
		}
	}

	/* first need to read the previous year primary and secondary data to normalize the harvest area */
	/* these data go into the prevprimary and prevsecondary arrays */
	/* this could also be normalized by the outyear area, which is the area at the end of the harvest year */
	strcpy(filenamestr, dyn_luh_file);
	if (opennetcdf(filenamestr) == 0) {
		printf("Dynamic hurtt pl file %s has not been created; current modyear = %li\n", filenamestr, modyear);
		exit(0);
	}
	readhurttdynprimary(modyear);
	readhurttdynsecondary(modyear);

	if (closenetcdf(filenamestr) == 0) {                       
		exit(0);
	}
	
    /* -adv
	 read in the reference year clm surface data, including the pfts
	 see clm.biuldnml.csh
	 the original file name for 2000 is surfdata_360x720_mcrop2000.nc
	 - put into new format: surfdata_360x720_mcrop2000_c03062014.nc then copied to ./surfdata_360x720_mcrop2000.nc
	 the original file name for 1850 is surfdata_360x720_mcrop1850_c08202013.nc, converted to new format: surfdata_360x720_mcrop1850_c01312014.nc
	 the 1850 file was updated again: surfdata_360x720_mcrop1850_c04022014.nc
	 the original file name is changed by clm.biuldnml.csh to remove time stamp so that this code doesn't have to change if the initial file changes
	 all other variables are constant, even the four monthly variables, so just read the non-pft variables from the static file
	 */
	/* the historical period (model year <2015) needs to use year 2000 as the reference to be consistent with archived runs */
	/* the previous year reference begins with model year 2015 to generate a consistent future trajectory */
	// get the base year clm surface data, including the pfts
	
	printf("reading reference year pft data\n");
	if (modyear < initial_future_year) {
		strcpy(filenamestr, pft_hist_ref_file); /*original file */
		if (opennetcdf(filenamestr) == 0) {
			printf("Reference pft pl file %s is not available; current modyear = %li\n", filenamestr, modyear);
			exit(0);
		}
		
		/* there is no urban data in the input files -adv */
		/* these values are constants and are used to set up a pftmask; the rest of the non-pft surface file data is not used - adv */
		readlandmask();
		readlandfrac();
		readlakefrac();
		readwetlandfrac();
		readicefrac();
		setvegbarefrac();
		
		/* now read the reference year pft data */
		readcurrentpft();
		readcurrentpftpct(hist_ref_year);
		
		if (closenetcdf(filenamestr) == 0) {
			exit(0);
		}
	}
	else {
		strcpy(filenamestr, dyn_pft_file);
		/* the initial dynamic file is simply a copy of the initial data file, this is done in the build process; see clm.buildnml.csh */
		if (opennetcdf(filenamestr) == 0) {
			printf("Dynamic pft pl file %s has not been created; current modyear = %li\n", filenamestr, modyear);
			exit(0);
		}
		
		/* there is no urban data in the input files -adv */
		/* these values are constants and are used to set up a pftmask; the rest of the non-pft surface file data is not used - adv */
		readlandmask();
		readlandfrac();
		readlakefrac();
		readwetlandfrac();
		readicefrac();
		setvegbarefrac();
		
		/* now read the reference year pft data */
		readcurrentpft();
		readcurrentpftpct(modyear);
		
		if (closenetcdf(filenamestr) == 0) {
			exit(0);
		}
	}
	
	/* !!! normalize the glmo land type data to percent of vegetated land unit to match pft processing -adv */
	/* this has to be done for the base hurtt crop and pasture data also, but only the base pasture data are being used at the moment -adv */
	/* do not normalize the primary and secondary to veg land unit because they are only used in relation to non-normalized harvest fractions,
	 to normalize the harvest fraction to the fraction of available area for harvest
	 alternatively, the harvest data and primary and secondary could be normalized, but this would introduce unnecessary calculations */
	
	// terminal/log output
	printf("writeinhurtt() before normalization to clm vegetated land unit\n");
	writeinhurtt();
	
	normglmo(inhurttcrop);
	normglmo(inhurttpasture);
	normglmo(inhurttbasecrop);
	normglmo(inhurttbasepasture);
	
    /* !!! write these post normalization to the log file also -adv */
	// the harvest data are not normalized by normglmo, they are normalized in sethurttland and stored in the output arrays
    printf("writeinhurtt() after non-harvest normalization to clm vegetated land unit\n");
    writeinhurtt();
    
	/* get the clm potential vegetation pft data -adv */
	strcpy(filenamestr, pot_veg_file);
	
	if (opennetcdf(filenamestr) == 0) {
		exit(0);
	}
	
	readpotvegpft();
	readpotvegpftpct();
	
	if (closenetcdf(filenamestr) == 0) {
		exit(0);
	}
	
	/* this puts the currentpft data into the output year pft array,
	 then adjusts the crops, then the pasture, then calculates harvest fractions
	 these adjustments are done in order so that the pasture adjustments depend somewhat on the crop adjustments
	 the harvest/grazing fractions are separate calculations
	 now this takes the actual model year and out year info as arguments
	 -adv */
	calchurtt((int) modyear, (int) outyear);
	
    // here to the 'also add' comment is for standalone only
#ifdef STANDALONE
	
	printf("write updated land use and harvest data with pfts\n");
	
	t = time(NULL);
	tm = localtime(&t);
	strftime(buf,250, "c%m%d%Y", tm);
	
	// !!!! This is the file to which data is outputted
	if (modyear < initial_future_year) {
		sprintf(filenamestr, "%s/%s_%i_%s.nc", out_dir, out_hist_land_filebase, *inyear, buf);  // use this naming structure for historical simulations
	}
	else {
		sprintf(filenamestr, "%s/%s_%i_%s.nc", out_dir, out_hist_land_filebase, *inyear, buf); // use this naming structure for future simulations
	}
	
	sprintf(msg, "cp -f %s ", out_land_template_file);
	strcat(msg, filenamestr);
	system(msg);
	//sprintf(filenamestr, buf);
	if (opennetcdf(filenamestr) == 0) {
		exit(0);
	}
	
	updatehurttpftpct();
	updatehurttvh1();
	updatehurttvh2();
	updatehurttsh1();
	updatehurttsh2();
	updatehurttsh3();
	updatehurttgrazing();
	
	// use the same time as the creation data above
	nc_put_att_text(innetcdfid, NC_GLOBAL, "creation_date",strlen(asctime(tm))-1, asctime(tm));
	
	if (closenetcdf(filenamestr) == 0) {
		exit(0);
	}
	
#endif
	
	// continue for both iESM and standalone
	
	/* also add the outhurttpftval[inpft][outgrid] values to the dynamic pl pft file */
	/* this function adds a record to the time dimension of PCT_PFT */
	strcpy(filenamestr, dyn_pft_file);
	
	printf("writing dynamic pft file\n");
	
	if (opennetcdf(filenamestr) == 0) {
		exit(0);
	}
	
	if (modyear == initial_hist_year || initial_future_year) {
		// set the creation date if this is the first model year
		t = time(NULL);
		tm = localtime(&t);
		nc_put_att_text(innetcdfid, NC_GLOBAL, "creation_date",strlen(asctime(tm))-1, asctime(tm));
	}
	
	writepftdynfile(outyear);
	
	if (closenetcdf(filenamestr) == 0) {                       
        exit(0);
    }
	
	// now for iESM only
	
#ifndef STANDALONE
	
    /* ---- fill plodata ---*/
    
    /* copy the output year pft data to the shared plodata array -adv */
    copy2plodata(plodata);
    
    writeplodata(plodata);
	
	/* !!! write the two available potential tree masks, but only as a diagnostic when needed -adv */
	if (0) {
		sprintf(filenamestr,"./mask_cropavailtree.dat");
		tempfile = fopen(filenamestr, "wb");
		if (tempfile == NULL) {
			exit(0);
		}
		fwrite(cropavailpotvegtreepftval, sizeof(int), MAXOUTPIX * MAXOUTLIN, tempfile);
		fclose(tempfile);
	
		sprintf(filenamestr,"./mask_pastavailtree.dat");
		tempfile = fopen(filenamestr, "wb");
		if (tempfile == NULL) {
			exit(0);
		}
		fwrite(pastureavailpotvegtreepftval, sizeof(int), MAXOUTPIX * MAXOUTLIN, tempfile);
		fclose(tempfile);
	}
	
#endif

}

#ifdef STANDALONE

// the following function drives the standalone code
// there is one required argument for the time period:
//	'historical' or 'future'
// the second requirement is an optional full output path
//	the default is './output'

int main(int argc, char **argv) {
	
	char out_dir[1000];
	
	if(argc < 2 || argc > 3){
		printf("Usage:\nThere is one requried argument for output file years:\n\thistorical = 1851 to 2015\n\tfuture = 2016 to 2100\nand one optional argument for a full output path:\n\tthe default output path is ./output");
		exit(0);
	}
	
	if(argc == 2){
		strcpy(out_dir, "./output");
		printf("The output path is ./output\n");
	} else {
		strcpy(out_dir, argv[2]);
		printf("The output path is %s\n", argv[2]);
	}
	
    int i=0;
    
    time_t t;
    struct tm* tm;
    
    t = time(NULL);
    tm = localtime(&t);
    fprintf(stdout, "\nProgram started at %s\n", asctime(tm));
    
    // note that these input values are inyear/outyear, which is modelyear+1
	if (!strcmp(argv[1], "historical")){
    	for (i = 1851; i < 2016; i++) { // use this line for historical simulation
			t = time(NULL);
			tm = localtime(&t);
			fprintf(stdout, "\ninyear %i started at %s\n", i, asctime(tm));
		
			updateannuallanduse(&i, 0, out_dir);
		}
	} else if (!strcmp(argv[1], "future")){
    	for (int i = 2016; i < 2101; i++) { // use this line for future simulation
			t = time(NULL);
			tm = localtime(&t);
			fprintf(stdout, "\ninyear %i started at %s\n", i, asctime(tm));
		
			updateannuallanduse(&i, 1, out_dir);
		}
    }
    
    t = time(NULL);
    tm = localtime(&t);
    fprintf(stdout, "\nProgram finished at %s\n", asctime(tm));
    
    return 0;
}

#endif
