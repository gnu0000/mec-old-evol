/*
 * Critters.c
 * Friday, 6/13/1997.
 * Craig Fitzgerald
 *
 * possible future refinements:
 *    multipart gene conditions
 *    stack based memory, bottom drops off
 *    external information memory mapped
 *    sexual reproduction
 *    separate m and T genes
 *    multipart genes
 *    scrolling screen if grid bigger than display
 *
 *    male/female/common specific chromosomes
 *    gene precidence values
 *
 * variable pop size
 * sexual reproduction
 * carnivore species
 * food sprout by other food?
 ***************************************************************************
 *
 *  gene description:
 *
 *  condition;weight;action
 *
 *
 *  positions              objects
 *  ---------              -------
 *    A B C  F= B 0        1     - anything
 *    7 0 1  R= 1 2 3      0     - nothing
 *    6 x 2  L= 7 6 5      F     - any edible
 *    5 4 3                P     - any preditor
 *                         L     - any flora
 *  comparitor             A     - any fauna
 *  -----------            ~c    - flora/fauna object with tag 'c'
 *   > =greater           
 *   < =less               
 *   = =equal
 *
 *  conditions:
 *  -----------
 *  O<object><position>                          - object at pos
 *  F<comparator><n>                             - food level
 *  1                                            - true
 *  M<Direct/Indirect><comparator><loc>:<value>  - memory
 *  @<condition index>                           - other condition
 *
 *
 *  action:
 *  -----------
 *  M<Direct/Indirect><Set/Inc/Dec><loc>:<value> - set memory  (any number)
 *  T<turn><Sit|Eat|Walk|Run>                    - movement    (1 is chosen)
 *
 ***************************************************************************
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


/***************************************************************************/
//INT   iCRITTERS         = 50;
//INT   iCRITTER_KILLOFF  = 15;
//INT   iCRITTER_BRANDNEW = 3;
//INT   iTURNS_PER_YEAR   = 2000;
//INT   iORG_MEM_SIZE     = 256;
//INT   iORG_GENE_SIZE    = 8;
//INT   iFOOD_VALUE       = 40;
//INT   iFOOD_PER_TURN    = 2;
//INT   iINITIALFOOD      = 100;
//BOOL  bFOODACCUMULATES  = FALSE;
//INT   iMAX_FOOD         = 2000;
//INT   iMIN_FOOD         = -10000;
//BOOL  bCONC             = TRUE;
//INT   iCONCRADIUS       = 6;
//INT   iCONCRATIO        = 10;
//INT   iWEIGHTBURN [10]  = {1, 1, 1, 1, 1, 1 ,2, 3, 4, 5};
//INT   iACTIONBURN [3]   = {1, 2, 8};
//PORG pHead     = NULL;
//PORG pTail     = NULL;
//PORG pFreePool = NULL;



ULONG ulYEARS      = 0;    // age of universe in years
ULONG ulTURNS      = 0;    // age of universe in turns
INT   iDELAY       = 0;    // delay between turns

PADAT pFauna       = NULL; // all animal species
PLDAT pFlora       = NULL; // all plant species

CHAR sz        [256];      // work buffer

/***************************************************************************/
/*                                                                         */
/* Driver Functions                                                        */
/*                                                                         */
/***************************************************************************/

static BOOL bHOLD [ABS_MAX_GENES];

void DoTurn (PADAT pa, PORG porg)
   {
   INT i, iSum, r, iWeight;
   PSZ psz, pszAction;

   memset (bHOLD, 0, pa->iGeneCount * sizeof (BOOL));

   /*--- first we find which genes are active for this turn ---*/
   for (iSum = i = 0; psz = porg->ppszGenes[i]; i++)
      {
      if (!EvalCondition (porg, psz))
         continue;

      SplitGene (psz, &iWeight, &pszAction);

      /*--- mem genes are independent ---*/
      if (*pszAction == 'M')
         {
         if (Rnd (iMAX_WEIGHT) < iWeight)
            {
            Action (porg, pszAction);
            porg->pulChoices[i]++;
            }
         }
      else /*--- only 1 movement gene is chosen per turn ---*/
         {
         iSum += iWeight;
         bHOLD[i] = TRUE;
         }
      }

   if (!iSum) // nothing to do
      {
      Action (porg, " ");
      }
   else /*--- choose movement gene based on chance weighting ---*/
      {
      r = Rnd (iSum);
      for (iSum = i = 0; psz = porg->ppszGenes[i]; i++)
         {
         if (!bHOLD[i])
            continue;
         SplitGene (psz, &iWeight, &pszAction);

         iSum += iWeight;
         if (iSum > r)
            break;
         }
      Action (porg, pszAction);
      porg->pulChoices[i]++;
      }
   porg->ulAge++;
   }


void DumpData ()
   {
//   INT   i;
//   PORG porg;
//
//   Log ("rank idx food");
//   Log ("-------------");
//   for (i=0, porg = pHead; porg; porg = porg->next, i++)
//      Log ("%2d   %2d  %4d", i, porg->iIdx, porg->iFood);
   }


void AddFlora (INT iSpeciesIndex, INT iCount)
   {
   INT   i, iY, iX;
   PCEL  pCel;
   PLDAT pl;

   pl = pFlora + iSpeciesIndex;
   for (i=0; i<iCount; i++)
      {
      if (pl->bConc)
         {
         if (Rnd (iMAX_WEIGHT) < pl->iConcWeight)
            while (TRUE)
               {
               iY = pl->iConcY + Rnd (pl->iConcRadius * 2) - pl->iConcRadius;
               iX = pl->iConcX + Rnd (pl->iConcRadius * 2) - pl->iConcRadius;
               if (InCircle (pl->iConcY, pl->iConcX, pl->iConcRadius, iY, iX))
                  break;
               }
         else
            while (TRUE)
               {
               iY = Rnd (iGRID_Y_SIZE);
               iX = Rnd (iGRID_X_SIZE);
               if (!InCircle (pl->iConcY, pl->iConcX, pl->iConcRadius, iY, iX))
                  break;
               }
         }
      else
         {
         iY = Rnd (iGRID_Y_SIZE);
         iX = Rnd (iGRID_X_SIZE);
         }
      pCel = GetCell (iY, iX);
      pCel->iFlora |= (CHAR)(1 << iSpeciesIndex);

      DisplayLoc (iY, iX);
      }
   }


void Examine (void)
   {
//   BYTE  p[2];
//   INT   i;
//
//   p[0] = ' ';
//   p[1] = 0x17;
//   for (i=0; i< iORG_GENE_SIZE + 6; i++)
//      ScrWriteNCell (p, 40, i, 0);
//
//   ScrSetCursorPos (1, 0);
//
//   printf ("  Loc: (%2d,%2d) Dir:%d  Food:%d  Age:%ld\n\n", 
//            pHead->iY, pHead->iX, pHead->iDir, pHead->iFood, pHead->ulAge);
//
//   printf (" invoked gene\n");
//   printf (" -----------------------------------\n");
//   for (i=0; i<iORG_GENE_SIZE; i++)
//      printf ("  %5ld  %s\n", pHead->pulChoices[i], pHead->ppszGenes[i]);
//
//   i = KeyGet (TRUE);
//   Refresh (0, 0, iORG_GENE_SIZE + 6, 40);
   }



INT Run (BOOL bStepMode)
   {
   PORG  porg;
   BOOL  bForward = FALSE, bExit = FALSE;
   INT   i, c, iSoFar;
   PADAT pa;

   while (!bExit)
      {
      bForward = !bForward;

      /*--- flora growth ---*/
      for (i=0; iLCOUNT; i++)
         AddFlora (i, pFlora[i].iGrowthPerTurn);

      /*--- fauna movement ---*/
      for (i=0; iACOUNT; i++)
         {
         pa = pFauna + i;

         if (bForward)
            for (porg = pa->pHead; porg; porg = porg->next)
               DoTurn (pa, porg);
         else
            for (porg = pa->pTail; porg; porg = porg->prev)
               DoTurn (pa, porg);
         }
      ulTURNS++;

      /*--- fauna spawning ---*/
      for (i=0; iACOUNT; i++)
         {
         pa = pFauna + i;
         if (!(ulTURNS % pa->iMateCycleLen))
            {
            Spawn (pa);
            UpdateStatus ();
            }
         }
      if (bStepMode)
         return 0;

      /*--- delay (while allowing key input) ---*/
      for (iSoFar = 12; TRUE; iSoFar += 12)
         {
         if (k_kbhit ())
            {
            c = KeyGet (TRUE);
            if (!MovementKey (c))
               return c;
            }
         else if (iDELAY)
            FineSleep (12);

         if (iSoFar >= iDELAY)
            break;
         }
      }
   return 0;
   }


void DebugBreak (void)
   {
   _asm int 3;
   }


void MainLoop (INT iNext)
   {
   INT i;

   while (TRUE)
      {
      i = (iNext ? iNext : KeyGet (TRUE));
      iNext = 0;

      switch (i)
         {
         case ' '  :
         case 0x0D : Run (FALSE);                break;
         case 'E'  : Examine ();                 break;
         case 'D'  : WriteCritters (szOUTFILE);  break;
         case 'R'  : Refresh (0, 0, 50, 80);     break;
         case 'S'  : Run (TRUE);                 break;
         case '1'  : iDELAY = 1000;              break;
         case '2'  : iDELAY = 500;               break;
         case '3'  : iDELAY = 300;               break;
         case '4'  : iDELAY = 250;               break;
         case '5'  : iDELAY = 200;               break;
         case '6'  : iDELAY = 150;               break;
         case '7'  : iDELAY = 100;               break;
         case '8'  : iDELAY = 50;                break;
         case '9'  : iDELAY = 0;                 break;
         case 'Z'  : DebugBreak ();              break;

         case 0x1b : return;

         default:
            MovementKey (i);

         }
      }
   }

/***************************************************************************/
/*                                                                         */
/* Init/Term/Main                                                          */
/*                                                                         */
/***************************************************************************/

void Init (void)
   {
   INT i, iY, iX;

   if (!(pgrid = calloc (iGRID_Y_SIZE * iGRID_X_SIZE, sizeof (CEL))))
      Error ("unable to alloc grid");

   if (bDISPLAY)
      {
      pmet = ScrInitMetrics ();
      ScrSetIntenseBkg (TRUE);
      ScrClear (0, 0, 50, 80, ' ', 0x1F);
      InitScreenMetrics ();
      ScrShowCursor (FALSE);
      }
   LogInit ();

   srand (ArgIs ("Seed") ? atoi (ArgGet("Seed", 0)) : (USHORT)time (NULL));

   /*--- setup terrain ---*/
   memset (pgrid, 0, iGRID_Y_SIZE * iGRID_X_SIZE * sizeof (CEL));
   for (iY=0; iY < iGRID_Y_SIZE; iY++)
      for (iX=0; iX < iGRID_X_SIZE; iX++)
         GetCell (iY, iX)->cTerrain = 0x10;

   for (i=0; iLCOUNT; i++)
      AddFlora (i, pFlora[i].iStartCount);

   if (*szINFILE)
      ReadCritters (szINFILE);
   else
      {
      for (i=0; iACOUNT; i++)
         Genesis (pFauna + i);
      }
//   InitObjects ();
   InitFineSleep (500);
   }


void Term (void)
   {
   PORG porg, porgNext;
   INT  i;

   WriteCritters (szOUTFILE);
   DumpData ();
   fclose (fpLOG);

   /*--- kill all critters ---*/
   for (i=0; iACOUNT; i++)
      for (porg = pFauna[i].pHead; porg; porg = porgNext)
         {
         porgNext = porg->next;
         KillCritter (porg);
         }
   MemFreeMemoryPool ();

   if (bDISPLAY)
      ScrShowCursor (TRUE);
   }


void Usage ()
   {
   printf ("CRITTERS     Evolution simulator              v1.0       %s  %s\n", __TIME__, __DATE__);
   printf ("\n");
   printf ("USAGE: iCRITTERS [options]\n");
   printf ("\n");
   printf ("WHERE: [options] are 0 or more of:\n");
   printf ("          /? .............. This help\n");
   printf ("          /NoDisplay ...... Don't display while running\n");
   printf ("          /ReadFile=file .. Read critters from this file at start\n");
   printf ("          /WriteFile=file . Write critters to this file at end\n");
   printf ("          /LogFile=file ... Send log here\n");
   printf ("          /CFGFile=file ... Read cfg options from here\n");
   printf ("          /Seed............ Random number Generator seed\n");
   printf ("\n");
   exit (0);
   } 


/*
 *
 *
 */
int CCONV main (int argc, char *argv[])
   {
#if defined (MEM_DEBUG)
      MemSetDebugMode (MEM_TEST | MEM_LOG | MEM_CLEAR | MEM_EXITLIST);
      MemSetDebugStream (fopen ("e.mem", "wt"));
#endif

   if (ArgBuildBlk ("? *^NoDisplay *^ReadFile% *^WriteFile% "
                    "*^LogFile% *^Seed% *^CfgFile"))
      Error ("%s", ArgGetErr ());
      
   if (ArgFillBlk (argv))
      Error ("%s", ArgGetErr ());

   if (ArgIs ("?"))
      Usage ();

   *szINFILE = '\0';
   ReadCfg (szCFGFILE);

   strcpy (szCFGFILE, (ArgIs ("CfgFile" )  ? ArgGet ("CfgFile"  , 0) : "critters.cfg"));
   strcpy (szINFILE,  (ArgIs ("ReadFile" ) ? ArgGet ("ReadFile" , 0) : szINFILE));
   strcpy (szOUTFILE, (ArgIs ("WriteFile") ? ArgGet ("WriteFile", 0) : "critters.dat"));
   strcpy (szLOGFILE, (ArgIs ("LogFile"  ) ? ArgGet ("LogFile"  , 0) : "critters.log"));

   bDISPLAY = (ArgIs ("NoDisplay") ? FALSE : bDISPLAY);

   Init ();

   if (bDISPLAY)
      MainLoop (' ');
   else
      Run (FALSE);

   Term ();
   exit (0);
   return 0;
   }

