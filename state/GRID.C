/*
 *
 * grid.c
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


INT   iGRID_Y_SIZE = 50;   // Y size of grid
INT   iGRID_X_SIZE = 80;   // X size of grid
PCEL  pgrid        = NULL; // the grid

INT   iY_BORDER;           // display metric
INT   iX_BORDER;           // display metric
INT   iY_ANCHOR;           // display metric
INT   iX_ANCHOR;           // display metric
PMET  pmet;                // display metrics

BOOL bDISPLAY = TRUE;             // display mode setting


/***************************************************************************/
/*                                                                         */
/* Display functions                                                       */
/*                                                                         */
/***************************************************************************/


void PtoS (INT iYp, INT iXp, PINT piYs, PINT piXs)
   {
   if (piYs) *piYs = (iYp + iGRID_Y_SIZE - iY_ANCHOR) % iGRID_Y_SIZE;
   if (piXs) *piXs = (iXp + iGRID_X_SIZE - iX_ANCHOR) % iGRID_X_SIZE;
   }

void StoP (INT iYs, INT iXs, PINT piYp, PINT piXp)
   {
   if (piYp) *piYp = (iYs + iY_ANCHOR) % iGRID_Y_SIZE;
   if (piXp) *piXp = (iXs + iX_ANCHOR) % iGRID_X_SIZE;
   }

void SetAnchor (INT iYRel, INT iXRel)
   {
   iY_ANCHOR = (iY_ANCHOR + iGRID_Y_SIZE + iYRel) % iGRID_Y_SIZE;
   iX_ANCHOR = (iX_ANCHOR + iGRID_X_SIZE + iXRel) % iGRID_X_SIZE;
   }

void InitScreenMetrics (void)
   {
   iY_ANCHOR = 0;
   iX_ANCHOR = 0;
   iY_BORDER = min ((INT)pmet->uYScrSize, iGRID_Y_SIZE) -2; // reserve bottom
   iX_BORDER = min ((INT)pmet->uXScrSize, iGRID_X_SIZE) -1;
   }

/*
 * uses grid coordinates
 */
void DisplayLoc (INT iY, INT iX)
   {
   BYTE *pb, p[2];
   PCEL pCel;
   INT  iYs, iXs;

   if (!bDISPLAY)
      return;

   pb = p;
   pCel = GetCell (iY, iX);
   PtoS (iY, iX, &iYs, &iXs);
   if (iYs > iY_BORDER || iXs > iX_BORDER)
      return;

   if (pCel->plist)
      {
      pb = pCel->plist->Display;
      pb[1] = (pb[1] & 0x0F) + pCel->cTerrain;
      }
   else
      {
      p[0] = (pCel->iFlora ? '.' : ' ');
      p[1] = pCel->cTerrain + 0x07;
      }
   ScrWriteNCell (pb, 1, iYs, iXs);
   }


/*
 * uses screen coordinates
 */
void Refresh (INT iYMin, INT iXMin, INT iYMax, INT iXMax)
   {
   INT iYs, iXs, iYp, iXp;

   for (iYs=iYMin; iYs<=iYMax; iYs++)
      for (iXs=iXMin; iXs<=iXMax; iXs++)
         {
         StoP (iYs, iXs, &iYp, &iXp);
         DisplayLoc (iYp, iXp);
         }
   }



BOOL MovementKey (INT c)
   {
   switch (c)
      {
      case 0x148: // up
         SetAnchor (-1, 0);
         ScrScrollDown (0, 0, iY_BORDER, iX_BORDER, 1, 0x17);
         Refresh (0, 0, 0, iX_BORDER);
         break;

      case 0x150: // dn
         SetAnchor (1, 0);
         ScrScrollUp (0, 0, iY_BORDER, iX_BORDER, 1, 0x17);
         Refresh (iY_BORDER, 0, iY_BORDER, iX_BORDER);
         break;

      case 0x14B: // lt
         SetAnchor (0, -1);
         ScrScrollLR (0, 0, iY_BORDER, iX_BORDER, 1, FALSE);
         Refresh (0, 0, iY_BORDER, 0);
         break;

      case 0x14D: // rt
         SetAnchor (0, 1);
         ScrScrollLR (0, 0, iY_BORDER, iX_BORDER, 1, TRUE);
         Refresh (0, iX_BORDER, iY_BORDER, iX_BORDER);
         break;

      case 0x149: // pgup
         SetAnchor (-iY_BORDER-1, 0);
         Refresh (0, 0, iY_BORDER, iX_BORDER);
         break;
         
      case 0x151: // pgdn
         SetAnchor (iY_BORDER+1, 0);
         Refresh (0, 0, iY_BORDER, iX_BORDER);
         break;
         
      case 0x173: // ctl lt
         SetAnchor (0, -iX_BORDER-1);
         Refresh (0, 0, iY_BORDER, iX_BORDER);
         break;

      case 0x174: // ctl rt
         SetAnchor (0, iX_BORDER+1);
         Refresh (0, 0, iY_BORDER, iX_BORDER);
         break;

      default : return FALSE;
      }
   return TRUE;
   }



PCEL GetCell (INT iY, INT iX)
   {
   if (iY >= iGRID_Y_SIZE || iX >= iGRID_X_SIZE)
      return NULL;

   return pgrid + (iY * iGRID_X_SIZE + iX);
   }


void UpdateStatus (void)
   {
   }


/***************************************************************************/



void GridRemove (PORG porg)
   {
   PCEL pCel;

   pCel = GetCell (porg->iY, porg->iX);

   if (!pCel->plist)
      Error ("grid location is empty (remove %p [%d,%d])", porg, porg->iY, porg->iX);
   if (porg->gnext)
      porg->gnext->gprev = porg->gprev;

   if (porg->gprev)
      porg->gprev->gnext = porg->gnext;
   else
      pCel->plist = porg->gnext;

   porg->gprev = NULL;
   porg->gnext = NULL;
   DisplayLoc (porg->iY, porg->iX);
   }


void GridAdd (PORG porg)
   {
   PCEL pCel;

   pCel = GetCell (porg->iY, porg->iX);

   porg->gprev = NULL;
   porg->gnext = pCel->plist;

   if (pCel->plist)
      pCel->plist->gprev = porg;
   pCel->plist  = porg;

   DisplayLoc (porg->iY, porg->iX);
   }


