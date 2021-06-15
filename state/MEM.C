/*
 *
 * mem.c
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


static PORG pFreePool;

/***************************************************************************/
/*                                                                         */
/* Memory functions                                                        */
/*                                                                         */
/***************************************************************************/

static PORG MemAllocOrg (void)
   {
   PORG porg;

   if (!(porg = calloc (1, sizeof (ORG))))
      Error ("calloc problem creating org");

   if (!(porg->piMem = (PINT) calloc (iORG_MEM_SIZE, sizeof (INT))))
      Error ("calloc problem creating org");

   if (!(porg->ppszGenes = (PPSZ) calloc (iORG_GENE_SIZE+1, sizeof (PSZ))))
      Error ("calloc problem creating org");

   if (!(porg->pulChoices = (PULONG) calloc (iORG_GENE_SIZE+1, sizeof (ULONG))))
      Error ("calloc problem creating org");

   for (j=0; j<iORG_GENE_SIZE; j++)
      if (!(porg->ppszGenes[j] = calloc (iMAX_GENE_STRING, sizeof (CHAR))))
         Error ("calloc problem creating org");

   return porg;
   }


PORG MemNewOrg (void)
   {
   PORG porg;

   if (!pFreePool)
      return MemAllocOrg ();

   porg = pFreePool;
   pFreePool = pFreePool->next;
   porg->prev = porg->next = NULL;
   return porg;
   }


void MemDeleteOrg (PORG porg)
   {
   INT j;

   porg->iIdx  = 0;
   porg->iY    = 0;
   porg->iX    = 0;
   porg->iDir  = 0; 
   porg->iFood = 0;
   porg->ulAge = 0;

   memset (porg->pulChoices, 0, iORG_GENE_SIZE * sizeof (ULONG));
   memset (porg->piMem     , 0, iORG_MEM_SIZE  * sizeof (INT));

   porg->prev  = porg->next  = NULL;
   porg->gprev = porg->gnext = NULL;

   for (j=0; j<iORG_GENE_SIZE; j++)
      *porg->ppszGenes[j] = '\0';

   porg->next = pFreePool;
   pFreePool = porg;
   }

void MemFreeMemoryPool (void)
   {
   PORG porg, porgNext;
   INT   i;

   for (porg = pFreePool; porg; porg = porgNext)
      {
      porgNext = porg->next;

      for (i = 0; porg->ppszGenes[i]; i++)
         free (porg->ppszGenes[i]);
      free (porg->ppszGenes);
      free (porg->pulChoices);
      free (porg->piMem);
      free (porg);
      }
   }


