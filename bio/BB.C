// bears move blocks if it can
// bears fight and one with most health wins
// bunnies dont move objects
// bunnies dont fight
// bunnies have infinite health
//
//
//

#define SPACE    0
#define BLOCK    1
#define RABBIT   2
#define BEAR     3

//  432
//  5 1  directions
//  678


typedef struct
   {
   CHAR   cObjType     ; //type of object to look for         (4)
   CHAR   cDistance    ; //distance to look for object        (3)
   CHAR   cDirection   ; //direction of object that was found (9)  (108)
   CHAR   cThinkChance ; // % chance to think
   CHAR   cUp          ; // chance to move up
   CHAR   cDown        ; // chance to move down
   CHAR   cRight       ; // chance to move right
   CHAR   cLeft        ; // chance to move left
   USHORT uThinkIndex  ; // next neuron
   } NEURON;
typedef NEURON *PNEURON;


typedef struct
   {
   CHAR     cXPos;         // current position
   CHAR     cYPos;         //
   USHORT   uHealth;       // current health points
   USHORT   uNeurons;      // # neurons
   USHORT   uMoves;        // # moves made
   USHORT   uThoughtTime;  //max 50 thoughts until a move is forced
   PNEURON  pn;
   } ANIMAL;
typedef ANIMAL *PANIMAL;


CreateAnimal (PANIMAL pa)
   {
   pa->pn = malloc (sizeof (NEURON) * 108);
   pa->uNeurons = 108;
   i=0;
   for (a=0; a<4; a++)
      for (b=1; b<4; b++)
         for (c=1; c<9; c++)
            pa->pn[i++].cObjType     = (CHAR)a;
            pa->pn[i++].cDistance    = (CHAR)b;
            pa->pn[i++].cDirection   = (CHAR)c;
            pa->pn[i++].cThinkChance = Rn2 (50, 49);
            pa->pn[i++].cUp          =
            pa->pn[i++].cDown        =
            pa->pn[i++].cRight       =
            pa->pn[i++].cLeft        =
            pa->pn[i++].uThinkIndex  = 

   }









