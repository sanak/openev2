/******************************************************************************
 * $Id$
 *
 * Project:  OpenEV
 * Purpose:  Rotation and Scaling editing mode in GvShapesLayer.
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 * Maintainer: Mario Beauchamp, starged@gmail.com
 *
 ******************************************************************************
 * Copyright (c) 2003, Frank Warmerdam <warmerdam@pobox.com>
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
 */

#include <stdio.h>
#include <gdk/gdkkeysyms.h>
#include "gvrotatetool.h"
#include "gvundo.h"
#include "cpl_error.h"
#include <GL/gl.h>
#include <math.h>

static void gv_rotate_tool_class_init(GvRotateToolClass *klass);
static void gv_rotate_tool_init(GvRotateTool *tool);
static gboolean gv_rotate_tool_draw(GvRotateTool *tool);
static gboolean gv_rotate_tool_button_press(GvTool *tool, GdkEventButton *event);
static gboolean gv_rotate_tool_button_release(GvTool *tool, GdkEventButton *event);
static gboolean gv_rotate_tool_motion_notify(GvTool *tool, GdkEventMotion *event);
static void gv_rotate_tool_key_press(GvTool *tool, GdkEventKey *event);
static void gv_rotate_tool_deactivate(GvTool *tool, GvViewArea *view);
static gint gv_rotate_tool_configure(GvRotateTool *tool);

#define HEAD_SIZE 6
#define ARROW_SIZE 45

enum
{
    RRMODE_DISPLAY,
    RRMODE_ROTATE, 
    RRMODE_SCALE,
    RRMODE_ROTATESCALE
};

#ifndef M_PI
#  define M_PI 3.14159265358979323846
#endif

GType
gv_rotate_tool_get_type(void)
{
    static GType rotate_tool_type = 0;

    if (!rotate_tool_type) {
        static const GTypeInfo rotate_tool_info =
        {
            sizeof(GvRotateToolClass),
            (GBaseInitFunc) NULL,
            (GBaseFinalizeFunc) NULL,
            (GClassInitFunc) gv_rotate_tool_class_init,
            /* reserved_1 */ NULL,
            /* reserved_2 */ NULL,
            sizeof(GvRotateTool),
            0,
            (GInstanceInitFunc) gv_rotate_tool_init,
        };
        rotate_tool_type = g_type_register_static (GV_TYPE_TOOL,
                                                  "GvRotateTool",
                                                  &rotate_tool_info, 0);
        }

    return rotate_tool_type;
}

static void
gv_rotate_tool_class_init(GvRotateToolClass *klass)
{
    GvToolClass *tool_class = GV_TOOL_CLASS (klass);

    tool_class->deactivate = gv_rotate_tool_deactivate;
    tool_class->button_press = gv_rotate_tool_button_press;
    tool_class->button_release = gv_rotate_tool_button_release;
    tool_class->motion_notify = gv_rotate_tool_motion_notify;
    tool_class->key_press = gv_rotate_tool_key_press;
}

static void
gv_rotate_tool_init(GvRotateTool *tool)
{
    GV_TOOL(tool)->cursor = gdk_cursor_new(GDK_TCROSS);
    tool->layer = NULL;
    tool->named_layer = NULL;
    tool->rrmode = RRMODE_DISPLAY;
    tool->shape_id = -1;
    tool->rotation = 0.0;
    tool->scaling = 1.0;
    tool->original = NULL;
}

GvTool *
gv_rotate_tool_new(void)
{
    GvRotateTool *tool = g_object_new(GV_TYPE_ROTATE_TOOL, NULL);

    return GV_TOOL(tool);
}

static gint gv_rotate_tool_layer_destroy(GvTool *rtool)
{
    GvRotateTool *tool = GV_ROTATE_TOOL(rtool);

    if (tool->layer)
        gv_rotate_tool_set_layer(tool, NULL);

    return 0;
}

/************************************************************************/
/*                      gv_rotate_tool_terminate()                      */
/*                                                                      */
/*      This breaks out of a non-display mode, restores the original    */
/*      shape and generally restores a sane state.                      */
/************************************************************************/

void 
gv_rotate_tool_terminate(GvRotateTool *tool)
{
    if( tool->rrmode == RRMODE_DISPLAY )
        return;

    if( tool->original != NULL 
        && gv_shapes_get_shape( tool->layer->data, tool->shape_id) != NULL )
    {
        gv_shapes_replace_shapes( tool->layer->data, 1, &(tool->shape_id), 
                                  &(tool->original), FALSE );
        tool->original = NULL;
        gv_undo_enable();
        gv_undo_open();
    }
    else if( tool->original != NULL )
    {
        gv_shape_delete( tool->original );
        tool->original = NULL;
        gv_undo_enable();
        gv_undo_open();
    }

    tool->shape_id = -1;
    tool->rrmode = RRMODE_DISPLAY;
    gv_view_area_queue_draw(GV_TOOL(tool)->view);
}

/************************************************************************/
/*                      gv_rotate_tool_set_layer()                      */
/************************************************************************/
void
gv_rotate_tool_set_layer(GvRotateTool *tool, GvShapeLayer *layer)
{
    if (GV_TOOL(tool)->view == NULL)
    {
        g_warning("gv_rotate_tool_set_layer(): inactive tool");
        return;
    }

    if( layer != NULL && gv_data_is_read_only( GV_DATA(layer) ) )
    {
        g_warning( "gv_rotate_tool_set_layer(): layer is read-only" );
        return;
    }

    gv_rotate_tool_terminate( tool );
    tool->shape_id = -1;

    /* Disconnect from the previous layer (for draw) */
    if (tool->layer)
    {
        tool->rrmode = RRMODE_DISPLAY;

        /** TODO: Not sure that we need to unselect ... try to remove later */

        gv_shape_layer_clear_selection(GV_SHAPE_LAYER(tool->layer));
        g_signal_handlers_disconnect_matched (tool->layer, G_SIGNAL_MATCH_DATA,
                                                0, 0, NULL, NULL, tool);
        gv_view_area_queue_draw(GV_TOOL(tool)->view);
    }

    if( layer == NULL )
        tool->layer = NULL;
    else
        tool->layer = GV_SHAPES_LAYER(layer);

    if (layer)
    {
        gv_view_area_set_active_layer(GV_TOOL(tool)->view, G_OBJECT(layer));

        /* Redraw when the layer draws */
        g_signal_connect_object(layer, "draw",
                                G_CALLBACK(gv_rotate_tool_draw),
                                GV_TOOL(tool), G_CONNECT_SWAPPED | G_CONNECT_AFTER);

        /* recover if layer destroyed */
        g_signal_connect_swapped(layer, "teardown",
                                G_CALLBACK(gv_rotate_tool_layer_destroy),
                                tool);
    }
}

/************************************************************************/
/*                   gv_rotate_tool_set_named_layer()                   */
/************************************************************************/

void
gv_rotate_tool_set_named_layer(GvRotateTool *tool, gchar *name)
{
    if (tool->named_layer)
    {
        g_free(tool->named_layer);
        tool->named_layer = NULL;
    }
    if (name)
        tool->named_layer = g_strdup(name);     
    /* Tool layer will be updated next time it is configured */
}

/************************************************************************/
/*                    gv_rotate_tool_setup_arrows()                     */
/*                                                                      */
/*      This function will compute the pivot location for the           */
/*      selected shape, and the corresponding up and right vectors      */
/*      for drawing the rotate/resize arrows.                           */
/************************************************************************/

static gint gv_rotate_tool_setup_arrows( GvRotateTool *tool )
{
    GvVertex3d pivot_3d;
    GvShape *shape = gv_shapes_get_shape( tool->layer->data, 
                                          tool->shape_id );

    if( shape == NULL )
    {
        CPLDebug( "OpenEV", "gv_rotate_tool_setup_arrows(), shape==NULL!" );
        tool->shape_id = -1;
        return 0;
    }

/* -------------------------------------------------------------------- */
/*      Compute the pivot location.                                     */
/* -------------------------------------------------------------------- */
    if( !gv_shape_get_center( shape, &pivot_3d ) )
        return 0;

    tool->v_pivot.x = pivot_3d.x;
    tool->v_pivot.y = pivot_3d.y;

/* -------------------------------------------------------------------- */
/*      Compute the up vector.                                          */
/* -------------------------------------------------------------------- */
    gv_view_area_correct_for_transform( GV_TOOL(tool)->view, 0.0, 1.0, 
                                        &(tool->v_up.x),
                                        &(tool->v_up.y) );
    gv_view_area_correct_for_transform( GV_TOOL(tool)->view, 1.0, 0.0, 
                                        &(tool->v_right.x),
                                        &(tool->v_right.y) );

    tool->rotation = 0.0;
    tool->scaling = 1.0;

    return 1;
}

/************************************************************************/
/*                    gv_rotate_tool_classify_hit()                     */
/*                                                                      */
/*      Does the mouse location in geo-coordinates hit near the head    */
/*      of the rotate or resize tool?                                   */
/************************************************************************/

int gv_rotate_tool_classify_hit( GvRotateTool *tool, gvgeocoord x, gvgeocoord y )
{
    gvgeocoord x_focus, y_focus, x_ul, y_ul, x_lr, y_lr;

    x_focus = tool->v_pivot.x + tool->v_up.x * ARROW_SIZE;
    y_focus = tool->v_pivot.y + tool->v_up.y * ARROW_SIZE;

    x_ul = x_focus - tool->v_right.x * (HEAD_SIZE*2+3) - tool->v_up.x*HEAD_SIZE;
    y_ul = y_focus - tool->v_right.y * (HEAD_SIZE*2+3) - tool->v_up.y*HEAD_SIZE;
    x_lr = x_focus + tool->v_right.x * (HEAD_SIZE*2+3) + tool->v_up.x*HEAD_SIZE;
    y_lr = y_focus + tool->v_right.y * (HEAD_SIZE*2+3) + tool->v_up.y*HEAD_SIZE;

    if( x >= MIN(x_ul,x_lr) && x <= MAX(x_ul,x_lr)
        && y >= MIN(y_ul,y_lr) && y <= MAX(y_ul,y_lr) )
    {
        return RRMODE_ROTATE;
    }

    x_focus = tool->v_pivot.x + tool->v_right.x * ARROW_SIZE;
    y_focus = tool->v_pivot.y + tool->v_right.y * ARROW_SIZE;

    x_ul = x_focus - tool->v_right.x * HEAD_SIZE - tool->v_up.x * HEAD_SIZE;
    y_ul = y_focus - tool->v_right.y * HEAD_SIZE - tool->v_up.y * HEAD_SIZE;
    x_lr = x_focus + tool->v_right.x * 3 + tool->v_up.x * HEAD_SIZE;
    y_lr = y_focus + tool->v_right.y * 3 + tool->v_up.y * HEAD_SIZE;

    if( x >= MIN(x_ul,x_lr) && x <= MAX(x_ul,x_lr)
        && y >= MIN(y_ul,y_lr) && y <= MAX(y_ul,y_lr) )
    {
        return RRMODE_SCALE;
    }

    return RRMODE_DISPLAY;
}

/************************************************************************/
/*                        gv_rotate_tool_draw()                         */
/*                                                                      */
/*      Draw callback invoked by system after all regular layers are    */
/*      drawn.                                                          */
/************************************************************************/

static gboolean
gv_rotate_tool_draw(GvRotateTool *tool)
{
/* -------------------------------------------------------------------- */
/*      If we have a selected shape, we need to draw the                */
/*      rotate/resize arrow handles.                                    */
/* -------------------------------------------------------------------- */
    if (tool->shape_id != -1 )
    {
        GvVertex v_up, v_right;
        double rad_rot;

        /*
        ** In display mode recompute the pivot, up and right vector each
        ** redraw. 
        */
        if( tool->rrmode == RRMODE_DISPLAY )
        {
            if( !gv_rotate_tool_setup_arrows( tool ) )
                return FALSE;
        }

        /* 
        ** Apply scaling and rotation.
        */

        rad_rot = (tool->rotation / 180.0) * M_PI;
        v_up.x = tool->v_up.x*cos(rad_rot) + tool->v_up.y*sin(rad_rot);
        v_up.y = - tool->v_up.x*sin(rad_rot) + tool->v_up.y*cos(rad_rot);
        v_right.x = tool->v_right.x*cos(rad_rot) + tool->v_right.y*sin(rad_rot);
        v_right.y = -tool->v_right.x*sin(rad_rot) + tool->v_right.y*cos(rad_rot);

        v_up.x *= tool->scaling;
        v_up.y *= tool->scaling;
        v_right.x *= tool->scaling;
        v_right.y *= tool->scaling;

        glColor4f( 1.0, 0.0, 0.0, 1.0 );

        if( tool->rrmode != RRMODE_SCALE )
        {
            /** Upward line with left/right rotate arrow heads **/
            glBegin( GL_LINES );
            glVertex2( tool->v_pivot.x, tool->v_pivot.y );
            glVertex2( tool->v_pivot.x + v_up.x * ARROW_SIZE, 
                       tool->v_pivot.y + v_up.y * ARROW_SIZE );

            /* left pointing arrow */
            glVertex2( tool->v_pivot.x 
                       + v_up.x * ARROW_SIZE
                       - v_right.x * HEAD_SIZE*2, 
                       tool->v_pivot.y 
                       + v_up.y * ARROW_SIZE
                       - v_right.y * HEAD_SIZE*2 );
            glVertex2( tool->v_pivot.x + v_up.x * ARROW_SIZE, 
                       tool->v_pivot.y + v_up.y * ARROW_SIZE );

            glVertex2( tool->v_pivot.x 
                       + v_up.x * ARROW_SIZE
                       - v_right.x * HEAD_SIZE*2, 
                       tool->v_pivot.y 
                       + v_up.y * ARROW_SIZE
                       - v_right.y * HEAD_SIZE*2 );
            glVertex2( tool->v_pivot.x 
                       + v_up.x * (ARROW_SIZE+HEAD_SIZE)
                       - v_right.x * HEAD_SIZE, 
                       tool->v_pivot.y 
                       + v_up.y * (ARROW_SIZE+HEAD_SIZE)
                       - v_right.y * HEAD_SIZE );

            glVertex2( tool->v_pivot.x 
                       + v_up.x * ARROW_SIZE
                       - v_right.x * HEAD_SIZE*2, 
                       tool->v_pivot.y 
                       + v_up.y * ARROW_SIZE
                       - v_right.y * HEAD_SIZE*2 );
            glVertex2( tool->v_pivot.x 
                       + v_up.x * (ARROW_SIZE-HEAD_SIZE)
                       - v_right.x * HEAD_SIZE, 
                       tool->v_pivot.y 
                       + v_up.y * (ARROW_SIZE-HEAD_SIZE)
                       - v_right.y * HEAD_SIZE );

            /* rightward  pointing arrow */
            glVertex2( tool->v_pivot.x 
                       + v_up.x * ARROW_SIZE
                       + v_right.x * HEAD_SIZE*2, 
                       tool->v_pivot.y 
                       + v_up.y * ARROW_SIZE
                       + v_right.y * HEAD_SIZE*2 );
            glVertex2( tool->v_pivot.x + v_up.x * ARROW_SIZE, 
                       tool->v_pivot.y + v_up.y * ARROW_SIZE );

            glVertex2( tool->v_pivot.x 
                       + v_up.x * ARROW_SIZE
                       + v_right.x * HEAD_SIZE*2, 
                       tool->v_pivot.y 
                       + v_up.y * ARROW_SIZE
                       + v_right.y * HEAD_SIZE*2 );
            glVertex2( tool->v_pivot.x 
                       + v_up.x * (ARROW_SIZE+HEAD_SIZE)
                       + v_right.x * HEAD_SIZE, 
                       tool->v_pivot.y 
                       + v_up.y * (ARROW_SIZE+HEAD_SIZE)
                       + v_right.y * HEAD_SIZE );

            glVertex2( tool->v_pivot.x 
                       + v_up.x * ARROW_SIZE
                       + v_right.x * HEAD_SIZE*2, 
                       tool->v_pivot.y 
                       + v_up.y * ARROW_SIZE
                       + v_right.y * HEAD_SIZE*2 );
            glVertex2( tool->v_pivot.x 
                       + v_up.x * (ARROW_SIZE-HEAD_SIZE)
                       + v_right.x * HEAD_SIZE, 
                       tool->v_pivot.y 
                       + v_up.y * (ARROW_SIZE-HEAD_SIZE)
                       + v_right.y * HEAD_SIZE );

            glEnd();
        }

        if( tool->rrmode != RRMODE_ROTATE )
        {

            /** Rightward arrow for resizing. **/
            glBegin( GL_LINES );
            glVertex2( tool->v_pivot.x, tool->v_pivot.y );
            glVertex2( tool->v_pivot.x + v_right.x * ARROW_SIZE, 
                       tool->v_pivot.y + v_right.y * ARROW_SIZE );

            glVertex2( tool->v_pivot.x 
                       + v_right.x * (ARROW_SIZE-HEAD_SIZE)
                       + v_up.x * HEAD_SIZE, 
                       tool->v_pivot.y 
                       + v_right.y * (ARROW_SIZE-HEAD_SIZE)
                       + v_up.y * HEAD_SIZE );
            glVertex2( tool->v_pivot.x + v_right.x * ARROW_SIZE, 
                       tool->v_pivot.y + v_right.y * ARROW_SIZE );

            glVertex2( tool->v_pivot.x 
                       + v_right.x * (ARROW_SIZE-HEAD_SIZE)
                       - v_up.x * HEAD_SIZE, 
                       tool->v_pivot.y 
                       + v_right.y * (ARROW_SIZE-HEAD_SIZE)
                       - v_up.y * HEAD_SIZE );
            glVertex2( tool->v_pivot.x + v_right.x * ARROW_SIZE, 
                       tool->v_pivot.y + v_right.y * ARROW_SIZE );

            glEnd();
        }
    }
    return FALSE;
}

/************************************************************************/
/*                    gv_rotate_tool_button_press()                     */
/************************************************************************/

static gboolean
gv_rotate_tool_button_press(GvTool *r_tool, GdkEventButton *event)
{
    GvRotateTool *tool = GV_ROTATE_TOOL(r_tool);

    if( event->state & (GDK_CONTROL_MASK|GDK_SHIFT_MASK) )
        return FALSE;

/* -------------------------------------------------------------------- */
/*      Have we selected an active control point on the scale/rotate    */
/*      dohickey?                                                       */
/* -------------------------------------------------------------------- */
    if( tool->rrmode == RRMODE_DISPLAY && tool->shape_id != -1 )
    {
        gv_view_area_map_pointer(GV_TOOL(tool)->view, event->x, event->y,
                                 &tool->v_head.x, &tool->v_head.y);

        /*
        ** Is this location a hit on an arrow head?
        */
        tool->rrmode = 
            gv_rotate_tool_classify_hit( tool, tool->v_head.x, tool->v_head.y );

        /*
        ** Copy the original state of this shape, and disable undo till we are
        ** done.
        */
        if( tool->rrmode != RRMODE_DISPLAY )
        {
            if( event->button != 1 )
                tool->rrmode = RRMODE_ROTATESCALE;

            tool->original = gv_shape_copy(
                gv_shapes_get_shape( tool->layer->data, tool->shape_id ));

            gv_undo_close();
            gv_undo_disable();
        }
    }

/* -------------------------------------------------------------------- */
/*      Are we selecting a shape?  Note, currently we cannot clear      */
/*      our selection in resize/rotate mode - should we be able to?     */
/* -------------------------------------------------------------------- */
    if (event->button == 1 && tool->rrmode == RRMODE_DISPLAY )
    {
        int        shape_id;

        if (!gv_rotate_tool_configure(tool)) return FALSE;

        if (gv_shape_layer_pick_shape(GV_SHAPE_LAYER(tool->layer), 
                                      GV_TOOL(tool)->view,
                                      event->x, event->y, &shape_id))
        {
            GvShape *shape;

            /* Is the shape rotatable? */
            shape = gv_shapes_get_shape( tool->layer->data, shape_id );

            if( TRUE )
            {
                gv_shape_layer_clear_selection(
                    GV_SHAPE_LAYER(tool->layer));
                gv_shape_layer_select_shape(
                    GV_SHAPE_LAYER(tool->layer), shape_id);
                tool->shape_id = shape_id;
                gv_view_area_queue_draw(GV_TOOL(tool)->view);
            }
        }
        return FALSE;
    }
    return FALSE;

}

/************************************************************************/
/*                   gv_rotate_tool_button_release()                    */
/************************************************************************/

static gboolean
gv_rotate_tool_button_release(GvTool *r_tool, GdkEventButton *event)
{
    GvRotateTool *tool = GV_ROTATE_TOOL(r_tool);

    if( tool->rrmode == RRMODE_DISPLAY )
        return FALSE;

    /* Put back original shape. */
    gv_shapes_replace_shapes( tool->layer->data, 1, &(tool->shape_id), 
                              &(tool->original), TRUE );

    /* re-enable undo */
    gv_undo_enable();
    gv_undo_open();

    /* Apply rotation or scaling to working shape. */
    if( tool->rrmode == RRMODE_SCALE 
        || tool->rrmode == RRMODE_ROTATESCALE )
        gv_shape_scale( tool->original, tool->scaling );

    if( tool->rrmode == RRMODE_ROTATE 
        || tool->rrmode == RRMODE_ROTATESCALE )
        gv_shape_rotate( tool->original, tool->rotation );

    /* Apply to the shapes object ... this will be undoable */
    gv_shapes_replace_shapes( tool->layer->data, 1, &(tool->shape_id), 
                              &(tool->original), TRUE );
    tool->original = NULL;

    tool->rrmode = RRMODE_DISPLAY;

    return FALSE;
}

/************************************************************************/
/*                    gv_rotate_tool_motion_notify()                    */
/************************************************************************/
static gboolean
gv_rotate_tool_motion_notify(GvTool *r_tool, GdkEventMotion *event)
{
    GvRotateTool *tool = GV_ROTATE_TOOL(r_tool);
    GvShape  *wrk_shape;

    if (tool->rrmode == RRMODE_DISPLAY )
        return FALSE;


    wrk_shape = gv_shape_copy(tool->original);

    /* Map pointer position to tail vertex */
    gv_view_area_map_pointer(GV_TOOL(tool)->view, event->x, event->y,
                             &tool->v_tail.x, &tool->v_tail.y);

    /* Compute rotation from base. */
    if( tool->rrmode == RRMODE_ROTATE 
        || tool->rrmode == RRMODE_ROTATESCALE )
    {
        double dx = tool->v_tail.x - tool->v_pivot.x;
        double dy = tool->v_tail.y - tool->v_pivot.y;

        if( fabs(dy) < fabs(dx)/1000000.0 )
            tool->rotation = M_PI/2;
        else
            tool->rotation = atan(fabs(dx/dy));


        if( dy < 0.0 && dx >= 0.0 )
            tool->rotation = M_PI - tool->rotation;
        else if( dy >= 0.0 && dx < 0.0 )
            tool->rotation = -tool->rotation;
        else if( dy < 0.0 && dx < 0.0 )
            tool->rotation -= M_PI;


        tool->rotation = (tool->rotation / M_PI) * 180;

        gv_shape_rotate( wrk_shape, tool->rotation );
    }

    /* Compute Scaling from base. */
    if( tool->rrmode == RRMODE_SCALE 
        || tool->rrmode == RRMODE_ROTATESCALE )
    {
        double dx, dy, old_length, new_length;

        dx = tool->v_tail.x - tool->v_pivot.x;
        dy = tool->v_tail.y - tool->v_pivot.y;
        new_length = sqrt(dx*dx + dy*dy);

        dx = tool->v_head.x - tool->v_pivot.x;
        dy = tool->v_head.y - tool->v_pivot.y;
        old_length = sqrt(dx*dx + dy*dy);

        tool->scaling = new_length / old_length;

        gv_shape_scale( wrk_shape, tool->scaling );
    }

    /* Apply to the shapes object ... this will create an undo step */
    gv_shapes_replace_shapes( tool->layer->data, 1, &(tool->shape_id), 
                              &wrk_shape, FALSE );

    gv_view_area_queue_draw(GV_TOOL(tool)->view);

    return FALSE;
}

/************************************************************************/
/*                      gv_rotate_tool_key_press()                      */
/************************************************************************/

static void
gv_rotate_tool_key_press(GvTool *rtool, GdkEventKey *event)
{
    GvRotateTool *tool = GV_ROTATE_TOOL(rtool);

    if (!gv_rotate_tool_configure(tool)) return;

    switch (event->keyval)
    {
        case GDK_Delete:
        case GDK_BackSpace:
        case GDK_Escape:
          if( tool->rrmode != RRMODE_DISPLAY )
          {
              gv_rotate_tool_terminate( tool );
          }
          break;
    }
}

static void
gv_rotate_tool_deactivate(GvTool *r_tool, GvViewArea *view)
{
    GvRotateTool *tool = GV_ROTATE_TOOL(r_tool);

    /* terminate any active modes */
    gv_rotate_tool_terminate(tool);

    /* Disconnect from layer */
    if (tool->layer)
        gv_rotate_tool_set_layer(tool, NULL);

    /* Call the parent class func */
    GV_TOOL_DEACTIVATE(tool, view);
}

static gint
gv_rotate_tool_configure(GvRotateTool *tool)
{
    /* Check that we still are working on the active layer */
    if (!tool->layer || G_OBJECT(tool->layer) !=
        gv_view_area_active_layer(GV_TOOL(tool)->view))
    {
        GObject *layer;

        if (tool->named_layer)
        {
            /* Look for named layer if given */
            layer = gv_view_area_get_named_layer(GV_TOOL(tool)->view,
                             tool->named_layer);
        }
        else
        {
            layer = gv_view_area_get_layer_of_type(GV_TOOL(tool)->view,
                                                   GV_TYPE_SHAPES_LAYER,
                                                   FALSE);
        }

        if (!layer)
        {
            g_warning("gv_rotate_tool_configure(): no shapes layer in view");
            return FALSE;
        }

        gv_rotate_tool_set_layer(tool, GV_SHAPE_LAYER(layer));
    }
    return tool->layer != NULL;
}
