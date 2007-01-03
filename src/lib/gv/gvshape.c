/******************************************************************************
 * $Id: gvshape.c,v 1.1.1.1 2005/04/18 16:38:34 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  GvShape (point/line/area/collection vector object)
 * Author:   Frank Warmerdam, warmerdam@pobox.com
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
 * $Log: gvshape.c,v $
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
 * Revision 1.21  2005/01/04 18:50:29  gmwalter
 * Checked in Aude's new gvshape function changes.
 *
 * Revision 1.20  2003/09/02 18:17:51  warmerda
 * Added support for serializing collection shapes
 *
 * Revision 1.19  2003/08/29 20:52:43  warmerda
 * added to/from xml translation for GvShape
 *
 * Revision 1.18  2003/06/26 02:46:21  pgs
 * added define for M_PI if it is not already defined
 *
 * Revision 1.17  2003/06/25 17:06:06  warmerda
 * added gv_shape_rotate(), gv_shape_scale() and related stuff
 *
 * Revision 1.16  2002/11/04 21:42:07  sduclos
 * change geometric data type name to gvgeocoord
 *
 * Revision 1.15  2002/05/07 02:51:15  warmerda
 * preliminary support for GVSHAPE_COLLECTION
 *
 * Revision 1.14  2002/03/07 18:31:56  warmerda
 * added preliminary gv_shape_clip_to_rect() implementation
 *
 * Revision 1.13  2001/12/08 04:49:38  warmerda
 * added point in polygon test
 *
 * Revision 1.12  2001/08/08 17:45:48  warmerda
 * GvShape now referenced counted
 *
 * Revision 1.11  2000/09/11 13:54:33  warmerda
 * fixed some bugs computing area of line features
 *
 * Revision 1.10  2000/09/11 13:47:24  warmerda
 * fixed bug in computing extents of area features
 *
 * Revision 1.9  2000/07/14 14:51:01  warmerda
 * fixed insert, and delete node support
 *
 * Revision 1.8  2000/06/20 13:26:55  warmerda
 * added standard headers
 *
 */
#include <assert.h>
#include <string.h>
#include "gvshapes.h"
#include "gvrenderinfo.h"
#include "cpl_string.h"
#include "cpl_error.h"

#ifndef M_PI
#define M_PI       3.14159265358979323846
#endif
static int  gv_shape_count = 0;

/************************************************************************/
/*                         gv_shape_get_count()                         */
/************************************************************************/

int gv_shape_get_count()

{
    return gv_shape_count;
}

/************************************************************************/
/*                            gv_shape_new()                            */
/*                                                                      */
/*      Note that the initial ref count of shapes returned by           */
/*      gv_shape_new() is zero.  If the caller wants to retain a        */
/*      reference to the shape (other than just adding to a             */
/*      GvShapes, or some other operation that increments the ref       */
/*      count) it should call gv_shape_ref().                           */
/************************************************************************/

GvShape *gv_shape_new( gint type )

{
    GvShape    *shape;

    gv_shape_count++;

    if( type == GVSHAPE_POINT )
    {
        shape = (GvShape *) g_new0( GvPointShape, 1 );
    }
    else if( type == GVSHAPE_LINE )
    {
        shape = (GvShape *) g_new0( GvLineShape, 1 );
    }
    else if( type == GVSHAPE_AREA )
    {
        shape = (GvShape *) g_new0( GvAreaShape, 1 );
        ((GvAreaShape *) shape)->fill_objects = -1;
    }
    else if( type == GVSHAPE_COLLECTION )
    {
        shape = (GvShape *) g_new0( GvCollectionShape, 1 );
    }
    else
    {
        g_warning( "Illegal shape type in gv_shape_new()" );
        shape = NULL;
    }

    if( shape != NULL )
    {
        shape->flags = type | 0;
        shape->ref_count = 0;
        gv_properties_init( &(shape->properties) );
    }

    return shape;
}

/************************************************************************/
/*                            gv_shape_ref()                            */
/************************************************************************/

void gv_shape_ref( GvShape *shape )

{
    shape->ref_count++;
}

/************************************************************************/
/*                           gv_shape_unref()                           */
/************************************************************************/

void gv_shape_unref( GvShape *shape )

{
    if( --shape->ref_count <= 0 )
        gv_shape_delete( shape );
}

/************************************************************************/
/*                          gv_shape_get_ref()                          */
/************************************************************************/

int gv_shape_get_ref( GvShape *shape )

{
    return shape->ref_count;
}

/************************************************************************/
/*                          gv_shape_delete()                           */
/************************************************************************/
void gv_shape_delete( GvShape *shape )

{
    gv_shape_count--;

    gv_properties_destroy( gv_shape_get_properties( shape ) );

    switch( gv_shape_type(shape) )
    {
      case GVSHAPE_AREA:
      {
          GvAreaShape *area = (GvAreaShape *) shape;
          int         ring;

          for( ring = 0; ring < area->num_rings; ring++ )
          {
              g_free( area->xyz_ring_nodes[ring] );
          }
	  area->num_rings = 0;
          if( area->xyz_ring_nodes != NULL ) {
              g_free( area->xyz_ring_nodes );
	      area->xyz_ring_nodes = NULL;
	  }
          if( area->num_ring_nodes != NULL ) {
              g_free( area->num_ring_nodes );
	      area->num_ring_nodes = NULL;
	  }
          if( area->fill ) {
              g_array_free( area->fill, TRUE );
	      area->fill = NULL;
	  }
          if( area->mode_offset ) {
              g_array_free( area->mode_offset, TRUE );
	      area->mode_offset = NULL;
	  }
      }
      break;

      case GVSHAPE_LINE:
      {
          GvLineShape  *line = (GvLineShape *) shape;

          if( line->xyz_nodes != NULL )
              g_free( line->xyz_nodes );
      }
      break;

      case GVSHAPE_COLLECTION:
      {
          GvCollectionShape  *col = (GvCollectionShape *) shape;
          int                 i;

          for( i = 0; i < col->geom_count; i++ )
              gv_shape_delete( col->geom_list[i] );

          if( col->geom_list != NULL )
              g_free( col->geom_list );
          col->geom_list = NULL;
      }
      break;

      case GVSHAPE_POINT:
      default:
        break;
    }

    g_free( shape );
}

/************************************************************************/
/*                           gv_shape_copy()                            */
/************************************************************************/
GvShape *gv_shape_copy( GvShape *source )

{
    GvShape   *target;

    target = gv_shape_new( gv_shape_type(source) );

    gv_properties_copy( &(source->properties), &(target->properties) );
    target->flags = source->flags;

    switch( gv_shape_type(source) )
    {
      case GVSHAPE_AREA:
      {
          GvAreaShape *tarea = (GvAreaShape *) target;
          GvAreaShape *sarea = (GvAreaShape *) source;

          tarea->num_rings = sarea->num_rings;
          if( sarea->num_rings > 0 )
          {
              int    ring;

              tarea->num_ring_nodes = g_new(int,sarea->num_rings);
              memcpy( tarea->num_ring_nodes, sarea->num_ring_nodes,
                      sizeof(int) * sarea->num_rings );

              tarea->xyz_ring_nodes = g_new(gvgeocoord *,sarea->num_rings);
              for( ring = 0; ring < sarea->num_rings; ring++ )
              {
                  tarea->xyz_ring_nodes[ring] =
                      g_new(gvgeocoord,3*tarea->num_ring_nodes[ring]);
                  memcpy( tarea->xyz_ring_nodes[ring],
                          sarea->xyz_ring_nodes[ring],
                          sizeof(gvgeocoord) * 3 * tarea->num_ring_nodes[ring] );
              }
          }
      }
      break;

      case GVSHAPE_LINE:
      {
          int  num_nodes = ((GvLineShape *) source)->num_nodes;

          ((GvLineShape *) target)->num_nodes = num_nodes;

          ((GvLineShape *) target)->xyz_nodes = g_new(gvgeocoord, 3 * num_nodes);
          memcpy( ((GvLineShape *) target)->xyz_nodes,
                  ((GvLineShape *) source)->xyz_nodes,
                  sizeof(gvgeocoord) * 3 * num_nodes );
      }
      break;

      case GVSHAPE_POINT:
        ((GvPointShape *) target)->x = ((GvPointShape *) source)->x;
        ((GvPointShape *) target)->y = ((GvPointShape *) source)->y;
        ((GvPointShape *) target)->z = ((GvPointShape *) source)->z;
        break;

      case GVSHAPE_COLLECTION:
      {
          GvCollectionShape *source_col = (GvCollectionShape *) source;
          int i;

          for( i = 0; i < source_col->geom_count; i++ )
              gv_shape_collection_add_shape(
                  target, gv_shape_copy( source_col->geom_list[i] ) );
      }
      break;

      default:
        assert(FALSE);
    }

    return target;
}

/************************************************************************/
/*                         gv_shape_get_rings()                         */
/************************************************************************/
gint gv_shape_get_rings( GvShape *shape )

{
    if( gv_shape_type(shape) == GVSHAPE_AREA )
        return ((GvAreaShape *) shape)->num_rings;
    else
        return 1;
}

/************************************************************************/
/*                         gv_shape_get_nodes()                         */
/************************************************************************/
gint gv_shape_get_nodes( GvShape *shape, int ring )

{
    if( gv_shape_type(shape) == GVSHAPE_AREA
        && ring >= 0 && ring < ((GvAreaShape *) shape)->num_rings )
    {
        return ((GvAreaShape *) shape)->num_ring_nodes[ring];
    }
    else if( ring == 0 && gv_shape_type(shape) == GVSHAPE_LINE )
    {
        return ((GvLineShape *) shape)->num_nodes;
    }
    else if( ring == 0 && gv_shape_type(shape) == GVSHAPE_POINT )
        return 1;
    else
        return 0;
}

/************************************************************************/
/*                          gv_shape_get_xyz()                          */
/************************************************************************/
gvgeocoord gv_shape_get_xyz( GvShape *shape, gint ring, gint node, gint off )

{
    if( gv_shape_type(shape) == GVSHAPE_AREA )
    {
        GvAreaShape *area = (GvAreaShape *) shape;

        if( ring >= 0 && ring < area->num_rings
            && node >= 0 && node < area->num_ring_nodes[ring] )
            return area->xyz_ring_nodes[ring][node*3+off];
        else
            return 0.0;
    }
    else if( gv_shape_type(shape) == GVSHAPE_LINE )
    {
        GvLineShape *line = (GvLineShape *) shape;

        if( ring == 0 && node >= 0 && node < line->num_nodes )
            return line->xyz_nodes[node*3+off];
        else
            return 0.0;
    }
    else if( gv_shape_type(shape) == GVSHAPE_POINT )
    {
        GvPointShape *point = (GvPointShape *) shape;

        if( ring != 0 || node != 0 )
            return 0.0;
        else if( off == 0 )
            return point->x;
        else if( off == 1 )
            return point->y;
        else if( off == 2 )
            return point->z;
    }

    return 0.0;
}

/************************************************************************/
/*                          gv_shape_set_xyz()                          */
/************************************************************************/
gint gv_shape_set_xyz( GvShape *shape, gint ring, gint node,
                       gvgeocoord x, gvgeocoord y, gvgeocoord z )

{
    if( gv_shape_type(shape) == GVSHAPE_AREA )
    {
        GvAreaShape *area = (GvAreaShape *) shape;

        if( ring < 0 || node < 0 )
            return FALSE;

        /* do we need to grow the ring list? */
        if( ring >= area->num_rings )
        {
            area->num_ring_nodes =
                g_renew(int, area->num_ring_nodes, ring+1 );
            memset( area->num_ring_nodes + area->num_rings, 0,
                    sizeof(int) * (ring + 1 - area->num_rings) );

            area->xyz_ring_nodes =
                g_renew(gvgeocoord *, area->xyz_ring_nodes, ring+1 );
            memset( area->xyz_ring_nodes + area->num_rings, 0,
                    sizeof(gvgeocoord *) * (ring + 1 - area->num_rings) );
            area->num_rings = ring+1;
        }

        /* do we need to grow the list of nodes? */
        if( area->num_ring_nodes[ring] <= node )
        {
            area->xyz_ring_nodes[ring] =
                g_renew(gvgeocoord, area->xyz_ring_nodes[ring], 3*(node+1) );
            memset( area->xyz_ring_nodes[ring] + 3*area->num_ring_nodes[ring],
                    0, sizeof(gvgeocoord)*3*(node+1-area->num_ring_nodes[ring]) );
            area->num_ring_nodes[ring] = node+1;
        }

        area->xyz_ring_nodes[ring][node*3  ] = x;
        area->xyz_ring_nodes[ring][node*3+1] = y;
        area->xyz_ring_nodes[ring][node*3+2] = z;

        area->fill_objects = -1;

        return TRUE;
    }
    else if( gv_shape_type(shape) == GVSHAPE_LINE )
    {
        GvLineShape *line = (GvLineShape *) shape;

        if( ring != 0 || node < 0 )
            return FALSE;

        if( node >= line->num_nodes )
        {
            line->xyz_nodes = g_renew(gvgeocoord, line->xyz_nodes, 3*(node+1));

            memset( line->xyz_nodes + line->num_nodes*3,
                    0, (node + 1 - line->num_nodes) * 3 * sizeof(gvgeocoord) );

            line->num_nodes = node+1;
        }

        line->xyz_nodes[node*3  ] = x;
        line->xyz_nodes[node*3+1] = y;
        line->xyz_nodes[node*3+2] = z;

        return TRUE;
    }
    else if( gv_shape_type(shape) == GVSHAPE_POINT )
    {
        GvPointShape *point = (GvPointShape *) shape;

        if( ring != 0 || node != 0 )
            return FALSE;

        point->x = x;
        point->y = y;
        point->z = z;

        return TRUE;
    }

    return FALSE;
}

/************************************************************************/
/*                         gv_shape_add_node()                          */
/************************************************************************/
gint gv_shape_add_node( GvShape *shape, gint ring, gvgeocoord x, gvgeocoord y, gvgeocoord z )

{
    if( gv_shape_type(shape) == GVSHAPE_POINT )
    {
        gv_shape_set_xyz( shape, ring, 0, x, y, z );
        return 0;
    }
    else
    {
        int   new_node = gv_shape_get_nodes( shape, ring );

        gv_shape_set_xyz( shape, ring, new_node, x, y, z );

        return new_node;
    }
}

/************************************************************************/
/*                        gv_shape_insert_node()                        */
/************************************************************************/
gint gv_shape_insert_node( GvShape *shape, gint ring, int node,
                           gvgeocoord x, gvgeocoord y, gvgeocoord z )

{
    int     i;

    if( node > gv_shape_get_nodes( shape, ring ) )
        return -1;

    for( i = gv_shape_get_nodes(shape,ring)-1; i >= node; i-- )
    {
        gv_shape_set_xyz( shape, ring, i+1,
                          gv_shape_get_x( shape, ring, i ),
                          gv_shape_get_y( shape, ring, i ),
                          gv_shape_get_z( shape, ring, i ) );
    }

    gv_shape_set_xyz( shape, ring, node, x, y, z );

    return node;
}

/************************************************************************/
/*                        gv_shape_delete_ring()                        */
/************************************************************************/
gint gv_shape_delete_ring( GvShape *shape, gint ring )

{
    if( gv_shape_type(shape) == GVSHAPE_POINT && ring == 0 )
    {
        ((GvPointShape *) shape)->x = 0.0;
        ((GvPointShape *) shape)->y = 0.0;
        ((GvPointShape *) shape)->z = 0.0;
        return TRUE;
    }
    else if( gv_shape_type(shape) == GVSHAPE_LINE && ring == 0 )
    {
        GvLineShape   *line = (GvLineShape *) shape;

        line->num_nodes = 0;

        return TRUE;
    }
    else if( gv_shape_type(shape) == GVSHAPE_AREA
             && ring >= 0
             && ring < gv_shape_get_rings(shape) )
    {
        GvAreaShape   *area = (GvAreaShape *) shape;

        g_free( area->xyz_ring_nodes[ring] );
        memmove( area->xyz_ring_nodes + ring,
                 area->xyz_ring_nodes + ring + 1,
                 sizeof(void*) * (area->num_rings - ring - 1) );
        memmove( area->num_ring_nodes + ring,
                 area->num_ring_nodes + ring + 1,
                 sizeof(int) * (area->num_rings - ring - 1) );
        area->fill_objects = -1;
        area->num_rings--;
        return TRUE;
    }

    return FALSE;

}

/************************************************************************/
/*                        gv_shape_delete_node()                        */
/************************************************************************/
gint gv_shape_delete_node( GvShape *shape, gint ring, gint node )

{
    if( gv_shape_type(shape) == GVSHAPE_POINT && ring == 0 && node == 0 )
    {
        ((GvPointShape *) shape)->x = 0.0;
        ((GvPointShape *) shape)->y = 0.0;
        ((GvPointShape *) shape)->z = 0.0;
        return TRUE;
    }
    else if( gv_shape_type(shape) == GVSHAPE_LINE && ring == 0
             && node >= 0 && node < gv_shape_get_nodes(shape,ring) )
    {
        GvLineShape   *line = (GvLineShape *) shape;

        memmove( line->xyz_nodes + node*3,
                 line->xyz_nodes + (node+1)*3,
                 sizeof(gvgeocoord) * 3 * (line->num_nodes - node - 1) );
        line->num_nodes--;
        return TRUE;
    }
    else if( gv_shape_type(shape) == GVSHAPE_AREA
             && ring < gv_shape_get_rings(shape)
             && node < gv_shape_get_nodes(shape,ring) )
    {
        GvAreaShape   *area = (GvAreaShape *) shape;

        memmove( area->xyz_ring_nodes[ring] + node*3,
                 area->xyz_ring_nodes[ring] + (node+1)*3,
                sizeof(gvgeocoord) * 3 * (area->num_ring_nodes[ring] - node - 1) );
        area->num_ring_nodes[ring]--;
        area->fill_objects = -1;
        return TRUE;
    }

    return FALSE;
}

/************************************************************************/
/*                        gv_shape_get_extents()                        */
/************************************************************************/
void gv_shape_get_extents( GvShape *shape, GvRect *rect )

{
    rect->x = rect->y = rect->width = rect->height = 0.0;

    if( gv_shape_type(shape) == GVSHAPE_POINT )
    {
        GvPointShape  *point = (GvPointShape *) shape;

        rect->x = point->x;
        rect->y = point->y;
        rect->width = 0.0;
        rect->height = 0.0;
    }
    else if( gv_shape_type(shape) == GVSHAPE_LINE )
    {
        GvLineShape *line = (GvLineShape *) shape;
        int         i;

        if( line->num_nodes > 0 )
        {
            rect->x = line->xyz_nodes[0];
            rect->y = line->xyz_nodes[1];
            rect->width = 0.0;
            rect->height = 0.0;
        }

        for( i = 1; i < line->num_nodes; i++ )
        {
            gvgeocoord     x = line->xyz_nodes[i*3  ];
            gvgeocoord     y = line->xyz_nodes[i*3+1];

            if( x < rect->x )
            {
                rect->width = rect->x + rect->width - x;
                rect->x = x;
            }
            else if( x > rect->x + rect->width )
            {
                rect->width = x - rect->x;
            }

            if( y < rect->y )
            {
                rect->height = rect->y + rect->height - y;
                rect->y = y;
            }
            else if( y > rect->y + rect->height )
            {
                rect->height = y - rect->y;
            }
        }
    }
    else if( gv_shape_type(shape) == GVSHAPE_AREA )
    {
        GvAreaShape *area = (GvAreaShape *) shape;
        int          ring, node, first = TRUE;

        for( ring = 0; ring < area->num_rings; ring++ )
        {
            for( node = 0; node < area->num_ring_nodes[ring]; node++ )
            {
                gvgeocoord     x = area->xyz_ring_nodes[ring][node*3  ];
                gvgeocoord     y = area->xyz_ring_nodes[ring][node*3+1];

                if( first )
                {
                    rect->x = x;
                    rect->width = 0;
                    rect->y = y;
                    rect->height = 0;
                    first = FALSE;
                }

                if( x < rect->x )
                {
                    rect->width = rect->x + rect->width - x;
                    rect->x = x;
                }
                else if( x > rect->x + rect->width )
                {
                    rect->width = x - rect->x;
                }

                if( y < rect->y )
                {
                    rect->height = rect->y + rect->height - y;
                    rect->y = y;
                }
                else if( y > rect->y + rect->height )
                {
                    rect->height = y - rect->y;
                }
            }
        }
    }
    else if( gv_shape_type(shape) == GVSHAPE_COLLECTION )
    {
        GvCollectionShape *col = (GvCollectionShape *) shape;
        int          geom;

        for( geom = 0; geom < col->geom_count; geom++ )
        {
            if( geom == 0 )
                gv_shape_get_extents( col->geom_list[geom], rect );
            else
            {
                GvRect   this_rect;

                gv_shape_get_extents( col->geom_list[geom], &this_rect );

                if( this_rect.x < rect->x )
                {
                    rect->width += (rect->x - this_rect.x);
                    rect->x = this_rect.x;
                }
                if( this_rect.y < rect->y )
                {
                    rect->height += (rect->y - this_rect.y);
                    rect->y = this_rect.y;
                }
                if( this_rect.x + this_rect.width >
                    rect->x + rect->width )
                {
                    rect->width = this_rect.x + this_rect.width - rect->x;
                }
                if( this_rect.y + this_rect.height >
                    rect->y + rect->height )
                {
                    rect->height = this_rect.y + this_rect.height - rect->y;
                }
            }
        }
    }
}

/************************************************************************/
/*                     gv_shape_point_in_polygon()                      */
/*                                                                      */
/*      Algorithm as presented by Darel R. Finley at                    */
/*      http://freeweb.pdq.net/smokin/polygon/                          */
/************************************************************************/

gint gv_shape_point_in_polygon( GvShape *shape_poly, double x, double y )

{
    int      ring;
    int      oddNODES = FALSE;
    GvAreaShape *area = (GvAreaShape *) shape_poly;

    if( gv_shape_type(shape_poly) != GVSHAPE_AREA )
        return FALSE;

    for( ring = 0; ring < area->num_rings; ring++ )
    {
        int edge;

        for( edge = 0; edge < area->num_ring_nodes[ring]; edge++ )
        {
            int j;
            gvgeocoord  *pt1, *pt2;

            if( edge == area->num_ring_nodes[ring]-1 )
                j = 0;
            else
                j = edge + 1;

            pt1 = area->xyz_ring_nodes[ring] + edge * 3;
            pt2 = area->xyz_ring_nodes[ring] + j * 3;

            if( (pt1[1] < y && pt2[1] >= y)
                || (pt2[1] < y && pt1[1] >= y) )
            {
                if( pt1[0] + (y-pt1[1]) / (pt2[1]-pt1[1]) * (pt2[0]-pt1[0])
                    < x )
                    oddNODES = !oddNODES;
            }
        }
    }

    return oddNODES;
}

/************************************************************************/
/*                     gv_shape_distance_from_polygon()                      */
/*                                                                      */
/************************************************************************/
gdouble gv_shape_distance_from_polygon( GvShape *shape_poly, double x, double y )

{
    int      ring;
    int vertical, horizontal;
    GvAreaShape *area = (GvAreaShape *) shape_poly;
    gvgeocoord  *ptA, *ptB, ptD[2];
    gdouble min_dist, dist, a,b,c;
    
    if ( gv_shape_type(shape_poly) != GVSHAPE_AREA )
        return -1.;

    /*initialize min distance*/
    ptA = area->xyz_ring_nodes[0];    
    min_dist = sqrt( pow((ptA[0]-x),2) + pow((ptA[1]-y),2));

    for( ring = 0; ring < area->num_rings; ring++ )
    {
        int edge;

        for( edge = 0; edge < area->num_ring_nodes[ring]; edge++ )
        {
            int j;

            if( edge == area->num_ring_nodes[ring]-1 )
                j = 0;
            else
                j = edge + 1;

            ptA = area->xyz_ring_nodes[ring] + edge * 3;
            ptB = area->xyz_ring_nodes[ring] + j * 3;

	    vertical = ( fabs(ptA[0] - ptB[0]) < 1.e-10 );
	    horizontal = ( fabs(ptA[1] - ptB[1]) < 1.e-10 );
	    if (vertical && horizontal) 
	      {
		dist = sqrt( pow((ptA[0]-x),2) + pow((ptA[1]-y),2));
		if ( dist < min_dist )
		  min_dist  = dist;
		continue;
	      }
	    else if ( vertical )
	      {
		ptD[0] = ptA[0];
		ptD[1] = y;
	      }
	    else if ( horizontal )
	      {
		ptD[0] = x;
		ptD[1] = ptA[0];
	      }
	    else
	      {
		/*Compute equation of line AB of the form y = ax + b */
		a = (ptA[1] - ptB[1]) / ( ptA[0] - ptB[0] );
		b = ptA[1] - a * ptA[0];

		/*Given D the intersection of AB and the perpendicular to (AB) passing by C,
		  compute equation of (CD) of the form y = -a x + c */
		c = y + x / a;

                /*Location of D*/
		ptD[1] = (a*a * c + b) / (1. + a*a);
		ptD[0] = (ptD[1] - b)/ a;

	      }
	    /* Make sure D is within segment AB, if not compute distance from A or B, whichever is the closet */
	    if ( ptA[0] < ptB[0] )
	      {
		if ( ptD[0] < ptA[0] )
		  {
		    ptD[0] = ptA[0];
		    ptD[1] = ptA[1];
		  }

		else if ( ptD[0] > ptB[0] )
		  {
		    ptD[0] = ptB[0];
		    ptD[1] = ptB[1];
		  }
	      }
	    else if ( ptB[0] < ptA[0] )
	      {
		if ( ptD[0] < ptB[0] )
		  {
		    ptD[0] = ptB[0];
		    ptD[1] = ptB[1];
		  }

		else if ( ptD[0] > ptA[0] )
		  {
		    ptD[0] = ptA[0];
		    ptD[1] = ptA[1];
		  }
	      }
	    else if ( ptA[1] < ptB[1] )
	      {
		if ( ptD[1] < ptA[1] )
		  {
		    ptD[0] = ptA[0];
		    ptD[1] = ptA[1];
		  }
		else if ( ptD[1] > ptB[1] )
		  {
		    ptD[0] = ptB[0];
		    ptD[1] = ptB[1];
		  }
	      }
	    else if ( ptB[1] < ptA[1] )
	      {
		if ( ptD[1] < ptB[1] )
		  {
		    ptD[0] = ptB[0];
		    ptD[1] = ptB[1];
		  }
		else if ( ptD[1] > ptA[1] )
		  {
		    ptD[0] = ptA[0];
		    ptD[1] = ptA[1];
		  }
	      }   



	    dist = sqrt( pow((ptD[0]-x),2) + pow((ptD[1]-y),2));
	    if ( dist < min_dist )
	      min_dist  = dist;

        }
    }

    return min_dist;
}



/************************************************************************/
/*                           _gv_intersect()                            */
/************************************************************************/

static void _gv_intersect( double *x1, double *y1, double *x2, double *y2,
                          double cut_line, int greater_flag )

{
    double  f;

    if( !greater_flag && (*x1 < cut_line && *x2 < cut_line) )
        return;
    else if( greater_flag && (*x1 < cut_line && *x2 < cut_line) )
        return;

    if( !greater_flag )
    {
        *x1 = - *x1;
        *x2 = - *x2;
        cut_line = - cut_line;
    }

    if( *x1 < cut_line && *x2 >= cut_line )
    {
        f = (cut_line - *x1) / (*x2 - *x1);
        *y1 = *y1 + (*y2 - *y1) * f;
        *x1 = *x1 + (*x2 - *x1) * f;
    }
    else if( *x2 < cut_line && *x1 >= cut_line )
    {
        f = (cut_line - *x2) / (*x1 - *x2);
        *y2 = *y2 + (*y1 - *y2) * f;
        *x2 = *x2 + (*x1 - *x2) * f;
    }

    if( !greater_flag )
    {
        *x1 = - *x1;
        *x2 = - *x2;
        cut_line = - cut_line;
    }
}

/************************************************************************/
/*                       gv_line_intersect_rect()                       */
/************************************************************************/

int gv_line_intersect_rect( double *x1, double *y1,
                            double *x2, double *y2,
                            GvRect *rect )

{
    _gv_intersect( x1, y1, x2, y2, rect->x, TRUE );
    _gv_intersect( x1, y1, x2, y2, rect->x + rect->width, FALSE );
    _gv_intersect( y1, x1, y2, x2, rect->y, TRUE );
    _gv_intersect( y1, x1, y2, x2, rect->y + rect->height, FALSE );

    if( *x1 < rect->x || *x2 < rect->x
        || *y1 < rect->y || *y2 < rect->y
        || *x1 > rect->x + rect->width
        || *x2 > rect->x + rect->width
        || *y1 > rect->y + rect->height
        || *y2 > rect->y + rect->height )
        return 0;

    return 1;
}


/************************************************************************/
/*                       gv_shape_clip_to_rect()                        */
/************************************************************************/

GvShape *gv_shape_clip_to_rect( GvShape *shape, GvRect *rect )

{
    GvRect extents;
    int    ring, out_ring = 0;
    GvShape *new_shape;

/* -------------------------------------------------------------------- */
/*      Get shape extents, and check for trivial cases of complete      */
/*      exclusion or inclusion.                                         */
/* -------------------------------------------------------------------- */
    gv_shape_get_extents( shape, &extents );

    if( extents.height < 0.0 )
    {
        extents.y = extents.y + extents.height;
        extents.height = -extents.height;
    }

    if( extents.x > rect->x + rect->width
        || extents.y > rect->y + rect->height
        || extents.x + extents.width < rect->x
        || extents.y + extents.height < rect->y )
        return NULL;

    if( extents.x >= rect->x
        && extents.x + extents.width <= rect->x + rect->width
        && extents.y >= rect->y
        && extents.y + extents.height <= rect->y + rect->height )
        return gv_shape_copy( shape );

    if( gv_shape_type(shape) == GVSHAPE_POINT )
        return gv_shape_copy( shape );

/* -------------------------------------------------------------------- */
/*      Process line segment by line segment                            */
/* -------------------------------------------------------------------- */
    new_shape = gv_shape_new( gv_shape_type(shape) );
    gv_properties_copy( &(shape->properties), &(new_shape->properties) );


    for( ring = 0; ring < gv_shape_get_rings(shape); ring++ )
    {
        double last_x, last_y, last_z, x, y;
        int node, last_inside, have_last_point;

        last_x = x = gv_shape_get_x(shape, ring, 0);
        last_y = y = gv_shape_get_y(shape, ring, 0);
        last_z = gv_shape_get_z(shape, ring, 0);

        if( x < rect->x || x > rect->x + rect->width
            || y < rect->y || y > rect->y + rect->height )
            last_inside = FALSE;
        else
        {
            last_inside = TRUE;
            gv_shape_add_node( new_shape, out_ring, last_x, last_y, last_z );
        }

        have_last_point = last_inside;

        for( node = 1; node < gv_shape_get_nodes(shape,ring); node++ )
        {
            int this_inside, z;

            x = gv_shape_get_x(shape, ring, node);
            y = gv_shape_get_y(shape, ring, node);
            z = gv_shape_get_z(shape,ring,node);

            if( x < rect->x || x > rect->x + rect->width
                || y < rect->y || y > rect->y + rect->height )
                this_inside = FALSE;
            else
                this_inside = TRUE;

            if( this_inside && last_inside )
            {
                last_x = x;
                last_y = y;
                last_z = z;

                gv_shape_add_node( new_shape, out_ring, x, y, z );
            }
            else if( last_inside && !this_inside )
            {
                gv_line_intersect_rect( &last_x, &last_y, &x, &y, rect );
                last_x = x;
                last_y = y;
                last_z = z;

                gv_shape_add_node( new_shape, out_ring, x, y, z );
            }
            else if( !last_inside )
            {
                double x_i, y_i;

                x_i = gv_shape_get_x(shape,ring,node-1);
                y_i = gv_shape_get_y(shape,ring,node-1);

                if( !gv_line_intersect_rect( &x_i, &y_i, &x, &y, rect ) )
                {
                    last_inside = FALSE;
                    continue;
                }

                if( have_last_point && (x_i != last_x && y_i != last_y) )
                {
                    if( (x_i == rect->x && last_x == rect->x + rect->width)
                        || (x_i == rect->x + rect->width
                            && last_x == rect->x) )
                    {
                        gv_shape_add_node( new_shape, out_ring,
                                           last_x, rect->y, z );
                        gv_shape_add_node( new_shape, out_ring,
                                           x_i, rect->y, z );
                    }
                    else if( (y_i == rect->y
                              && last_y == rect->y + rect->height)
                             || (y_i == rect->y + rect->height
                                 && last_y == rect->y) )
                    {
                        gv_shape_add_node( new_shape, out_ring,
                                           rect->x, last_y, z );
                        gv_shape_add_node( new_shape, out_ring,
                                           rect->x, y_i, z );
                    }
                    else if( x_i == rect->x || x_i == rect->x + rect->width )
                        gv_shape_add_node( new_shape, out_ring,
                                           x_i, last_y, z );
                    else
                        gv_shape_add_node( new_shape, out_ring,
                                           last_x, y_i, z );
                }

                gv_shape_add_node( new_shape, out_ring, x_i, y_i, z );
                gv_shape_add_node( new_shape, out_ring, x, y, z );

                last_x = x;
                last_y = y;
            }

            last_inside = this_inside;
        }

        if( gv_shape_get_nodes(new_shape,out_ring) > 0 )
            out_ring++;
    }

    return new_shape;
}

/************************************************************************/
/*                   gv_shape_collection_add_shape()                    */
/*                                                                      */
/*      Add a shape to a collection.  The passed shape is added         */
/*      directly (not copied), but the reference count is incremented.  */
/************************************************************************/

void gv_shape_collection_add_shape( GvShape *col_shape, GvShape *shape )

{
    GvCollectionShape *collection = (GvCollectionShape *) col_shape;

    if( gv_shape_type(col_shape) != GVSHAPE_COLLECTION )
        return;

    collection->geom_list = (GvShape **)
        g_renew(GvShape*, collection->geom_list, collection->geom_count+1);

    collection->geom_list[collection->geom_count] = shape;
    gv_shape_ref( shape );

    collection->geom_count++;
}

/************************************************************************/
/*                   gv_shape_collection_get_count()                    */
/************************************************************************/

int gv_shape_collection_get_count( GvShape *col_shape )

{
    GvCollectionShape *collection = (GvCollectionShape *) col_shape;

    if( gv_shape_type(col_shape) != GVSHAPE_COLLECTION )
        return 0;
    else
        return collection->geom_count;
}

/************************************************************************/
/*                   gv_shape_collection_get_shape()                    */
/************************************************************************/

GvShape *gv_shape_collection_get_shape( GvShape *col_shape, int shp_index )

{
    GvCollectionShape *collection = (GvCollectionShape *) col_shape;

    if( gv_shape_type(col_shape) != GVSHAPE_COLLECTION )
        return NULL;

    else if( shp_index < 0 || shp_index >= collection->geom_count )
        return NULL;

    else
        return collection->geom_list[shp_index];
}

/************************************************************************/
/*                        gv_shape_get_center()                         */
/************************************************************************/

gint gv_shape_get_center( GvShape *shape, GvVertex3d *xyz )

{
    GvRect extent;

    gv_shape_get_extents( shape, &extent );
    xyz->x = extent.x + extent.width * 0.5;
    xyz->y = extent.y + extent.height * 0.5;
    xyz->z = 0.0;

    return 1;
}

/************************************************************************/
/*                     gv_shape_update_attribute()                      */
/************************************************************************/

gint gv_shape_update_attribute( GvShape *shape,
                                const char *tool,
                                const char *attribute,
                                const char *update_value )

{
    const char *original_ogrfs;
    int ret = 0, tool_index, tool_count;
    char **tool_list;

/* -------------------------------------------------------------------- */
/*      Get the ogrfs to operate on and split it.                       */
/* -------------------------------------------------------------------- */
    original_ogrfs =
        gv_properties_get( gv_shape_get_properties(shape), "_gv_ogrfs" );
    if( original_ogrfs == NULL )
        return 0;

    tool_list = gv_ogrfs_split_tools( original_ogrfs );

/* -------------------------------------------------------------------- */
/*      Loop over tools changing them.                                  */
/* -------------------------------------------------------------------- */
    tool_count = CSLCount( tool_list );
    for( tool_index = 0; tool_index < tool_count; tool_index++ )
    {
        const char *id;
        char *remaining, *value, *next_remaining;
        int value_len;
        char *new_tool_string = NULL;

        if( tool != NULL && !EQUALN(tool_list[tool_index],tool,strlen(tool)) )
            continue;

        /* Loop over args */

        remaining = tool_list[tool_index];
        while( (id = gv_ogrfs_get_arg( remaining, &next_remaining, &value,
                                       &value_len )) != NULL )
        {
            if( EQUALN(id,attribute,strlen(attribute)) )
            {
                char *replacement_value = NULL;
                int value_offset = value - tool_list[tool_index];

                if( EQUAL(attribute,"a") )
                {
                    char new_value[100];

                    sprintf( new_value, "%.3g", atof(update_value)+atof(value) );
                    replacement_value = CPLStrdup( new_value );
                }
                else if( EQUAL(attribute,"s") )
                {
                    char new_value[100];

                    sprintf( new_value, "%.8g", atof(update_value)*atof(value) );
                    replacement_value = CPLStrdup( new_value );
                }
                else
                    replacement_value = CPLStrdup( update_value );

                new_tool_string = (char *)
                    CPLCalloc(1,strlen(tool_list[tool_index])+strlen(replacement_value)+1);
                strcpy( new_tool_string, tool_list[tool_index] );
                strcpy( new_tool_string+value_offset, replacement_value );
                strcat( new_tool_string, value + value_len );
                break;
            }
            remaining = next_remaining;
        }

        /* We got to the end of the args without the attribute, add it now. */
        if( id == NULL )
        {
            new_tool_string = (char *)
                CPLCalloc(1,strlen(tool_list[tool_index])+strlen(update_value) + strlen(attribute) + 10);
            strcpy( new_tool_string, tool_list[tool_index] );
            sprintf( new_tool_string + strlen(new_tool_string) - 1,
                     ",%s:%s)", attribute, update_value );
            ret = 1;
        }

        CPLFree( tool_list[tool_index] );
        tool_list[tool_index] = new_tool_string;
        ret = 1;
    }

/* -------------------------------------------------------------------- */
/*      If we got a replacement, update the _gv_ogrfs attribute.        */
/* -------------------------------------------------------------------- */
    if( ret )
    {
        int sum_tool_len = 1;
        char *new_ogrfs;

        for( tool_index = 0; tool_index < tool_count; tool_index++ )
            sum_tool_len += strlen(tool_list[tool_index]) + 1;

        new_ogrfs = (char *) CPLMalloc(sum_tool_len);

        for( tool_index = 0; tool_index < tool_count; tool_index++ )
        {
            if( tool_index == 0 )
                strcpy( new_ogrfs, tool_list[tool_index] );
            else
            {
                strcat( new_ogrfs, ";" );
                strcat( new_ogrfs, tool_list[tool_index] );
            }
        }

        gv_properties_set( gv_shape_get_properties(shape), "_gv_ogrfs",
                           new_ogrfs );
        CPLFree( new_ogrfs );
    }

    CSLDestroy( tool_list );

    return ret;
}

/************************************************************************/
/*                          gv_shape_rotate()                           */
/************************************************************************/

gint gv_shape_rotate( GvShape *shape, double angle_in_degrees )

{
    if( gv_shape_type( shape ) == GVSHAPE_POINT )
    {
        char angle_as_string[60];

        /* ogrfs angles are counter clockwise, angle_in_degrees is clockwise*/
        sprintf( angle_as_string, "%.3g", - angle_in_degrees );
        return gv_shape_update_attribute( shape, "SYMBOL", "a", angle_as_string);
    }
    else
    {
        GvVertex3d pivot;
        int ring_count, ring;

        if( !gv_shape_get_center( shape, &pivot ) )
            return 0;

        ring_count = gv_shape_get_rings( shape );
        for( ring = 0; ring < ring_count; ring++ )
        {
            int node_count, node;

            node_count = gv_shape_get_nodes( shape, ring );
            for( node = 0; node < node_count; node++ )
            {
                double new_x, new_y, rad_rot, dx, dy;

                rad_rot = (angle_in_degrees / 180.0) * M_PI;

                dx = gv_shape_get_x(shape,ring,node) - pivot.x;
                dy = gv_shape_get_y(shape,ring,node) - pivot.y;

                new_x = pivot.x + dx * cos(rad_rot) + dy * sin(rad_rot);
                new_y = pivot.y - dx * sin(rad_rot) + dy * cos(rad_rot);

                gv_shape_set_xyz( shape, ring, node,
                                  new_x, new_y,
                                  gv_shape_get_z( shape, ring, node ) );
            }
        }
    }

    return 0;
}

/************************************************************************/
/*                           gv_shape_scale()                           */
/************************************************************************/

gint gv_shape_scale( GvShape *shape, double new_scale )

{
    if( gv_shape_type( shape ) == GVSHAPE_POINT )
    {
        char scale_as_string[60];

        sprintf( scale_as_string, "%.3g", new_scale );
        return gv_shape_update_attribute( shape, "SYMBOL", "s", scale_as_string);
    }
    else
    {
        GvVertex3d pivot;
        int ring_count, ring;

        if( !gv_shape_get_center( shape, &pivot ) )
            return 0;

        ring_count = gv_shape_get_rings( shape );
        for( ring = 0; ring < ring_count; ring++ )
        {
            int node_count, node;

            node_count = gv_shape_get_nodes( shape, ring );
            for( node = 0; node < node_count; node++ )
            {
                double new_x, new_y;

                new_x = (gv_shape_get_x(shape,ring,node) - pivot.x) * new_scale
                    + pivot.x;
                new_y = (gv_shape_get_y(shape,ring,node) - pivot.y) * new_scale
                    + pivot.y;

                gv_shape_set_xyz( shape, ring, node,
                                  new_x, new_y,
                                  gv_shape_get_z( shape, ring, node ) );
            }
        }
    }

    return 0;
}

/************************************************************************/
/*                     gv_shape_add_ring_from_xml()                     */
/*                                                                      */
/*      This is just intended for local use to avoid                    */
/*      gv_shape_from_xml_tree() from being too complex.                */
/************************************************************************/

static void gv_shape_add_ring_from_xml( GvShape *psShape, CPLXMLNode *psNode )

{
    int  ring = gv_shape_get_rings( psShape );

    if( psNode == NULL || !EQUAL(psNode->pszValue,"ring") || psNode->psChild == NULL )
        return;

    if( gv_shape_type(psShape) != GVSHAPE_AREA )
        ring = 0;

/* -------------------------------------------------------------------- */
/*      The "new" case is that the contents of the ring element are     */
/*      one text value with a coordinate list in a similar format to    */
/*      what GML does.                                                  */
/* -------------------------------------------------------------------- */
    if( psNode->psChild->psNext == NULL
        && psNode->psChild->eType == CXT_Text )
    {
        char    **triplets, **coords;
        const char *value;
        int    node;

        value = psNode->psChild->pszValue;
        triplets = CSLTokenizeStringComplex( value, " \t\r\n",
                                             FALSE, FALSE );
        node = 0;
        while ( triplets[node] )
        {
            coords = CSLTokenizeStringComplex( triplets[node], ",",
                                               FALSE, FALSE );
            if( CSLCount(coords) == 3 )
            {
                gv_shape_add_node( psShape, ring, atof(coords[0]),
                                   atof(coords[1]), atof(coords[2]) );
            }
            else if ( CSLCount(coords) == 2 )
            {
                gv_shape_add_node( psShape, ring, atof(coords[0]),
                                   atof(coords[1]), 0.0 );
            }
            node++;

            if( coords )
                CSLDestroy( coords );
        }
                
        if( triplets )
            CSLDestroy( triplets );
    }
/* -------------------------------------------------------------------- */
/*      The "old" case is having each node as a subelement, and each    */
/*      x, y, z ordinate as a subelement.                               */
/* -------------------------------------------------------------------- */
    else
    {
        for( psNode = psNode->psChild; psNode != NULL; psNode = psNode->psNext)
        {
            if( EQUAL(psNode->pszValue,"node") )
            {
                double x, y, z;

                x = atof(CPLGetXMLValue(psNode,"x", "0.0"));
                y = atof(CPLGetXMLValue(psNode,"y", "0.0"));
                z = atof(CPLGetXMLValue(psNode,"z", "0.0"));

                gv_shape_add_node( psShape, ring, x, y, z );
            }
        }
    }
}

/************************************************************************/
/*                     gv_shape_property_from_xml()                     */
/************************************************************************/

static void gv_shape_property_from_xml( GvShape *psShape, 
                                        CPLXMLNode *psProperty )

{
    const char *pszName, *pszValue;
                
    pszName = CPLGetXMLValue( psProperty, "name", "" );
    pszValue = CPLGetXMLValue( psProperty, "value", NULL );

    if( pszValue == NULL && psProperty->psChild != NULL
        && psProperty->psChild->psNext != NULL 
        && psProperty->psChild->psNext->eType == CXT_Text )
        pszValue = psProperty->psChild->psNext->pszValue;
                
    if( strlen(pszName) > 0 && pszValue != NULL )
    {
        gv_properties_set( &(psShape->properties), 
                           pszName, pszValue );
    }
}

/************************************************************************/
/*                       gv_shape_from_xml_tree()                       */
/*                                                                      */
/*      Convert an XML tree representation of a GvShape into an         */
/*      instance.                                                       */
/************************************************************************/

GvShape *gv_shape_from_xml_tree( CPLXMLNode *psTree )

{
    int        nGType;
    GvShape    *psShape;
    static const char *pszError = "Invalid GvShape XML tree in gv_shape_from_xml_tree()";

    if( psTree == NULL || psTree->eType != CXT_Element
        || !EQUAL(psTree->pszValue,"GvShape") )
    {
        CPLError( CE_Failure, CPLE_AppDefined, pszError );
        return NULL;
    }

    nGType = atoi(CPLGetXMLValue( psTree, "type", "-1" ));
    if( nGType == -1 )
    {
        CPLError( CE_Failure, CPLE_AppDefined, pszError );
        return NULL;
    }

    psShape = gv_shape_new( nGType );
    if( psShape == NULL )
        return NULL;

    for( psTree = psTree->psChild; psTree != NULL; psTree = psTree->psNext )
    {
        if( EQUAL(psTree->pszValue,"rings") )
        {
            CPLXMLNode *psRing;

            for( psRing = psTree->psChild; psRing != NULL; 
                 psRing = psRing->psNext )
            {
                if( EQUAL(psRing->pszValue,"ring") )
                {
                    gv_shape_add_ring_from_xml( psShape, psRing );
                }
            }
        }
        else if( EQUAL(psTree->pszValue,"Properties") )
        {
            CPLXMLNode *psProperty;

            for( psProperty = psTree->psChild; psProperty != NULL;
                 psProperty = psProperty->psNext )
            {
                gv_shape_property_from_xml( psShape, psProperty );
            }
        }
        /* The old format has properties directly below the GvShape */
        else if( EQUAL(psTree->pszValue,"Property") )
        {
            gv_shape_property_from_xml( psShape, psTree );
        }
        else if( EQUAL(psTree->pszValue,"SubShapes") )
        {
            CPLXMLNode *psChild;

            for( psChild = psTree->psChild; psChild != NULL;
                 psChild = psChild->psNext )
            {
                GvShape *psSubshape = gv_shape_from_xml_tree( psChild );
                if( psSubshape != NULL )
                    gv_shape_collection_add_shape( psShape, psSubshape );
            }
        }
    }

    return psShape;
}

/************************************************************************/
/*                        gv_shape_to_xml_tree()                        */
/************************************************************************/

CPLXMLNode *gv_shape_to_xml_tree( GvShape *psShape )

{
    CPLXMLNode *psRoot; 
    int iRing, nRingCount, nPropertyCount;
    char szText[100];
    GvProperties *psProps;

    psRoot = CPLCreateXMLNode( NULL, CXT_Element, "GvShape" );
    
    sprintf( szText, "%d", gv_shape_type(psShape) );
    CPLCreateXMLNode( 
        CPLCreateXMLNode( psRoot, CXT_Attribute, "type" ),
        CXT_Text, szText );

/* -------------------------------------------------------------------- */
/*      Translate "rings" into coordinate lists.                        */
/* -------------------------------------------------------------------- */
    if( gv_shape_get_rings( psShape ) > 0 
        && gv_shape_get_nodes( psShape, 0 ) > 0  )
    {
        CPLXMLNode *psRings = CPLCreateXMLNode( psRoot, CXT_Element, "rings");

        nRingCount = gv_shape_get_rings( psShape );
        for( iRing = 0; iRing < nRingCount; iRing++ )
        {
            char *pszCoordList;
            int  nMaxLen, nUsed = 0, iNode;
            int  nNodeCount = gv_shape_get_nodes( psShape, iRing );

            nMaxLen = nNodeCount * 60;
            pszCoordList = (char *) CPLMalloc(nMaxLen);
            
            for( iNode = 0; iNode < nNodeCount; iNode++ )
            {
                double x, y, z;

                if( iNode > 0 )
                    pszCoordList[nUsed++] = ' ';

                x = gv_shape_get_x( psShape, iRing, iNode );
                y = gv_shape_get_y( psShape, iRing, iNode );
                z = gv_shape_get_z( psShape, iRing, iNode );

                if( x == (int) x && y == (int) y && z == (int) z )
                    sprintf( pszCoordList+nUsed, "%d,%d,%d", 
                             (int) x, (int) y, (int) z );
                else if( fabs(x) < 370 && fabs(y) < 370 )
                    sprintf( pszCoordList+nUsed, "%.16g,%.16g,%.16g", 
                             x, y, z );
                else if( fabs(x) > 100000000.0 || fabs(y) > 100000000.0 
                         || fabs(z) > 100000000.0 )
                    sprintf( pszCoordList+nUsed, "%.16g,%.16g,%.16g", 
                             x, y, z );
                else
                    sprintf( pszCoordList+nUsed, "%.3f,%.3f,%.3f", x, y, z );
                nUsed += strlen(pszCoordList+nUsed);
            }

            CPLCreateXMLElementAndValue( psRings, "ring", pszCoordList );
            CPLFree( pszCoordList );
        }
    }

/* -------------------------------------------------------------------- */
/*      Translate properties.                                           */
/* -------------------------------------------------------------------- */
    psProps = gv_shape_get_properties(psShape);
    nPropertyCount = gv_properties_count( psProps );
    
    if( nPropertyCount > 0 )
    {
        int iProperty;
        CPLXMLNode *psPRoot = CPLCreateXMLNode( psRoot, CXT_Element, 
                                                "Properties" );
        
        for( iProperty = 0; iProperty < nPropertyCount; iProperty++ )
        {
            CPLXMLNode *psProperty;
            
            psProperty = CPLCreateXMLNode( psPRoot, CXT_Element, "Property" );
            
            CPLCreateXMLNode( 
                CPLCreateXMLNode( psProperty, CXT_Attribute, "name" ), 
                CXT_Text, gv_properties_get_name_by_index(psProps,iProperty) );
            CPLCreateXMLNode( psProperty, CXT_Text, 
                    gv_properties_get_value_by_index(psProps,iProperty) );
        }
    }

/* -------------------------------------------------------------------- */
/*      Translate sub-shapes.                                           */
/* -------------------------------------------------------------------- */
    if( gv_shape_collection_get_count( psShape ) > 0 )
    {
        CPLXMLNode *psSubShapes = CPLCreateXMLNode( psRoot, CXT_Element, 
                                                    "SubShapes");
        int  iShape, nShapeCount;

        nShapeCount = gv_shape_collection_get_count( psShape );
        for( iShape = 0; iShape < nShapeCount; iShape++ )
        {
            CPLAddXMLChild( 
                psSubShapes,
                gv_shape_to_xml_tree( 
                    gv_shape_collection_get_shape( psShape, iShape ) ) );
        }
    }

    return psRoot;
}
