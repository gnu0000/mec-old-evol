/*
 *
 * org.c
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


int CCONV cmporg (const void *p1, const void *p2)
   {
   PORG pp1, pp2;

   pp1 = *(PORG *)p1;
   pp2 = *(PORG *)p2;

   return pp2->iFood - pp1->iFood;
   }


void SortCritters (void)
   {
   PORG porg, *pporg;
   INT   i, iCount;

   /*--- order by food ---*/
   for (i=0, porg = pHead; porg; porg = porg->next, i++)
      ;
   pporg = (PORG *) calloc (i+1, sizeof (PORG));
   for (i=0, porg = pHead; porg; porg = porg->next, i++)
      pporg[i] = porg;

   iCount = i;
   qsort (pporg, iCount, sizeof (PORG), cmporg);
   pporg[0]->prev        = NULL;
   pporg[iCount-1]->next = NULL;
   for (i=0; i+1<iCount; i++)
      {
      pporg[i]->next   = pporg[i+1];
      pporg[i+1]->prev = pporg[i];
      }
   pHead = pporg[0];
   pTail = pporg[iCount-1];
   free (pporg);
   }




void ColorCritters (void)
   {
   PORG porg;

   for (porg = pHead; porg; porg = porg->next)
      porg->Display[1] = 0x0F;

   /*--- color top 3 ---*/
   pHead->Display[1]             = 0x0C;
   pHead->next->Display[1]       = 0x0E;
   pHead->next->next->Display[1] = 0x0A;

   }

