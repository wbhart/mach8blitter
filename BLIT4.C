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
#include <math.h>

/* two axis rotation factors */
#define DELTA1 5
#define DELTA2 6

/* One and Phi scaled for a semiradius of 127 */
#define U 67
#define P 108

int colours[20] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
                  11, 12, 13, 14, 15, 1, 2, 3, 4, 5};

int vertices[12][3] = {{P, 0, U}, {P, 0, -U}, {-P, 0, -U}, {-P, 0, U},
                       {U, P, 0}, {-U, P, 0}, {-U, -P, 0}, {U, -P, 0},
                       {0, U, P}, {0, -U, P}, {0, -U, -P}, {0, U, -P}};
int faces[20][3] = {{0, 1, 7}, {0, 4, 7}, {6, 2, 3}, {2, 5, 3},
                    {7, 6, 9}, {6, 7, 10}, {4, 5, 1}, {4, 8, 5},
                    {9, 8, 0}, {8, 9, 3}, {10, 11, 2}, {10, 1, 11},
                    {0, 7, 9}, {7, 1, 10}, {9, 6, 3}, {6, 10, 2},
                    {1, 4, 11}, {0, 8, 4}, {5, 8, 3}, {5, 2, 11}};
 
extern line_8514(int x0, int y0, int x1, int y1);
extern fill_8514(int x0, int y0, int x1, int y1, int colour);
extern blit_8514(int x0, int y0, int x1, int y1, int colour);
extern blank_8514(int x0, int y0, int x1, int y1);
extern set_scissor_8514(int x0, int y0, int x1, int y1);
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
static HSCOL_DATA colour   = { 4, 0 };	/* set colour block	*/

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
HSPAL_DATA svpal_data
		       = { sizeof(svpal_data) - 2 }; /* palette save area */
HSMODE_DATA mode = { 1, 1 };
HRECT_DATA rect = { 8, { 0, 0 }, 100, 100 };

void draw_quad(int, int, int, int, int, int, int, int, int);
void draw_triangle(int, int, int, int, int, int, int);

void sincos_precomp(double * sintab, double * costab)
{
   double theta = 0.0;
   const double PI = 3.14159265359;
   const double delta = 2.0*PI/256.0;
   int i;

   for (i = 0; i < 256; i++)
   {
      costab[i] = cos(theta);
      sintab[i] = sin(theta);

      theta += delta;
   }
}

/*
   We perform 255 prerotations of the x, y and z unit
   vectors by angles (at different rates) around the
   z and then around the x axes.

   Rotating a point (a, b, c) by the same amounts can
   then be computed as [a*(x1[i], x2[i], x3[i]) +
   b*(y1[i], y2[i], y3[i]) + c*(z1[i], z2[i], z3[i])].

   In theory one could use a lookup table for the
   multiplications, but it would be large.
*/
void rotate_precomp(int * x1, int * x2, int * x3,
                    int * y1, int * y2, int * y3,
                    int * z1, int * z2, int * z3,
                    double * sintab, double * costab)
{
   int i;
   int theta1 = 0, theta2 = 0;
   double x, y, z;

   for (i = 0; i < 256; i++)
   {
      /* rotate unit vector in x direction by DELTA1 */
      x = costab[theta1];
      y = -sintab[theta1];
      /* z = 0 */
      
      /* further rotate by DELTA2 */
      x1[i] = (int) (127*x + 0.5);
      x2[i] = (int) (127*costab[theta2]*y + 0.5);
      x3[i] = -(int) (127*sintab[theta2]*y + 0.5);

      /* rotate unit vector in y direction by DELTA1 */
      x = sintab[theta1];
      y = costab[theta1];
      /* z = 0 */

      /* further rotate by DELTA2 */
      y1[i] = 0;
      y2[i] = (int) (127*costab[theta2]*y + 0.5);
      y3[i] = -(int) (127*sintab[theta2]*y + 0.5);

      /* rotate unit vector in z direction by DELTA1 */
      /* x = 0; */
      /* y = 0; */
      /* z = 1.0; */

      /* further rotate by DELTA2 */
      z1[i] = 0;
      z2[i] = (int) (127*sintab[theta2] + 0.5);
      z3[i] = (int) (127*costab[theta2] + 0.5);

      /* increment by DELTA1 and DELTA2 steps in two axes */
      theta1 = ((theta1 + DELTA1) & 0xff);
      theta2 = ((theta2 + DELTA2) & 0xff);
   }
}

/*
   Return just the z projection of the normal
   to the plane defined by (x0, y0, z0) and
   (x1, y1, z1)
*/
int zproj(int x0, int y0,
            int x1, int y1)
{
   return x0*y1 - y0*x1;   
}

/*
   return a normal (x, y, z) to the plane defined by
   (x0, y0, z0) and (x1, y1, z1)
*/
void normal(int * x, int * y, int * z,
            int x0, int y0, int z0,
            int x1, int y1, int z1)
{
   (*x) = y0*z1 - z0*y1;
   (*y) = z0*x1 - x0*z1;
   (*z) = x0*y1 - y0*x1;   
}

void orthonormal(int * x, int * y, int * z,
            int x0, int y0, int z0,
            int x1, int y1, int z1)
{
   long t, s = 0;
   double d;

   normal(x, y, z, x0, y0, z0, x1, y1, z1);
   
   t = ((long) (*x)) * ((long) (*x));
   s += t;
   t = ((long) (*y)) * ((long) (*y));
   s += t;
   t = ((long) (*z)) * ((long) (*z));
   s += t;
   
   if (s == 0)
      s = 1;

   d = sqrt(s);
   (*x) = (int) ((*x)/d);
   (*y) = (int) ((*y)/d);
   (*z) = (int) ((*z)/d);
}

/*
   Rotate the vertices of the polyhedron i steps
   where i is in [0, 255] and store results in
   the arrays xs, ys, zs
*/
void rotate_poly(int * xs, int * ys, int * zs,
                    int * x1, int * x2, int * x3,
                    int * y1, int * y2, int * y3,
                    int * z1, int * z2, int * z3,
                    int i)
{
   int j;
   int x, y, z;

   for (j = 0; j < 12; j++)
   {
      x = vertices[j][0];
      y = vertices[j][1];
      z = vertices[j][2];

      xs[j] = ((x*x1[i] + y*y1[i] + z*z1[i]) >> 8);       
      ys[j] = ((x*x2[i] + y*y2[i] + z*z2[i]) >> 8);       
      zs[j] = ((x*x3[i] + y*y3[i] + z*z3[i]) >> 8);       
   }
}

/*
   If the given face n, whose coordinates after
   rotation are in xs, ys and zs at index n,
   is not culled, return 1 and set pi = (rxi, ryi)
   to the three points of the triangle in 2D,
   else return 0
*/
int rotate_triangle(int * rx0, int * ry0, int * rx1,
                    int * ry1, int * rx2, int * ry2,
                    int * xs, int * ys, int * zs,
                    int n)
{
   int x0, y0, z0, x1, y1, z1, x2, y2, z2;
   int vert;

   vert = faces[n][0];
   x0 = xs[vert];
   y0 = ys[vert];
   /* z0 = zs[vert]; */
 
   vert = faces[n][1];
   x1 = xs[vert];
   y1 = ys[vert];
   /* z1 = zs[vert]; */
 
   vert = faces[n][2];
   x2 = xs[vert];
   y2 = ys[vert];
   /* z2 = zs[vert]; */
 
   if (zproj(x1 - x0, y1 - y0, x2 - x0, y2 - y0) <= 0)
      return 0;

   (*rx0) = x0;
   (*ry0) = y0;
   (*rx1) = x1;
   (*ry1) = y1;
   (*rx2) = x2;
   (*ry2) = y2;

   return 1;
}
  
int main(void)
{
  int stataddr, i, n, frame;
  int rx0 = 0, ry0 = 0, rx1 = 0, ry1 = 0, rx2 = 0, ry2 = 0;
  double sintab[256];
  double costab[256];
  int x1[256], x2[256], x3[256];
  int y1[256], y2[256], y3[256];
  int z1[256], z2[256], z3[256];
  int xs[12], ys[12], zs[12];

  _BX = 25; /* number of paragraphs */
  _AH = 0x48;
  geninterrupt(0x21);
  stataddr = _AX;

  sincos_precomp(sintab, costab);
  rotate_precomp(x1, x2, x3, y1, y2, y3, z1, z2, z3, sintab, costab);

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

  set_scissor_8514(0, 0, 1024, 480);
  xor_8514();

  for (frame = 0; frame < 256; frame++)
  {
     rotate_poly(xs, ys, zs, x1, x2, x3, y1, y2, y3,
                 z1, z2, z3, frame);

     draw_triangle(0, 0, 0, 128, 0, 128, 128);
     draw_triangle(0, 0, 0, 128, 128, 0, 128);

     for (n = 0; n < 20; n++)
     {
        if (rotate_triangle(&rx0, &ry0, &rx1, &ry1, &rx2, &ry2,
                            xs, ys, zs, n))
        {
           draw_triangle(colours[n], rx0 + 64, ry0 + 64,
                         rx1 + 64, ry1 + 64, rx2 + 64, ry2 + 64);
        }
     }
  }

  getchar();

  HCLOSE((char far *)&close_data);	/* close AF Interface	*/

  HRPAL((char far *)&svpal_data); 

  return 0;
}

void draw_triangle(int col, int x0, int y0, int x1, int y1, int x2, int y2)
{
   int rect_x0, rect_y0, rect_x1, rect_y1;

   set_fgcolour_8514(col);

   x0 += 640;
   x1 += 640;
   x2 += 640;

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

   fill_8514(rect_x0, rect_y0, rect_x1, rect_y1, col);

   blit_8514(rect_x0, rect_y0, rect_x1, rect_y1, col);

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

