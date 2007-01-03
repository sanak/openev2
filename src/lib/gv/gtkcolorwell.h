/******************************************************************************
 * $Id: gtkcolorwell.h,v 1.1.1.1 2005/04/18 16:38:33 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  Color well button for GTK+
 * Author: Federico Mena <federico@nuclecu.unam.mx>
 *         Paul J.Y. Lahaie <pjlahaie@atlsci.com>
 *
 ******************************************************************************
 * Copyright (C) 1998 Red Hat Software, Inc.
 * Copyright (C) 2000 Atlantis Scientific, Inc. (www.atlsci.com)
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
 * $Log: gtkcolorwell.h,v $
 * Revision 1.1.1.1  2005/04/18 16:38:33  uid1026
 * Import reorganized openev tree with initial gtk2 port changes
 *
 * Revision 1.1.1.1  2005/03/07 21:16:36  uid1026
 * openev gtk2 port
 *
 * Revision 1.1.1.1  2005/02/08 00:50:26  uid1026
 *
 * Imported sources
 *
 * Revision 1.3  2001/09/26 20:29:35  pgs
 * removed extraneous variable da (unused)
 *
 * Revision 1.2  2000/06/20 13:27:08  warmerda
 * added standard headers
 *
 */

#ifndef GTK_COLOR_WELL_H
#define GTK_COLOR_WELL_H

#include <gtk/gtkbutton.h>

#define GTK_TYPE_COLOR_WELL            (gtk_color_well_get_type ())
#define GTK_COLOR_WELL(obj)            (GTK_CHECK_CAST ((obj), GTK_TYPE_COLOR_WELL, GtkColorWell))
#define GTK_COLOR_WELL_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_COLOR_WELL, GtkColorWellClass))
#define GTK_IS_COLOR_WELL(obj)         (GTK_CHECK_TYPE ((obj), GTK_TYPE_COLOR_WELL))
#define GTK_IS_COLOR_WELL_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_COLOR_WELL))


typedef struct _GtkColorWell GtkColorWell;
typedef struct _GtkColorWellClass GtkColorWellClass;

struct _GtkColorWell {
    GtkButton button;

    gdouble r, g, b, a;	/* Red, green, blue, and alpha values */

    GtkWidget *preview;	/* Pixmap with the sample contents */

    GtkWidget *color_dialog;	/* Color selection dialog */

    gchar *title;		/* Title for the color selection window */
    guint cont_sig_id;
    guint use_alpha : 1;	/* Use alpha or not */
    guint continuous : 1;

};

struct _GtkColorWellClass {
	GtkButtonClass parent_class;

	/* Signal that is emitted when the color is set.  The rgba values are in the [0, 65535]
	 * range.  If you need a different color format, use the provided functions to get the
	 * values from the color picker.
	 */
        /*  (should be gushort, but Gtk can't marshal that.) */
	void (* color_set) (GtkColorWell *cw, guint r, guint g, guint b, guint a);
};


/* Standard Gtk function */
GtkType gtk_color_well_get_type (void);

/* Creates a new color picker widget */
GtkWidget *gtk_color_well_new (const gchar *title);

/* Set/get the color in the picker.  Values are in [0.0, 1.0] */
void gtk_color_well_set_d (GtkColorWell *cp, gdouble r, gdouble g, gdouble b, gdouble a);
void gtk_color_well_get_d (GtkColorWell *cp, gdouble *r, gdouble *g, gdouble *b, gdouble *a);

/* Set/get the color in the picker.  Values are in [0, 255] */
void gtk_color_well_set_i8 (GtkColorWell *cp, guint8 r, guint8 g, guint8 b, guint8 a);
void gtk_color_well_get_i8 (GtkColorWell *cp, guint8 *r, guint8 *g, guint8 *b, guint8 *a);

/* Set/get the color in the picker.  Values are in [0, 65535] */
void gtk_color_well_set_i16 (GtkColorWell *cp, gushort r, gushort g, gushort b, gushort a);
void gtk_color_well_get_i16 (GtkColorWell *cp, gushort *r, gushort *g, gushort *b, gushort *a);

/* Sets whether the picker should use the alpha channel or not */
void gtk_color_well_set_use_alpha (GtkColorWell *cp, gboolean use_alpha);

/* Sets whether the picker should update the colors continuously */
void gtk_color_well_set_continuous(GtkColorWell *cw, gboolean update_continuous);

/* Sets the title for the color selection dialog */
void gtk_color_well_set_title (GtkColorWell *cp, const gchar *title);


#endif
