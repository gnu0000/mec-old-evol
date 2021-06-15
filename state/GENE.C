/*
 *
 * gene.c
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

/***************************************************************************/
/*                                                                         */
/* Util Functions                                                          */
/*                                                                         */
/***************************************************************************/

void SplitGene (PSZ psz, PINT piWeight, PPSZ ppszAction)
   {
   PSZ p;

   if (!(p = strchr (psz, ';')))
      Error ("Bad Gene String: %s", psz);
   *ppszAction = p + 1;
   *piWeight = atoi (*ppszAction );

   if (!(p = strchr (*ppszAction, ';')))
      Error ("Bad Gene String: %s", psz);
   *ppszAction = p + 1;
   }


static void _movpos (INT y, INT x, INT iDir, PINT py, PINT px)
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
      default:
         Error ("Unknown direction: %d", iDir);
      }
   if (y < 0) y += iGRID_Y_SIZE;
   if (x < 0) x += iGRID_X_SIZE;
   *py = y % iGRID_Y_SIZE;
   *px = x % iGRID_X_SIZE;
   }


static void GetPos (INT iY, INT iX, INT iDir, CHAR cPos, PINT py, PINT px)
   {
   if (cPos == 'X')
      {
      *py = iY;
      *px = iX;
      return;
      }
   if (cPos < 'A')
      {
      iDir = (iDir + 8 + cPos - '0') % 8;
      _movpos (iY, iX, iDir, py, px);
      return;
      }
   _movpos (iY, iX, iDir, py, px);
   _movpos (*py, *px, iDir, py, px);

   if (cPos == 'A')
      _movpos (*py, *px, (iDir + 8 - 2) % 8, py, px);
   else if (cPos == 'C')
      _movpos (*py, *px, (iDir + 8 + 2) % 8, py, px);
   }


/***************************************************************************/
/*                                                                         */
/* Gene Action Functions                                                   */
/*                                                                         */
/***************************************************************************/

static void Move (PORG porg)
   {
   INT y, x;

   _movpos (porg->iY, porg->iX, porg->iDir, &y, &x);
   GridRemove (porg);
   porg->iY = y;
   porg->iX = x;
   GridAdd (porg);
   }


static void Eat (PORG porg)
   {
   PCEL  pCel;
   PADAT pa;
   PORG  porgTmp;
   INT   i;

   pa   = porg->pa;
   pCel = GetCell (porg->iY, porg->iX);

   /*--- edible fauna? ---*/
   for (porgTmp=pCel->plist; porgTmp; porgTmp=porgTmp->gnext)
      if (!strchr (pa->pszDiet, porgTmp->cSymbol))
         {
         porg->iFood += porgTmp->pa->iCalorieValue + porgTmp->iFood;
         porg->iFood = min (porg->iFood, pa->iMaxCalories);
         KillCritter (porgTmp);
         return; // burp!
         }

   /*-- edible flora? ---*/
   for (i=0; pCel->iFlora && i<iLCOUNT; i++)  
      if ((pCel->iFlora & (1 << i)) && !strchr (pa->pszDiet, pFlora[i].cSymbol))
         {
         porg->iFood += pFlora[i].iCalorieValue;
         porg->iFood = min (porg->iFood, pa->iMaxCalories);
         pCel->iFlora &= ~(1 << i);
         return; // burp!
         }
   }


static void Burn (PORG porg, CHAR cAction)
   {
   INT   iPct;
   PADAT pa;

   pa = porg->pa;

   /*--- harder to move if your fat ---*/
   iPct = (porg->iFood > 0 ? ((porg->iFood*10-1)/pa->iMaxCalories) : 0);

   pa->iWeightBurn

   switch (cAction)
      {
      case 'E': iActionIdx = 0;  break; // eat  
      case 'W': iActionIdx = 1;  break; // walk 
      case 'R': iActionIdx = 2;  break; // run  
      default : iActionIdx = 0;  break; // sit  
      }
   porg->iFood -= pa->iWeightBurn[iPct] * pa->iActionBurn[iActionIdx];

   porg->iFood = max (porg->iFood, pa->iMinCalories);


/******************************************************/
/******************************************************/
/* kill critter if he reaches min calories            */
/* maybe not here but in turn loop ?                  */
/******************************************************/
/******************************************************/
   }


static void TurnMove (PORG porg, PSZ pszAction)
   {
   INT iNewDir;

   iNewDir = (porg->iDir + 8 + pszAction[1] - '0') % 8;
   porg->iDir = iNewDir;

   switch (pszAction[2])
      {
      case 'S': // sit
         break;

      case 'E': // eat
         Eat (porg);
         break;

      case 'W': // walk
         Move (porg);
         break;

      case 'R': // run
         Move (porg);
         Move (porg);
         break;

      default:
         Error ("Unknown movement: %s", pszAction);
      }
   Burn (porg, pszAction[2]);
   }


static void SetMem (PORG porg, PSZ pszAction)
   {
   PINT pi;
   INT  iLoc, iVal;
   PSZ  pszTmp;
   BOOL bIndirect;
   PADAT pa;

   pa = porg->pa;

   bIndirect = (pszAction[1] == 'I');
   iLoc = atoi (pszAction+3);
   if (bIndirect)
      iLoc = porg->piMem[iLoc % pa->iMemSize];

   pi = porg->piMem + (iLoc % pa->iMemSize);
   if (!(pszTmp = strchr (pszAction+3, ':')))
      Error ("Bad Gene Segment: %s", pszTmp);
   iVal = atoi (pszTmp+1);

   switch (pszAction[2])
      {
      case 'S': *pi  = iVal; break;
      case 'I': *pi += iVal; break;
      case 'D': *pi -= iVal; break;

      default:
         Error ("Unknown mem mode: %s", pszAction);
      }
   if (*pi > iMAX_MEM_VAL) *pi -= iMAX_MEM_VAL;
   if (*pi < 0)            *pi += iMAX_MEM_VAL;
   }


void Action (PORG porg, PSZ pszAction)
   {
   switch (*pszAction)
      {
      case 'M':
         SetMem (porg, pszAction);
         break;

      case 'T':
         TurnMove (porg, pszAction);
         break;

      case ' ':
         Burn (porg, ' '); // no action = sit
         break;

      default:
         Error ("Unknown action: %s", pszAction);
      }
   }


/***************************************************************************/
/*                                                                         */
/* Gene Eval Functions                                                     */
/*                                                                         */
/***************************************************************************/


static BOOL ObjAtLoc (PADAT pa, INT iY, INT iX, CHAR cObj, CHAR cExtra)
   {
   PCEL  pCel;
   PORG porg;
   PADAT pa;

   pa = porg->pa;

   pCel = GetCell (iY, iX);

   switch (cObj)
      {
      case '1': // anything         
         return !!(pCel->iFlora || pCel->plist);

      case '0': // nothing         
         return !(pCel->iFlora || pCel->plist);

      case 'F': // any flora
         return !!(pCel->iFlora);

      case 'P': // any fauna
         return !!(pCel->iFlora || pCel->plist);

      case 'L': // any edible          
         /*-- edible flora? ---*/
         for (i=0; pCel->iFlora && i<iLCOUNT; i++)  
            if ((pCel->iFlora & (1 << i)) && !strchr (pa->pszDiet, pFlora[i].cSymbol))
               return TRUE;

         /*--- edible fauna? ---*/
         for (porg=pCel->plist; porg; porg=porg->gnext)
            if (!strchr (pa->pszDiet, porg->cSymbol))
               return TRUE;
         return FALSE;

      case 'A': // any preditor         
         for (porg=pCel->plist; porg; porg=porg->gnext)
            if (!strchr (porg->pa->pszDiet, pa->cSymbol))
               return TRUE;
         return FALSE;

      case '~': // specific flora/fauna 
         /*-- flora? ---*/
         for (i=0; pCel->iFlora && i<iLCOUNT; i++)  
            if ((pCel->iFlora & (1 << i)) && (pFlora[i].cSymbol == cExtra))
               return TRUE;

         /*--- fauna? ---*/
         for (porg=pCel->plist; porg; porg=porg->gnext)
            if (porg->cSymbol == cExtra)
               return TRUE;
         return FALSE;

      default:
         Error ("Unknown Object: %c Current Object: %s", cObj, pa->pszName);
      }
   }


/*
 * O<object><position> - object at pos
 *
 */
static BOOL EvalO  (PORG porg, PSZ psz)
   {
   INT iY, iX;

   GetPos (porg->iY, porg->iX, porg->iDir, psz[2], &iY, &iX);
   return ObjAtLoc (porg->pa, iY, iX, psz[1], psz[2]);
   }


/*
 * F<comparator><n> - food level
 *
 */
static BOOL EvalF  (PORG porg, PSZ psz)
   {
   INT   iVal, iPct;
   PADAT pa;

   pa = porg->pa;

   iVal = atoi (psz+2);
   iPct = (porg->iFood > 0 ? ((porg->iFood*10-1)/pa->iMaxCalories) : 0);

   switch (psz[1])
      {
      case '>': return !!(iPct >  iVal);
      case '<': return !!(iPct <  iVal);
      case '=': return !!(iPct == iVal);
      case 'S': return !!(porg->iFood > pa->iMateCalorieLimit);
      default :
         Error ("Unknown calorie comparison: %s", psz);
      }
   }


/*
 * 1 - true
 *
 */
static BOOL Eval1  (PORG porg, PSZ psz)
   {
   return TRUE;
   }


/*
 * M<Direct/Indirect><comparator><loc>:<value>- memory
 *
 */
static BOOL EvalM (PORG porg, PSZ psz)
   {
   INT  iLoc, iVal, iMem;
   PSZ  pszTmp;
   BOOL bIndirect;
   PADAT pa;

   pa = porg->pa;
   bIndirect = (psz[1] == 'I');
   iLoc = atoi (psz+3);
   if (bIndirect)
      iLoc = porg->piMem[iLoc % pa->iMemCount];

   iMem = porg->piMem[iLoc % pa->iMemCount];

   if (!(pszTmp = strchr (psz+3, ':')))
      Error ("Bad Gene Segment: %s", psz);
   iVal = atoi (pszTmp+1);

   switch (psz[2])
      {
      case '>': return !!(iMem > iVal);
      case '<': return !!(iMem < iVal);
      case '=': return !!(iMem == iVal);
      default :
         Error ("Unknown operator: %s", psz);
      }
   return FALSE;
   }


BOOL EvalCondition (PORG porg, PSZ psz)
   {
   switch (*psz)
      {
      case 'O': return EvalO  (porg, psz);
      case 'F': return EvalF  (porg, psz);
      case '1': return Eval1  (porg, psz);
      case 'M': return EvalM  (porg, psz);
      case '!': return !EvalCondition (porg, psz+1);
      default :
         Error ("Unknown condition: %s", psz);
      }
   return FALSE;
   }



/***************************************************************************/
/*                                                                         */
/* Gene Eval Functions                                                     */
/*                                                                         */
/***************************************************************************/


static PSZ AddRnd (PSZ psz, UINT uVal)
   {
   return psz + sprintf (psz, "%d", Rnd(uVal));
   }

static CHAR RandomChar (PSZ psz)
   {
   return psz [Rnd(strlen (psz))];
   }


CHAR szEXISTINGOBJECTS [64];

void InitObjects (void)
   {
   j = 0;

   for (i=0; i<iLCOUNT; i++)
      szEXISTINGOBJECTS [j++] = pFlora[i].cSymbol;
   for (i=0; i<iACOUNT; i++)
      szEXISTINGOBJECTS [j++] = pFauna[i].cSymbol;

   szEXISTINGOBJECTS [j] = '\0';
   }

CHAR szCONDS  [] = "OF1M";
CHAR szOBJS   [] = "10FPLA~";
CHAR szPOS    [] = "01234567ABCFLRX";
CHAR szCMP    [] = "><=";
CHAR szACTION [] = "MT";
CHAR szSETTYP [] = "SID";
CHAR szMOVTYP [] = "SEWR";
CHAR szMMODE  [] = "DI";


PSZ AddRandomObject (PSZ psz)
   {
   *psz++ = c = szOBJS [Rnd (strlen(szOBJS))];

   if (c == '*')
      *psz++ = szEXISTINGOBJECTS [Rnd (strlen(szEXISTINGOBJECTS))];

   return psz;
   }


static PSZ AddCondition (PADAT pa, PSZ psz)
   {
   BOOL bNot;

   if (bNot = Rnd (2))
      *psz++ = '!';

   switch (*psz++ = RandomChar (szCONDS))
      {
      case 'O': // object at position condition
         *psz++ = c = RandomChar (szOBJS);
         if (c == '~')
            *psz++ = RandomChar (szEXISTINGOBJECTS);
         *psz++ = szPOS[(Rnd(2) ? Rnd(15) : 11+Rnd(4))];
         break;

      case 'F': // food level condition
         if (!Rnd(4))
            {
            *psz++ = 'S'; // enough food to spawn (mate) condition
            }
         else
            {
            *psz++ = RandomChar (szCMP);
            psz    = AddRnd (psz, 10); food level in 1/10 unit
            }
         break;
         
      case '1': // TRUE
         if (bNot) // !1 condition not useful (not TRUE)
            {
            psz -= 2;
            *psz++ = '1';
            }
         break;

      case 'M': // memory comparison
         *psz++ = RandomChar (szMMODE);
         *psz++ = RandomChar (szCMP);
         psz    = AddRnd (psz, iORG_MEM_SIZE);
         *psz++ = ':';
         psz    = AddRnd (psz, iMAX_MEM_VAL);
         break;

      default:
         Error ("Unknown condition: %s", psz);
      }
   return psz;
   }


static PSZ AddAction (PADAT pa, PSZ psz)
   {
   switch (*psz++ = szACTION[Rnd(2)])
      {
      case 'M': // set memory
         *psz++ = RandomChar (szMMODE);
         *psz++ = RandomChar (szSETTYP);
         psz    = AddRnd (psz, iORG_MEM_SIZE);
         *psz++ = ':';
         psz    = AddRnd (psz, iMAX_MEM_VAL);
         break;

      case 'T': // run/walk/sit/eat
         *psz++ = szPOS[Rnd(8)];
         *psz++ = RandomChar (szMOVTYP);
         break;

      default:
         Error ("Unknown action: %s", psz);
      }
   return psz;
   }


static PSZ NewGene (PADAT pa, PSZ pszNew)
   {
   PSZ  psz;

   psz = AddCondition (pa, pszNew);

   /*--- add weight ---*/
   *psz++ = ';';
   psz = AddRnd (psz, iMAX_WEIGHT);

   *psz++ = ';';
   psz = AddAction (pa, psz);

   *psz = '\0';
   return pszNew;
   }



static PSZ MutateGene (PADAT pa, PSZ pszGene)
   {
   SplitGene (pszGene, &iWeight, &pszAction);
   switch (Rnd (3))
      {
      case 0: // new condition
         psz = AddCondition (pa, szTmp);
         if (!(pszTmp = strchr (pszGene, ';')))
            Error ("Bad Gene String: %s", pszGene);
         strcpy (psz, pszTmp);
         break;

      case 1: // new weight
         strcpy (szTmp, pszGene);
         if (!(psz = strchr (szTmp, ';')))
            Error ("Bad Gene String: %s", szTmp);
         psz = AddRnd (psz + 1, iMAX_WEIGHT);
         *psz++ = ';';
         strcpy (psz, pszAction);
         break;

      case 2: // new action
         strcpy (szTmp, pszGene);
         if (!(psz = strrchr (szTmp, ';')))
            Error ("Bad Gene String: %s", szTmp);
         psz = AddAction (pa, psz+1);
         *psz = '\0';
         break;
      }
   strcpy (pszGene, szTmp);
   return pszGene;
   }


// modify or replace gene
static PSZ CosmicRay_G (PORG porg)
   {
   PADAT pa;

   pa = porg->pa;
   if (Rnd (iMAX_WEIGHT) < pa->iMutationRates[0]) // minor mutation
      MutateGene (pa, porg->ppszGenes[Rnd(pa->iGeneCount)]);

   if (Rnd (iMAX_WEIGHT) < pa->iMutationRates[1]) // major mutation
      NewGene (pa, porg->ppszGenes[Rnd(pa->iGeneCount)]);

   if (Rnd (iMAX_WEIGHT) < pa->iMutationRates[2]) // scramble!
      {
      for (i=0; i<pa->iGeneCount; i++)
         MutateGene (pa, porg->ppszGenes[Rnd(pa->iGeneCount)]);
      NewGene (pa, porg->ppszGenes[Rnd(pa->iGeneCount)]);
      NewGene (pa, porg->ppszGenes[Rnd(pa->iGeneCount)]);
      }
   }


/*
 * used by qsort to put movement genes above mem set genes
 */
int CCONV cmpgene (const void *p1, const void *p2)
   {
   PSZ psz1, psz2, pszA1, pszA2;
   INT iWeight;

   psz1 = *(PPSZ)p1;
   psz2 = *(PPSZ)p2;

   SplitGene (psz1, &iWeight, &pszA1);
   SplitGene (psz2, &iWeight, &pszA2);

   return (INT)*pszA2 - (INT)*pszA1;
   }


/*
 * creates genes for pChild
 * pChild->pa must be already set!
 */
void NewGenes (PORG pChild, PORG pMom, PORG pDad)
   {
   INT  i;
   PADAT pa;

   pa = pChild->pa;

   if (pMom)
      {
      for (i=0; i<pa->iGeneCount; i++)
         strcpy (pChild->ppszGenes[i], (Rnd(2) ? pMom->ppszGenes[i], pDad->ppszGenes[i]);
      CosmicRay_G (pChild);
//    qsort (pChild->ppszGenes, iORG_GENE_SIZE, sizeof (PSZ), cmpgene);
      }
   else
      {
      for (bLook=TRUE, i=0; i<pa->iGeneCount; i++)
         {
         if (bLook && pa->ppszInstincts && pa->ppszInstincts[i])
            {
            strcpy (pChild->ppszGenes[i], pa->ppszInstincts[i]);
            }
         else
            {
            bLook = FALSE;
            NewGene (pa, pChild->ppszGenes[i]);
            }
//    qsort (pChild->ppszGenes, iORG_GENE_SIZE, sizeof (PSZ), cmpgene);
      }
   return ppszNew;
   }

/***************************************************************************/

static void CosmicRay_M (PORG pChild)
   {
   INT   iLoc, iVal;
   PADAT pa;

   pa = pChild->pa;

   if (Rnd (iMAX_WEIGHT) < pa->iMutationRates[0]) // minor mutation
      {
      iLoc = Rnd (pa->iMemCount);
      pChild->piMem[iLoc] += Rnd (pChild->piMem[iLoc]/5) - pChild->piMem[iLoc]/10;
      }
   if (Rnd (iMAX_WEIGHT) < pa->iMutationRates[1]) // major mutation
      {
      for (i=0; i < pa->iMemSize/16; i++)
         {
         iLoc = Rnd (pa->iMemSize);
         pChild->piMem[iLoc] = Rnd (iMAX_MEM_VAL);
         }
      }
   if (Rnd (iMAX_WEIGHT) < pa->iMutationRates[2]) // scramble!
      {
      for (i=0; i<pa->iMemSize; i++)
         pChild->piMem[i] = Rnd (iMAX_MEM_VAL);
      }
   }

/*
 * creates starting memory for a child
 * pChild->pa must be already set!
 */
void NewMem (PORG pChild, PORG pMom, PORG pDad)
   {
   INT   i;
   PADAT pa;

   pa = pChild->pa;

   if (piMom)
      {
      for (i=0; i<pa->iMemCount; i++)
         pChild->piMem[i] = (Rnd (2) ? pMom->piMem[i] : pDat->piMem[i]);
      CosmicRay_M (pChild);
      }
   else // new mem from scratch
      {
      for (i=0; i<pa->iMemCount; i++)
         pChild->piMem[i] = Rnd (iMAX_MEM_VAL);
      }
   }


