/******************************************************************************
 * $Id$
 *
 * Project:  OpenEV
 * Purpose:  Base class for vector display layers (eventually this will
 *           merge with GvShapesLayer).
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
 * $Log: gvshapelayer.c,v $
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
 * Revision 1.26  2003/05/16 18:26:33  pgs
 * added initial code for propogating colors to sub-symbols
 *
 * Revision 1.25  2003/04/17 13:56:12  warmerda
 * added more assserts
 *
 * Revision 1.24  2003/04/09 16:52:23  pgs
 * added shadow, halo and bgcolor to LABELs
 *
 * Revision 1.23  2003/04/07 15:10:14  pgs
 * added pattern support to pen objects
 *
 * Revision 1.22  2003/02/28 16:47:25  warmerda
 * split up render part handling to support vector symbols
 *
 * Revision 1.21  2003/02/27 04:00:18  warmerda
 * added scale_dep flag handling
 *
 * Revision 1.20  2003/02/14 20:12:43  pgs
 * added support for line widths in PENs
 *
 * Revision 1.19  2002/11/15 05:04:43  warmerda
 * added LABEL anchor point support
 *
 * Revision 1.18  2002/11/14 20:11:22  warmerda
 * preliminary support for gvsymbolmanager from Paul
 *
 * Revision 1.17  2002/11/04 21:42:07  sduclos
 * change geometric data type name to gvgeocoord
 *
 * Revision 1.16  2002/09/30 20:50:18  warmerda
 * removed restart debug message
 *
 * Revision 1.15  2002/09/30 17:36:04  warmerda
 * Increate name buffer size in gv_shape_layer_pick_node(). This fixes an odd
 * crash on Linux/XiGraphics combo, and ensures that a hit is detected even if
 * there are overlapping nodes as occurs with the start node in a closed area.
 *
 * Revision 1.14  2002/02/22 20:16:07  warmerda
 * added brush tool support
 *
 * Revision 1.13  2002/02/22 19:27:16  warmerda
 * added support for pen tools
 *
 * Revision 1.12  2001/11/09 16:04:53  warmerda
 * default color to white in renderinfo
 *
 * Revision 1.11  2001/04/09 18:17:34  warmerda
 * added subselection, and renderinfo support
 *
 * Revision 1.10  2000/08/10 20:44:27  warmerda
 * don't try to select shapes in a zero sized rectangle
 *
 * Revision 1.9  2000/06/20 13:26:55  warmerda
 * added standard headers
 *
 */

#include "gvshapelayer.h"
#include "gvrenderinfo.h"
#include "cpl_error.h"
#include <GL/gl.h>
#include <GL/glu.h>

#define PICK_SIZE 4.0

enum
{
    DRAW_SELECTED,
    DELETE_SELECTED,
    TRANSLATE_SELECTED,
    PICK_SHAPE,
    PICK_NODE,
    GET_NODE,
    MOVE_NODE,
    INSERT_NODE,
    DELETE_NODE,
    NODE_MOTION,
    SELECTION_CHANGED,
    SUBSELECTION_CHANGED,
    LAST_SIGNAL
};

static void gv_shape_layer_class_init(GvShapeLayerClass *klass);
static void gv_shape_layer_init(GvShapeLayer *layer);
static void gv_shape_layer_finalize(GObject *gobject);
static void gv_shape_layer_selection_changed(GvShapeLayer *layer);
static void gv_shape_layer_subselection_changed(GvShapeLayer *layer);

static GvLayerClass *parent_class = NULL;
static guint shape_layer_signals[LAST_SIGNAL] = { 0 };

GType
gv_shape_layer_get_type(void)
{
    static GType shape_layer_type = 0;

    if (!shape_layer_type) {
        static const GTypeInfo shape_layer_info =
        {
            sizeof(GvShapeLayerClass),
            (GBaseInitFunc) NULL,
            (GBaseFinalizeFunc) NULL,
            (GClassInitFunc) gv_shape_layer_class_init,
            /* reserved_1 */ NULL,
            /* reserved_2 */ NULL,
            sizeof(GvShapeLayer),
            0,
            (GInstanceInitFunc) gv_shape_layer_init,
        };
        shape_layer_type = g_type_register_static (GV_TYPE_LAYER,
                                            "GvShapeLayer",
                                            &shape_layer_info, 0);
    }

    return shape_layer_type;
}

static void
gv_shape_layer_class_init(GvShapeLayerClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    parent_class = g_type_class_peek_parent (klass);

    shape_layer_signals[DRAW_SELECTED] =
      g_signal_new ("draw-selected",
		    G_TYPE_FROM_CLASS (klass),
		    G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
		    G_STRUCT_OFFSET (GvShapeLayerClass, draw_selected),
		    NULL, NULL,
		    g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1,
		    G_TYPE_POINTER);

    shape_layer_signals[DELETE_SELECTED] =
      g_signal_new ("delete-selected",
		    G_TYPE_FROM_CLASS (klass),
		    G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
		    G_STRUCT_OFFSET (GvShapeLayerClass, delete_selected),
		    NULL, NULL,
		    g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

    shape_layer_signals[TRANSLATE_SELECTED] =
      g_signal_new ("translate-selected",
		    G_TYPE_FROM_CLASS (klass),
		    G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
		    G_STRUCT_OFFSET (GvShapeLayerClass, translate_selected),
		    NULL, NULL,
		    g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1,
		    G_TYPE_POINTER);

    shape_layer_signals[PICK_SHAPE] =
      g_signal_new ("pick-shape",
		    G_TYPE_FROM_CLASS (klass),
		    G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
		    G_STRUCT_OFFSET (GvShapeLayerClass, pick_shape),
		    NULL, NULL,
		    g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

    shape_layer_signals[PICK_NODE] =
      g_signal_new ("pick-node",
		    G_TYPE_FROM_CLASS (klass),
		    G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
		    G_STRUCT_OFFSET (GvShapeLayerClass, pick_node),
		    NULL, NULL,
		    g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

    shape_layer_signals[GET_NODE] =
      g_signal_new ("get-node",
		    G_TYPE_FROM_CLASS (klass),
		    G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
		    G_STRUCT_OFFSET (GvShapeLayerClass, get_node),
		    NULL, NULL,
		    g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1,
		    G_TYPE_POINTER);

    shape_layer_signals[MOVE_NODE] =
      g_signal_new ("move-node",
		    G_TYPE_FROM_CLASS (klass),
		    G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
		    G_STRUCT_OFFSET (GvShapeLayerClass, move_node),
		    NULL, NULL,
		    g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1,
		    G_TYPE_POINTER);

    shape_layer_signals[INSERT_NODE] =
      g_signal_new ("insert-node",
		    G_TYPE_FROM_CLASS (klass),
		    G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
		    G_STRUCT_OFFSET (GvShapeLayerClass, insert_node),
		    NULL, NULL,
		    g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1,
		    G_TYPE_POINTER);

    shape_layer_signals[DELETE_NODE] =
      g_signal_new ("delete-node",
		    G_TYPE_FROM_CLASS (klass),
		    G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
		    G_STRUCT_OFFSET (GvShapeLayerClass, delete_node),
		    NULL, NULL,
		    g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1,
		    G_TYPE_POINTER);

    shape_layer_signals[NODE_MOTION] =
      g_signal_new ("node-motion",
		    G_TYPE_FROM_CLASS (klass),
		    G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
		    G_STRUCT_OFFSET (GvShapeLayerClass, node_motion),
		    NULL, NULL,
		    g_cclosure_marshal_VOID__INT, G_TYPE_NONE, 1,
		    G_TYPE_INT);

    shape_layer_signals[SELECTION_CHANGED] =
      g_signal_new ("selection-changed",
		    G_TYPE_FROM_CLASS (klass),
		    G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
		    G_STRUCT_OFFSET (GvShapeLayerClass, selection_changed),
		    NULL, NULL,
		    g_cclosure_marshal_VOID__INT, G_TYPE_NONE, 1,
		    G_TYPE_POINTER);

    shape_layer_signals[SUBSELECTION_CHANGED] =
      g_signal_new ("subselection-changed",
		    G_TYPE_FROM_CLASS (klass),
		    G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
		    G_STRUCT_OFFSET (GvShapeLayerClass, subselection_changed),
		    NULL, NULL,
		    g_cclosure_marshal_VOID__INT, G_TYPE_NONE, 1,
		    G_TYPE_POINTER);

    /* ---- Override finalize ---- */
    object_class->finalize = gv_shape_layer_finalize;

    klass->draw_selected = NULL;
    klass->delete_selected = NULL;
    klass->translate_selected = NULL;
    klass->pick_shape = NULL;
    klass->pick_node = NULL;
    klass->get_node = NULL;
    klass->move_node = NULL;
    klass->insert_node = NULL;
    klass->delete_node = NULL;
    klass->node_motion = NULL;
}

static void
gv_shape_layer_init(GvShapeLayer *layer)
{
    GvColor default_color = {1.0, 1.0, 1.0, 1.0};

    layer->selected = g_array_new(FALSE, TRUE, sizeof(gint));
    layer->flags = 0;
    gv_color_copy(layer->color, default_color);

    layer->subselection = -1;
    layer->render_index = NULL;
    layer->label_render = NULL;
    layer->symbol_render = NULL;
    layer->pen_render = NULL;
    layer->brush_render = NULL;

    layer->scale_dep_flags = NULL;
}

void
gv_shape_layer_select_shape(GvShapeLayer *layer, gint shape_id)
{
    g_return_if_fail(shape_id >= 0 && shape_id < layer->selected->len);
    if( g_array_index(layer->selected, gint, shape_id) == 0 )
    {
        g_array_index(layer->selected, gint, shape_id) = 1;
        gv_shape_layer_selection_changed(layer);

        gv_shape_layer_subselect_shape(layer, shape_id);
    }
}

void
gv_shape_layer_deselect_shape(GvShapeLayer *layer, gint shape_id)
{
    g_return_if_fail(shape_id >= 0 && shape_id < layer->selected->len);
    if( g_array_index(layer->selected, gint, shape_id) != 0 )
    {
        if( layer->subselection == shape_id )
            gv_shape_layer_subselect_shape(layer, -1);

        g_array_index(layer->selected, gint, shape_id) = 0;
        gv_shape_layer_selection_changed(layer);
    }
}

void
gv_shape_layer_clear_selection(GvShapeLayer *layer)
{
    gv_shape_layer_subselect_shape(layer, -1);

    memset(layer->selected->data, 0, layer->selected->len * sizeof(gint));

    gv_shape_layer_selection_changed(layer);
}

void
gv_shape_layer_select_all(GvShapeLayer *layer)
{
    memset(layer->selected->data, 0xff, layer->selected->len * sizeof(gint));
    gv_shape_layer_selection_changed(layer);
}

gint
gv_shape_layer_is_selected(GvShapeLayer *layer, gint shape_id)
{
    g_return_val_if_fail(shape_id >= 0 && shape_id < layer->selected->len, 0);
    return g_array_index(layer->selected, gint, shape_id);
}

gint
gv_shape_layer_get_selected(GvShapeLayer *layer, GArray *shapes)
{
    gint ii, hit=FALSE;

    for (ii=0; ii < layer->selected->len; ++ii) {
        if (g_array_index(layer->selected, gint, ii)) {
	    hit = TRUE;
	    g_array_append_val(shapes, ii);
        }
    }

    return hit;
}

gint
gv_shape_layer_selected(GvShapeLayer *layer, gint what, void *retval)
{
    gint i, hit=FALSE;
    GArray *shapes=NULL;

    if (what == GV_ALL)
    {
        hit = FALSE;
        shapes = (GArray*)retval;
    }

    for (i=0; i < layer->selected->len; ++i)
    {
        if (g_array_index(layer->selected, gint, i))
        {
            if (what == GV_ALL)
            {
                hit = TRUE;
                g_array_append_val(shapes, i);
            }
            else
            {
                if (what == GV_FIRST) *(gint*)retval = i;
                return TRUE;
            }
        }
    }
    return hit;
}

void
gv_shape_layer_delete_selected(GvShapeLayer *layer)
{
    g_signal_emit(layer, shape_layer_signals[DELETE_SELECTED], 0);
}

void
gv_shape_layer_translate_selected(GvShapeLayer *layer, gvgeocoord dx, gvgeocoord dy)
{
    GvVertex delta;

    delta.x = dx;
    delta.y = dy;
    g_signal_emit(layer, shape_layer_signals[TRANSLATE_SELECTED], 0, &delta);
}

void
gv_shape_layer_selected_motion(GvShapeLayer *layer, gvgeocoord dx, gvgeocoord dy)
{
    layer->selected_motion.x = dx;
    layer->selected_motion.y = dy;
}

void
gv_shape_layer_draw_selected(GvShapeLayer *layer, guint when, GvViewArea *view)
{
    switch (when)
    {
    case GV_ALWAYS:
        layer->flags &= ~GV_DELAY_SELECTED;
        break;

    case GV_LATER:
        layer->flags |= GV_DELAY_SELECTED;
        break;

    case GV_NOW:
        g_signal_emit(layer, shape_layer_signals[DRAW_SELECTED], 0, view);
        break;

    default:
        g_warning("gv_shape_layer_draw_selected(): invalid argument");
        break;
    }
}

void
gv_shape_layer_get_node(GvShapeLayer *layer, GvNodeInfo *info)
{
    g_signal_emit(layer, shape_layer_signals[GET_NODE], 0, info);
}

void
gv_shape_layer_move_node(GvShapeLayer *layer, GvNodeInfo *info)
{
    g_signal_emit(layer, shape_layer_signals[MOVE_NODE], 0, info);
}

void
gv_shape_layer_insert_node(GvShapeLayer *layer, GvNodeInfo *info)
{
    g_signal_emit(layer, shape_layer_signals[INSERT_NODE], 0, info);
}

void
gv_shape_layer_delete_node(GvShapeLayer *layer, GvNodeInfo *info)
{
    g_signal_emit(layer, shape_layer_signals[DELETE_NODE], 0, info);
}

void gv_shape_layer_node_motion(GvShapeLayer *layer, gint shape_id)
{
    g_signal_emit(layer, shape_layer_signals[NODE_MOTION], 0, shape_id);
}

void
gv_shape_layer_select_region(GvShapeLayer *layer, GvViewArea *view,
                 GvRect *rect)
{
    GLuint *buf;
    GLint hits, i;

    if (layer->selected->len == 0) return;

    if( rect->width == 0.0 || rect->height == 0.0 )
        return;

    buf = g_new(GLuint, layer->selected->len * 4);
    g_return_if_fail(buf);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    /* Rect is in screen coordinates with origin in center of view */
    gluOrtho2D(rect->x, rect->x + rect->width,
               rect->y, rect->y + rect->height);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glRotate(view->state.rot, 0.0, 0.0, 1.0);
    glScale(view->state.linear_zoom * view->state.flip_x,
            view->state.linear_zoom * view->state.flip_y, 1.0);
    glTranslate(view->state.tx, view->state.ty, 0.0);

    glSelectBuffer(layer->selected->len * 4, buf);
    glRenderMode(GL_SELECT);

    glInitNames();
    glPushName(-1);

    g_signal_emit(layer, shape_layer_signals[PICK_SHAPE], 0);

    hits = glRenderMode(GL_RENDER);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    layer->flags |= GV_SUPPRESS_SELCHANGED;
    gv_shape_layer_clear_selection(layer);

    /*
    fprintf(stdout, "Hits: %d\n", hits);
    fflush(stdout);
    */

    for (i=0; i < hits; ++i)
    {
        gv_shape_layer_select_shape(layer, (gint)buf[4*i+3]);
    }

    layer->flags &= ~GV_SUPPRESS_SELCHANGED;

    gv_shape_layer_selection_changed(layer);

    if( hits > 0 )
        gv_shape_layer_subselect_shape( layer, (gint) buf[3] );

    g_free(buf);
}

gint
gv_shape_layer_pick_shape(GvShapeLayer *layer, GvViewArea *view,
              gvgeocoord x, gvgeocoord y, gint *shape_id)
{
    GLuint *buf;
    GLint hits;
    GLint vp[4];


    if (layer->selected->len == 0) return FALSE;

    buf = g_new(GLuint, layer->selected->len * 4);
    g_return_val_if_fail(buf, FALSE);

    vp[0] = vp[1]  = 0;
    vp[2] = (GLint)view->state.shape_x;
    vp[3] = (GLint)view->state.shape_y;

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluPickMatrix(x, vp[3]-y, PICK_SIZE, PICK_SIZE, vp);
    gluOrtho2D(-vp[2]/2.0, vp[2]/2.0, -vp[3]/2.0, vp[3]/2.0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glRotate(view->state.rot, 0.0, 0.0, 1.0);
    glScale(view->state.linear_zoom * view->state.flip_x,
         view->state.linear_zoom * view->state.flip_y, 1.0);
    glTranslate(view->state.tx, view->state.ty, 0.0);

    glSelectBuffer(layer->selected->len * 4, buf);
    glRenderMode(GL_SELECT);

    glInitNames();
    glPushName(-1);

    g_signal_emit(layer, shape_layer_signals[PICK_SHAPE], 0);

    hits = glRenderMode(GL_RENDER);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    if (hits > 0 && shape_id)
    {
        /* If there is more than one hit, we arbitrarily pick the first one */
        *shape_id = (gint)buf[3];
    }

    g_free(buf);
    return (hits > 0);
}

gint
gv_shape_layer_pick_node(GvShapeLayer *layer, GvViewArea *view,
             gvgeocoord x, gvgeocoord y, gint *before,
             GvNodeInfo *node_info)
{
    GLuint *buf;
    GLint hits;
    GLint vp[4];


    if (layer->selected->len == 0) return FALSE;

    buf = g_new0(GLuint, 12*3);
    g_return_val_if_fail(buf, FALSE);

    vp[0] = vp[1]  = 0;
    vp[2] = (GLint)view->state.shape_x;
    vp[3] = (GLint)view->state.shape_y;

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluPickMatrix(x, vp[3]-y, PICK_SIZE, PICK_SIZE, vp);
    gluOrtho2D(-vp[2]/2.0, vp[2]/2.0, -vp[3]/2.0, vp[3]/2.0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glRotate(view->state.rot, 0.0, 0.0, 1.0);
    glScale(view->state.linear_zoom * view->state.flip_x,
         view->state.linear_zoom * view->state.flip_y, 1.0);
    glTranslate(view->state.tx, view->state.ty, 0.0);

    glSelectBuffer(12*3, buf);
    glRenderMode(GL_SELECT);

    glInitNames();
    glPushName(-1);

    g_signal_emit(layer, shape_layer_signals[PICK_NODE], 0);

    hits = glRenderMode(GL_RENDER);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    if (hits > 0 && node_info)
    {
    /* If there is more than one hit, we arbitrarily pick the first one */

    /* Contents of the name stack is encoded as follows:
          buf[3]  = 0 for node hit, 1 for segment hit
          buf[4]  = ring id (always 0 for lines)
          buf[5]  = node id
    */
    node_info->ring_id = (gint)buf[4];
    node_info->node_id = (gint)buf[5];

    gv_shape_layer_selected(layer, GV_FIRST, &node_info->shape_id);
    }
    if (hits > 0 && before)
    {
    *before = (gint)buf[3];
    }
    if( hits < 0 )
    {
        CPLDebug( "GvShapeLayer",
                  "Name buffer overflow in gv_shape_layer_pick_node()" );
    }

    g_free(buf);
    return (hits > 0);
}

void
gv_shape_layer_set_num_shapes(GvShapeLayer *layer, gint num_shapes)
{
    int   old_scaledep_words, new_scaledep_words;

    if (num_shapes == layer->selected->len)
        return;

    /* Do we need to grow the scaledep words array? */
    new_scaledep_words = (num_shapes+31) / 32;
    old_scaledep_words = (layer->selected->len+31) / 32;

    if( new_scaledep_words > old_scaledep_words )
    {
        if( layer->scale_dep_flags != NULL )
            layer->scale_dep_flags =
                g_renew( gint, layer->scale_dep_flags, new_scaledep_words );
        else
            layer->scale_dep_flags =
                g_new( gint, new_scaledep_words );

        memset( layer->scale_dep_flags + old_scaledep_words,
                0, (new_scaledep_words - old_scaledep_words) * 4 );
    }

    /* reset the size of the selected array */
    g_array_set_size(layer->selected, num_shapes);
    gv_shape_layer_clear_selection(layer);

    if( layer->render_index != NULL )
    {
        /* new entries automatically set to zero (GVP_UNINITIALIZED_PART) */
        g_array_set_size(layer->render_index, num_shapes);
    }
}

void
gv_shape_layer_set_color(GvShapeLayer *layer, GvColor color)
{
    gv_color_copy(layer->color, color);
    gv_layer_display_change( GV_LAYER(layer), NULL );
}

static void
gv_shape_layer_selection_changed(GvShapeLayer *layer)

{
    if( !(layer->flags & GV_SUPPRESS_SELCHANGED) )
        g_signal_emit(layer, shape_layer_signals[SELECTION_CHANGED], 0);
}

static void
gv_shape_layer_subselection_changed(GvShapeLayer *layer)

{
    if( !(layer->flags & GV_SUPPRESS_SELCHANGED) )
        g_signal_emit(layer, shape_layer_signals[SUBSELECTION_CHANGED], 0);
}

static void
gv_shape_layer_finalize(GObject *gobject)
{
    CPLDebug( "OpenEV", "gv_shape_layer_finalize(%s)",
              gv_data_get_name( GV_DATA(gobject) ) );
    GvShapeLayer *layer;

    /* GTK2 PORT... Override GObject finalize, test for and set NULLs
       as finalize may be called more than once. */

    layer = GV_SHAPE_LAYER(gobject);

    gv_shape_layer_clear_all_renderinfo( layer );

    if (layer->selected != NULL) {
        g_array_free(layer->selected, TRUE);
        layer->selected = NULL;
    }
    if( layer->scale_dep_flags != NULL ) {
        g_free( layer->scale_dep_flags );
        layer->scale_dep_flags = NULL;
    }

    /* Call parent class function */
    G_OBJECT_CLASS(parent_class)->finalize(gobject);
}

/************************************************************************/
/*                  gv_shape_layer_get_subselection()                   */
/************************************************************************/

gint gv_shape_layer_get_subselection( GvShapeLayer *layer )
{
    return layer->subselection;
}

/************************************************************************/
/*                   gv_shape_layer_subselect_shape()                   */
/************************************************************************/

void gv_shape_layer_subselect_shape( GvShapeLayer *layer, gint shape_id )

{
    if( (shape_id == -1 || gv_shape_layer_is_selected(layer, shape_id))
        && shape_id != layer->subselection )
    {
        layer->subselection = shape_id;
        gv_shape_layer_subselection_changed(layer);
    }
}

/************************************************************************/
/*                      gv_shape_layer_get_part()                       */
/************************************************************************/

GvRenderPart *gv_shape_layer_get_part( GvShapeLayer *layer, guint part_index )


{
    int     part_type, raw_index;

    if( layer->render_index == NULL )
        return NULL;

    if( part_index == GVP_LAST_PART || part_index == GVP_UNINITIALIZED_PART )
        return NULL;

    raw_index = gv_part_index_to_index(part_index);
    part_type = gv_part_index_to_type(part_index);

    g_assert( raw_index >= 0 );
    g_assert( part_type == GvLabelPart || part_type == GvSymbolPart
              || part_type == GvPenPart || part_type == GvBrushPart );

    if( part_type == GvLabelPart )
    {
        g_assert( raw_index < layer->label_render->len );

        if( raw_index >= layer->label_render->len )
            return NULL;

        g_assert( g_array_index(layer->label_render,
                                GvLabelRenderPart,raw_index).next_part
                  != GVP_UNINITIALIZED_PART );

        return (GvRenderPart *)
            &(g_array_index(layer->label_render,
                            GvLabelRenderPart,raw_index));
    }
    else if( part_type == GvSymbolPart )
    {
        g_assert( raw_index < layer->symbol_render->len );

        if( raw_index >= layer->symbol_render->len )
            return NULL;

        g_assert( g_array_index(layer->symbol_render,
                                GvSymbolRenderPart,raw_index).next_part
                  != GVP_UNINITIALIZED_PART );

        return (GvRenderPart *)
            &(g_array_index(layer->symbol_render,
                            GvSymbolRenderPart,raw_index));
    }
    else if( part_type == GvPenPart )
    {
        g_assert( raw_index < layer->pen_render->len );

        if( raw_index >= layer->pen_render->len )
            return NULL;

        g_assert( g_array_index(layer->pen_render,
                                GvPenRenderPart,raw_index).next_part
                  != GVP_UNINITIALIZED_PART );

        return (GvRenderPart *)
            &(g_array_index(layer->pen_render,
                            GvPenRenderPart,raw_index));
    }
    else if( part_type == GvBrushPart )
    {
        g_assert( raw_index < layer->brush_render->len );

        if( raw_index >= layer->brush_render->len )
            return NULL;

        g_assert( g_array_index(layer->brush_render,
                                GvBrushRenderPart,raw_index).next_part
                  != GVP_UNINITIALIZED_PART );

        return (GvRenderPart *)
            &(g_array_index(layer->brush_render,
                            GvBrushRenderPart,raw_index));
    }
    else
        return NULL;
}

/************************************************************************/
/*                gv_shape_layer_get_first_part_index()                 */
/************************************************************************/

guint gv_shape_layer_get_first_part_index( GvShapeLayer *layer, gint shape_id)

{
    guint   part_index;

    if( layer->render_index == NULL )
        return GVP_UNINITIALIZED_PART;

    g_return_val_if_fail( shape_id >= 0 && shape_id < layer->render_index->len,
                          GVP_LAST_PART );

    part_index = g_array_index(layer->render_index, guint, shape_id);

    return part_index;
}

/************************************************************************/
/*               gv_shape_layer_initialize_renderindex()                */
/************************************************************************/

void gv_shape_layer_initialize_renderindex( GvShapeLayer *layer )

{
/* -------------------------------------------------------------------- */
/*      Allocate render_index if it doesn't exist yet.                  */
/* -------------------------------------------------------------------- */
    if( layer->render_index == NULL )
    {
        layer->render_index = g_array_new(FALSE, TRUE, sizeof(gint));
        g_array_set_size(layer->render_index, layer->selected->len );

        layer->label_render =
            g_array_new(FALSE, TRUE, sizeof(GvLabelRenderPart));
        layer->symbol_render =
            g_array_new(FALSE, TRUE, sizeof(GvSymbolRenderPart));
        layer->pen_render =
            g_array_new(FALSE, TRUE, sizeof(GvPenRenderPart));
        layer->brush_render =
            g_array_new(FALSE, TRUE, sizeof(GvBrushRenderPart));
    }
}

/************************************************************************/
/*                     gv_shape_layer_create_part()                     */
/************************************************************************/

guint gv_shape_layer_create_part( GvShapeLayer *layer, gint part_type )

{
    int   part_index;

    g_return_val_if_fail( layer != NULL, GVP_UNINITIALIZED_PART );

    if( layer->render_index == NULL )
        gv_shape_layer_initialize_renderindex( layer );

/* -------------------------------------------------------------------- */
/*      Find the new part index in the target part type table.          */
/* -------------------------------------------------------------------- */
    if( part_type == GvLabelPart )
    {
        GvLabelRenderPart   label_info;

        memset( &label_info, 0, sizeof(label_info) );
        label_info.next_part = GVP_LAST_PART;
        label_info.scale = 1.0;
        label_info.color[0] = 1.0;
        label_info.color[1] = 1.0;
        label_info.color[2] = 1.0;
        label_info.color[3] = 1.0;
        label_info.b_color_initialized = 0;
        label_info.background_color[0] = 1.0;
        label_info.background_color[1] = 1.0;
        label_info.background_color[2] = 1.0;
        label_info.background_color[3] = 1.0;
        label_info.b_background_color_initialized = 0;
        label_info.anchor = GLRA_LOWER_LEFT;
        label_info.halo = FALSE;
        label_info.shadow = FALSE;

        part_index = ((layer->label_render->len) << 3) | GvLabelPart;
        g_array_append_val( layer->label_render, label_info );

        return part_index;
    }
    else if( part_type == GvSymbolPart )
    {
        GvSymbolRenderPart  symbol_info;

        memset( &symbol_info, 0, sizeof(symbol_info) );
        symbol_info.next_part = GVP_LAST_PART;
        symbol_info.scale = 1.0;
        symbol_info.color[0] = 1.0;
        symbol_info.color[1] = 1.0;
        symbol_info.color[2] = 1.0;
        symbol_info.color[3] = 1.0;
        symbol_info.b_color_initialized = 0;
        symbol_info.part_index = GVP_UNINITIALIZED_PART;

        part_index = ((layer->symbol_render->len) << 3) | GvSymbolPart;
        g_array_append_val( layer->symbol_render, symbol_info );

        return part_index;
    }
    else if( part_type == GvPenPart )
    {
        GvPenRenderPart pen_info;

        memset( &pen_info, 0, sizeof(pen_info) );
        pen_info.next_part = GVP_LAST_PART;
        pen_info.color[0] = 1.0;
        pen_info.color[1] = 1.0;
        pen_info.color[2] = 1.0;
        pen_info.color[3] = 1.0;
        pen_info.b_color_initialized = 0;
        pen_info.width = 1.0;
        pen_info.pattern = "ogr-pen-0";

        part_index = ((layer->pen_render->len) << 3) | GvPenPart;
        g_array_append_val( layer->pen_render, pen_info );

        return part_index;
    }
    else if( part_type == GvBrushPart )
    {
        GvBrushRenderPart   brush_info;

        memset( &brush_info, 0, sizeof(brush_info) );
        brush_info.next_part = GVP_LAST_PART;
        brush_info.fore_color[0] = 1.0;
        brush_info.fore_color[1] = 1.0;
        brush_info.fore_color[2] = 1.0;
        brush_info.fore_color[3] = 1.0;
        brush_info.b_fore_color_initialized = 0;

        part_index = ((layer->brush_render->len) << 3) | GvBrushPart;
        g_array_append_val( layer->brush_render, brush_info );

        return part_index;
    }
    else
    {
        return GVP_UNINITIALIZED_PART;
    }
}

/************************************************************************/
/*                     gv_shape_layer_chain_part()                      */
/*                                                                      */
/*      Append this part to the the chain of which the                  */
/*      base_part_index is the head (or at least a member).             */
/*                                                                      */
/*      Returns the base part index, which may new_part_index if it     */
/*      is the first in the chain.                                      */
/************************************************************************/

guint gv_shape_layer_chain_part( GvShapeLayer *layer, gint base_part_index,
                                 gint new_part_index )

{
    GvRenderPart *last_part;

    if( base_part_index == GVP_LAST_PART
        || base_part_index == GVP_UNINITIALIZED_PART )
        return new_part_index;

    last_part = gv_shape_layer_get_part( layer, base_part_index );

    while( last_part != NULL && last_part->next_part != GVP_LAST_PART )
        last_part = gv_shape_layer_get_part( layer, last_part->next_part );

    if( last_part != NULL )
        last_part->next_part = new_part_index;

    return base_part_index;
}

/************************************************************************/
/*                      gv_shape_layer_add_part()                       */
/*                                                                      */
/*      Returns GVP_UNITIALIZED_PART (0) if add fails.                  */
/************************************************************************/

guint gv_shape_layer_add_part( GvShapeLayer *layer, gint shape_id,
                               gint part_type )

{
    guint   part_index;

    g_return_val_if_fail( layer != NULL
                          && shape_id >= 0
                          && shape_id < layer->selected->len,
                          GVP_UNINITIALIZED_PART );

    if( layer->render_index == NULL )
        gv_shape_layer_initialize_renderindex( layer );

/* -------------------------------------------------------------------- */
/*      A part type of GVP_LAST_PART is taken as an indication that     */
/*      this shape should just have a GVP_LAST_PART code set for the    */
/*      shape as a whole.  There is no render information for this      */
/*      shape, but it is initialized.                                   */
/* -------------------------------------------------------------------- */
    if( part_type == GVP_LAST_PART )
    {
        g_array_index(layer->render_index,guint,shape_id) = GVP_LAST_PART;
        return GVP_LAST_PART;
    }

/* -------------------------------------------------------------------- */
/*      Create and initialize the part.                                 */
/* -------------------------------------------------------------------- */
    part_index = gv_shape_layer_create_part( layer, part_type );

    if( part_index == GVP_UNINITIALIZED_PART )
        return part_index;

/* -------------------------------------------------------------------- */
/*      Chain this part at the end of any existing parts for this       */
/*      shape, or if it is the first it will be set as the first part.  */
/* -------------------------------------------------------------------- */
    g_array_index(layer->render_index,guint,shape_id) =
        gv_shape_layer_chain_part(
            layer,
            gv_shape_layer_get_first_part_index(layer, shape_id),
            part_index );

    return part_index;
}

/************************************************************************/
/*                  gv_shape_layer_clear_shape_parts()                  */
/************************************************************************/

void gv_shape_layer_clear_shape_parts( GvShapeLayer *layer, gint shape_id )

{
    if( layer == NULL || shape_id < 0 || shape_id >= layer->selected->len
        || layer->render_index == NULL )
        return;

    gv_shape_layer_clear_part( layer,
        gv_shape_layer_get_first_part_index( layer, shape_id ) );

    g_array_index( layer->render_index, guint, shape_id )
        = GVP_UNINITIALIZED_PART;
}


/************************************************************************/
/*                     gv_shape_layer_clear_part()                      */
/*                                                                      */
/*      De-initialized the indicated part, and any parts linked         */
/*      after it.  Returns parts to GVP_UNINITIALIZED state.            */
/************************************************************************/

void gv_shape_layer_clear_part( GvShapeLayer *layer, guint part_index )

{
    int         part_type = gv_part_index_to_type(part_index);
    GvRenderPart    *part_info;

    part_info = gv_shape_layer_get_part( layer, part_index );
    if( part_info == NULL )
        return;

    if( part_info->next_part != GVP_LAST_PART )
        gv_shape_layer_clear_part( layer, part_info->next_part );

    if( part_type == GvLabelPart )
    {
        GvLabelRenderPart *label_info = (GvLabelRenderPart *) part_info;

        if( label_info->text != NULL )
            g_free( label_info->text );
    }

    if( part_type == GvSymbolPart )
    {
        GvSymbolRenderPart *symbol_info = (GvSymbolRenderPart *) part_info;

        if( symbol_info->symbol_id != NULL )
            g_free( symbol_info->symbol_id );
    }

    part_info->next_part = GVP_UNINITIALIZED_PART;
}

/************************************************************************/
/*                gv_shape_layer_clear_all_renderinfo()                 */
/*                                                                      */
/*      Quickly wipe all rendering info, and recover all memory         */
/*      associated with keeping rendering info.                         */
/************************************************************************/

void gv_shape_layer_clear_all_renderinfo( GvShapeLayer *layer )

{
    int     i;

    /*
     * When the render info changes, it is possible for the scale dependentness
     * of things to change to, so clear the flags will be be rebuilt next
     * time the features are drawn.
     */
    if( layer->scale_dep_flags != NULL
        && layer->selected != NULL )
        memset( layer->scale_dep_flags, 0,
                ((layer->selected->len+31) / 32) * 4 );

    if( layer->render_index == NULL )
        return;

    g_array_free( layer->render_index, TRUE );
    layer->render_index = NULL;

    for( i=0; i<layer->symbol_render->len; i++ )
    {
        GvSymbolRenderPart *symbol_info;
        symbol_info = &(g_array_index(layer->symbol_render,
                                      GvSymbolRenderPart, i));
        if (symbol_info->next_part != GVP_UNINITIALIZED_PART)
            if (symbol_info->symbol_id != NULL)
                g_free( symbol_info->symbol_id );
    }

    g_array_free( layer->symbol_render, TRUE );
    layer->symbol_render = NULL;

    for( i = 0; i < layer->label_render->len; i++ )
    {
        GvLabelRenderPart  *label_info;

        label_info = &(g_array_index(layer->label_render,GvLabelRenderPart,i));
        if( label_info->next_part != GVP_UNINITIALIZED_PART )
        {
            if( label_info->text != NULL )
                g_free( label_info->text );
        }
    }

    g_array_free( layer->label_render, TRUE );
    layer->label_render = NULL;

    g_array_free( layer->pen_render, TRUE );
    layer->pen_render = NULL;

    g_array_free( layer->brush_render, TRUE );
    layer->pen_render = NULL;

}

/************************************************************************/
/*                    gv_shape_layer_get_scale_dep()                    */
/************************************************************************/

int gv_shape_layer_get_scale_dep( GvShapeLayer *layer, gint shape_id )

{
    if( layer->scale_dep_flags == NULL )
        return 0;

    if( shape_id < 0 || shape_id >= layer->selected->len )
        return 0;

    return layer->scale_dep_flags[shape_id >> 5] & (1 << (shape_id & 0x1f) );
}

/************************************************************************/
/*                    gv_shape_layer_set_scale_dep()                    */
/************************************************************************/

void gv_shape_layer_set_scale_dep( GvShapeLayer *layer, gint shape_id,
                                   int scale_dep )

{
    if( layer->scale_dep_flags == NULL )
        return;

    if( shape_id < 0 || shape_id >= layer->selected->len )
        return;

    if( scale_dep )
        layer->scale_dep_flags[shape_id >> 5] |= (1 << (shape_id & 0x1f) );
    else
        layer->scale_dep_flags[shape_id >> 5] &= ~(1 << (shape_id & 0x1f) );
}

