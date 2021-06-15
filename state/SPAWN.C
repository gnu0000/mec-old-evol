//void RefreshPopulation (void)
//   {
//   INT   i, j, iKeep, iDups, iTotalFood;
//   PORG porg, porgNext, pOldHead;
//
//   pOldHead = pHead;
//   SortCritters ();
//
//   /*--- skip by 1st n best critters ---*/
//   iKeep = iCRITTERS - iCRITTER_KILLOFF;
//   for (i=0, porg = pHead; porg && (i < iKeep); porg = porg->next, i++)
//      ;
//
//   /*--- Log info ---*/
//   for (j=iGRID_Y_SIZE*iGRID_X_SIZE, iTotalFood = 0; j; j--)
//      iTotalFood += pgrid[j].iFlora;
//
//   Log ("Refresh: Keep Threshold = %4d, total food = %d %c", 
//         porg->iFood, iTotalFood, (pOldHead==pHead ? '.': 'x'));
//   if (!bDISPLAY)
//      printf ("Year = %4ld,  Keep Threshold = %4d, total food = %d %c\n", 
//               ulYEARS, porg->iFood, iTotalFood, (pOldHead==pHead ? '.': 'x'));
//
//   /*-- kill worst critters ---*/
//   for (; porg; porg = porgNext)
//      {
//      porgNext = porg->next;
//      KillCritter (porg);
//      }
//
//   /*--- duplicate best critters ---*/
//   iDups = iCRITTER_KILLOFF - iCRITTER_BRANDNEW;
//   for (i=0, porg = pHead; porg && i < iDups; porg = porg->next, i++)
//      DivideCritter (porg); // add to end!
//
//   /*--- a few brand new ones ---*/
//   Genesis (iCRITTER_BRANDNEW);
//
//   /*--- color top 3 ---*/
//   ColorCritters ();
//   }



void DivideCritter (PORG porgOld)
   {
   PORG porg;

   porg = NewCell ();

   porg->iIdx       = porgOld->iIdx;
   porg->Display[0] = porgOld->Display[0];
   porg->Display[1] = porgOld->Display[1];
   porg->iDir       = Rnd(8);
   porg->iFood      = porgOld->iFood;

   porg->iY = (porgOld->iY + iGRID_Y_SIZE + Rnd(3) - 1) % iGRID_Y_SIZE;
   porg->iX = (porgOld->iX + iGRID_X_SIZE + Rnd(3) - 1) % iGRID_X_SIZE;

   NewGenes (porg->ppszGenes, porgOld->ppszGenes);
   NewMem (porg->piMem, porgOld->piMem);
   GridAdd (porg);
   }



void KillCritter (PORG porg);
   {
   GridRemove (porg);

   if (porg->next)
      porg->next->prev = porg->prev;
   else
      pTail = porg->prev;

   if (porg->prev)
      porg->prev->next = porg->next;
   else
      pHead = porg->next;

   porg->prev = porg->next = NULL; // not needed

   AddToFreePool (porg);
   }



//void Genesis (INT iCount)
void Genesis (PADAT pa)
   {
   INT  i;
   PORG porg;

   for (i=0; i<iCount; i++)
      {
      porg = NewCell ();
      porg->iIdx = i;
      porg->Display[0] = 'p';
      porg->Display[1] = 0x0F;

      porg->iX    = Rnd(iGRID_X_SIZE);
      porg->iY    = Rnd(iGRID_Y_SIZE);
      porg->iDir  = Rnd(8);
      porg->iFood = 500;

      NewGenes (porg->ppszGenes, NULL);
      NewMem   (porg->piMem    , NULL);
      GridAdd (porg);
      }
   }



void Spawn (PADAT pa)
   {
   }


/***************************************************************************/
/*                                                                         */
/* Creation Functions                                                      */
/*                                                                         */
/***************************************************************************/



static PORG NewCell (void)
   {
   PORG porg;

   porg = GetFromFreePool ();

   if (!pHead)
      {
      pHead = pTail = porg;
      }
   else
      {
      pTail->next = porg;
      porg->prev = pTail;
      porg->next = NULL;
      pTail = porg;
      }
   return porg;
   }
