/*
 *
 * e.c
 * Friday, 6/13/1997.
 *
 *
 * possible future refinements:
 *    stack based memory, bottom drops off
 *    external information memory mapped
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GnuType.h>
#include <GnuScr.h>
#include <GnuMisc.h>
#include <Gnukbd.h>

///***************************************************************************/
//
// condition;weight;action
//
//
//
// positions              objects
// ---------              -------
//   A B C  F= B 0        p - putz
//   7 0 1  R= 1 2 3      m - monster
//   6 x 2  L= 7 6 5      f - food
//   5 4 3  
//
// comparitor
// -----------
//  > =greater
//  < =less
//  = =equal
//
// conditions:
// -----------
// O<object><position>       - object at pos
// F<comparator><n>          - food level
// 1                         - true
// M<comparator><loc>:<value>- memory
// @<condition index>        - other condition
//
//
// action:
// -----------
// M<Set/Inc/Dec><loc>:<value> - set memory  (any number)
// T<turn><Sit|Eat|Walk|Run>   - movement    (1 is chosen)
//
//
///***************************************************************************/

#define CRITTERS            100
#define CRITTER_KILLOFF      30
#define CRITTER_BRANDNEW      3



#define DELAY           500
#define GRID_Y_SIZE     50
#define GRID_X_SIZE     80

#define PUTZ_MEM_SIZE   256
#define PUTZ_GENE_SIZE  32

#define MAX_FOOD        2000
#define MAX_MEM_VAL     32767

#define MAX_WEIGHT      1000

#define FOOD_VALUE      50

#define FOOD_PER_TURN   5

//#define TURNS_PER_YEAR  2000
// for debug
#define TURNS_PER_YEAR  20

typedef struct _putz
   {
   INT  iIdx;      // body index
   INT  iX, iY;    // position
   INT  iDir;      // direction
   INT  iFood;     // food level
   BYTE Display[2];// display att

   PPSZ ppszGenes; // gene list
   PINT piMem;     // memory array

   struct _putz *prev;  // list of all cells
   struct _putz *next;  //

   struct _putz *gnext; // list of cells at this location
   struct _putz *gprev; // list of cells at this location
   } PUTZ;
typedef PUTZ *PPUTZ;


PPUTZ pHead = NULL;
PPUTZ pTail = NULL;

typedef struct _cell
   {
   char cTerrain;
   char cFood;
   struct _putz *plist;
   } CEL;
typedef CEL *PCEL;

CEL grid [GRID_Y_SIZE][GRID_X_SIZE];

FILE *fpLOG;

/***************************************************************************/

void LogInit (void)
   {
   fpLOG = fopen ("e.log", "wt");
   }


UINT Log (PSZ psz, ...)
   {
   va_list vlst;

   va_start (vlst, psz);
   vfprintf (fpLOG, psz, vlst);
   va_end (vlst);
   fprintf (fpLOG, "\n");
   return 0;
   }


/***************************************************************************/

int Modo (INT i, INT iMod)
   {
   while (i < 0) 
      i += iMod;
   i %= iMod;
   return i;
   }

void SplitGene (PSZ psz, PINT piWeight, PPSZ ppszAction)
   {
   *ppszAction = strchr (psz, ';') + 1;
   *piWeight = atoi (*ppszAction );
   *ppszAction = strchr (*ppszAction, ';') + 1;
   }



/***************************************************************************/


void DisplayLoc (INT iY, INT iX)
   {
   BYTE *pb, p[2];
   PCEL pCel;

   pb = p;
   pCel = grid[iY] + iX;
   if (pCel->plist)
      {
      pb = pCel->plist->Display;
      pb[1] = (pb[1] & 0x0F) + pCel->cTerrain;
      }
   else
      {
      p[0] = (pCel->cFood ? '.' : ' ');
      p[1] = pCel->cTerrain + 0x02;
      }
   ScrWriteNCell (pb, 1, iY, iX);
   }

void GridRemove (PPUTZ pputz)
   {
   PCEL pCel;

   pCel = grid[pputz->iY] + pputz->iX;

   if (!pCel->plist)
      Error ("grid location is empty (remove %p [%d,%d])", pputz, pputz->iY, pputz->iX);

   if (pputz->gnext)
      {
      pputz->gnext->gprev = pputz->gprev;
      }

   if (pputz->gprev)
      {
      pputz->gprev->gnext = pputz->gnext;
      }
   else
      {
      pCel->plist = pputz->gnext;
      }

   pputz->gprev = NULL;
   pputz->gnext = NULL;

   DisplayLoc (pputz->iY, pputz->iX);
   }

void GridAdd (PPUTZ pputz)
   {
   PCEL pCel;

   pCel = grid[pputz->iY] + pputz->iX;

   pputz->gprev = NULL;
   pputz->gnext = pCel->plist;

   if (pCel->plist)
      pCel->plist->gprev = pputz;
   pCel->plist  = pputz;

   DisplayLoc (pputz->iY, pputz->iX);
   }

/***************************************************************************/
/***************************************************************************/

PPUTZ NewCell (void)
   {
   PPUTZ pputz;

   pputz = calloc (1, sizeof (PUTZ));

   if (!pHead)
      {
      pHead = pTail = pputz;
      }
   else
      {
      pTail->next = pputz;
      pputz->prev = pTail;
      pTail = pputz;
      }
   return pputz;
   }

PSZ AddRnd (PSZ psz, UINT uVal)
   {
   return psz + sprintf (psz, "%d", Rnd(uVal));
   }


CHAR szCONDS  [] = "OF1M@";
CHAR szOBJS   [] = "pmf";
CHAR szPOS    [] = "01234567ABCFLR";
CHAR szCMP    [] = "><=";
CHAR szACTION [] = "MT";
CHAR szSETTYP [] = "SID";
CHAR szMOVTYP [] = "SEWR";


PSZ AddCondition (PSZ psz)
   {
   if (Rnd (2))
      *psz++ = '!';

//   switch (*psz++ = szCONDS[Rnd (5)])  //@ not implemented yet
   switch (*psz++ = szCONDS[Rnd (4)])
      {
      case 'O':
         *psz++ = szOBJS[Rnd(3)];
         *psz++ = szPOS[(Rnd(2) ? Rnd(14) : 11+Rnd(3))];
         break;

      case 'F':
         *psz++ = szCMP[Rnd(3)];
         psz    = AddRnd (psz, MAX_FOOD);
         break;
         
      case '1':
         *psz++ = '1';
         break;

      case 'M':
         psz    = AddRnd (psz, PUTZ_MEM_SIZE);
         *psz++ = szCMP[Rnd(3)];
         psz    = AddRnd (psz, MAX_MEM_VAL);
         break;

//      case '@':  // later
//         psz  = AddRnd (psz, PUTZ_GENE_SIZE);
//         break;
      }
   return psz;
   }


PSZ AddAction (PSZ psz)
   {
   switch (*psz++ = szACTION[Rnd(2)])
      {
      case 'M':
         *psz++ = szSETTYP[Rnd(3)];
         psz = AddRnd (psz, PUTZ_MEM_SIZE);
         *psz++ = ':';
         psz = AddRnd (psz, MAX_MEM_VAL);
         break;

      case 'T':
         *psz++ = szPOS[Rnd(8)];
         *psz++ = szMOVTYP[Rnd(4)];
         break;
      }
   return psz;
   }


PSZ NewGene (PSZ pszOld)
   {
   CHAR sz[256];
   PSZ  psz;

   psz = AddCondition (sz);

   /*--- add weight ---*/
   *psz++ = ';';
   psz = AddRnd (psz, MAX_WEIGHT);

   *psz++ = ';';
   psz = AddAction (psz);

   *psz = '\0';
   return strdup (sz);
   }

// modify or replace gene
PSZ CosmicRay_G (PPSZ ppsz)
   {
   CHAR szTmp [128];
   INT  iWeight;
   PSZ  psz, pszAction;

   if (!Rnd(2)) 
      {
      free (*ppsz);
      *ppsz = NewGene (NULL);
      }
   else
      {
      SplitGene (*ppsz, &iWeight, &pszAction);
      switch (Rnd (3))
         {
         case 0: // new condition
            psz = AddCondition (szTmp);
            strcpy (psz, strchr (*ppsz, ';'));
            break;

         case 1: // new weight
            strcpy (szTmp, *ppsz);
            psz = strchr (szTmp, ';') +1;
            psz = AddRnd (psz, MAX_WEIGHT);
            *psz++ = ';';
            strcpy (psz, pszAction);
            break;

         case 2: // new action
            strcpy (szTmp, *ppsz);
            psz = strrchr (szTmp, ';') +1;
            psz = AddAction (psz);
            *psz = '\0';
            break;
         }
      free (*ppsz);
      *ppsz = strdup (szTmp);
      }
   return *ppsz;
   }


void CosmicRay_M (PINT piMem)
   {
   INT iLoc, iVal;

   iLoc = Rnd (PUTZ_MEM_SIZE);
   iVal = Rnd (MAX_MEM_VAL);

   piMem[iLoc] = iVal;
   }


PPSZ NewGenes (PPSZ ppszOld)
   {
   PPSZ ppsz;
   INT  i;

   ppsz = (PPSZ) calloc (PUTZ_GENE_SIZE + 1, sizeof (PSZ));
   for (i=0; i<PUTZ_GENE_SIZE; i++)
      {
      if (ppszOld)
         {
         ppsz[i] = strdup (ppszOld[i]);
         CosmicRay_G (ppsz + i);
         }
      else
         {
         ppsz[i] = NewGene (NULL);
         }
      }
   ppsz[PUTZ_GENE_SIZE] = NULL;
   return ppsz;
   }


PINT NewMem (PINT piOld)
   {
   PINT piMem;
   INT  i;

   piMem = (PINT) calloc (PUTZ_MEM_SIZE, sizeof (INT));

   if (piOld)
      {
      memcpy (piMem, piOld, PUTZ_MEM_SIZE * sizeof (INT));
      CosmicRay_M (piMem);
      }
   else
      {
      for (i=0; i<PUTZ_MEM_SIZE; i++)
         piMem[i] = rand ();
      }
   return piMem;
   }


void KillCritter (PPUTZ pputz)
   {
   INT i;

   if (pputz->next)
      pputz->next->prev = pputz->prev;
   else
      pTail = pputz->prev;
      

   if (pputz->prev)
      pputz->prev->next = pputz->next;
   else
      pHead = pputz->next;

   pputz->prev = pputz->next = NULL; // not needed

   for (i = 0; pputz->ppszGenes[i]; i++)
      free (pputz->ppszGenes[i]);
   free (pputz->ppszGenes);
   free (pputz);
   }

void DivideCritter (PPUTZ pputzOld)
   {
   PPUTZ pputz;

   pputz = NewCell ();

   pputz->iIdx       = pputzOld->iIdx;
   pputz->Display[0] = pputzOld->Display[0];
   pputz->Display[1] = pputzOld->Display[1];
   pputz->iY         = Modo (pputzOld->iY + Rnd(3) - 1, GRID_Y_SIZE);
   pputz->iX         = Modo (pputzOld->iX + Rnd(3) - 1, GRID_X_SIZE);
   pputz->iDir       = Rnd(8);
   pputz->iFood      = pputzOld->iFood;
   pputz->ppszGenes  = NewGenes (pputzOld->ppszGenes);
   pputz->piMem      = NewMem   (pputzOld->piMem    );

   GridAdd (pputz);
   }


void Genesis (INT iCount)
   {
   INT  i;
   PPUTZ pputz;

   for (i=0; i<iCount; i++)
      {
      pputz = NewCell ();
      pputz->iIdx = i;
      pputz->Display[0] = 'p';
      pputz->Display[1] = 0x07;

      pputz->iX    = Rnd(GRID_X_SIZE);
      pputz->iY    = Rnd(GRID_Y_SIZE);
      pputz->iDir  = Rnd(8);
      pputz->iFood = 500;

      pputz->ppszGenes = NewGenes (NULL);
      pputz->piMem     = NewMem   (NULL);

      GridAdd (pputz);
      }
   }

/***************************************************************************/
/***************************************************************************/

void _movpos (INT y, INT x, INT iDir, PINT py, PINT px)
   {
   switch (iDir)
      {
      case 0: y--;      break;
      case 1: y--; x++; break;
      case 2:      x++; break;
      case 3: y++; x++; break;
      case 4: y++;      break;
      case 5: y++; x--; break;
      case 6:      x--; break;
      case 7: y--; x--; break;
      }
   if (y < 0) y += GRID_Y_SIZE;
   if (x < 0) x += GRID_X_SIZE;
   *py = y % GRID_Y_SIZE;
   *px = x % GRID_X_SIZE;
   }


void GetPos (INT iY, INT iX, INT iDir, CHAR cPos, PINT py, PINT px)
   {
   if (cPos == 'X')
      {
      *py = iY;
      *px = iX;
      return;
      }
   if (cPos < 'A')
      {
      iDir = Modo (iDir + cPos - '0', 8);
      _movpos (iY, iX, iDir, py, px);
      return;
      }
   _movpos (iY, iX, iDir, py, px);
   _movpos (*py, *px, iDir, py, px);

   if (cPos == 'A')
      _movpos (*py, *px, Modo (iDir + -2, 8), py, px);
   else if (cPos == 'C')
      _movpos (*py, *px, Modo (iDir + 2, 8), py, px);
   }


void Move (PPUTZ pputz)
   {
   INT y, x;

   _movpos (pputz->iY, pputz->iX, pputz->iDir, &y, &x);
   GridRemove (pputz);
   pputz->iY = y;
   pputz->iX = x;
   GridAdd (pputz);
   }

void Eat (PPUTZ pputz)
   {
   PCEL pCel;

   pCel = grid[pputz->iY] + pputz->iX;
   pputz->iFood = min (MAX_FOOD, pputz->iFood + (INT)pCel->cFood * FOOD_VALUE);
   pCel->cFood  = 0;
   }


void TurnMove (PPUTZ pputz, PSZ pszAction)
   {
   INT iNewDir, iBurnIncrement;

   iNewDir = Modo (pputz->iDir + pszAction[1] - '0', 8);
   pputz->iDir = iNewDir;

   /*--- harder to move if your fat ---*/
   iBurnIncrement = (pputz->iFood * 3 > MAX_FOOD * 2 ? 2 : 1);

   switch (pszAction[2])
      {
      case 'S': // sit
         pputz->iFood -= iBurnIncrement;
         break;

      case 'E': // eat
         pputz->iFood -= iBurnIncrement;
         Eat (pputz);
         break;

      case 'W': // walk
         pputz->iFood -= 2 * iBurnIncrement;
         Move (pputz);
         break;

      case 'R': // run
         pputz->iFood -= 8 * iBurnIncrement;
         Move (pputz);
         Move (pputz);
         break;
      }
   }


void SetMem (PPUTZ pputz, PSZ pszAction)
   {
   PINT pi;
   INT  iLoc, iVal;
   PSZ  pszTmp;

   iLoc = atoi (pszAction+2);
   pszTmp = strchr (pszAction+2, ':') + 1;
   iVal = atoi (pszTmp);
   pi = pputz->piMem + (iLoc % PUTZ_MEM_SIZE);
   
   switch (pszAction[1])
      {
      case 'S': *pi  = iVal; break;
      case 'I': *pi += iVal; break;
      case 'D': *pi -= iVal; break;
      }
   if (*pi > MAX_MEM_VAL) *pi -= MAX_MEM_VAL;
   if (*pi < 0)           *pi += MAX_MEM_VAL;
   }


void Action (PPUTZ pputz, PSZ pszAction)
   {
   switch (*pszAction)
      {
      case 'M':
         SetMem (pputz, pszAction);
         break;

      case 'T':
         TurnMove (pputz, pszAction);
         break;
      }
   }

/**************************************************************************/

BOOL ObjAtLoc (INT iY, INT iX, CHAR cObj)
   {
   PCEL  pCel;
   PPUTZ pputz;

   pCel = grid[iY] + iX;

   if (cObj == 'f')
      return pCel->cFood;

   for (pputz = pCel->plist; pputz; pputz = pputz->gnext)
      {
      if (pputz->Display[0] == cObj)
         return TRUE;
      }
   return FALSE;
   }


// O<object><position>       - object at pos
BOOL EvalO  (PPUTZ pputz, PSZ psz)
   {
   INT iY, iX;

   GetPos (pputz->iY, pputz->iX, pputz->iDir, psz[2], &iY, &iX);
   return ObjAtLoc (iY, iX, psz[1]);
   }

// F<comparator><n>          - food level
BOOL EvalF  (PPUTZ pputz, PSZ psz)
   {
   INT iVal;

   iVal = atoi (psz+2);
   switch (psz[1])
      {
      case '>': return !!(pputz->iFood > iVal);
      case '<': return !!(pputz->iFood < iVal);
      case '=': return !!(pputz->iFood == iVal);
      }
   return FALSE;
   }

// 1                         - true
BOOL Eval1  (PPUTZ pputz, PSZ psz)
   {
   return TRUE;
   }

// M<comparator><loc>:<value>- memory
BOOL EvalM (PPUTZ pputz, PSZ psz)
   {
   INT iLoc, iVal, iMem;
   PSZ pszTmp;

   iLoc = atoi (psz+2);
   pszTmp = strchr (psz+2, ':') + 1;
   iVal = atoi (pszTmp);

   iMem = pputz->piMem[iLoc % PUTZ_MEM_SIZE];
   switch (psz[1])
      {
      case '>': return !!(iMem > iVal);
      case '<': return !!(iMem < iVal);
      case '=': return !!(iMem == iVal);
      }
   return FALSE;
   }

// @<condition index>        - other condition
BOOL EvalAt (PPUTZ pputz, PSZ psz)
   {
   // not done
   return FALSE;
   }


BOOL EvalCondition (PPUTZ pputz, PSZ psz)
   {
   switch (*psz)
      {
      case 'O': return EvalO  (pputz, psz);
      case 'F': return EvalF  (pputz, psz);
      case '1': return Eval1  (pputz, psz);
      case 'M': return EvalM  (pputz, psz);
      case '@': return EvalAt (pputz, psz);
      case '!': return !EvalCondition (pputz, psz+1);
      }
   return FALSE;
   }

/**************************************************************************/

BOOL bHOLD [PUTZ_GENE_SIZE];

void DoTurn (PPUTZ pputz)
   {
   INT i, iSum, r, iWeight;
   PSZ psz, pszAction;

   memset (bHOLD, 0, PUTZ_GENE_SIZE * sizeof (BOOL));

   for (iSum = i = 0; psz = pputz->ppszGenes[i]; i++)
      {
      if (!EvalCondition (pputz, psz))
         continue;

      SplitGene (psz, &iWeight, &pszAction);

      if (*pszAction == 'M')
         {
         if (Rnd (MAX_WEIGHT) < iWeight)
            Action (pputz, pszAction);
         }
      else
         {
         iSum += iWeight;
         bHOLD[i] = TRUE;
         }
      }
   r = Rnd (iSum);

   for (iSum = i = 0; i < PUTZ_GENE_SIZE; i++)
      {
      if (!bHOLD[i])
         continue;

      SplitGene (pputz->ppszGenes[i], &iWeight, &pszAction);
      iSum += iWeight;

      if (iSum > r)
         break;
      }
   Action (pputz, pszAction);
   }

int cmpputz (const void *p1, const void *p2)
   {
   PPUTZ pp1, pp2;

   pp1 = *(PPUTZ *)p1;
   pp2 = *(PPUTZ *)p2;

   return pp2->iFood - pp1->iFood;
   }


void SortCritters (void)
   {
   PPUTZ pputz, ppputz[CRITTERS];
   INT   i, iCount;

   /*--- order by food ---*/
   for (i=0, pputz = pHead; pputz; pputz = pputz->next, i++)
      ppputz[i] = pputz;
   iCount = i;
   qsort (ppputz, iCount, sizeof (PPUTZ), cmpputz);

   ppputz[0]->prev        = NULL;
   ppputz[iCount-1]->next = NULL;
   for (i=0; i+1<iCount; i++)
      {
      ppputz[i]->next   = ppputz[i+1];
      ppputz[i+1]->prev = ppputz[i];
      }
   pHead = ppputz[0];
   pTail = ppputz[iCount-1];
   }


void DumpData (UINT iKey)
   {
   PPUTZ pputz;
   INT   i, j;

   SortCritters ();
   for (i=0, pputz = pHead; pputz; pputz = pputz->next, i++)
      {
      Log ("%3d) %3d, %5d, %2d, %2d", 
         i, pputz->iIdx, pputz->iFood, pputz->iY, pputz->iX);

      if (iKey == '1')
         for (j=0; pputz->ppszGenes[j]; j++)
            Log ("   %s", pputz->ppszGenes[j]);
      }
   }

void DumpList (void)
   {
   INT   i;
   PPUTZ pputz;

   for (i=0, pputz = pHead; pputz; pputz = pputz->next, i++)
      Log ("DUMP: %2d, %p (%p, %p)", i, pputz, pputz->prev, pputz->next);
   }



void RefreshPopulation (void)
   {
   INT   i, iKeep, iDups;
   PPUTZ pputz, pputzNext;

   SortCritters ();

   /*--- skip by 1st n best critters ---*/
   iKeep = CRITTERS - CRITTER_KILLOFF;
   for (i=0, pputz = pHead; pputz && i < iKeep; pputz = pputz->next, i++)
      ;

   Log ("Refresh: Keep Threshold = %d", pputz->iFood);
   fflush (fpLOG);

DumpList ();

   /*-- kill worst critters ---*/
   for (; pputz; pputz = pputzNext)
      {
      pputzNext = pputz->next;
      KillCritter (pputz);
      }

//DumpList ();
//exit (1);

   /*--- duplicate best critters ---*/
   iDups = CRITTER_KILLOFF - CRITTER_BRANDNEW;

   for (i=0, pputz = pHead; pputz && i < iDups; pputz = pputz->next, i++)
      DivideCritter (pputz); // add to end!

   /*--- a few brand new ones ---*/
   Genesis (CRITTER_BRANDNEW);


DumpList ();
DumpData ('1');
exit (1);
   }


void AddFood (INT uCount)
   {
   INT  i, iY, iX;
   PCEL pCel;

   for (i=0; i<uCount; i++)
      {
      iY = Rnd (GRID_Y_SIZE);
      iX = Rnd (GRID_X_SIZE);

      pCel = grid[iY] + iX;
      pCel->cFood++;

      DisplayLoc (iY, iX);
      }
   }



ULONG Live (void)
   {
   PPUTZ pputz;
   ULONG ulTurns;
   BOOL  bForward = TRUE;

   ulTurns = 0;

   while (!k_kbhit ())
      {
      if (bForward)
         {
         for (pputz = pHead; pputz; pputz = pputz->next)
            {
            DoTurn (pputz);
            }
         }
      else
         {
         for (pputz = pTail; pputz; pputz = pputz->prev)
            {
            DoTurn (pputz);
            }
         }
      bForward = !bForward;

      AddFood (FOOD_PER_TURN);

      ulTurns++;
      if (!(ulTurns % TURNS_PER_YEAR))
         RefreshPopulation ();
      }
   return ulTurns;
   }


void Init (void)
   {
   INT iY, iX;

   LogInit ();

   memset (grid, 0, GRID_Y_SIZE * GRID_X_SIZE * sizeof (CEL));
   for (iY=0; iY < GRID_Y_SIZE; iY++)
      for (iX=0; iX < GRID_X_SIZE; iX++)
         grid[iY][iX].cTerrain = 0x10;

   AddFood (100);
   Genesis (CRITTERS);
   }


int main (int argc, char *argv[])
   {
   INT i;

   ScrInitMetrics ();
   ScrSetIntenseBkg (TRUE);

   ScrShowCursor (FALSE);
   ScrClear (0, 0, 50, 80, ' ', 0x1F);

   Init ();
   Live ();
   ScrShowCursor (TRUE);
   i = KeyGet (TRUE);
   DumpData (i);

   fclose (fpLOG);

   return 0;
   }

