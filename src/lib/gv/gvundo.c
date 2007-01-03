/******************************************************************************
 * $Id: gvundo.c,v 1.1.1.1 2005/04/18 16:38:34 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  Generic undo management.
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
 * $Log: gvundo.c,v $
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
 * Revision 1.6  2002/09/30 20:50:32  warmerda
 * fixed serious bug in gv_undo_end_group
 *
 * Revision 1.5  2000/06/20 13:26:55  warmerda
 * added standard headers
 *
 */

#include "gvundo.h"
#include "gextra.h"
#include <gtk/gtksignal.h>

#define DEFAULT_STACK_MAX   64

typedef struct _GvUndo GvUndo;

struct _GvUndo
{
    GList *undo_stack;
    gint undo_stack_count;
    gint stack_max;
    gint undo_enabled : 1;
    gint undo_open : 1;
    gint group_locked : 1;
    gint next_group;
};

static void gv_undo_data_changing(GvData *data, gpointer change_info);
static void gv_undo_data_destroy(GvData *data);

static GvUndo *undo = NULL;

static void
gv_undo_init(void)
{
    undo = g_new(GvUndo, 1);
    undo->undo_stack = NULL;
    undo->undo_stack_count = 0;
    undo->stack_max = DEFAULT_STACK_MAX;
    undo->undo_enabled = TRUE;
    undo->undo_open = TRUE;
    undo->next_group = 1;
    undo->group_locked = FALSE;
}

void
gv_undo_register_data(GvData *data)
{
    if (!undo) gv_undo_init();

    gtk_signal_connect(GTK_OBJECT(data), "changing",
		       GTK_SIGNAL_FUNC(gv_undo_data_changing), NULL);

    gtk_signal_connect(GTK_OBJECT(data), "destroy",
		       GTK_SIGNAL_FUNC(gv_undo_data_destroy), NULL);
}

void
gv_undo_open(void)
{
    if (!undo) gv_undo_init();
    undo->undo_open = TRUE;
}

void
gv_undo_close(void)
{
    if (!undo) gv_undo_init();
    undo->undo_open = FALSE;
}

void
gv_undo_push(GvDataMemento *memento)
{
    if (!undo) gv_undo_init();

    /* set group id */
    memento->group = undo->next_group;
    if( !undo->group_locked )
        undo->next_group++;

    /* push on undo stack */
    undo->undo_stack = g_list_push(undo->undo_stack, memento);

    /* discard oldest undo step if we are at the limit */
    if (++undo->undo_stack_count > undo->stack_max)
    {
	GList *remove;
	GvDataMemento *memento;

	remove = g_list_last(undo->undo_stack);
	memento = (GvDataMemento*)remove->data;
	undo->undo_stack = g_list_remove_link(undo->undo_stack, remove);
	undo->undo_stack_count--;

	gv_data_del_memento(memento->data, memento);
    }
}

void
gv_undo_pop(void)
{
    GvDataMemento *memento;
    gint open;
    gint pop_group;

    if (!undo) gv_undo_init();
    if (!undo->undo_enabled || undo->undo_stack_count == 0) return;

    if( undo->group_locked )
        gv_undo_end_group( undo->next_group );

    /* pop all the undo's of a group */
    pop_group = ((GvDataMemento *) undo->undo_stack->data)->group;
    while( undo->undo_stack_count > 0 
           && ((GvDataMemento *) undo->undo_stack->data)->group == pop_group )
    {
        memento = (GvDataMemento*)undo->undo_stack->data;
        undo->undo_stack = g_list_pop(undo->undo_stack);
        undo->undo_stack_count--;

        /* Close the undo object temporarily while undoing */
        open = undo->undo_open;
        undo->undo_open = FALSE;
        gv_data_set_memento(memento->data, memento);
        undo->undo_open = open;
    }
}

void
gv_undo_clear(void)
{
    GvDataMemento *memento;

    if (!undo) gv_undo_init();    
    while (undo->undo_stack_count)
    {
	memento = (GvDataMemento*)undo->undo_stack->data;
	undo->undo_stack = g_list_pop(undo->undo_stack);
	undo->undo_stack_count--;
	
	gv_data_del_memento(memento->data, memento);
    }

    undo->group_locked = FALSE;
}

void
gv_undo_enable()
{
    if (!undo) gv_undo_init();
    undo->undo_enabled = TRUE;
}

void
gv_undo_disable()
{
    if (!undo) gv_undo_init();
    undo->undo_enabled = FALSE;
}

gint
gv_undo_can_undo(void)
{
    if (!undo) gv_undo_init();
    return (undo->undo_enabled && undo->undo_stack_count > 0);
}

gint gv_undo_start_group(void)

{
    if (!undo) gv_undo_init();

    if( undo->group_locked )
    {
        /* failure */
        return 0;
    }

    undo->group_locked = TRUE;
    return undo->next_group;
}

void 
gv_undo_end_group(gint group)

{
    if (!undo) gv_undo_init();

    if( !undo->group_locked || undo->next_group != group)
    {
        /* failure */
        return;
    }

    undo->group_locked = FALSE;
    undo->next_group++;
}


static void
gv_undo_data_changing(GvData *data, gpointer change_info)
{
    GvDataMemento *memento;

    if (!undo->undo_open) return;

    memento = gv_data_get_memento(data, change_info);
    if (memento)
    {
	gv_undo_push(memento);
    }
}

static void
gv_undo_data_destroy(GvData *data)
{
    /* FIXME: remove mementos corresponding to data from stack */
    /* For now just clear the stack */
    gv_undo_clear();
}
