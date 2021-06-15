/*
 *
 * io.c
 * Thursday, 6/26/1997.
 * Craig Fitzgerald
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


FILE *fpLOG;            // log file handle

CHAR szINFILE  [256];   // input file name
CHAR szOUTFILE [256];   // output file name
CHAR szLOGFILE [256];   // log file name

INT  iLCOUNT;           // # flora species
INT  iACOUNT;           // # fauna species


/***************************************************************************/
/*                                                                         */
/* I/O functions                                                           */
/*                                                                         */
/***************************************************************************/


void WriteCritters (PSZ pszFile)
   {
   FILE  *fp;
   INT   i, j, k;
   PORG porg;

   SortCritters ();
   if (!(fp = fopen (pszFile, "wt")))
      Error ("Cannot open critters file: %s", pszFile);

   fprintf (fp, ";CRIT0, index, age, food, genesize, memsize/16\n");
   fprintf (fp, ";CRIT1, ypos,  xpos, direction\n");
   fprintf (fp, ";CRIT2, origindex, displaychar, displayatt\n");
   fprintf (fp, ";GENE,  count, genestring\n");
   fprintf (fp, ";MEM,   entries x 16\n");

   for (i=0, porg = pHead; porg; porg = porg->next, i++)
      {
      fprintf (fp, "CRIT0, %d, %ld, %d, %d, %d\n", i, porg->ulAge, porg->iFood, iORG_GENE_SIZE, iORG_MEM_SIZE/16);
      fprintf (fp, "CRIT1, %d, %d, %d\n", porg->iY, porg->iX, porg->iDir);
      fprintf (fp, "CRIT2, %d, %d, %d\n", porg->iIdx, (INT)porg->Display[0], (INT)porg->Display[1]);

      for (j=0; j<iORG_GENE_SIZE; j++)
         fprintf (fp, "GENE,  %5ld, %s\n", porg->pulChoices[j], porg->ppszGenes[j]);
      for (j=0; j<iORG_MEM_SIZE/16; j++)
         {
         fprintf (fp, "MEM%1x,   ", j);
         for (k=0; k<16; k++)
            fprintf (fp, "%5d%s", porg->piMem[j*16+k], (k<15 ? ", " : "\n"));
         }
      }
   fclose (fp);
   }


static BOOL ReadTo (FILE *fp, PSZ pszHeader, PPSZ ppsz)
   {
   while (TRUE)
      {
      if (FilReadLine (fp, sz, ";\n", 256) == -1)
         return FALSE;
      if (!strnicmp (sz, pszHeader, strlen (pszHeader)))
         {
         SetPtrs (sz, ppsz);
         return TRUE;
         }
      }
   }


static void GetLine (FILE *fp, PSZ pszHeader, PPSZ ppsz)
   {
   if (FilReadLine (fp, sz, ";\n", 256) == -1)
      Error ("End of file found line type %s expected", pszHeader);
   if (strnicmp (sz, pszHeader, strlen (pszHeader)))
      Error ("line %ld: Line type %s expected, got %s", FilGetLine (), pszHeader, sz);
   SetPtrs (sz, ppsz);
   }


static PORG ReadCritter (FILE *fp)
   {
   INT   j, k, iGenes, iMem;
   PSZ   ppsz[30];
   PORG porg;

   if (!ReadTo (fp, "CRIT0", ppsz))
      return NULL;

   porg = NewCell ();
   porg->ulAge = atol (ppsz[2]);
   porg->iFood = atoi (ppsz[3]);
   iGenes       = atoi (ppsz[4]);
   iMem         = atoi (ppsz[5]);

   GetLine (fp, "CRIT1", ppsz);
   porg->iY    = atoi (ppsz[1]);
   porg->iX    = atoi (ppsz[2]);
   porg->iDir  = atoi (ppsz[3]);

   GetLine (fp, "CRIT2", ppsz);
   porg->iIdx       = atoi (ppsz[1]);
   porg->Display[0] = atoi (ppsz[2]);;
   porg->Display[1] = atoi (ppsz[3]);;

   for (j=0; j<iGenes; j++)
      {
      GetLine (fp, "GENE", ppsz);
      porg->pulChoices[j] = atol (ppsz[1]);
      strcpy (porg->ppszGenes[j], ppsz[2]);
      }
   for (j=0; j<iMem; j++)
      {
      GetLine (fp, "MEM", ppsz);
      for (k=0; k<16; k++)
         porg->piMem[j*16+k] = atoi (ppsz[k+1]);
      }
   return porg;
   }


INT ReadCritters (PSZ pszFile)
   {
   FILE  *fp;
   INT   i;
   PORG porg;

   if (!(fp = fopen (pszFile, "rt")))
      Error ("Cannot open critters file: %s", pszFile);

   for (i=0; porg = ReadCritter (fp); i++)
      GridAdd (porg);

   fclose (fp);
   ColorCritters ();
   return i;
   }


/***************************************************************************/

static PSZ GetEntryS (PSZ pszFile, PSZ pszSect, PSZ pszEntry, BOOL bRequired)
   {
   *sz = '\0';
   if (!CfgGetLine (pszFile, pszSect, pszEntry,  sz) && bRequired)
      Error ("Cannot Find Entry: %s in Section: [%s] in file: %s", pszEntry, pszSect, pszFile);
   StrStrip (StrClip (sz, " \t"), " \t");
   return sz;
   }

static INT GetEntryI (PSZ pszFile, PSZ pszSect, PSZ pszEntry, BOOL bRequired)
   {
   GetEntryS (pszFile, pszSect, pszEntry, bRequired);
   return atoi (sz);
   }

static CHAR GetEntryC  (PSZ pszFile, PSZ pszSect, PSZ pszEntry, BOOL bRequired)
   {
   GetEntryS (pszFile, pszSect, pszEntry, bRequired);
   return *sz;
   }

static BOOL GetEntryB  (PSZ pszFile, PSZ pszSect, PSZ pszEntry, BOOL bRequired)
   {
   GetEntryS (pszFile, pszSect, pszEntry, bRequired);
   return StrTrue (sz);
   }

static PPSZ GetEntrySL (PSZ pszFile, PSZ pszSect, PSZ pszEntry, BOOL bRequired)
   {
   PPSZ ppsz;

   GetEntryS (pszFile, pszSect, pszEntry, bRequired);
   ppsz = StrMakePPSZ (sz, ",", TRUE, TRUE, NULL);
   return PPSZ;
   }

static PINT GetEntryIL (PSZ pszFile, PSZ pszSect, PSZ pszEntry, INT iEntries, BOOL bRequired)
   {
   PPSZ ppsz;
   PINT pint;

   pint = calloc (iEntries+1, sizeof (INT);
   GetEntryS (pszFile, pszSect, pszEntry, bRequired);
   ppsz = StrMakePPSZ (sz, ",", TRUE, TRUE, &iCols);

   for (i=0; i<iCols && i<iEntries; i++)
      pint[i] = atoi (ppsz[i]);

   MemFreePPSZ (ppsz);
   return pint;
   }

/***************************************************************************/

PPSZ ppszFlora;
PPSZ ppszFauna; 

static PINT XlateDiet (PPSZ ppsz) 
   {
   }


static void ReadFauna (PSZ pszFile, PSZ pszSpecies, PADAT padat)
   {
   PPSZ ppsz;

   pldat->pszName           = strdup (pszSpecies);
   pldat->cSymbol           = GetEntryC (pszFile, pszSpecies, "Symbol", 1);
   pldat->cColor[2]         = GetEntryIL(pszFile, pszSpecies, "Color", 2, 1));
   pldat->ulMaxAge          = GetEntryI (pszFile, pszSpecies, "MaxAge", 1));
   pldat->iMinCalories      = GetEntryI (pszFile, pszSpecies, "MinCalories"    ,  1));
   pldat->iMaxCalories      = GetEntryI (pszFile, pszSpecies, "MaxCalories"    ,  1));
   pldat->iGeneCount        = GetEntryI (pszFile, pszSpecies, "GeneCount"      ,  1));
   pldat->iMemCount         = GetEntryI (pszFile, pszSpecies, "MemCount "      ,  1));
   pldat->iStartPop         = GetEntryI (pszFile, pszSpecies, "StartPopulation",  1));
   pldat->iStartCalorieVal  = GetEntryI (pszFile, pszSpecies, "StartCalories"  ,  1));
   pldat->iMateCycleLen     = GetEntryI (pszFile, pszSpecies, "MateCycleLength" , 1));
   pldat->iMateCalorieLimit = GetEntryI (pszFile, pszSpecies, "MateCalorieLimit", 1));
   pldat->iKidBirthCount    = GetEntryI (pszFile, pszSpecies, "KidBirthCount"   , 1));
   pldat->iKidFoodXfer      = GetEntryI (pszFile, pszSpecies, "KidFoodXfer"     , 1));
   pldat->iKidChanceFemale  = GetEntryI (pszFile, pszSpecies, "KidChanceFemale" , 1));
   pldat->piWeightBurn      = GetEntryIL(pszFile, pszSpecies, "WeightBurnRates", 10, 1));
   pldat->piActionBurn      = GetEntryIL(pszFile, pszSpecies, "ActionBurnRates", 3,  1));
   pldat->piMutationRates   = GetEntryI (pszFile, pszSpecies, "MutationRates",   3,  1));
   pldat->iViableTime       = GetEntryI (pszFile, pszSpecies, "ViableTime",       1));
   pldat->ppszInstincts     = GetEntrySL(pszFile, pszSpecies, "Instincts",        1));

   ppsz = GetEntrySL(pszFile, pszSpecies, "Diet", 1));
   pldat->iDiet             = XlateDiet (ppsz);
   MemFreePPSZ (ppsz, 0);
   }


static void ReadFlora (PSZ pszFile, PSZ pszSpecies, PADAT pldat)
   {
   pldat->pszName        = strdup (pszSpecies);
   pldat->cSymbol        = GetEntryC (pszFile, pszSpecies, "Symbol",       1);
   pldat->cColor         = GetEntryI (pszFile, pszSpecies, "Color",        1));
   pldat->iStartCount    = GetEntryI (pszFile, pszSpecies, "StartCount",   1));
   pldat->iCalorieValue  = GetEntryI (pszFile, pszSpecies, "CaloricValue", 1));
   pldat->iGrowthPerTurn = GetEntryI (pszFile, pszSpecies, "GrowthPerTurn",1));
   pldat->bConc          = GetEntryB (pszFile, pszSpecies, "Concentration",1));
   pldat->iConcWeight    = GetEntryI (pszFile, pszSpecies, "ConcWeight",   1));
   pldat->iConcY         = GetEntryI (pszFile, pszSpecies, "ConcYCenter",  1));
   pldat->iConcX         = GetEntryI (pszFile, pszSpecies, "ConcXCenter",  1));
   pldat->iConcRadius    = GetEntryI (pszFile, pszSpecies, "ConcRadius",   1));
   }


void ReadCfg (PSZ pszFile)
   {
   INT i;

   if (access (pszFile, 0))
      Error ("Unable to find cfg file: %s", pszFile);

   if (GetEntry (pszFile, "Settings", "LoadFile", 0)
      strcpy (szINFILE, sz);

   bDISPLAY =  StrTrue (GetEntry (pszFile, "Settings", "Display",   1));
   iY_GRID_SIZE = atoi (GetEntry (pszFile, "Settings", "GridYSize", 1));
   iX_GRID_SIZE = atoi (GetEntry (pszFile, "Settings", "GridXSize", 1));

   ppszFlora = GetList (pszFile, "Settings", "Flora", 1, &iLCOUNT));
   ppszFauna = GetList (pszFile, "Settings", "Fauna", 1, &iACOUNT));

   pFlora = calloc (iLCOUNT+1, sizeof (LDAT));
   pFauna = calloc (iACOUNT+1, sizeof (ADAT));

   for (i=0; i<iLCOUNT; i++)
      ReadFlora (pszFile, ppszFlora[i], pFlora+i);

   for (i=0; i<iACOUNT; i++)
      ReadFauna (pszFile, ppszFauna[i], pFauna+i);

   MemFreePPSZ (ppszFlora, iLCOUNT);
   MemFreePPSZ (ppszFauna, iACOUNT);
   }



//void ReadCfg (PSZ pszFile)
//   {
//   INT i;
//   PSZ   ppsz[30];
//
//   if (CfgGetLine (pszFile, "Settings", "Critters", sz) && *sz)
//      iCRITTERS         = atoi (sz);
//   if (CfgGetLine (pszFile, "Settings", "Killoff", sz) && *sz)
//      iCRITTER_KILLOFF  = atoi (sz);
//   if (CfgGetLine (pszFile, "Settings", "BrandNew", sz) && *sz)
//      iCRITTER_BRANDNEW = atoi (sz);
//   if (CfgGetLine (pszFile, "Settings", "GridYSize", sz) && *sz)
//      iGRID_Y_SIZE      = atoi (sz);
//   if (CfgGetLine (pszFile, "Settings", "GridXSize", sz) && *sz)
//      iGRID_X_SIZE      = atoi (sz);
//   if (CfgGetLine (pszFile, "Settings", "TurnsPerYear", sz) && *sz)
//      iTURNS_PER_YEAR   = atoi (sz);
//   if (CfgGetLine (pszFile, "Settings", "QuickStart", sz) && *sz)
//      bQUICKSTART       = StrTrue (sz);
//
//   if (CfgGetLine (pszFile, "Critters", "MemSize", sz) && *sz)
//      iORG_MEM_SIZE    = atoi (sz);
//   if (CfgGetLine (pszFile, "Critters", "GeneSize", sz) && *sz)
//      iORG_GENE_SIZE   = min (atoi (sz), ABS_MAX_GENES);
//   if (CfgGetLine (pszFile, "Critters", "MaxFood", sz) && *sz)
//      iMAX_FOOD         = atoi (sz);
//   if (CfgGetLine (pszFile, "Critters", "MinFood", sz) && *sz)
//      iMIN_FOOD         = atoi (sz);
//   if (CfgGetLine (pszFile, "Food", "FoodPerTurn", sz) && *sz)
//      iFOOD_PER_TURN    = atoi (sz);
//   if (CfgGetLine (pszFile, "Food", "FoodValue", sz) && *sz)
//      iFOOD_VALUE       = atoi (sz);
//   if (CfgGetLine (pszFile, "Food", "InitialFood", sz) && *sz)
//      iINITIALFOOD      = atoi (sz);
//   if (CfgGetLine (pszFile, "Food", "FoodAccumulates", sz) && *sz)
//      bFOODACCUMULATES  = StrTrue (sz);
//   if (CfgGetLine (pszFile, "Food", "Concentration",       sz) && *sz)
//      bCONC         = StrTrue (sz);
//   if (CfgGetLine (pszFile, "Food", "ConcentrationRadius", sz) && *sz)
//      iCONCRADIUS   = atoi (sz);
//   if (CfgGetLine (pszFile, "Food", "ConcentrationRatio",  sz) && *sz)
//      iCONCRATIO    = atoi (sz);
//
//   if (CfgGetLine (pszFile, "Food", "WeightBurnRate", sz) && *sz)
//      {
//      SetPtrs (sz, ppsz);
//      for (i=0; i<10; i++)
//         iWEIGHTBURN [i]  = max (1, atoi (ppsz[i]));
//      }
//
//   if (CfgGetLine (pszFile, "Food", "ActionBurnRate", sz) && *sz)
//      {
//      SetPtrs (sz, ppsz);
//      for (i=0; i<3; i++)
//         iACTIONBURN [i]  = max (1, atoi (ppsz[i]));
//      }
//   }
