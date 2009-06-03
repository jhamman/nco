/* $Header: /data/zender/nco_20150216/nco/src/nco/nco_cnk.c,v 1.14 2009-06-03 00:02:04 zender Exp $ */

/* Purpose: NCO utilities for chunking */

/* Copyright (C) 1995--2009 Charlie Zender
   License: GNU General Public License (GPL) Version 3
   See http://www.gnu.org/copyleft/gpl.html for full license text */

/* Usage:
   ncks -O -4 -D 4 --cnk_scl=8 ~/nco/data/in.nc ~/foo.nc
   ncks -O -4 -D 4 --cnk_scl=8 ${DATA}/dstmch90/dstmch90_clm.nc ~/foo.nc
   ncks -O -4 -D 4 --cnk_dmn time,10 ~/nco/data/in.nc ~/foo.nc
   ncks -O -4 -D 4 --cnk_dmn lat,64 --cnk_dmn lon,128 ${DATA}/dstmch90/dstmch90_clm.nc ~/foo.nc 
   ncks -O -4 -D 4 --cnk_plc=uck ~/foo.nc ~/foo.nc
   ncks -O -4 -D 4 --cnk_plc=g2d --cnk_map=rd1 --cnk_dmn lat,64 --cnk_dmn lon,128 ${DATA}/dstmch90/dstmch90_clm.nc ~/foo.nc

   This couplet chunks data then unchunks it back to its original state:
   ncks -O -4 -D 4 --cnk_plc=all ~/nco/data/in.nc ~/foo.nc
   ncks -O -4 -D 4 --cnk_plc=uck ~/foo.nc ~/foo.nc

   ncecat testing:
   ncecat -O -4 -D 4 --cnk_plc=all -p ~/nco/data in.nc in.nc ~/foo.nc
   ncecat -O -4 -D 4 --cnk_plc=g2d --cnk_map=rd1 --cnk_dmn lat,64 --cnk_dmn lon,128 -p ${DATA}/dstmch90 dstmch90_clm.nc dstmch90_clm.nc ~/foo.nc */

#include "nco_cnk.h" /* Chunking */

const char * /* O [sng] Chunking map string */
nco_cnk_map_sng_get /* [fnc] Convert chunking map enum to string */
(const int nco_cnk_map) /* I [enm] Chunking map */
{
  /* Purpose: Convert chunking map enum to string */
  switch(nco_cnk_map){
  case nco_cnk_map_nil:
    return "nil";
  case nco_cnk_map_dmn:
    return "dmn";
  case nco_cnk_map_rd1:
    return "rd1";
  case nco_cnk_map_scl:
    return "scl";
  case nco_cnk_map_prd:
    return "prd";
  default: nco_dfl_case_cnk_map_err(); break;
  } /* end switch */
  /* Some compilers, e.g., SGI cc, need return statement to end non-void functions */
  return (char *)NULL;
} /* end nco_cnk_map_sng_get() */

const char * /* O [sng] Chunking policy string */
nco_cnk_plc_sng_get /* [fnc] Convert chunking policy enum to string */
(const int nco_cnk_plc) /* I [enm] Chunking policy */
{
  /* Purpose: Convert chunking policy enum to string */
  switch(nco_cnk_plc){
  case nco_cnk_plc_nil:
    return "nil";
  case nco_cnk_plc_all:
    return "all";
  case nco_cnk_plc_g2d:
    return "g2d";
  case nco_cnk_plc_g3d: 
    return "g3d";
  case nco_cnk_plc_xpl: 
    return "xpl";
  case nco_cnk_plc_uck:
    return "uck";
  default: nco_dfl_case_cnk_plc_err(); break;
  } /* end switch */
  /* Some compilers, e.g., SGI cc, need return statement to end non-void functions */
  return (char *)NULL;
} /* end nco_cnk_plc_sng_get() */

void 
nco_dfl_case_cnk_map_err(void) /* [fnc] Print error and exit for illegal switch(cnk_map) case */
{
  /* Purpose: Convenience routine for printing error and exiting when
     switch(cnk_map) statement receives an illegal default case

     Placing this in its own routine also has the virtue of saving many lines 
     of code since this function is used in many many switch() statements. */
  const char fnc_nm[]="nco_dfl_case_cnk_map_err()";
  (void)fprintf(stdout,"%s: ERROR switch(cnk_map) statement fell through to default case, which is unsafe. This catch-all error handler ensures all switch(cnk_map) statements are fully enumerated. Exiting...\n",fnc_nm);
  nco_err_exit(0,fnc_nm);
} /* end nco_dfl_case_cnk_map_err() */

void 
nco_dfl_case_cnk_plc_err(void) /* [fnc] Print error and exit for illegal switch(cnk_plc) case */
{
  /* Purpose: Convenience routine for printing error and exiting when
     switch(cnk_plc) statement receives an illegal default case

     Placing this in its own routine also has the virtue of saving many lines 
     of code since this function is used in many many switch() statements. */
  const char fnc_nm[]="nco_dfl_case_cnk_plc_err()";
  (void)fprintf(stdout,"%s: ERROR switch(cnk_plc) statement fell through to default case, which is unsafe. This catch-all error handler ensures all switch(cnk_plc) statements are fully enumerated. Exiting...\n",fnc_nm);
  nco_err_exit(0,fnc_nm);
} /* end nco_dfl_case_cnk_plc_err() */

cnk_sct ** /* O [sct] Structure list with user-specified chunking information */
nco_cnk_prs /* [fnc] Create chunking structures with name and chunksize elements */
(const int cnk_nbr, /* I [nbr] Number of chunksizes specified */
 CST_X_PTR_CST_PTR_CST_Y(char,cnk_arg)) /* I [sng] List of user-specified chunksizes */
{
  /* Purpose: Determine name and chunksize elements from user arguments
     Routine merely evaluates syntax of input expressions and does not 
     attempt to validate dimensions or chunksizes against input file.
     Routine based on nco_lmt_prs() */
  
  /* Valid syntax adheres to nm,cnk_sz */
  
  void nco_usg_prn(void);
  
  char **arg_lst;
  
  const char dlm_sng[]=",";
  
  cnk_sct **cnk=NULL_CEWI;
  
  int idx;
  int arg_nbr;
  
  if(cnk_nbr > 0) cnk=(cnk_sct **)nco_malloc(cnk_nbr*sizeof(cnk_sct *));
  for(idx=0;idx<cnk_nbr;idx++){
    /* Process chunksize specifications as normal text list */
    /* fxm: probably need to free arg_lst sometime... */
    arg_lst=nco_lst_prs_2D(cnk_arg[idx],dlm_sng,&arg_nbr);
    
    /* Check syntax */
    if(
       arg_nbr < 2 || /* Need more than just dimension name */
       arg_nbr > 2 || /* Too much information */
       arg_lst[0] == NULL || /* Dimension name not specified */
       False){
      (void)fprintf(stdout,"%s: ERROR in chunksize specification for dimension %s\n%s: HINT Conform request to chunksize documentation at http://nco.sf.net/nco.html#cnk\n",prg_nm_get(),cnk_arg[idx],prg_nm_get());
      nco_exit(EXIT_FAILURE);
    } /* end if */
    
    /* Initialize structure */
    /* cnk strings that are not explicitly set by user remain NULL, i.e., 
       specifying default setting will appear as if nothing at all was set.
       Hopefully, in routines that follow, branch followed when dimension has
       all default settings specified (e.g.,"-d foo,,,,") yields same answer
       as branch for which no hyperslab along that dimension was set. */
    cnk[idx]=(cnk_sct *)nco_malloc(sizeof(cnk_sct));
    cnk[idx]->nm=NULL;
    cnk[idx]->is_usr_spc_cnk=True; /* True if any part of limit is user-specified, else False */
    
    /* Fill in structure */
    cnk[idx]->nm=arg_lst[0];
    cnk[idx]->sz=strtoul(arg_lst[1],(char **)NULL,10);
    
    /* Free current pointer array to strings
       Strings themselves are untouched and will be free()'d with chunk structures 
       in nco_cnk_lst_free() */
    arg_lst=(char **)nco_free(arg_lst);
  } /* end loop over cnk structure list */
  
  return cnk;
} /* end nco_cnk_prs() */

cnk_sct ** /* O [sct] Pointer to free'd structure list */
nco_cnk_lst_free /* [fnc] Free memory associated with chunking structure list */
(cnk_sct **cnk_lst, /* I/O [sct] Chunking structure list to free */
 const int cnk_nbr) /* I [nbr] Number of chunking structures in list */
{
  /* Threads: Routine is thread safe and calls no unsafe routines */
  /* Purpose: Free all memory associated with dynamically allocated chunking structure list */
  int idx;

  for(idx=0;idx<cnk_nbr;idx++){
    cnk_lst[idx]=nco_cnk_free(cnk_lst[idx]);
  } /* end loop over idx */

  /* Free structure pointer last */
  cnk_lst=(cnk_sct **)nco_free(cnk_lst);

  return cnk_lst;
} /* end nco_cnk_lst_free() */

cnk_sct * /* O [sct] Pointer to free'd chunking structure */
nco_cnk_free /* [fnc] Free all memory associated with chunking structure */
(cnk_sct *cnk) /* I/O [sct] Chunking structure to free */
{
  /* Threads: Routine is thread safe and calls no unsafe routines */
  /* Purpose: Free all memory associated with a dynamically allocated chunking structure */
  cnk->nm=(char *)nco_free(cnk->nm);
  /* Free structure pointer last */
  cnk=(cnk_sct *)nco_free(cnk);

  return NULL;
} /* end nco_cnk_free() */

void
nco_cnk_sz_set /* [fnc] Set chunksize parameters */
(const int nc_id, /* I [id] netCDF file ID */
 CST_X_PTR_CST_PTR_CST_Y(lmt_all_sct,lmt_all_lst), /* I [sct] Hyperslab limits */
 const int lmt_all_lst_nbr, /* I [nbr] Number of hyperslab limits */
 int * const cnk_map_ptr, /* I/O [enm] Chunking map */
 int * const cnk_plc_ptr, /* I/O [enm] Chunking policy */
 const size_t cnk_sz_scl, /* I [nbr] Chunk size scalar */
 CST_X_PTR_CST_PTR_CST_Y(cnk_sct,cnk), /* I [sct] Chunking information */
 const int cnk_nbr) /* I [nbr] Number of dimensions with user-specified chunking */
{
  /* Purpose: Use chunking map and policy to determine chunksize list */
  const char fnc_nm[]="nco_cnk_sz_set()"; /* [sng] Function name */

  char dmn_nm[NC_MAX_NAME];
  char var_nm[NC_MAX_NAME];

  int *dmn_id;

  int cnk_idx;
  int dmn_idx;
  int cnk_map; /* [enm] Chunking map */
  int cnk_plc; /* [enm] Chunking policy */
  int dmn_nbr; /* [nbr] Number of dimensions in variable */
  int fl_fmt; /* [enm] Input file format */
  int lmt_idx;
  int nbr_dmn_fl; /* [nbr] Number of dimensions in file */
  int rcd_dmn_id;
  int srg_typ; /* [enm] Storage type */
  int var_idx;
  int var_nbr;

  long dmn_sz;

  nco_bool flg_cnk=False; /* [flg] Chunking requested */

  nc_type var_typ_dsk;
  
  size_t *cnk_sz; /* [nbr] Chunksize list */
  size_t cnk_sz_dfl; /* [nbr] Chunksize default */

  /* Did user explicitly request chunking? */
  if(cnk_nbr > 0 || cnk_sz_scl > 0UL || *cnk_map_ptr != nco_cnk_map_nil || *cnk_plc_ptr != nco_cnk_plc_nil) flg_cnk=True;

  if(!flg_cnk) return;

  /* Set actual chunk policy and map to defaults as necessary
     This rather arcane procedure saves a few lines of code in calling program
     (because defaults not set there) while maintaining correctness of arguments */
  if(*cnk_map_ptr == nco_cnk_map_nil) *cnk_map_ptr=nco_cnk_map_get((char *)NULL);
  if(*cnk_plc_ptr == nco_cnk_plc_nil) *cnk_plc_ptr=nco_cnk_plc_get((char *)NULL);
  cnk_map=*cnk_map_ptr;
  cnk_plc=*cnk_plc_ptr;

  /* Bail on unsupported options */
  if(cnk_plc == nco_cnk_plc_xpl){
    (void)fprintf(stderr,"%s: ERROR cnk_plc = %s not yet supported\n",prg_nm_get(),nco_cnk_plc_sng_get(cnk_plc));
    nco_exit(EXIT_FAILURE);
  } /* endif */

  /* Does output file support chunking? */
  (void)nco_inq_format(nc_id,&fl_fmt);
  if(fl_fmt != NC_FORMAT_NETCDF4){
    (void)fprintf(stderr,"%s: WARNING Output file format is %s so chunking request will be ignored\n",prg_nm_get(),nco_fmt_sng(fl_fmt));
    return;
  } /* endif dbg */

  /* Vet input */
  if(cnk_map == nco_cnk_map_scl && cnk_sz_scl <= 0){
    (void)fprintf(stderr,"%s: ERROR cnk_sz_scl = %lu must be greater than 0\n",prg_nm_get(),(unsigned long)cnk_sz_scl);
    nco_exit(EXIT_FAILURE);
  } /* endif cnk_sz_scl */

  if(dbg_lvl_get() >= nco_dbg_fl) (void)fprintf(stderr,"%s: INFO Requested chunking or unchunking\n",prg_nm_get());
  if(dbg_lvl_get() >= nco_dbg_scl){
    (void)fprintf(stderr,"cnk_plc: %s\n",nco_cnk_plc_sng_get(cnk_plc));
    (void)fprintf(stderr,"cnk_map: %s\n",nco_cnk_map_sng_get(cnk_map));
    (void)fprintf(stderr,"cnk_sz_scl: %lu\n",(unsigned long)cnk_sz_scl);
    if(cnk_nbr > 0){
      (void)fprintf(stderr,"idx dmn_nm\tcnk_sz:\n");
      for(cnk_idx=0;cnk_idx<cnk_nbr;cnk_idx++) (void)fprintf(stderr,"%2d %s\t%lu\n",cnk_idx,cnk[cnk_idx]->nm,(unsigned long)cnk[cnk_idx]->sz);
    } /* cnk_nbr == 0 */
  } /* endif dbg */

  /* Get record dimension ID */
  (void)nco_inq(nc_id,&nbr_dmn_fl,&var_nbr,(int *)NULL,&rcd_dmn_id);
  
  /* NB: Assumes variable IDs range from [0..var_nbr-1] */
  for(var_idx=0;var_idx<var_nbr;var_idx++){

    /* Initialize storage type for this variable */
    srg_typ=NC_CONTIGUOUS; /* [enm] Storage type */
    cnk_sz=NULL; /* [nbr] Chunksize list */

    /* Get type and number of dimensions for variable */
    (void)nco_inq_var(nc_id,var_idx,var_nm,&var_typ_dsk,&dmn_nbr,(int *)NULL,(int *)NULL);
    
    if(dmn_nbr == 0) continue; /* Skip chunking calls for scalars */

    /* Explicitly turn off chunking for arrays that are... */
    if((cnk_plc == nco_cnk_plc_g2d && dmn_nbr < 2) || /* ...much too small... */
       (cnk_plc == nco_cnk_plc_g3d && dmn_nbr < 3) || /* ...too small... */
       (cnk_plc == nco_cnk_plc_uck) || /* ...intentionally unchunked... */
       False){
      if(dbg_lvl_get() > nco_dbg_scl) (void)fprintf(stderr,"%s: INFO %s turning off chunking for %s\n",prg_nm_get(),fnc_nm,var_nm);

      /* Leave chunking on for coordinates... */

      /* Turn chunking off for this variable */
      (void)nco_def_var_chunking(nc_id,var_idx,srg_typ,cnk_sz);
      /* Skip to next variable in loop */
      continue;
    } /* end if */

    /* Variable will definitely be chunked */
    srg_typ=NC_CHUNKED; /* [enm] Storage type */

    /* Allocate space to hold dimension IDs */
    dmn_id=(int *)nco_malloc(dmn_nbr*sizeof(int));
    /* Allocate space to hold chunksizes */
    cnk_sz=(size_t *)nco_malloc(dmn_nbr*sizeof(size_t));
    
    /* Get dimension IDs */
    (void)nco_inq_vardimid(nc_id,var_idx,dmn_id);
    
    /* Default "equal" chunksize for each dimension */
    cnk_sz_dfl=cnk_sz_scl;
    if(cnk_map == nco_cnk_map_prd){
      double cnk_sz_prd_dbl; /* [nbr] Chunksize product, double precision */
      double cnk_sz_eql_dbl; /* [nbr] Chunksize equal, double precision */
      double cnk_sz_dfl_dbl; /* [nbr] Chunksize default, double precision */
      cnk_sz_prd_dbl=cnk_sz_scl;
      cnk_sz_eql_dbl=pow(cnk_sz_prd_dbl,1.0/dmn_nbr);
      cnk_sz_dfl_dbl=ceil(cnk_sz_eql_dbl);
      cnk_sz_dfl=(size_t)cnk_sz_dfl_dbl;
    } /* endif map_prd */

    for(dmn_idx=0;dmn_idx<dmn_nbr;dmn_idx++){

      /* Get dimension name and size */
      (void)nco_inq_dim(nc_id,dmn_id[dmn_idx],dmn_nm,&dmn_sz);
      
      /* Is this a record dimension? */
      if(dmn_id[dmn_idx] == rcd_dmn_id){
	/* Does policy specify record dimension treatment? */
	if(cnk_map == nco_cnk_map_rd1){
	  cnk_sz[dmn_idx]=1UL;
	  /* Skip to next dimension in loop */
	  continue;
	} /* !nco_cnk_map_rd1 */
	/* Record dimension size in output file is zero until first write
	   Obtain record dimension size from lmt_all structure */
	for(lmt_idx=0;lmt_idx<lmt_all_lst_nbr;lmt_idx++){
	  if(!strcmp(dmn_nm,lmt_all_lst[lmt_idx]->dmn_nm)){
	    cnk_sz[dmn_idx]=lmt_all_lst[lmt_idx]->dmn_cnt;
	    break;
	  } /* end if */
	} /* end loop over limit */
	if(dmn_sz == 0L){
	  (void)fprintf(stderr,"%s: ERROR %s reports variable %s has dim_sz == 0L for dimension %s. This should not occur and it will cause chunking to fail...\n",prg_nm_get(),fnc_nm,var_nm,dmn_nm);
	} /* endif err */
      } /* endif record dimension */

      /* Non-record sizes default to cnk_sz_dfl or to dimension size */
      if(cnk_sz_dfl > 0UL){
	/* Propagate scalar chunksize, if specified */
	cnk_sz[dmn_idx]=(cnk_sz_dfl <= (size_t)dmn_sz) ? cnk_sz_dfl : (size_t)dmn_sz;
      }else{
	/* Dimensions not in user-specified chunksize list get default */
	cnk_sz[dmn_idx]=dmn_sz;
      } /* !cnk_sz_dfl */

      /* Explicit chunk specifications override all else */
      for(cnk_idx=0;cnk_idx<cnk_nbr;cnk_idx++){
	/* Match on name not ID */
	if(!strcmp(cnk[cnk_idx]->nm,dmn_nm)){
	  if(cnk[cnk_idx]->sz > (size_t)dmn_sz) (void)fprintf(stderr,"%s: WARNING %s trimming user-specified chunksize = %lu to %s size = %lu\n",prg_nm_get(),fnc_nm,(unsigned long)cnk[cnk_idx]->sz,dmn_nm,dmn_sz);
	  /* Out-of-bounds sizes would fail in HDF library in nc_enddef(), so trim */
	  cnk_sz[dmn_idx]=(size_t)dmn_sz;
	  break;
	} /* end if */
      } /* end loop over cnk */

    } /* end loop over dmn */
    
    if(dbg_lvl_get() >= nco_dbg_scl){
      (void)fprintf(stderr,"idx nm\tdmn_sz\tcnk_sz for %s:\n",var_nm);
      for(dmn_idx=0;dmn_idx<dmn_nbr;dmn_idx++){
	(void)nco_inq_dimlen(nc_id,dmn_id[dmn_idx],&dmn_sz);
	(void)nco_inq_dimname(nc_id,dmn_id[dmn_idx],dmn_nm);
	(void)fprintf(stderr,"%2d %s\t%lu\t%lu\n",dmn_idx,dmn_nm,dmn_sz,(unsigned long)cnk_sz[dmn_idx]);
      } /* end loop over dmn */
    } /* endif dbg */

    /* Turn chunking on for this variable */
    (void)nco_def_var_chunking(nc_id,var_idx,srg_typ,cnk_sz);
    
    /* Free space holding dimension IDs and chunksizes */
    dmn_id=(int *)nco_free(dmn_id);
    cnk_sz=(size_t *)nco_free(cnk_sz);

  } /* end loop over var */
  
  return;
} /* end nco_cnk_sz_set() */

int /* O [enm] Chunking map */
nco_cnk_map_get /* [fnc] Convert user-specified chunking map to key */
(const char *nco_cnk_map_sng) /* [sng] User-specified chunking map */
{
  /* Purpose: Process ncpdq '-P' command line argument
     Convert user-specified string to chunking map
     Return nco_cnk_map_nil by default */
  const char fnc_nm[]="nco_cnk_map_get()"; /* [sng] Function name */
  char *prg_nm; /* [sng] Program name */
  prg_nm=prg_nm_get(); /* [sng] Program name */

  if(nco_cnk_map_sng == NULL){
    if(dbg_lvl_get() >= nco_dbg_std) (void)fprintf(stdout,"%s: INFO %s reports %s invoked without explicit chunking map. Defaulting to chunking map \"rd1\".\n",prg_nm,fnc_nm,prg_nm);
    return nco_cnk_map_rd1;
  } /* endif */

  if(!strcmp(nco_cnk_map_sng,"nil")) return nco_cnk_map_nil;
  if(!strcmp(nco_cnk_map_sng,"cnk_map_nil")) return nco_cnk_map_nil;
  if(!strcmp(nco_cnk_map_sng,"dmn")) return nco_cnk_map_dmn;
  if(!strcmp(nco_cnk_map_sng,"cnk_map_dmn")) return nco_cnk_map_dmn;
  if(!strcmp(nco_cnk_map_sng,"rd1")) return nco_cnk_map_rd1;
  if(!strcmp(nco_cnk_map_sng,"cnk_map_rd1")) return nco_cnk_map_rd1;
  if(!strcmp(nco_cnk_map_sng,"scl")) return nco_cnk_map_scl;
  if(!strcmp(nco_cnk_map_sng,"cnk_map_scl")) return nco_cnk_map_scl;
  if(!strcmp(nco_cnk_map_sng,"prd")) return nco_cnk_map_prd;
  if(!strcmp(nco_cnk_map_sng,"cnk_map_prd")) return nco_cnk_map_prd;

  (void)fprintf(stderr,"%s: ERROR %s reports unknown user-specified chunking map %s\n",prg_nm_get(),fnc_nm,nco_cnk_map_sng);
  nco_exit(EXIT_FAILURE);
  return nco_cnk_map_nil; /* Statement should not be reached */
} /* end nco_cnk_map_get() */

int /* O [enm] Chunking policy */
nco_cnk_plc_get /* [fnc] Convert user-specified chunking policy to key */
(const char *nco_cnk_plc_sng) /* [sng] User-specified chunking policy */
{
  /* Purpose: Process ncpdq '-P' command line argument
     Convert user-specified string to chunking operation type 
     Return nco_cnk_plc_g2d by default */
  const char fnc_nm[]="nco_cnk_plc_get()"; /* [sng] Function name */
  char *prg_nm; /* [sng] Program name */
  prg_nm=prg_nm_get(); /* [sng] Program name */

  if(nco_cnk_plc_sng == NULL){
    if(dbg_lvl_get() >= nco_dbg_std) (void)fprintf(stdout,"%s: INFO %s reports %s invoked without explicit chunking policy. Defaulting to chunking policy \"g2d\".\n",prg_nm,fnc_nm,prg_nm);
    return nco_cnk_plc_g2d;
  } /* endif */

  if(!strcmp(nco_cnk_plc_sng,"nil")) return nco_cnk_plc_nil;
  if(!strcmp(nco_cnk_plc_sng,"cnk_nil")) return nco_cnk_plc_nil;
  if(!strcmp(nco_cnk_plc_sng,"plc_nil")) return nco_cnk_plc_nil;
  if(!strcmp(nco_cnk_plc_sng,"all")) return nco_cnk_plc_all;
  if(!strcmp(nco_cnk_plc_sng,"cnk_all")) return nco_cnk_plc_all;
  if(!strcmp(nco_cnk_plc_sng,"plc_all")) return nco_cnk_plc_all;
  if(!strcmp(nco_cnk_plc_sng,"g2d")) return nco_cnk_plc_g2d;
  if(!strcmp(nco_cnk_plc_sng,"cnk_g2d")) return nco_cnk_plc_g2d;
  if(!strcmp(nco_cnk_plc_sng,"plc_g2d")) return nco_cnk_plc_g2d;
  if(!strcmp(nco_cnk_plc_sng,"g3d")) return nco_cnk_plc_g3d;
  if(!strcmp(nco_cnk_plc_sng,"cnk_g3d")) return nco_cnk_plc_g3d;
  if(!strcmp(nco_cnk_plc_sng,"plc_g3d")) return nco_cnk_plc_g3d;
  if(!strcmp(nco_cnk_plc_sng,"xpl")) return nco_cnk_plc_xpl;
  if(!strcmp(nco_cnk_plc_sng,"cnk_xpl")) return nco_cnk_plc_xpl;
  if(!strcmp(nco_cnk_plc_sng,"plc_xpl")) return nco_cnk_plc_xpl;
  if(!strcmp(nco_cnk_plc_sng,"uck")) return nco_cnk_plc_uck;
  if(!strcmp(nco_cnk_plc_sng,"cnk_uck")) return nco_cnk_plc_uck;
  if(!strcmp(nco_cnk_plc_sng,"plc_uck")) return nco_cnk_plc_uck;
  if(!strcmp(nco_cnk_plc_sng,"unchunk")) return nco_cnk_plc_uck;

  (void)fprintf(stderr,"%s: ERROR %s reports unknown user-specified chunking policy %s\n",prg_nm_get(),fnc_nm,nco_cnk_plc_sng);
  nco_exit(EXIT_FAILURE);
  return nco_cnk_plc_nil; /* Statement should not be reached */
} /* end nco_cnk_plc_get() */

#if 0
/* NB: Following routines are placeholders, currently not used */
size_t * /* O [nbr] Chunksize array for variable */
nco_cnk_sz_get /* [fnc] Determine chunksize array */
(const int nc_id, /* I [id] netCDF file ID */
 const char * const var_nm, /* I [sng] Variable name */
 const int cnk_map, /* I [enm] Chunking map */
 const int cnk_plc, /* I [enm] Chunking policy */
 const size_t cnk_sz_scl, /* I [nbr] Chunk size scalar */
 CST_X_PTR_CST_PTR_CST_Y(cnk_sct,cnk), /* I [sct] Chunking information */
 const int cnk_nbr) /* I [nbr] Number of dimensions with user-specified chunking */
{
  /* Purpose: Use chunking map and policy to determine chunksize list */
  int dmn_nbr; /* [nbr] Number of dimensions in variable */
  int dmn_idx;
  int idx;
  int rec_dmn_id;
  
  size_t *cnk_sz; /* [nbr] Chunksize list */
  
  /* Get record dimension ID */
  (void)nco_inq(nc_id,(int *)NULL,(int *)NULL,(int *)NULL,&rec_dmn_id);

  /* Get type and number of dimensions and attributes for variable */

  cnk_sz=(size_t *)nco_malloc(dmn_nbr*sizeof(size_t));
  
  return cnk_sz;
} /* end nco_cnk_sz_get() */

nco_bool /* O [flg] NCO will attempt to chunk variable */
nco_is_chunkable /* [fnc] Will NCO attempt to chunk variable? */
(const nc_type nc_typ_in) /* I [enm] Type of input variable */
{
  /* Purpose: Determine whether NCO should attempt to chunk a given type
     Chunking certain variable types is not recommended, e.g., chunking NC_CHAR
     and NC_BYTE makes no sense, because precision would needlessly be lost.
     Routine should be consistent with nco_cnk_plc_typ_get()
     NB: Routine is deprecated in favor of more flexible nco_cnk_plc_typ_get() */
  const char fnc_nm[]="nco_is_chunkable()"; /* [sng] Function name */

  (void)fprintf(stdout,"%s: ERROR deprecated routine %s should not be called\n",prg_nm_get(),fnc_nm);
  nco_exit(EXIT_FAILURE);

  switch(nc_typ_in){ 
  case NC_FLOAT: 
  case NC_DOUBLE: 
  case NC_INT64: 
  case NC_UINT64: 
  case NC_INT: 
  case NC_UINT: 
    return True;
    break;
  case NC_SHORT: 
  case NC_USHORT: 
  case NC_CHAR: 
  case NC_BYTE: 
  case NC_UBYTE: 
  case NC_STRING:
    return False;
    break;
  default: nco_dfl_case_nc_type_err(); break;
  } /* end switch */ 

  /* Some compilers, e.g., SGI cc, need return statement to end non-void functions */
  return False;
} /* end nco_is_chunkable() */

nco_bool /* O [flg] Variable is chunked on disk */
nco_cnk_dsk_inq /* [fnc] Check whether variable is chunked on disk */
(const int nc_id, /* I [idx] netCDF file ID */
 var_sct * const var) /* I/O [sct] Variable */
{
  /* Purpose: Check whether variable is chunked on disk and set variable members 
     cnk_dsk and cnk_sz accordingly */
  /* ncea -O -D 3 -v cnk ~/nco/data/in.nc ~/nco/data/foo.nc */
  
  int rcd; /* [rcd] Return success code */
  int srg_typ; /* [enm] Storage type */
  
  /* Set some defaults in variable structure for safety in case of early return
     Flags for variables without valid scaling information should appear 
     same as flags for variables with _no_ scaling information
     Set has_scl_fct, has_add_fst in var_dfl_set()
     typ_uck:
     1. is required by ncra nco_cnv_mss_val_typ() 
     2. depends on var->type and so should not be set in var_dfl_set()
     3. is therefore set to default here */
  var->cnk_dsk=False; /* [enm] Variable is chunked on disk */

  rcd=nco_inq_var_chunking(nc_id,var->id,&srg_typ,var->cnk_sz);

  return var->cnk_dsk; /* [flg] Variable is chunked on disk (valid scale_factor, add_offset, or both attributes exist) */
  
} /* end nco_cnk_dsk_inq() */

#endif /* endif 0 */
