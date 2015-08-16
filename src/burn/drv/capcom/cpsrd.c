#include "cps.h"

// CPS Scroll2 with Row scroll - Draw
static INT32 nKnowBlank=-1;	// The tile we know is blank
static INT32 nFirstY, nLastY;
static INT32 bVCare;

static inline UINT16 *FindTile(INT32 fx,INT32 fy)
{
   INT32 p=((fy&0x30)<<8) | ((fx&0x3f)<<6) | ((fy&0x0f)<<2);
   return (UINT16 *)(CpsrBase + p);
}

static void Cps2TileLine(INT32 y,INT32 sx)
{
   INT32 x;
   INT32 ix=(sx>>4)+1; sx&=15; sx=16-sx;
   INT32 sy=16-(nCpsrScrY&15);
   INT32 iy=(nCpsrScrY>>4)+1;

   nCpstY=sy+(y<<4);

   for (x=-1; x<24; x++)
   {
      UINT16 *pst;
      INT32 t,a;

      nCpstType = CTT_16X16;

      // Don't need to clip except around the border
      if (bVCare || x<0 || x>=24-1)
         nCpstType |= CTT_CARE;

      pst = FindTile(ix+x,iy+y);

      t = BURN_ENDIAN_SWAP_INT16(pst[0]);
      t<<=7; // Get real tile address
      t+=nCpsGfxScroll[2]; // add on offset to scroll tiles

      if (t==nKnowBlank)
         continue; // Don't draw: we know it's blank

      a = BURN_ENDIAN_SWAP_INT16(pst[1]);

      CpstSetPal(0x40 | (a&0x1f));
      nCpstX=sx+(x<<4); nCpstTile=t; nCpstFlip=(a>>5)&3;
      if (Cps2tOne())
         nKnowBlank=t;
   }
}

static void Cps2TileLineRows(INT32 y,struct CpsrLineInfo *pli)
{
  INT32 iy,x;
  INT32 nLimLeft,nLimRight;
  INT32 nTileCount=pli->nTileEnd-pli->nTileStart;
  INT32 sy=16-(nCpsrScrY&15); iy=(nCpsrScrY>>4)+1;
  nCpstY=sy+(y<<4);
  CpstRowShift=pli->Rows;

  // If these rowshift limits go off the edges, we should take
  // care drawing the tile.
  nLimLeft =pli->nMaxLeft;
  nLimRight=pli->nMaxRight;

  for (x=0; x<nTileCount; x++, nLimLeft+=16, nLimRight+=16)
  {
     UINT16 *pst;
     INT32 t,a;
     INT32 tx=pli->nTileStart+x;
     // See if we have to clip vertically anyway
     INT32 bCare=bVCare;

     if (bCare==0) // If we don't...
     {
        // Check screen limits of this tile
        if (nLimLeft <      0)
           bCare=1; // Will cross left edge
        if (nLimRight> 384-16)
           bCare=1; // Will cross right edge
     }

     nCpstType = CTT_16X16 | CTT_ROWS;

     if (bCare)
        nCpstType |= CTT_CARE;

     pst = FindTile(tx,iy+y);
     t = BURN_ENDIAN_SWAP_INT16(pst[0]);
     t<<=7; // Get real tile address
     t+=nCpsGfxScroll[2]; // add on offset to scroll tiles

     if (t==nKnowBlank)
        continue; // Don't draw: we know it's blank

     a = BURN_ENDIAN_SWAP_INT16(pst[1]);

     CpstSetPal(0x40 | (a&0x1f));

     nCpstX=x<<4; nCpstTile=t; nCpstFlip=(a>>5)&3;
     if(Cps2tOne())
        nKnowBlank=t;
  }
}

INT32 Cps2rRender(void)
{
   INT32 y;
   struct CpsrLineInfo *pli;
   if (!CpsrBase)
      return 1;

   nKnowBlank = -1;					// We don't know which tile is blank yet

   nLastY = (nEndline + (nCpsrScrY & 15)) >> 4;
   nFirstY = (nStartline + (nCpsrScrY & 15)) >> 4;

   for (y = nFirstY - 1, pli = CpsrLineInfo + nFirstY; y < nLastY; y++, pli++)
   {
      bVCare = ((y << 4) < nStartline) | (((y << 4) + 16) >= nEndline);

      if (pli->nWidth==0)
         Cps2TileLine(y,pli->nStart);	// no rowscroll needed
      else
         Cps2TileLineRows(y,pli);		// row scroll
   }

   return 0;
}
