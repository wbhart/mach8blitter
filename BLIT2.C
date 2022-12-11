#include <stdlib.h>
#include <conio.h>
#include <stdio.h>
#include <process.h>
#include <dos.h>
#include <fcntl.h>
#include <sys\types.h>
#include <time.h>
#include <sys\stat.h>
#include <io.h>
#include <string.h>
#include <alloc.h>

extern line_8514(int x0, int y0, int x1, int y1);
extern fill_8514(int x0, int y0, int x1, int y1, int colour);
extern blit_8514(int x0, int y0, int x1, int y1, int colour);
extern blank_8514(int x0, int y0, int x1, int y1);
extern xor_8514();
extern noxor_8514();
extern set_current_8514(int x0, int y0);
extern set_fgcolour_8514(char colour);

/******************************************/
/* (C) Copyright IBM Corporation 1986	  */
/******************************************/

/************************************************************************/
/* This program is written to demonstrate and introduce 		*/
/* the advanced function Interface					*/
/*									*/
/* For use with the IBM C 1.00 compiler with /Ze option (any model).	*/
/*									*/
/* CALLAFI.OBJ must be provided at the LINK stage.			*/
/* This provides the assembler 'glue' which allows IBM 'C' to call      */
/* the Advanced Function Interface.					*/
/************************************************************************/

#include "ibmafi.h"     /* The macros and structure definitions */
 
HOPEN_DATA open_data = { 3, 0, 0 };	     /* Hopen Data blk	*/
HCLOSE_DATA close_data = { 2, 0 };	     /* Hclose Data blk */
HINIT_DATA state_data = { 2, 0 };	     /* State Data ptr	*/

HEAR_DATA earfill = { 1, 0 };	/* End area fill block		*/

HLINE_DATA(6) hexag_s = {24,   201,240,	260,139,  380,139,  439,240,
				      380,343,	260,343};

HCHST_DATA(128) xystring;	/* Character string block	*/
HSCS_DATA set_cs = { 4, 0 };	/* Set Character set block	*/
static HSCOL_DATA colour   = { 4, 0 };	/* set colour block		*/

HQMODE_DATA query_mode  = { 18 }; /* query modes block		*/


char palette[18][4] = {	/* palette definition */
	    /*	    R	 B    G 			*/
		   {0,	 0,   0, 0},	/* black   0	*/
		 {0x24,0x24,0x24, 0},	/* gray    1	*/
		 {0x94,0x94,0x94, 0},	/* white   2	*/
		 {0xfc,0xfc,0xfc, 0},	/* l white 3	*/
		    {0,0x70,0x70, 0},	/* cyan    4	*/
		    {0,0xfc,0xfc, 0},	/* l cyan  5	*/
		 {0x70,	 0,0x48, 0},	/* brown   6	*/
		 {0xfc,0x24,0xfc, 0},	/* yellow  7	*/
		    {0,0x70,   0, 0},	/* blue    8	*/
		    {0,0xfc,   0, 0},	/* l blue  9	*/
		 {0x70,	 0,   0, 0},	/* red	   a	*/
		 {0xfc,0x24,0x24, 0},	/* l red   b	*/
		 {0x70,0x70,   0, 0},	/* magenta c	*/
		 {0xb0,0xfc,   0, 0},	/* l magenta d	*/
		    {0,	 0, 0x70, 0},	/* green   e	*/
		 {0x24,0x24,0xfc, 0},	/* l green f	*/
		    {0,	 0,   0, 0},	/* 2 spare to allow for shuffling */
		    {0,	 0,   0, 0}
				};

HLDPAL_DATA tab = { 10, 0, 0, 0, 16, (char far *) palette };
HINT_DATA hint_data = { 4, 0x80000000L };  /* flyback wait */
HSPAL_DATA svpal_data
		       = { sizeof(svpal_data) - 2 }; /* palette save area */
HSMODE_DATA mode = { 1, 1 };
HRECT_DATA rect = { 8, { 0, 0 }, 100, 100 };

void draw_quad(int, int, int, int, int, int, int, int, int);
void draw_triangle(int, int, int, int, int, int, int, int);

int main(void)
{
  int stataddr, i;

_BX = 25; /* number of paragraphs */
_AH = 0x48;
geninterrupt(0x21);
stataddr = _AX;

  getafi();

  HSPAL((char far *)&svpal_data);

  HOPEN((char far *)&open_data);	/* open Advanced Function Interface	*/

  state_data.segment = stataddr;	/* addr of Task Dep. buffer */
  HINIT((char far *)&state_data);			/* to Interface 		*/

  HSMODE(&mode);

  HLDPAL((char far *)&tab); 	/* load initial palette */

#if 0
    draw_quad(4,    98,141,107,127,232,343,214,343);	/* cyan 	*/
    draw_quad(5,   110,121,118,106,255,343,237,343);	/* light cyan	*/
    draw_quad(6,   178,200,295,0,  312,0  ,187,216);	/* brown	*/
    draw_quad(7,   190,221,318,0,  335,0  ,199,236);	/* yellow	*/
    draw_quad(8,   284, 96,514, 96,523,112,275,112);	/* blue 	*/
    draw_quad(9,   273,117,526,117,535,133,263,133);	/* light blue	*/
    draw_quad(10,   408,139,426,139,542,337,533,353);	/* red		*/
    draw_quad(11,   385,139,403,139,530,358,521,373);	/* light red	*/
    draw_quad(12,   454,266,463,282,348,479,330,479);	/* magenta	*/
    draw_quad(13,   442,246,451,261,325,479,307,479);	/* light magenta*/
    draw_quad(14,   116,367,366,367,357,382,124,382);	/* green	*/
    draw_quad(15,   104,348,376,348,369,362,113,362);	/* light green	*/
#endif

  colour.index = 3; /* dim white  on this palette */
  HSCOL((char far *)&colour);	/* set hexagon colour */

#if 0
  HBAR();	/* begin area */
    HLINE((char far *)&hexag_s);
  HEAR((char far *)&earfill);	/* end area	*/
#endif

  delay(1000);

  while ((!kbhit()) || (0x0d != getch()))	/* wait for ENTER */
  {
    delay(100);

    memmove(palette[ 6],palette[ 4],48);
    memmove(palette[ 4],palette[16],8);

    HINT((char far *)&hint_data);		/* wait for frame flyback */

    HLDPAL((char far *)&tab);		/* load updated palette */
  }

  xor_8514();

  for (i = 0; i < 10; i++)
  {
     draw_triangle(random(15) + 1, random(320), random(480),
                   random(320), random(480), random(320), random(480), 1);

     getchar();
  }

  for (i = 0; i < 1000; i++)
  {
/*
     colour.index = (long) random(16);
     HSCOL((char far *)&colour);

     rect.coord.x_coord = random(639);
     rect.coord.y_coord = random(479);
     rect.width = random(640 - rect.coord.x_coord);
     rect.height = random(480 - rect.coord.y_coord);

     HRECT((char far *)&rect);     
*/

/*
     draw_quad(random(16), random(640), random(480), random(640), random(480),
                          random(640), random(480), random(640), random(480));
*/

     draw_triangle(random(15) + 1, random(320), random(480),
                   random(320), random(480), random(320), random(480), 0);
  }

  HCLOSE((char far *)&close_data);	/* close AF Interface	*/

  HRPAL((char far *)&svpal_data); 

  return 0;
}

void draw_triangle(int col, int x0, int y0, int x1, int y1, int x2, int y2, int delay)
{
   int rect_x0, rect_y0, rect_x1, rect_y1;

   set_fgcolour_8514(col);
   xor_8514();
   line_8514(x0, y0, x1, y1);
   line_8514(x1, y1, x2, y2);
   line_8514(x2, y2, x0, y0);

   if (x0 < x1)
      rect_x0 = x0, rect_x1 = x1;
   else
      rect_x0 = x1, rect_x1 = x0;

   if (x2 > rect_x1)
      rect_x1 = x2;
   else if (x2 < rect_x0)
      rect_x0 = x2;

   if (y0 < y1)
      rect_y0 = y0, rect_y1 = y1;
   else
      rect_y0 = y1, rect_y1 = y0;

   if (y2 > rect_y1)
      rect_y1 = y2;
   else if (y2 < rect_y0)
      rect_y0 = y2;

   if (delay)
      getchar();

   fill_8514(rect_x0, rect_y0, rect_x1, rect_y1, col);

   if (delay)
      getchar();

   blit_8514(rect_x0, rect_y0, rect_x1, rect_y1, col);

   if (delay)
      getchar();

   noxor_8514();
   blank_8514(rect_x0, rect_y0, rect_x1, rect_y1);
}

void draw_quad(int col,int x0,int y0,int x1,int y1,int x2,int y2,int x3,int y3)	/* draw a quadrilateral */
{
static HLINE_DATA(4) quad = {16,   0,0, 0,0, 0,0, 0,0};

  quad.coords[0].x_coord = x0;	 /* fill in coords	*/
  quad.coords[0].y_coord = y0;

  quad.coords[1].x_coord = x1;
  quad.coords[1].y_coord = y1;

  quad.coords[2].x_coord = x2;
  quad.coords[2].y_coord = y2;

  quad.coords[3].x_coord = x3;
  quad.coords[3].y_coord = y3;

  colour.index = (long) col;
  HSCOL((char far *)&colour);	/* set supplied colour		*/

  HBAR();		/* begin area			*/
  HLINE((char far *)&quad); 	/* draw quadrilateral		*/
  HEAR((char far *)&earfill);	/* end area			*/

}

