/*
 *
 * critters.h
 * Friday, 6/27/1997.
 *
 */

#define iMAX_WEIGHT      1000     // max weighting for all chance vars (.1%)
#define ABS_MAX_STATES   256      // absolute largest # of states


typedef struct 
   {
   CHAR  szCond[6];     // condition (or action)
   UCHAR uTrue;         // next state index if cond is true
   UCHAR uFalse;        // next state index if cond is false
   } ST, *PST;          

/*
 * This structure contains information about an
 * individual critter
 */
typedef struct _org
   {
   INT    iIdx;         // body index  ???used???
   INT    iX, iY;       // critter location in grid
   INT    iDir;         // current direction
   INT    iFood;        // food level
   ULONG  ulAge;        // age

   CHAR   cSymbol;      // display symbol
   CHAR   cColor;       // display color
   CHAR   bSex;         // critter's sex

   PST    pStates;      // state list
   UCHAR  uState        // current state
   PULONG pulChoices;   // state chosen count (for reporting)

   struct _adat *pa;    // species metadata

   struct _org *prev;   // list of all critters of this type
   struct _org *next;   //

   struct _org *gnext;  // list of all critters at this grid location
   struct _org *gprev;  //
   } ORG, *PORG;


/*
 * This structure contains information about a
 * critter species
 */
typedef struct _adat
   {
   PSZ   pszName;          // organism type name
   CHAR  cSymbol;          // display symbol
   CHAR  cColor[2];        // display character color for males/females
   PSZ   pszDiet;          // ordered by preference
   ULONG ulMaxAge;         // max age 
   INT   iCalorieValue;    // calorie value of this organism type
   INT   iMaxCalories;     // stomach room
   INT   iMinCalories;     // critter dies if below this value
   INT   iStateCount;      // number of states
   INT   iStartPop;        // original number of critters
   INT   iStartCalorieVal; // amount of food in new critters stomachs
   INT   iKidBirthCount;   // # kids generated in mating
   INT   iKidFoodXfer;     // ammt (/1000 %) of food transferred to kids
   INT   iKidChanceFemale; // chance (/1000 %) of child being female
   INT   iActionBurn [3];  // food burn weight depending on action
   INT   iMutationRates[3];// chance (/1000) of minor gene mutation
   PPSZ  ppszInstincts;    // default states for starting population

   ULONG ulYEARS;          // # cycles for this species ?
   PORG  pHead;            // list of current living critters of this type
   PORG  pTail;            //
   } ADAT;
typedef ADAT *PADAT;


/*
 * This structure contains information about a
 * specific species of edible plants
 */
typedef struct
   {
   PSZ   pszName;       // organism type name
   CHAR  cSymbol;       // Display symbol
   CHAR  cColor;        // display character color for males/females
   INT   iStartCount;   // # food elements to place at init time
   INT   iCalorieValue; // calorie value of 1 food unit
   INT   iGrowthPerTurn;// # of units grown per turn
   BOOL  bConc;         // TRUE if food has a concentration area
   INT   iConcRadius;   // radius of concentration area
   INT   iConcWeight;   // (/1000 %) of all food sprouts up in conc area
   INT   iConcY;        // Y ctr of concentration area
   INT   iConcX;        // X ctr of concentration area
   } LDAT;
typedef LDAT *PLDAT;



/*
 * This structure stores information about a single
 * location in the world grid - it is alloced in a 2d array
 */
typedef struct _cell
   {
   char cTerrain;       // terrain type (currently only used for color)
   char iFlora;         // food types here
   struct _org *plist;  // list of critters at this location
   } CEL;
typedef CEL *PCEL;


extern ULONG ulTURNS;    // age of universe in turns
extern INT   iDELAY ;    // delay between turns
extern CHAR  sz[];       // temp buffer
extern PADAT pFauna;     // all animals
extern PLDAT pFlora;     // all plants

extern INT   iLCOUNT;    // # flora species
extern INT   iACOUNT;    // # fauna species

extern INT   iGRID_Y_SIZE; // Y size of grid
extern INT   iGRID_X_SIZE; // X size of grid
extern PCEL  pgrid;        // the grid

extern INT   iY_BORDER;    // display metric
extern INT   iX_BORDER;    // display metric
extern INT   iY_ANCHOR;    // display metric
extern INT   iX_ANCHOR;    // display metric
extern PMET  pmet;         // display metrics

extern BOOL  bDISPLAY;     // display mode setting

extern FILE *fpLOG;        // log file handle

extern CHAR szINFILE [];   // input file name
extern CHAR szOUTFILE[];   // output file name
extern CHAR szLOGFILE[];   // log file name
extern CHAR szCFGFILE[];   // cfg file name

/***************************************************************************/

PPSZ NewGenes (PPSZ ppszNew, PPSZ ppszOld);

void Action (PORG porg, PSZ pszAction);

BOOL EvalCondition (PORG porg, PSZ psz);

/***************************************************************************/

void PtoS (INT iYp, INT iXp, PINT piYs, PINT piXs);

void StoP (INT iYs, INT iXs, PINT piYp, PINT piXp);

void SetAnchor (INT iYRel, INT iXRel);

void InitScreenMetrics (void);

void DisplayLoc (INT iY, INT iX, UINT);

void Refresh (INT iYMin, INT iXMin, INT iYMax, INT iXMax);

BOOL MovementKey (INT c);

PCEL GetCell (INT iY, INT iX);

void GridRemove (PORG porg);

void GridAdd (PORG porg);

void UpdateStatus (void);

/***************************************************************************/

void ReadCfg (PSZ pszFile);

INT  ReadCritters (PSZ pszFile);

void WriteCritters (PSZ pszFile);

/***************************************************************************/

PORG MemNewOrg (void);

void MemDeleteOrg (PORG porg);

void MemFreeMemoryPool (void);

/***************************************************************************/

void SortCritters (void);

void ColorCritters (void);

//PINT NewMem (PINT piNew, PINT piOld);


/***************************************************************************/

void LogInit (void);

UINT CCONV Log (PSZ psz, ...);

void SetPtrs (PSZ pszStr, PPSZ ppszPtrs);

BOOL InCircle (INT yCtr, INT xCtr, INT iRad, INT yPos, INT xPos);

/***************************************************************************/

void Genesis (PADAT pa);

void Spawn (PADAT pa);

void KillCritter (PORG porg);

void DivideCritter (PORG porgOld);

void SplitGene (PSZ psz, PINT piWeight, PPSZ ppszAction);
