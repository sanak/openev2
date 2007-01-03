/******************************************************************************
 * $Id: gvtypes.h,v 1.1.1.1 2005/04/18 16:38:34 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  Assortment of OpenEV types.
 * Author:   OpenEV Team
 *
 ******************************************************************************
 * Copyright (c) 2000, Atlantis Scientific Inc. (www.atlsci.com)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 ******************************************************************************
 *
 * $Log: gvtypes.h,v $
 * Revision 1.1.1.1  2005/04/18 16:38:34  uid1026
 * Import reorganized openev tree with initial gtk2 port changes
 *
 * Revision 1.1.1.1  2005/03/07 21:16:36  uid1026
 * openev gtk2 port
 *
 * Revision 1.1.1.1  2005/02/08 00:50:26  uid1026
 *
 * Imported sources
 *
 * Revision 1.15  2004/03/05 23:40:12  sduclos
 * remove commentd define GV_USE_DOUBLE.. , remove TAB
 *
 * Revision 1.14  2002/11/04 21:42:07  sduclos
 * change geometric data type name to gvgeocoord
 *
 * Revision 1.13  2000/06/20 13:27:08  warmerda
 * added standard headers
 *
 */

#ifndef __GV_TYPES_H__
#define __GV_TYPES_H__

#include <glib.h>
#include <glib-object.h>
#include "gv_config.h"

#define GV_NONE    0x000
#define GV_ALWAYS  0x001
#define GV_LATER   0x002
#define GV_NOW     0x003
#define GV_FIRST   0x004
#define GV_ALL     0x005

/* GvShapeLayer.flags - bit flags */
#define GV_DELAY_SELECTED      0x0001
#define GV_SUPPRESS_SELCHANGED 0x0002
#define GV_ALT_SELECTED        0x0004

#define GV_TESS_NONE 0
#define GV_TESS_LOCK -1

#define GV_CHANGE_ADD      0x001
#define GV_CHANGE_REPLACE  0x002
#define GV_CHANGE_DELETE   0x003

#define GV_MINFLOAT G_MINFLOAT
#define GV_MAXFLOAT G_MAXFLOAT

#define GV_USE_DOUBLE_PRECISION_COORD 1

#ifdef GV_USE_DOUBLE_PRECISION_COORD
#
#   define glVertex2   glVertex2d
#   define glVertex2v  glVertex2dv
#   define glVertex3   glVertex3d
#   define glScale     glScaled
#   define glRotate    glRotated
#   define glTranslate glTranslated
#   define glRasterPos2 glRasterPos2d
#   define glRasterPos3 glRasterPos3d
#
#   define GL_GEOCOORD GL_DOUBLE
#   define GLgeocoord  GLdouble
#
#   define Ccast   "d"
#   define CC      "dd"
#   define CCC     "ddd"
#   define CCCC    "dddd"
#   define CCCCCC  "dddddd"
#
#else
#
#   define glVertex2   glVertex2f
#   define glVertex2v  glVertex2fv
#   define glVertex3   glVertex3f
#   define glScale     glScalef
#   define glRotate    glRotatef
#   define glTranslate glTranslatef
#   define glRasterPos2 glRasterPos2f
#   define glRasterPos3 glRasterPos3f
#
#   define GL_GEOCOORD GL_FLOAT
#   define GLgeocoord  GLfloat
#
#   define Ccast   "f"
#   define CC      "ff"
#   define CCC     "fff"
#   define CCCC    "ffff"
#   define CCCCCC  "ffffff"
#
#endif

// SD Note: sizeof(GLgeocoord) == sizeof(gvgeocoord)
#ifdef GV_USE_DOUBLE_PRECISION_COORD
typedef double gvgeocoord;
#else
typedef float  gvgeocoord;
#endif

typedef float gvfloat;

typedef float GvColor[4];  /* RGBA, range [0-1] */

typedef struct _GvRect GvRect;
typedef struct _GvVertex GvVertex;
typedef struct _GvVertex3d GvVertex3d;
typedef struct _GvNodeInfo GvNodeInfo;
typedef struct _GvShapeChangeInfo GvShapeChangeInfo;

struct _GvRect
{
    gvgeocoord x, y, width, height;
};

struct _GvVertex
{
    gvgeocoord x, y;
};

struct _GvVertex3d
{
    gvgeocoord x, y, z;
};

struct _GvNodeInfo
{
    gint shape_id;
    gint ring_id;
    gint node_id;
    GvVertex *vertex;
};

struct _GvShapeChangeInfo
{
    gint change_type;
    gint num_shapes;
    gint *shape_id;
};

typedef struct _GvRasterChangeInfo
{
    gint  change_type;
    gint  x_off;
    gint  y_off;
    gint  width;
    gint  height;
} GvRasterChangeInfo;

/* FIXME: Where is a good place for this? */
#define gv_color_copy(d,s)  {d[0]=s[0];d[1]=s[1];d[2]=s[2];d[3]=s[3];}

#endif /* __GV_TYPES_H__ */



