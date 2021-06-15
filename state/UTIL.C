/*
 *
 * util.c
 * Friday, 6/27/1997.
 *
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <GnuType.h>
#include <GnuMem.h>
#include <GnuScr.h>
#include <GnuMisc.h>
#include <GnuKbd.h>
#include <GnuArg.h>
#include <GnuFile.h>
#include <GnuStr.h>
#include <GnuCfg.h>
#include "CRITTERS.H"


void LogInit (void)
   {
   fpLOG = fopen (szLOGFILE, "wt");
   }


UINT CCONV Log (PSZ psz, ...)
   {
   va_list vlst;

   va_start (vlst, psz);
   vfprintf (fpLOG, psz, vlst);
   va_end (vlst);
   fprintf (fpLOG, "\n");
   fflush (fpLOG);
   return 0;
   }

void SetPtrs (PSZ pszStr, PPSZ ppszPtrs)
   {
   PSZ psz;
   INT i;

   ppszPtrs[0] = psz = pszStr;
   for (i=1; psz = strchr (psz+1, ','); i++)
      ppszPtrs[i] = StrStrip (psz+1, " \t");

   ppszPtrs[i] = NULL;
   }


BOOL InCircle (INT yCtr, INT xCtr, INT iRad, INT yPos, INT xPos)
   {
   INT y, x;

   y   = yPos - yCtr;
   x   = xPos - xCtr;
   return (y*y + x*x <= iRad * iRad);
   }



