/*
 *  sranimator.c -- external interface to the "xtango" package.
 *  Stephen Hartley, Drexel University, 1994
 *
 *  This file (and only this file) is based on code from version 1.52 of the
 *  xtango package, available by anonymous FTP from ftp.cc.gatech.edu.  It
 *  is included by permission of the following license statement, also part
 *  of the package, which is reproduced in its entirety:
 *  
 *      The software contained in this anonymous ftp archive is not in the
 *      public domain.  It is freely available without fee.  By retrieving
 *      copies of files from this archive, you agree to abide by the following
 *      conditions and understandings:
 *      
 *      1.  That the software systems being provided in this archive has 
 *          been Copyrighted in the name of its authors and the Georgia
 *          Institute of Technology and that ownership of these systems 
 *          remains with them.
 *      
 *      2.  That Licensee is permitted to use, copy, modify and distribute
 *          this software and its documentation for any purpose other than
 *          its incorporation into a commercial product without fee, provided
 *          that the Copyright notice appears on all copies.
 *      
 *      3.  That the Licensee shall not use the name, logo, or any other
 *          symbol of Georgia Tech nor the names of any of its employees nor
 *          any adaptation thereof in any advertising, promotional or sales
 *          literature without the prior written approval of the Institute.
 *      
 *      4.  That this software is provided "as is", without warranty of any
 *          kind expressed or implied including, but not limited to, the
 *          suitability of this software for any purpose.
 *      
 *      5.  That Georgia Institute of Technology shall not be liable for
 *          any damages suffered by Licensee from the use of this software.
 */

#include <stdio.h>
#include "xtango.h"

char *malloc();

typedef enum {
  Rect,
  Line,
  Circ,
  Tria,
  Ctxt,
  Ltxt
  } ObjType;

typedef struct ObjNode {
   int     id;
   ObjType type;
   struct ObjNode *next;
 } *ObjPtr;


void bg();
void coords();
void delay();
void line();
void rectangle();
void circle();
void triangle();
void text();
void bigtext();
void fonttext();
void delete();
void move();
void moverelative();
void moveto();
void jump();
void jumprelative();
void jumpto();
void color();
void fill();
void vis();
void raise();
void lower();
void exchangepos();
void switchpos();
void swapid();
void resize();
void zoom();

double getFill();
double getWidth();
double getStyle();
int getArrows();
double fixY();
int validColor();
void addObject();
ObjPtr findObject();
void removeObject();
TANGO_COLOR getColor();
int stdColor();
double locX(), locY();


ObjPtr  Head = NULL;
ObjPtr  Tail = NULL;




/* These added for SR. */
void BEGIN_TANGOalgoOp() {
   fprintf(stderr, "Press RUN ANIMATION button to begin\n");
#if defined(ultrix) || defined(__alpha) || defined(__hpux)
   TANGOalgoOp("BEGIN");
#else
   TANGOalgoOp(NULL,"BEGIN");
#endif
}
void IDS_ASSOCmake() {
   ASSOCmake("IDS",1);
}
void END_TANGOalgoOp() {
   fprintf(stderr, "Press QUIT button to end\n");
#if defined(ultrix) || defined(__alpha) || defined(__hpux)
   TANGOalgoOp("END");			/* never returns */
#else
   TANGOalgoOp(NULL,"END");		/* never returns */
#endif
}





void bg(color) char *color; {
   TANGOset_bgcolor(color);
}


void coords(lx, by, rx, ty) double lx,by,rx,ty; {
   if ((lx >= rx) || (by >= ty)) {
      fprintf(stderr, "Invalid coords (%f,%f) (%f,%f) passed to coords command (Ignoring)\n",lx,by,rx,ty);
      return;
    }
   TANGOset_coord(lx, ty, rx, by);   /* reverse the y's */
}


void delay(frames) int frames; {
   TANGO_PATH path;
   TANGO_TRANS trans;
   path = TANGOpath_null(frames);
   trans = TANGOtrans_create(TANGO_TRANS_TYPE_DELAY, NULL, path);
   TANGOtrans_perform(trans);
   TANGOpath_free(1,path);
   TANGOtrans_free(1,trans);
}



void line (id, lx, ly, sx, sy, color, width, style, arrows) int id; double lx,ly,sx,sy; char color[],width[],style[],arrows[]; {
   double widthval, styleval;
   int arrowsval;
   TANGO_COLOR col;
   TANGO_IMAGE im;
   TANGO_PATH path;
   TANGO_TRANS trans;

   if (stdColor(color))
      col = getColor(color);
   else
      col = TANGOload_color(color);
   widthval = getWidth(width);
   if (widthval == -1.0) {
      fprintf(stderr, "Invalid width argument ->%s<- given to line command.  (Ignoring)\n",width);
      return;
    }
   styleval = getStyle(style);
   if (styleval == -1.0) {
      fprintf(stderr, "Invalid style argument ->%s<- given to line command.  (Ignoring)\n",style);
      return;
    }
   arrowsval = getArrows(arrows);
   if (arrowsval == -1) {
      fprintf(stderr, "Invalid arrows argument ->%s<- given to line command.  (Ignoring)\n",arrows);
      return;
    }
   im = TANGOimage_create(TANGO_IMAGE_TYPE_LINE, lx, fixY(ly), 1, col, sx, -sy,
                          widthval, styleval, arrowsval);
   ASSOCstore("IDS",id,im);
   path = TANGOpath_null(1);
   trans = TANGOtrans_create(TANGO_TRANS_TYPE_DELAY, NULL, path);
   TANGOtrans_perform(trans);
   TANGOpath_free(1,path);
   TANGOtrans_free(1,trans);
   if (findObject(id))
      fprintf(stderr, "Warning: object with id=%d already exists\n",id);
   addObject(id, Line);
 }


void rectangle(id, lx, ly, sx, sy, color, fill) int id; double lx,ly,sx,sy; char color[],fill[]; {
   double fillval;
   TANGO_COLOR col;
   TANGO_IMAGE im;
   TANGO_PATH path;
   TANGO_TRANS trans;

   if (stdColor(color))
      col = getColor(color);
   else
      col = TANGOload_color(color);
   fillval = getFill(fill);
   if (fillval == -1.0) {
      fprintf(stderr, "Invalid fill argument ->%s<- given to rectangle command.  (Ignoring)\n",fill);
      return;
    }
   im = TANGOimage_create(TANGO_IMAGE_TYPE_RECTANGLE, lx, fixY(ly)-sy,
                          1, col, sx, sy, fillval);
   ASSOCstore("IDS",id,im);
   path = TANGOpath_null(1);
   trans = TANGOtrans_create(TANGO_TRANS_TYPE_DELAY, NULL, path);
   TANGOtrans_perform(trans);
   TANGOpath_free(1,path);
   TANGOtrans_free(1,trans);
   if (findObject(id))
      fprintf(stderr, "Warning: object with id=%d already exists\n",id);
   addObject(id, Rect);
 }


void circle(id, lx, ly, rad, color, fill) int id; double lx,ly,rad; char color[],fill[]; {
   double fillval;
   TANGO_COLOR col;
   TANGO_IMAGE im;
   TANGO_PATH path;
   TANGO_TRANS trans;

   if (stdColor(color))
      col = getColor(color);
   else
      col = TANGOload_color(color);
   fillval = getFill(fill);
   if (fillval == -1.0) {
      fprintf(stderr, "Invalid fill argument ->%s<- given to circle command.  (Ignoring)\n",fill);
      return;
    }
   im = TANGOimage_create(TANGO_IMAGE_TYPE_CIRCLE, lx, fixY(ly), 1, col, rad,
                          fillval);
   ASSOCstore("IDS",id,im);
   path = TANGOpath_null(1);
   trans = TANGOtrans_create(TANGO_TRANS_TYPE_DELAY, NULL, path);
   TANGOtrans_perform(trans);
   TANGOpath_free(1,path);
   TANGOtrans_free(1,trans);
   if (findObject(id))
      fprintf(stderr, "Warning: object with id=%d already exists\n",id);
   addObject(id, Circ);
 }


void triangle(id, lx, ly, vx0, vy0, vx1, vy1, color, fill) int id; double lx,ly,vx0,vx1,vy0,vy1; char color[],fill[]; {
   double vx[2],vy[2],fillval;
   TANGO_COLOR col;
   TANGO_IMAGE im;
   TANGO_PATH path;
   TANGO_TRANS trans;

   vx[0] = vx0-lx;    /* make them relative */
   vy[0] = -(vy0-ly);   /* y coords are flipped */
   vx[1] = vx1-lx;
   vy[1] = -(vy1-ly);
   if (stdColor(color))
      col = getColor(color);
   else
      col = TANGOload_color(color);
   fillval = getFill(fill);
   if (fillval == -1.0) {
      fprintf(stderr, "Invalid fill argument ->%s<- given to triangle command.  (Ignoring)\n",fill);
      return;
    }
   im = TANGOimage_create(TANGO_IMAGE_TYPE_POLYGON, lx, fixY(ly), 1,
                         col, 3, vx, vy, fillval);
   ASSOCstore("IDS",id,im);
   path = TANGOpath_null(1);
   trans = TANGOtrans_create(TANGO_TRANS_TYPE_DELAY, NULL, path);
   TANGOtrans_perform(trans);
   TANGOpath_free(1,path);
   TANGOtrans_free(1,trans);
   if (findObject(id))
      fprintf(stderr, "Warning: object with id=%d already exists\n",id);
   addObject(id, Tria);
 }


void text(id, lx, ly, cen, color, str) int id,cen; double lx,ly; char color[], str[]; {
   fonttext(id, lx, ly, cen, color,
#ifdef ultrix
                          "fixed",
#else
                          "variable",
#endif
                          str);
 }



void bigtext(id, lx, ly, cen, color, str) int id,cen; double lx,ly; char color[], str[]; {
   fonttext(id, lx, ly, cen, color,
#ifdef ultrix
                          "variable",
#else
                          "12x24",
#endif
                          str);
 }


void fonttext(id, lx, ly, cen, color, fontname, str) int id,cen; double lx,ly; char color[], fontname[], str[]; {
   TANGO_COLOR col;
   TANGO_IMAGE im;
   TANGO_PATH path;
   TANGO_TRANS trans;

   if (stdColor(color))
      col = getColor(color);
   else
      col = TANGOload_color(color);
   im = TANGOimage_create(TANGO_IMAGE_TYPE_TEXT, lx, fixY(ly), 1, col, fontname,
                          str,cen);
   ASSOCstore("IDS",id,im);
   path = TANGOpath_null(1);
   trans = TANGOtrans_create(TANGO_TRANS_TYPE_DELAY, NULL, path);
   TANGOtrans_perform(trans);
   TANGOpath_free(1,path);
   TANGOtrans_free(1,trans);
   if (findObject(id))
      fprintf(stderr, "Warning: object with id=%d already exists\n",id);
   if (cen)
      addObject(id, Ctxt);
   else
      addObject(id, Ltxt);
 }




void delete(id) int id; {
   ObjPtr obj;
   TANGO_IMAGE im;
   TANGO_PATH path;
   TANGO_TRANS trans;

   if (!(obj = findObject(id))) {
      fprintf(stderr, "Error: attempt to delete object with nonexistent id=%d\n",id);
      return;
    }
   im = (TANGO_IMAGE) ASSOCretrieve("IDS",id);
   path = TANGOpath_null(1);
   trans = TANGOtrans_create(TANGO_TRANS_TYPE_DELETE, im, path);
   TANGOtrans_perform(trans);
   ASSOCdelete("IDS",id);
   TANGOpath_free(1,path);
   TANGOtrans_free(1,trans);
   removeObject(id);
}


void move(id, tx, ty) int id; double tx,ty; {
   double fty;
   ObjPtr obj;
   TANGO_IMAGE im;
   TANGO_LOC loc1,loc2;
   TANGO_PATH path;
   TANGO_TRANS trans;

   if (!(obj = findObject(id))) {
      fprintf(stderr, "Error: attempt to move object with nonexistent id=%d\n",id);
      return;
    }
   fty = fixY(ty);
   im = (TANGO_IMAGE) ASSOCretrieve("IDS",id);
   if (obj->type == Line)
      loc1 = TANGOimage_loc(im, TANGO_PART_TYPE_C);
   else if (obj->type == Rect)
      loc1 = TANGOimage_loc(im, TANGO_PART_TYPE_SW);
   else if (obj->type == Circ)
      loc1 = TANGOimage_loc(im, TANGO_PART_TYPE_C);
   else if (obj->type == Tria)
      loc1 = TANGOimage_loc(im, TANGO_PART_TYPE_C);
   else if (obj->type == Ctxt)
      loc1 = TANGOimage_loc(im, TANGO_PART_TYPE_C);
   else if (obj->type == Ltxt)
      loc1 = TANGOimage_loc(im, TANGO_PART_TYPE_SW);
   loc2 = TANGOloc_create(tx,fty);
   path = TANGOpath_motion(loc1,loc2,TANGO_PATH_TYPE_STRAIGHT);
   trans = TANGOtrans_create(TANGO_TRANS_TYPE_MOVE, im, path);
   TANGOtrans_perform(trans);
   TANGOpath_free(1,path);
   TANGOtrans_free(1,trans);
}


void moverelative(id, tx, ty) int id; double tx,ty; {
   ObjPtr obj;
   TANGO_IMAGE im;
   TANGO_LOC loc1,loc2;
   TANGO_PATH path;
   TANGO_TRANS trans;

   if (!(obj = findObject(id))) {
      fprintf(stderr, "Error: attempt to moverelative object with nonexistent id=%d\n",id);
      return;
    }
   im = (TANGO_IMAGE) ASSOCretrieve("IDS",id);
   loc1 = TANGOloc_create(0.0,0.0);
   loc2 = TANGOloc_create(tx,-ty);
   path = TANGOpath_motion(loc1,loc2,TANGO_PATH_TYPE_STRAIGHT);
   trans = TANGOtrans_create(TANGO_TRANS_TYPE_MOVE, im, path);
   TANGOtrans_perform(trans);
   TANGOpath_free(1,path);
   TANGOtrans_free(1,trans);
}


void moveto(id1, id2) int id1,id2; {
   ObjPtr obj1,obj2;
   TANGO_IMAGE im1,im2;
   TANGO_LOC loc1,loc2;
   TANGO_PATH path;
   TANGO_TRANS trans;

   if (!(obj1 = findObject(id1))) {
      fprintf(stderr, "Error: attempt to moveto object with nonexistent id=%d\n",id1);
      return;
    }
   if (!(obj2 = findObject(id2))) {
      fprintf(stderr, "Error: attempt to moveto object with nonexistent id=%d\n",id2);
      return;
    }
   im1 = (TANGO_IMAGE) ASSOCretrieve("IDS",id1);
   im2 = (TANGO_IMAGE) ASSOCretrieve("IDS",id2);
   if (obj1->type == Line)
      loc1 = TANGOimage_loc(im1, TANGO_PART_TYPE_C);
   else if (obj1->type == Rect)
      loc1 = TANGOimage_loc(im1, TANGO_PART_TYPE_SW);
   else if (obj1->type == Circ)
      loc1 = TANGOimage_loc(im1, TANGO_PART_TYPE_C);
   else if (obj1->type == Tria)
      loc1 = TANGOimage_loc(im1, TANGO_PART_TYPE_C);
   else if (obj1->type == Ctxt)
      loc1 = TANGOimage_loc(im1, TANGO_PART_TYPE_C);
   else if (obj1->type == Ltxt)
      loc1 = TANGOimage_loc(im1, TANGO_PART_TYPE_SW);
   if (obj2->type == Line)
      loc2 = TANGOimage_loc(im2, TANGO_PART_TYPE_C);
   else if (obj2->type == Rect)
      loc2 = TANGOimage_loc(im2, TANGO_PART_TYPE_SW);
   else if (obj2->type == Circ)
      loc2 = TANGOimage_loc(im2, TANGO_PART_TYPE_C);
   else if (obj2->type == Tria)
      loc2 = TANGOimage_loc(im2, TANGO_PART_TYPE_C);
   else if (obj2->type == Ctxt)
      loc2 = TANGOimage_loc(im2, TANGO_PART_TYPE_C);
   else if (obj2->type == Ltxt)
      loc2 = TANGOimage_loc(im2, TANGO_PART_TYPE_SW);
   path = TANGOpath_motion(loc1,loc2,TANGO_PATH_TYPE_STRAIGHT);
   trans = TANGOtrans_create(TANGO_TRANS_TYPE_MOVE, im1, path);
   TANGOtrans_perform(trans);
   TANGOpath_free(1,path);
   TANGOtrans_free(1,trans);
}


void jump(id, tx, ty) int id; double tx,ty; {
   double fty;
   ObjPtr obj;
   TANGO_IMAGE im;
   TANGO_LOC loc1,loc2;
   TANGO_PATH path;
   TANGO_TRANS trans;

   if (!(obj = findObject(id))) {
      fprintf(stderr, "Error: attempt to jump object with nonexistent id=%d\n",id);
      return;
    }
   fty = fixY(ty);
   im = (TANGO_IMAGE) ASSOCretrieve("IDS",id);
   if (obj->type == Line)
      loc1 = TANGOimage_loc(im, TANGO_PART_TYPE_C);
   else if (obj->type == Rect)
      loc1 = TANGOimage_loc(im, TANGO_PART_TYPE_SW);
   else if (obj->type == Circ)
      loc1 = TANGOimage_loc(im, TANGO_PART_TYPE_C);
   else if (obj->type == Tria)
      loc1 = TANGOimage_loc(im, TANGO_PART_TYPE_C);
   else if (obj->type == Ctxt)
      loc1 = TANGOimage_loc(im, TANGO_PART_TYPE_C);
   else if (obj->type == Ltxt)
      loc1 = TANGOimage_loc(im, TANGO_PART_TYPE_SW);
   loc2 = TANGOloc_create(tx,fty);
   path = TANGOpath_distance(loc1,loc2,1000.0);
   trans = TANGOtrans_create(TANGO_TRANS_TYPE_MOVE, im, path);
   TANGOtrans_perform(trans);
   TANGOpath_free(1,path);
   TANGOtrans_free(1,trans);
}


void jumprelative(id, tx, ty) int id; double tx,ty; {
   ObjPtr obj;
   TANGO_IMAGE im;
   TANGO_LOC loc1,loc2;
   TANGO_PATH path;
   TANGO_TRANS trans;

   if (!(obj = findObject(id))) {
      fprintf(stderr, "Error: attempt to jumprelative object with nonexistent id=%d\n",id);
      return;
    }
   im = (TANGO_IMAGE) ASSOCretrieve("IDS",id);
   loc1 = TANGOloc_create(0.0,0.0);
   loc2 = TANGOloc_create(tx,-ty);
   path = TANGOpath_distance(loc1,loc2,1000.0);
   trans = TANGOtrans_create(TANGO_TRANS_TYPE_MOVE, im, path);
   TANGOtrans_perform(trans);
   TANGOpath_free(1,path);
   TANGOtrans_free(1,trans);
}


void jumpto(id1, id2) int id1,id2; {
   ObjPtr obj1,obj2;
   TANGO_IMAGE im1,im2;
   TANGO_LOC loc1,loc2;
   TANGO_PATH path;
   TANGO_TRANS trans;

   if (!(obj1 = findObject(id1))) {
      fprintf(stderr, "Error: attempt to jumpto object with nonexistent id=%d\n",id1);
      return;
    }
   if (!(obj2 = findObject(id2))) {
      fprintf(stderr, "Error: attempt to jumpto object with nonexistent id=%d\n",id2);
      return;
    }
   im1 = (TANGO_IMAGE) ASSOCretrieve("IDS",id1);
   im2 = (TANGO_IMAGE) ASSOCretrieve("IDS",id2);
   if (obj1->type == Line)
      loc1 = TANGOimage_loc(im1, TANGO_PART_TYPE_C);
   else if (obj1->type == Rect)
      loc1 = TANGOimage_loc(im1, TANGO_PART_TYPE_SW);
   else if (obj1->type == Circ)
      loc1 = TANGOimage_loc(im1, TANGO_PART_TYPE_C);
   else if (obj1->type == Tria)
      loc1 = TANGOimage_loc(im1, TANGO_PART_TYPE_C);
   else if (obj1->type == Ctxt)
      loc1 = TANGOimage_loc(im1, TANGO_PART_TYPE_C);
   else if (obj1->type == Ltxt)
      loc1 = TANGOimage_loc(im1, TANGO_PART_TYPE_SW);
   if (obj2->type == Line)
      loc2 = TANGOimage_loc(im2, TANGO_PART_TYPE_C);
   else if (obj2->type == Rect)
      loc2 = TANGOimage_loc(im2, TANGO_PART_TYPE_SW);
   else if (obj2->type == Circ)
      loc2 = TANGOimage_loc(im2, TANGO_PART_TYPE_C);
   else if (obj2->type == Tria)
      loc2 = TANGOimage_loc(im2, TANGO_PART_TYPE_C);
   else if (obj2->type == Ctxt)
      loc2 = TANGOimage_loc(im2, TANGO_PART_TYPE_C);
   else if (obj2->type == Ltxt)
      loc2 = TANGOimage_loc(im2, TANGO_PART_TYPE_SW);
   path = TANGOpath_distance(loc1,loc2,1000.0);
   trans = TANGOtrans_create(TANGO_TRANS_TYPE_MOVE, im1, path);
   TANGOtrans_perform(trans);
   TANGOpath_free(1,path);
   TANGOtrans_free(1,trans);
}


void color(id, colname) int id; char colname[]; {
   ObjPtr obj;
   TANGO_COLOR col;
   TANGO_IMAGE im;
   TANGO_PATH path;
   TANGO_TRANS trans;

   if (!(obj = findObject(id))) {
      fprintf(stderr, "Error: attempt to color object with nonexistent id=%d\n",id);
      return;
    }
   if (!validColor(colname)) {
      fprintf(stderr, "Cannot change object %d to color %s.  (Ignored)\n",id,colname);
      return;
    }
   im = (TANGO_IMAGE) ASSOCretrieve("IDS",id);
   col = getColor(colname);
   path = TANGOpath_color(col);
   trans = TANGOtrans_create(TANGO_TRANS_TYPE_COLOR, im, path);
   TANGOtrans_perform(trans);
   TANGOpath_free(1,path);
   TANGOtrans_free(1,trans);
}


void fill(id, fill) int id; char fill[]; {
   double fillval,v;
   ObjPtr obj;
   TANGO_IMAGE im;
   TANGO_PATH path1,path2;
   TANGO_TRANS trans1,trans2,both;

   if (!(obj = findObject(id))) {
      fprintf(stderr, "Error: attempt to fill object with nonexistent id=%d\n",id);
      return;
    }
   fillval = getFill(fill);
   if (fillval == -1.0) {
      fprintf(stderr, "Invalid fill argument ->%s<- given to fill command.  (Ignoring)\n",fill);
      return;
    }
   if (fillval == 0.0)
      v = -1.0;
   else if (fillval == 1.0)
      v = 1.0;
   im = (TANGO_IMAGE) ASSOCretrieve("IDS",id);
   if ((fillval < 0.24) || (fillval > 0.76)) {
      path1 = TANGOpath_create(1,&v,&v);
      trans1 = TANGOtrans_create(TANGO_TRANS_TYPE_FILL, im, path1);
      TANGOtrans_perform(trans1);
      TANGOpath_free(1,path1);
      TANGOtrans_free(1,trans1);
    }
   else {   /* half-tone */
      v = -1.0;
      path1 = TANGOpath_create(1,&v,&v);
      v = fillval;
      path2 = TANGOpath_create(1,&v,&v);
      trans1 = TANGOtrans_create(TANGO_TRANS_TYPE_FILL, im, path1);
      trans2 = TANGOtrans_create(TANGO_TRANS_TYPE_FILL, im, path2);
      both = TANGOtrans_compose(2,trans1,trans2);
      TANGOtrans_perform(both);
      TANGOpath_free(2,path1,path2);
      TANGOtrans_free(3,trans1,trans2,both);
    }
}


void resize(id, rx, ry) int id; double rx, ry; {
   double fry;
   ObjPtr obj;
   TANGO_IMAGE im;
   TANGO_PATH path;
   TANGO_TRANS trans;

   if (!(obj = findObject(id))) {
      fprintf(stderr, "Error: attempt to resize object with nonexistent id=%d\n",id);
      return;
    }
   fry = -ry; /* reverse sign of y offset since animator is upside down */
   im = (TANGO_IMAGE) ASSOCretrieve("IDS",id);
   path = TANGOpath_create(1,&rx,&fry);
   if (obj->type == Tria) {
      trans = TANGOtrans_create(TANGO_TRANS_TYPE_RESIZE1, im, path);
   } else {
      trans = TANGOtrans_create(TANGO_TRANS_TYPE_RESIZE, im, path);
   }
   TANGOtrans_perform(trans);
   TANGOpath_free(1,path);
   TANGOtrans_free(1,trans);
}


void zoom(id, rx, ry) int id; double rx, ry; {
   ObjPtr obj;
   TANGO_IMAGE im;
   TANGO_PATH path;
   TANGO_TRANS trans;

   if (!(obj = findObject(id))) {
      fprintf(stderr, "Error: attempt to resize object with nonexistent id=%d\n",id);
      return;
    }
   im = (TANGO_IMAGE) ASSOCretrieve("IDS",id);
   path = TANGOpath_create(1,&rx,&ry);
   trans = TANGOtrans_create(TANGO_TRANS_TYPE_ZOOM, im, path);
   TANGOtrans_perform(trans);
   TANGOpath_free(1,path);
   TANGOtrans_free(1,trans);
}


void vis(id) int id; {
   ObjPtr obj;
   TANGO_IMAGE im;
   TANGO_PATH path;
   TANGO_TRANS trans;

   if (!(obj = findObject(id))) {
      fprintf(stderr, "Error: attempt to vis object with nonexistent id=%d\n",id);
      return;
    }
   im = (TANGO_IMAGE) ASSOCretrieve("IDS",id);
   path = TANGOpath_null(1);
   trans = TANGOtrans_create(TANGO_TRANS_TYPE_VISIBLE, im, path);
   TANGOtrans_perform(trans);
   TANGOpath_free(1,path);
   TANGOtrans_free(1,trans);
}


void raise(id) int id; {
   ObjPtr obj;
   TANGO_IMAGE im;
   TANGO_PATH path;
   TANGO_TRANS trans;

   if (!(obj = findObject(id))) {
      fprintf(stderr, "Error: attempt to raise object with nonexistent id=%d\n",id);
      return;
    }
   im = (TANGO_IMAGE) ASSOCretrieve("IDS",id);
   path = TANGOpath_null(1);
   trans = TANGOtrans_create(TANGO_TRANS_TYPE_RAISE, im, path);
   TANGOtrans_perform(trans);
   TANGOpath_free(1,path);
   TANGOtrans_free(1,trans);
}


void lower(id) int id; {
   ObjPtr obj;
   TANGO_IMAGE im;
   TANGO_PATH path;
   TANGO_TRANS trans;

   if (!(obj = findObject(id))) {
      fprintf(stderr, "Error: attempt to lower object with nonexistent id=%d\n",id);
      return;
    }
   im = (TANGO_IMAGE) ASSOCretrieve("IDS",id);
   path = TANGOpath_null(1);
   trans = TANGOtrans_create(TANGO_TRANS_TYPE_LOWER, im, path);
   TANGOtrans_perform(trans);
   TANGOpath_free(1,path);
   TANGOtrans_free(1,trans);
}


void exchangepos(id1, id2) int id1,id2; {
   ObjPtr obj1,obj2;
   TANGO_IMAGE im1,im2;
   TANGO_LOC loc1,loc2;
   TANGO_PATH path1,path2;
   TANGO_TRANS trans1,trans2,both;

   if (!(obj1 = findObject(id1))) {
      fprintf(stderr, "Error: attempt to exchangepos object with nonexistent id=%d\n",id1);
      return;
    }
   if (!(obj2 = findObject(id2))) {
      fprintf(stderr, "Error: attempt to exchangepos object with nonexistent id=%d\n",id2);
      return;
    }
   im1 = (TANGO_IMAGE) ASSOCretrieve("IDS",id1);
   im2 = (TANGO_IMAGE) ASSOCretrieve("IDS",id2);
   if (obj1->type == Line)
      loc1 = TANGOimage_loc(im1, TANGO_PART_TYPE_C);
   else if (obj1->type == Rect)
      loc1 = TANGOimage_loc(im1, TANGO_PART_TYPE_SW);
   else if (obj1->type == Circ)
      loc1 = TANGOimage_loc(im1, TANGO_PART_TYPE_C);
   else if (obj1->type == Tria)
      loc1 = TANGOimage_loc(im1, TANGO_PART_TYPE_C);
   else if (obj1->type == Ctxt)
      loc1 = TANGOimage_loc(im1, TANGO_PART_TYPE_C);
   else if (obj1->type == Ltxt)
      loc1 = TANGOimage_loc(im1, TANGO_PART_TYPE_SW);
   if (obj2->type == Line)
      loc2 = TANGOimage_loc(im2, TANGO_PART_TYPE_C);
   else if (obj2->type == Rect)
      loc2 = TANGOimage_loc(im2, TANGO_PART_TYPE_SW);
   else if (obj2->type == Circ)
      loc2 = TANGOimage_loc(im2, TANGO_PART_TYPE_C);
   else if (obj2->type == Tria)
      loc2 = TANGOimage_loc(im2, TANGO_PART_TYPE_C);
   else if (obj2->type == Ctxt)
      loc2 = TANGOimage_loc(im2, TANGO_PART_TYPE_C);
   else if (obj2->type == Ltxt)
      loc2 = TANGOimage_loc(im2, TANGO_PART_TYPE_SW);
   path1 = TANGOpath_motion(loc1,loc2,TANGO_PATH_TYPE_STRAIGHT);
   path2 = TANGOpath_reverse(path1);
   trans1 = TANGOtrans_create(TANGO_TRANS_TYPE_MOVE, im1, path1);
   trans2 = TANGOtrans_create(TANGO_TRANS_TYPE_MOVE, im2, path2);
   both = TANGOtrans_compose(2,trans1,trans2);
   TANGOtrans_perform(both);
   TANGOpath_free(2,path1,path2);
   TANGOtrans_free(3,trans1,trans2,both);
}


void switchpos(id1, id2) int id1,id2; {
   ObjPtr obj1,obj2;
   TANGO_IMAGE im1,im2;
   TANGO_LOC loc1,loc2;
   TANGO_PATH path1,path2;
   TANGO_TRANS trans1,trans2,both;

   if (!(obj1 = findObject(id1))) {
      fprintf(stderr, "Error: attempt to switchpos object with nonexistent id=%d\n",id1);
      return;
    }
   if (!(obj2 = findObject(id2))) {
      fprintf(stderr, "Error: attempt to switchpos object with nonexistent id=%d\n",id2);
      return;
    }
   im1 = (TANGO_IMAGE) ASSOCretrieve("IDS",id1);
   im2 = (TANGO_IMAGE) ASSOCretrieve("IDS",id2);
   if (obj1->type == Line)
      loc1 = TANGOimage_loc(im1, TANGO_PART_TYPE_C);
   else if (obj1->type == Rect)
      loc1 = TANGOimage_loc(im1, TANGO_PART_TYPE_SW);
   else if (obj1->type == Circ)
      loc1 = TANGOimage_loc(im1, TANGO_PART_TYPE_C);
   else if (obj1->type == Tria)
      loc1 = TANGOimage_loc(im1, TANGO_PART_TYPE_C);
   else if (obj1->type == Ctxt)
      loc1 = TANGOimage_loc(im1, TANGO_PART_TYPE_C);
   else if (obj1->type == Ltxt)
      loc1 = TANGOimage_loc(im1, TANGO_PART_TYPE_SW);
   if (obj2->type == Line)
      loc2 = TANGOimage_loc(im2, TANGO_PART_TYPE_C);
   else if (obj2->type == Rect)
      loc2 = TANGOimage_loc(im2, TANGO_PART_TYPE_SW);
   else if (obj2->type == Circ)
      loc2 = TANGOimage_loc(im2, TANGO_PART_TYPE_C);
   else if (obj2->type == Tria)
      loc2 = TANGOimage_loc(im2, TANGO_PART_TYPE_C);
   else if (obj2->type == Ctxt)
      loc2 = TANGOimage_loc(im2, TANGO_PART_TYPE_C);
   else if (obj2->type == Ltxt)
      loc2 = TANGOimage_loc(im2, TANGO_PART_TYPE_SW);
   path1 = TANGOpath_distance(loc1,loc2,1000.0);
   path2 = TANGOpath_reverse(path1);
   trans1 = TANGOtrans_create(TANGO_TRANS_TYPE_MOVE, im1, path1);
   trans2 = TANGOtrans_create(TANGO_TRANS_TYPE_MOVE, im2, path2);
   both = TANGOtrans_compose(2,trans1,trans2);
   TANGOtrans_perform(both);
   TANGOpath_free(2,path1,path2);
   TANGOtrans_free(3,trans1,trans2,both);
}


void swapid(id1, id2) int id1,id2; {
   ObjPtr obj1,obj2;
   TANGO_IMAGE ti;

   if (!(obj1 = findObject(id1))) {
      fprintf(stderr, "Error: attempt to swapid object with nonexistent id=%d\n",id1);
      return;
    }
   if (!(obj2 = findObject(id2))) {
      fprintf(stderr, "Error: attempt to swapid object with nonexistent id=%d\n",id2);
      return;
    }
   ti = (TANGO_IMAGE) ASSOCretrieve("IDS",id1);
   ASSOCstore("IDS",id1,ASSOCretrieve("IDS",id2));
   ASSOCstore("IDS",id2,ti);
}


double
getWidth(s)
   char *s;
{
   if (!strcmp(s,"thin"))
      return 0.0;
   else if (!strcmp(s, "medthick"))
      return 0.5;
   else if (!strcmp(s, "thick"))
      return 1.0;
   else
      return -1.0;
}


double
getStyle(s)
   char *s;
{
   if (!strcmp(s,"dotted"))
      return 0.0;
   else if (!strcmp(s, "dashed"))
      return 0.5;
   else if (!strcmp(s, "solid"))
      return 1.0;
   else
      return -1.0;
}


int
getArrows(s)
   char *s;
{
   if (!strcmp(s,"none"))
      return 0;
   else if (!strcmp(s, "forward"))
      return 1;
   else if (!strcmp(s, "backward"))
      return 2;
   else if (!strcmp(s, "bidirectional"))
      return 3;
   else
      return -1;
}


double
getFill(s)
   char *s;
{
   if (!strcmp(s,"outline"))
      return 0.0;
   else if (!strcmp(s, "light"))
      return 0.25;
   else if (!strcmp(s, "half"))
      return 0.5;
   else if (!strcmp(s, "heavy"))
      return 0.75;
   else if (!strcmp(s, "solid"))
      return 1.0;
   else
      return -1.0;
}


double
fixY(y)
   double y;
{
   double lx,by,rx,ty;

   TANGOinq_coord(&lx,&by,&rx,&ty);
   return (by-y+ty);
}


int
validColor(name)
   char *name;
{
   if (!strcmp(name,"black"))
      return 1;
   else if (!strcmp(name,"white"))
      return 1;
   else if (!strcmp(name,"red"))
      return 1;
   else if (!strcmp(name,"green"))
      return 1;
   else if (!strcmp(name,"blue"))
      return 1;
   else if (!strcmp(name,"yellow"))
      return 1;
   else if (!strcmp(name,"maroon"))
      return 1;
   else if (!strcmp(name,"orange"))
      return 1;
   else
      return 0;
}


void
addObject(id, type)
   int id;
   ObjType type;
{
   ObjPtr o;

   o = (ObjPtr) malloc(sizeof(struct ObjNode));
   if (!Head)
      Head = o;
   if (Tail)
      Tail->next = o;
   o->id = id;
   o->type = type;
   o->next = NULL;
   Tail = o;
}


ObjPtr
findObject(id)
   int id;
{
   ObjPtr o;

   for (o=Head; o; o=o->next)
       if (o->id == id)
          return(o);
   return(NULL);
}


void
removeObject(id)
   int id;
{
   ObjPtr o,old;

   if (Head->id == id) {
      if (Head == Tail)
         Tail = NULL;
      o = Head;
      Head = Head->next;
      free(o);
      return;
    }
   old = Head;
   for (o=Head->next; o; o=o->next) {
       if (o->id == id) {
          old->next = o->next;
          if (o == Tail)
             Tail = old;
          free(o);
          return;
	}
       old = o;
     }
}


int
getColor(str)
   char *str;
{
   if (strcmp("white",str) == 0)
      return( TANGO_COLOR_WHITE );
   else if (strcmp("black",str) == 0)
      return( TANGO_COLOR_BLACK );
   else if (strcmp("red",str) == 0)
      return( TANGO_COLOR_RED );
   else if (strcmp("orange",str) == 0)
      return( TANGO_COLOR_ORANGE );
   else if (strcmp("yellow",str) == 0)
      return( TANGO_COLOR_YELLOW );
   else if (strcmp("green",str) == 0)
      return( TANGO_COLOR_GREEN );
   else if (strcmp("blue",str) == 0)
      return( TANGO_COLOR_BLUE );
   else if (strcmp("maroon",str) == 0)
      return( TANGO_COLOR_MAROON );
   else
      return( TANGO_COLOR_BLACK );
}


int
stdColor(str)
   char *str;
{
   if (strcmp("white",str) == 0)
      return( 1 );
   else if (strcmp("black",str) == 0)
      return( 1 );
   else if (strcmp("red",str) == 0)
      return( 1 );
   else if (strcmp("orange",str) == 0)
      return( 1 );
   else if (strcmp("yellow",str) == 0)
      return( 1 );
   else if (strcmp("green",str) == 0)
      return( 1 );
   else if (strcmp("blue",str) == 0)
      return( 1 );
   else if (strcmp("maroon",str) == 0)
      return( 1 );
   else
      return( 0 );
}


double locX(id) int id; {
   ObjPtr obj;
   TANGO_IMAGE im;
   TANGO_LOC loc;

   if (!(obj = findObject(id))) {
      fprintf(stderr, "Error: attempt to loc object with nonexistent id=%d\n",id);
      return 0.0;
    }
   im = (TANGO_IMAGE) ASSOCretrieve("IDS",id);
   if (obj->type == Line)
      loc = TANGOimage_loc(im, TANGO_PART_TYPE_C);
   else if (obj->type == Rect)
      loc = TANGOimage_loc(im, TANGO_PART_TYPE_SW);
   else if (obj->type == Circ)
      loc = TANGOimage_loc(im, TANGO_PART_TYPE_C);
   else if (obj->type == Tria)
      loc = TANGOimage_loc(im, TANGO_PART_TYPE_C);
   else if (obj->type == Ctxt)
      loc = TANGOimage_loc(im, TANGO_PART_TYPE_C);
   else if (obj->type == Ltxt)
      loc = TANGOimage_loc(im, TANGO_PART_TYPE_SW);
   return TANGOloc_X(loc);
}


double locY(id) int id; {
   ObjPtr obj;
   TANGO_IMAGE im;
   TANGO_LOC loc;

   if (!(obj = findObject(id))) {
      fprintf(stderr, "Error: attempt to loc object with nonexistent id=%d\n",id);
      return 0.0;
    }
   im = (TANGO_IMAGE) ASSOCretrieve("IDS",id);
   if (obj->type == Line)
      loc = TANGOimage_loc(im, TANGO_PART_TYPE_C);
   else if (obj->type == Rect)
      loc = TANGOimage_loc(im, TANGO_PART_TYPE_SW);
   else if (obj->type == Circ)
      loc = TANGOimage_loc(im, TANGO_PART_TYPE_C);
   else if (obj->type == Tria)
      loc = TANGOimage_loc(im, TANGO_PART_TYPE_C);
   else if (obj->type == Ctxt)
      loc = TANGOimage_loc(im, TANGO_PART_TYPE_C);
   else if (obj->type == Ltxt)
      loc = TANGOimage_loc(im, TANGO_PART_TYPE_SW);
   return fixY(TANGOloc_Y(loc));
}
