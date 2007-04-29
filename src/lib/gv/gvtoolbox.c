/******************************************************************************
 * $Id$
 *
 * Project:  OpenEV
 * Purpose:  Container for available editing tools, manages which is active.
 * Author:   OpenEV Team
 * Maintainer: Mario Beauchamp, starged@gmail.com
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
 */

#include "gvtoolbox.h"

static void gv_toolbox_class_init(GvToolboxClass *klass);
static void gv_toolbox_init(GvToolbox *toolbox);
static void gv_toolbox_activate(GvTool *toolbox, GvViewArea *view);
static void gv_toolbox_deactivate(GvTool *toolbox, GvViewArea *view);
static gboolean gv_toolbox_view_event(GvViewArea *view, GdkEvent *event,
                                        GvToolbox *toolbox);
static void gv_toolbox_switch_to_view(GvToolbox *toolbox, GvViewArea *view);
static void gv_toolbox_dispose(GObject *object);
static void gv_toolbox_finalize(GObject *gobject);

static GvToolClass *parent_class = NULL;

GType
gv_toolbox_get_type(void)
{
    static GType toolbox_type = 0;

    if (!toolbox_type) {
        static const GTypeInfo toolbox_info =
        {
            sizeof(GvToolboxClass),
            (GBaseInitFunc) NULL,
            (GBaseFinalizeFunc) NULL,
            (GClassInitFunc) gv_toolbox_class_init,
            /* reserved_1 */ NULL,
            /* reserved_2 */ NULL,
            sizeof(GvToolbox),
            0,
            (GInstanceInitFunc) gv_toolbox_init,
        };
        toolbox_type = g_type_register_static (GV_TYPE_TOOL,
                                              "GvToolbox",
                                              &toolbox_info, 0);
        }

    return toolbox_type;
}

static void
gv_toolbox_class_init(GvToolboxClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    GvToolClass *tool_class = GV_TOOL_CLASS (klass);

    parent_class = g_type_class_peek_parent (klass);

    /* ---- Override finalize ---- */
    object_class->finalize = gv_toolbox_finalize;
    object_class->dispose = gv_toolbox_dispose;

    tool_class->activate = gv_toolbox_activate;
    tool_class->deactivate = gv_toolbox_deactivate;
}

static void
gv_toolbox_init(GvToolbox *toolbox)
{
    toolbox->tools = g_hash_table_new(g_str_hash, g_str_equal);
    toolbox->active_tool = NULL;
    toolbox->views = NULL;
}

GvTool *
gv_toolbox_new(void)
{
    GvToolbox *tool = g_object_new(GV_TYPE_TOOLBOX, NULL);

    return GV_TOOL(tool);
}

void
gv_toolbox_add_tool(GvToolbox *toolbox, gchar *name, GvTool *tool)
{
    if (g_hash_table_lookup(toolbox->tools, name))
    {
        g_warning("gv_toolbox_add_tool(): tool %s already exists", name);
        return;
    }

    g_object_ref(tool);
    g_hash_table_insert(toolbox->tools, g_strdup(name), (gpointer)tool);

    if( GV_TOOL(toolbox)->bounded ) 
        gv_tool_set_boundary( tool, &(GV_TOOL(toolbox)->boundary) );
}

void
gv_toolbox_activate_tool(GvToolbox *toolbox, gchar *tool_name)
{
    GvTool *tool;

    if (tool_name)
    {
        tool = (GvTool*)g_hash_table_lookup(toolbox->tools, tool_name);
        if (!tool)
        {
            g_warning("gv_toolbox_activate_tool(): no tool %s in toolbox",
                  tool_name);
            return;
        }
    }
    else
    {
        tool = NULL;
    }

    /* Check if tool is already active */
    if (tool == toolbox->active_tool) return;

    /* Deactivate currently active tool */
    if (toolbox->active_tool && GV_TOOL(toolbox)->view)
        gv_tool_deactivate(toolbox->active_tool, GV_TOOL(toolbox)->view);

    /* Activate new tool */
    toolbox->active_tool = tool;
    if (toolbox->active_tool && GV_TOOL(toolbox)->view)
        gv_tool_activate(toolbox->active_tool, GV_TOOL(toolbox)->view);
}

/*******************************************************/

static void
gv_toolbox_activate(GvTool *tool, GvViewArea *view)
{
    GvToolbox *toolbox = GV_TOOLBOX(tool);

    if (!g_list_find(toolbox->views, view))
    {
        g_object_ref(view);
        toolbox->views = g_list_append(toolbox->views, (gpointer)view);

        g_signal_connect(view, "button_press_event",
                        G_CALLBACK(gv_toolbox_view_event),
                        toolbox);

        g_signal_connect(view, "key_press_event",
                        G_CALLBACK(gv_toolbox_view_event),
                        toolbox);
    }

    gv_toolbox_switch_to_view(toolbox, view);
}

static void
gv_toolbox_deactivate(GvTool *tool, GvViewArea *view)
{
    GvToolbox *toolbox = GV_TOOLBOX( tool );

    if (!g_list_find(toolbox->views, view))
    {
        g_warning("gv_toolbox_activate(): view not active");
        return;
    }

    if (view == GV_TOOL(toolbox)->view)
    {
        if (toolbox->active_tool)
            gv_tool_deactivate(toolbox->active_tool, view);
        GV_TOOL(toolbox)->view = NULL;
    }

    g_signal_handlers_disconnect_matched (view, G_SIGNAL_MATCH_DATA,
                                            0, 0, NULL, NULL, toolbox);
    toolbox->views = g_list_remove(toolbox->views, view);
    g_object_unref(view);    
}

static gboolean
gv_toolbox_view_event(GvViewArea *view, GdkEvent *event, GvToolbox *toolbox)
{
    /* Check whether this view is the active view */
    if (view != GV_TOOL(toolbox)->view)
        gv_tool_activate( GV_TOOL(toolbox), view );

    return FALSE;
}

static void
gv_toolbox_switch_to_view(GvToolbox *toolbox, GvViewArea *view)
{
    /* Deactivate the active tool */
    if (toolbox->active_tool && GV_TOOL(toolbox)->view)
        gv_tool_deactivate(toolbox->active_tool, GV_TOOL(toolbox)->view);

    /* Switch views */
    GV_TOOL(toolbox)->view = view;

    /* Reactivate the active tool */
    if (toolbox->active_tool && view)
        gv_tool_activate(toolbox->active_tool, view);
}

static gboolean
unref_object_foreach(gpointer key, gpointer value, gpointer data)
{
    g_object_unref(value);
    return TRUE;
}

static void
gv_toolbox_dispose(GObject *object)
{
    GvToolbox *toolbox = GV_TOOLBOX(object);    

    /* Remove all views */
    while (toolbox->views)
    {
        GvViewArea *view = GV_VIEW_AREA (toolbox->views->data);
        gv_tool_deactivate(GV_TOOL(toolbox), view);
    }

    /* Remove all tools */
    if (toolbox->tools != NULL) {
        g_hash_table_foreach_remove(toolbox->tools, unref_object_foreach, NULL);
        toolbox->tools = NULL;
    }

    /* Call parent class function */
    G_OBJECT_CLASS(parent_class)->dispose(object);         
}

static void
gv_toolbox_finalize(GObject *gobject)
{
    GvToolbox *toolbox = GV_TOOLBOX(gobject);

    if (toolbox->tools != NULL) {
        g_hash_table_destroy(toolbox->tools);
        toolbox->tools = NULL;
    }

    G_OBJECT_CLASS(parent_class)->finalize(gobject);
}
