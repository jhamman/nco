/* $Header: /data/zender/nco_20150216/nco/src/nco/pck.c,v 1.7 2000-08-31 17:58:20 zender Exp $ */

/* Purpose: NCO utilities for packing and unpacking variables */

/* Copyright (C) 1995--2000 Charlie Zender

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.
   
   As a special exception to the terms of the GPL, you are permitted 
   to link the NCO source code with the NetCDF and HDF libraries 
   and distribute the resulting executables under the terms of the GPL, 
   but in addition obeying the extra stipulations of the netCDF and 
   HDF library licenses.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

   The file LICENSE contains the GNU General Public License, version 2
   It may be viewed interactively by typing, e.g., ncks -L

   The author of this software, Charlie Zender, would like to receive
   your suggestions, improvements, bug-reports, and patches for NCO.
   Please contact the project at http://sourceforge.net/projects/nco or by writing

   Charlie Zender
   Department of Earth System Science
   University of California at Irvine
   Irvine, CA 92697-3100
 */

/* Standard header files */
#include <stdio.h>              /* stderr, FILE, NULL, etc. */
#include <stdlib.h>             /* atof, atoi, malloc, getopt */
#include <string.h>             /* strcmp. . . */
#include <unistd.h>             /* POSIX stuff */

#include <netcdf.h>             /* netCDF definitions */
#include "nc.h"                 /* netCDF operator universal def'ns */
#include <assert.h>             /* assert() debugging macro */

/* Notes on packing/unpacking:
   Routines in this file must be used in correct order:
   pck_dsk_inq(): called first, e.g., in var_fll(), before var_prc copied to var_prc_out
   var_upk(): called in data retrieval routine, e.g., in var_get()
   var_pck(): called just before writing output file, e.g., in main()
*/     

bool /* O [flg] Variable is packed on disk */
pck_dsk_inq /* [fnc] Check whether variable is packed on disk */
(int nc_id, /* I [idx] netCDF file ID */
 var_sct *var) /* I/O [sct] Variable */
{
  /* Purpose: Check whether variable is packed on disk and set variable members 
     pck_dsk, has_scl_fct, has_add_fst, and typ_upk accordingly
     pck_dsk_inq() is mean to be called early in a program, e.g., in var_fll() 
     It is best if pck_dsk_inq() is called before input list is duplicated to output list */
  /* ncea -O -D 3 -v pck ~/nco/data/in.nc ~/nco/data/foo.nc */

  /* From netCDF User's Guide: */
  /* scale_factor: If present for a variable, the data are to be multiplied by this factor after the data are read by the application that accesses the data */
  /* add_offset: If present for a variable, this number is to be added to the data after is is read by the application that acceses the data. If both scale_factor and add_offset attributes are present, the data are first scaled before the offset is added. 
     When scale_factor and add_offset are used for packing, the associated variable (containing the packed data) is typically of type byte or short, whereas the unpacked values are intended to be of type float or double. The attributes scale_factor and add_offset should both be of the type intended for the unpacked data, e.g., float or double. */

  int rcd; /* [rcd] Return success code */

  nc_type scl_fct_typ; /* [idx] Type of scale_factor attribute */
  int scl_fct_lng; /* [idx] Number of elements in scale_factor attribute */
  nc_type add_fst_typ; /* [idx] Type of add_offset attribute */
  int add_fst_lng; /* [idx] Number of elements in add_offset attribute */
  /* Vet scale_factor */
  
  /* netCDF 2.x ncattinq() returns -1 on failure, so compare rcd to -1 */
  ncopts=0; 
  rcd=ncattinq(nc_id,var->id,"scale_factor",&scl_fct_typ,&scl_fct_lng);
  ncopts=NC_VERBOSE | NC_FATAL; 
  /* netCDF 3.x nc_inq_att() returns 0 on success, so compare rcd to NC_NOERR */
  /*  rcd=nc_inq_att(nc_id,var->id,"scale_factor",&scl_fct_typ,(size_t *)&scl_fct_lng);*/
  if(rcd != -1){
    if(scl_fct_typ != NC_FLOAT && scl_fct_typ != NC_DOUBLE){
      (void)fprintf(stderr,"%s: WARNING pck_dsk_inq() reports scale_factor for %s is not NC_FLOAT or NC_DOUBLE. Will not attempt to unpack using scale_factor.\n",prg_nm_get(),var->nm); 
      return False;
    } /* endif */
    if(scl_fct_lng != 1){
      (void)fprintf(stderr,"%s: WARNING pck_dsk_inq() reports %s has scale_factor of length %d. Will not attempt to unpack using scale_factor\n",prg_nm_get(),var->nm,scl_fct_lng); 
      return False;
    } /* endif */
    var->has_scl_fct=True; /* [flg] Valid scale_factor attribute exists */
    var->typ_upk=scl_fct_typ; /* Type of variable when unpacked (expanded) (in memory) */
  } /* endif */

  /* Vet add_offset */
  ncopts=0; 
  rcd=ncattinq(nc_id,var->id,"add_offset",&add_fst_typ,&add_fst_lng);
  ncopts=NC_VERBOSE | NC_FATAL; 
  if(rcd != -1){
    if(add_fst_typ != NC_FLOAT && add_fst_typ != NC_DOUBLE){
      (void)fprintf(stderr,"%s: WARNING pck_dsk_inq() reports add_offset for %s is not NC_FLOAT or NC_DOUBLE. Will not attempt to unpack.\n",prg_nm_get(),var->nm); 
      return False;
    } /* endif */
    if(add_fst_lng != 1){
      (void)fprintf(stderr,"%s: WARNING pck_dsk_inq() reports %s has add_offset of length %d. Will not attempt to unpack.\n",prg_nm_get(),var->nm,add_fst_lng); 
      return False;
    } /* endif */
    var->has_add_fst=True; /* [flg] Valid add_offset attribute exists */
    var->typ_upk=add_fst_typ; /* Type of variable when unpacked (expanded) (in memory) */
  } /* endif */

  if(var->has_scl_fct && var->has_add_fst){
    if(scl_fct_typ != add_fst_typ){
      (void)fprintf(stderr,"%s: WARNING pck_dsk_inq() reports type of scale_factor does not equal type of add_offset. Will not attempt to unpack.\n",prg_nm_get());
      return False;
    } /* endif */
  } /* endif */

  if(var->has_scl_fct || var->has_add_fst){
    /* Variable is considered packed iff either or both valid scale_factor or add_offset exist */
    var->pck_dsk=True; /* [flg] Variable is packed on disk */
    /* If variable is packed on disk and is in memory then variable is packed in memory */
    var->pck_ram=True; /* [flg] Variable is packed in memory */
    var->typ_upk=scl_fct_typ; /* Type of variable when unpacked (expanded) (in memory) */
    if(dbg_lvl_get() > 2) (void)fprintf(stderr,"%s: PACKING Variable %s is type %s packed into type %s\n",prg_nm_get(),var->nm,nco_typ_sng(var->typ_upk),nco_typ_sng(var->typ_dsk));
  } /* endif */

  return var->pck_dsk;
  
} /* end pck_dsk_inq */

var_sct * /* O [sct] Unpacked variable */
var_upk /* [fnc] Unpack variable in memory */
(var_sct *var) /* I/O [sct] Variable to be unpacked */
{
  /* Purpose: Unpack variable
     Routine is inverse of var_pck(): var_upk(var_pck(var))=var */

  var_sct *scl_fct; /* [sct] Variable structure for scale_factor */
  var_sct *add_fst; /* [sct] Variable structure for add_offset */

  /* Return if variable in memory is not currently packed */
  if(!var->pck_ram) return var;

  /* Routine should be called with variable already in memory */
  if(var->val.vp == NULL) (void)fprintf(stdout,"%s: ERROR var_upk() called with empty var->val.vp\n",prg_nm_get());

  /* Packed variables are guaranteed to have both scale_factor and add_offset
     The scale factor is guaranteed to be of type NC_FLOAT or NC_DOUBLE and of size 1 (a scalar) */

  /* Convert scalar values of scale_factor and add_offset into NCO variables */

  if(var->has_scl_fct){ /* [flg] Valid scale_factor attribute exists */
    var->scl_fct.vp=(void *)nco_malloc(nctypelen(var->typ_upk));
    (void)ncattget(var->nc_id,var->id,"scale_factor",var->scl_fct.vp);
    scl_fct=scl_ptr_mk_var(var->scl_fct,var->typ_upk); /* [sct] Variable structure for scale_factor */
    /* Convert var to type of scale_factor for expansion */
    var=var_conform_type(scl_fct->type,var);
    /* Multiply var by scale_factor */
    (void)var_multiply(scl_fct->type,var->sz,var->has_mss_val,var->mss_val,scl_fct->val,var->val);
  } /* endif */

  if(var->has_add_fst){ /* [flg] Valid add_offset attribute exists */
    var->add_fst.vp=(void *)nco_malloc(nctypelen(var->typ_upk));
    (void)ncattget(var->nc_id,var->id,"add_offset",var->add_fst.vp);
    add_fst=scl_ptr_mk_var(var->add_fst,var->typ_upk); /* [sct] Variable structure for add_offset */
    /* Convert var to type of add_offset for expansion */
    if(var->type != add_fst->type) var=var_conform_type(add_fst->type,var);
    /* Add add_offset to var */
    if(var->tally == NULL) (void)fprintf(stdout,"%s: ERROR var->tally==NULL in var_upk(), no room for incrementing tally while in var_add()\n",prg_nm_get());
    (void)var_add(add_fst->type,var->sz,var->has_mss_val,var->mss_val,var->tally,add_fst->val,var->val);
    /* Reset tally buffer to zero for any subsequent arithmetic */
    (void)zero_long(var->sz,var->tally);
  } /* endif */

  if((var->has_scl_fct || var->has_add_fst) && (var->sz == 1L && var->type == NC_DOUBLE)){
    (void)fprintf(stdout,"%s: DEBUG var_upk() reports unpacked var->val.dp[0]=%g\n",prg_nm_get(),var->val.dp[0]);
  } /* endif */

  /* For now, free the packing variables */
  scl_fct=var_free(scl_fct); /* [sct] Variable structure for scale_factor */
  add_fst=var_free(add_fst); /* [sct] Variable structure for add_offset */

  /* Tell the world */  
  var->pck_ram=False;

  (void)fprintf(stderr,"%s: PACKING Unpacked %s\n",prg_nm_get(),var->nm);
  (void)fprintf(stderr,"%s: WARNING Writing unpacked data to disk, or repacking and writing packed data, is not yet supported, output disk values of %s will be incorrect.\n",prg_nm_get(),var->nm);

  return var;
  
} /* end var_upk */

var_sct * /* O [sct] Packed variable */
var_pck /* [fnc] Pack variable in memory */
(var_sct *var, /* I/O [sct] Variable to be packed */
 nc_type typ_pck, /* I [enm] Type of variable when packed (on disk). This should be same as typ_dsk except in cases where variable is packed in input file and unpacked in output file. */
 bool USE_EXISTING_PCK) /* I [flg] Use existing packing scale_factor and add_offset */
{
  /* Purpose: Pack variable 
     Routine is inverse of var_uck(): var_pck(var_upk(var))=var */

  double scl_fct_dbl; /* [sct] Double precision value of scale_factor */
  double add_fst_dbl; /* [sct] Double precision value of add_offset */

  /*  var_sct *scl_fct=NULL; */ /* [sct] Variable structure for scale_factor */
  /* var_sct *add_fst=NULL; */ /* [sct] Variable structure for add_offset */

  /* Return if variable in memory is currently packed */
  if(var->pck_ram) return var;

  /* Routine should be called with variable already in memory */
  if(var->val.vp == NULL) (void)fprintf(stdout,"%s: ERROR var_pck() called with empty var->val.vp\n",prg_nm_get());
  
  /* Packed type must be NC_CHAR or NC_SHORT */
  if(typ_pck != NC_CHAR && typ_pck != NC_SHORT) (void)fprintf(stdout,"%s: ERROR var_pck() called with invalid packed type typ_pck = %s, \n",prg_nm_get(),nco_typ_sng(typ_pck));

  /* Source type must be NC_LONG, NC_FLOAT, or NC_DOUBLE */
  if(var->type == NC_SHORT || var->type == NC_CHAR || var->type == NC_BYTE) (void)fprintf(stdout,"%s: ERROR var_pck() called with invalid source type var->type = %s, \n",prg_nm_get(),nco_typ_sng(var->type));

  if(USE_EXISTING_PCK){
    /* Assume var->scl_fct.vp and var->add_fst.vp are already in memory from pck_dsk_inq() */
  }else{
    /* Compute packing parameters to apply to var */
    double bppv_dbl; /* [frc] Double precision value of bits per packed value */
    double max_mns_min_dbl; /* [frc] Maximum value minus minimum value */

    ptr_unn ptr_unn_min; /* [ptr] Pointer union to minimum value of variable */
    ptr_unn ptr_unn_max; /* [ptr] Pointer union to maximum value of variable */
    
    var_sct *var_min; /* [sct] Minimum value of variable */
    var_sct *var_max; /* [sct] Maximum value of variable */
    var_sct *var_max_dup; /* [sct] Copy of Maximum value of variable */
    var_sct *hlf_var; /* [sct] NCO variable for value 0.5 */
    var_sct *zero_var; /* [sct] NCO variable for value 0.0 */
    var_sct *bppv; /* [sct] NCO variable for number of bits per packed value */
    
    val_unn hlf_unn; /* [frc] Generic container for value 0.5 */
    val_unn zero_unn; /* [frc] Generic container for value 0.0 */
    val_unn bppv_unn; /* [nbr] Generic container for bits per packed value */

    /* Initialize data */
    hlf_unn.d=0.5; /* Generic container for value 0.5 */
    zero_unn.d=0.0; /* Generic container for value 0.0 */

    /* Derive scalar values for scale_factor and add_offset */
    if(var->scl_fct.vp != NULL){(void)free(var->scl_fct.vp); var->scl_fct.vp=NULL;}
    if(var->add_fst.vp != NULL){(void)free(var->add_fst.vp); var->add_fst.vp=NULL;}
    var->scl_fct.vp=(void *)nco_malloc(nctypelen(var->type));
    var->add_fst.vp=(void *)nco_malloc(nctypelen(var->type));
    ptr_unn_min.vp=(void *)nco_malloc(nctypelen(var->type));
    ptr_unn_max.vp=(void *)nco_malloc(nctypelen(var->type));

    /* Find minimum and maximum values in data */
    (void)var_avg_reduce_max(var->type,var->sz,1L,var->has_mss_val,var->mss_val,var->val,ptr_unn_min);
    (void)var_avg_reduce_min(var->type,var->sz,1L,var->has_mss_val,var->mss_val,var->val,ptr_unn_max);

    /* Convert to NC_DOUBLE before 0.5*(min+max) operation */
    var_min=scl_ptr_mk_var(ptr_unn_min,var->type);
    var_min=var_conform_type(NC_DOUBLE,var_min);
    var_max=scl_ptr_mk_var(ptr_unn_max,var->type);
    var_max=var_conform_type(NC_DOUBLE,var_max);
    /* Copy var_max for use in scale_factor computation */
    var_max_dup=var_dup(var_max);
    hlf_var=scl_mk_var(hlf_unn,NC_DOUBLE); /* [sct] NCO variable for value one half */

    /* add_offset is 0.5*(min+max) */
    if(var->tally == NULL) (void)fprintf(stdout,"%s: ERROR var->tally==NULL in var_pck(), no room for incrementing tally while in var_add()\n",prg_nm_get());
    /* var_max->val is overridden with add_offset answers, no longer valid as var_max */
    (void)var_add(NC_DOUBLE,1L,var->has_mss_val,var->mss_val,var->tally,var_min->val,var_max->val);
    (void)var_multiply(NC_DOUBLE,1L,var->has_mss_val,var->mss_val,hlf_var->val,var_max->val);
    /* Reset tally buffer to zero for any subsequent arithmetic */
    (void)zero_long(var->sz,var->tally);
    /* Contents of var_max are actually add_offset */
    (void)val_conform_type(NC_DOUBLE,var_max->val,var->type,var->add_fst);

    /* scale_factor is bppv/(max-min) where
       bppv = bits per packed value and 
       bppv = 256 iff var->typ_pck == NC_CHAR
       bppv = 256*256 iff var->typ_pck == NC_SHORT */
    if(typ_pck == NC_CHAR){
      bppv_dbl=256; /* [sct] Double precision value of bits per packed value */
    }else if(typ_pck == NC_SHORT){
      bppv_dbl=65536; /* [sct] Double precision value of bits per packed value */
    } /* end else */
    bppv_unn.d=bppv_dbl; /* Generic container for bits per packed value */
    bppv=scl_mk_var(bppv_unn,NC_DOUBLE); /* [sct] Variable structure for bits per packed value */

    /* scale_factor = bppv/(max-min)
       add_offset = 0.5*(min+max)
       pck = (upk-add_offset)/scale_factor = bppv*pck ...
       upk = scale_factor*pck + add_offset = (bppv*pck)/(max-min) + 0.5*(min+max) */

    /* scale_factor is bppv/(max-min)
       If max-min = 0 then variable is constant value so scale_factor=0.0 and add_offset=var
       If bppv > (max-min) there is more than 1 bit per packed value (packing is not very lossy)
       If bppv < (max-min) there is less than 1 bit per packed value (packing is lossy) */
    (void)var_subtract(NC_DOUBLE,1L,var->has_mss_val,var->mss_val,var_min->val,var_max_dup->val);
    /* max-min is currently stored in var_max_dup */
    max_mns_min_dbl=ptr_unn_2_scl_dbl(var_max_dup->val,var_max_dup->type); 

    if(max_mns_min_dbl != 0.0){
      (void)var_divide(NC_DOUBLE,1L,var->has_mss_val,var->mss_val,var_max_dup->val,bppv->val);
      /* Contents of bppv are actually scale_factor */
      (void)val_conform_type(NC_DOUBLE,bppv->val,var->type,var->scl_fct);
    }else{
      /* Variable is a constant */
      zero_var=scl_mk_var(zero_unn,var->type); /* [sct] NCO variable for value 0.0 */
      /* Set scale_factor to 0.0 */
      (void)memcpy(var->scl_fct.vp,zero_var->val.vp,nctypelen(var->type));
      if(zero_var != NULL) zero_var=var_free(zero_var);
      /* Set add_offset to variable value */
      (void)memcpy(var->add_fst.vp,var->val.vp,nctypelen(var->type));
    } /* end else */

    /* Free minimum and maximum values */
    if(ptr_unn_min.vp != NULL){(void)free(ptr_unn_min.vp); ptr_unn_min.vp=NULL;}
    if(ptr_unn_max.vp != NULL){(void)free(ptr_unn_max.vp); ptr_unn_max.vp=NULL;}

    /* Free variables */
    if(var_min != NULL) var_min=var_free(var_min);
    if(var_max != NULL) var_max=var_free(var_max);
    if(var_max_dup != NULL) var_max_dup=var_free(var_max_dup);
    if(hlf_var != NULL) hlf_var=var_free(hlf_var);
    if(bppv != NULL) bppv=var_free(bppv);

    /* Do not bother creating superfluous scale_factor (0.0 or 1.0) or add_offset (0.0) */
    scl_fct_dbl=ptr_unn_2_scl_dbl(var->scl_fct,var->type); 
    add_fst_dbl=ptr_unn_2_scl_dbl(var->add_fst,var->type);

    if(scl_fct_dbl != 0.0 && scl_fct_dbl != 1.0) var->has_scl_fct=True; /* [flg] Valid scale_factor attribute exists */
    if(add_fst_dbl != 0.0) var->has_add_fst=True; /* [flg] Valid add_offset attribute exists */

  } /* not USE_EXISTING_PCK */

  /* Create double precision value of scale_factor for diagnostics */
  if(var->has_scl_fct){ /* [flg] Valid scale_factor attribute exists */
    scl_fct_dbl=ptr_unn_2_scl_dbl(var->scl_fct,var->type); 
    if(scl_fct_dbl == 0.0) (void)fprintf(stdout,"%s: WARNING var_pck() reports scl_fct_dbl = 0.0\n",prg_nm_get());
  } /* endif */
  
  /* Create double precision value of add_offset for diagnostics */
  if(var->has_add_fst){ /* [flg] Valid add_offset attribute exists */
    add_fst_dbl=ptr_unn_2_scl_dbl(var->add_fst,var->type);
  } /* endif */
  
  if(dbg_lvl_get() >= 3) (void)fprintf(stdout,"%s: %s: scl_fct_dbl = %g, add_fst_dbl = %g\n",prg_nm_get(),var->nm,scl_fct_dbl,add_fst_dbl);
  
  /* Create NCO variables for scale factor and add_offset
     This is only necessary if packing arithmetic is performed at different precision than var->type
     This would only be the case if NC_LONG were being packed (unlikely)
     Otherwise, just use var->scl_fct and var->add_fst directly */

  /*  scl_fct=scl_ptr_mk_var(var->scl_fct,var->type);*/
  /*  scl_fct=var_conform_type(var->type,scl_fct);*/
  /*add_fst=scl_mk_var(var->add_fst,var->type);*/
  /*  add_fst=var_conform_type(var->type,add_fst);*/

  /* Packing factors now exist and are guaranteed to be of same type as variable in memory */

  /* Apply scale_factor and add_offset to reduce variable size */
  if(var->has_add_fst){ /* [flg] Valid add_offset attribute exists */
    /* Subtract add_offset from var */
    (void)var_subtract(var->type,var->sz,var->has_mss_val,var->mss_val,var->add_fst,var->val);
  } /* endif */

  if(var->has_scl_fct){ /* [flg] Valid scale_factor attribute exists */
    /* Divide var by scale_factor */
    if(scl_fct_dbl != 0.0) (void)var_divide(var->type,var->sz,var->has_mss_val,var->mss_val,var->scl_fct,var->val); 
  } /* endif */

  if((var->has_scl_fct || var->has_add_fst) && (var->sz == 1L && var->type == NC_SHORT)){
    (void)fprintf(stdout,"%s: DEBUG var->val.sp[0]=%d\n",prg_nm_get(),var->val.sp[0]);
  } /* endif */

  /* Convert variable to user-specified packed type */
  var=var_conform_type(typ_pck,var);

  /* Tell the world */
  var->pck_ram=True; /* [flg] Variable is packed in memory */

  /* For now, free the packing variables */
  /*  if(scl_fct != NULL){scl_fct=var_free(scl_fct); scl_fct=NULL;}*/
  /*  if(add_fst != NULL){add_fst=var_free(add_fst); add_fst=NULL;}*/

  (void)fprintf(stderr,"%s: PACKING Packed %s\n",prg_nm_get(),var->nm);
  (void)fprintf(stderr,"%s: WARNING Writing unpacked data to disk, or repacking and writing packed data, is not yet supported, output disk values of %s will be incorrect.\n",prg_nm_get(),var->nm);

  return var;
  
} /* end var_pck */
