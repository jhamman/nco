/* $Header: /data/zender/nco_20150216/nco/src/nco/nco_sld.h,v 1.2 2015-01-21 20:06:40 dywei2 Exp $ */

/* Purpose: Description (definition) of Swath-Like Data (SLD) functions */

/* Copyright (C) 2015--2015 Charlie Zender
   This file is part of NCO, the netCDF Operators. NCO is free software.
   You may redistribute and/or modify NCO under the terms of the 
   GNU General Public License (GPL) Version 3 with exceptions described in the LICENSE file */

/* Usage:
   #include "nco_sld.h" *//* Swath-Like Data */

#ifndef NCO_SLD_H
#define NCO_SLD_H

/* Standard header files */
#include <stdio.h> /* stderr, FILE, NULL, printf */
#include <stdlib.h> /* atof, atoi, malloc, getopt */
#include <string.h> /* strcmp() */



/* 3rd party vendors */
#include <netcdf.h> /* netCDF definitions and C library */
#include "nco_netcdf.h" /* NCO wrappers for netCDF C library */

/* Personal headers */
#include "nco.h" /* netCDF Operator (NCO) definitions */
#include "nco_mmr.h" /* Memory management */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct {
  char *key;
  char *value;
} kvmap_sct;

int hdlscrip(char *scripflnm, kvmap_sct *smps); 
kvmap_sct nco_sng2map(char *str,  kvmap_sct sm); /* parse a line return a name-value pair kvmap */
int nco_sng2array(const char *delim, const char *str, char **sarray); /* split str by delim to sarray returns size of sarray */
char * nco_sng_strip(char *str); /* remove heading and trailing blanks */
void nco_kvmaps_free(kvmap_sct *kvmaps); /* release memory */
void nco_kvmap_prn(kvmap_sct sm);  /* print kvmap contents */
void nco_lsd_set(/* set lsd based user specifications */
 char *const lsd_arg[], /* I [sng] List of user-specified lsd */
 const int cnk_nbr, /* I [nbr] Number of lsd specified */
 trv_tbl_sct * const trv_tbl); /* I/O [sct] Traversal table */

void
trv_tbl_lsd_set_dflt /* Set the lsd value for all non-coordinate vars */
(const int lsd,
 trv_tbl_sct * const trv_tbl);         /* I/O [sct] Traversal table */

void
trv_tbl_lsd_set_var
(const char * const var_nm_fll,        /* I [sng] Variable name to find */
 const int lsd,                        /* I [nbr] Least significant digit */
 trv_tbl_sct * const trv_tbl);         /* I/O [sct] Traversal table */

#ifdef __cplusplus
} /* end extern "C" */
#endif /* __cplusplus */

#endif /* NCO_SLD_H */
