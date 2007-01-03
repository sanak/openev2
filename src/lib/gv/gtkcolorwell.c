/******************************************************************************
 * $Id: gtkcolorwell.c,v 1.1.1.1 2005/04/18 16:38:33 uid1026 Exp $
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
 * $Log: gtkcolorwell.c,v $
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
 * Revision 1.7  2001/09/27 00:39:14  pgs
 * fixed initialization problem with resizing preview widget
 *
 * Revision 1.6  2001/09/26 20:28:52  pgs
 * fixed bug in set functions that called gtk_widget_draw( cp->da ) - da is never
 * initialized to anything and isn't used.
 *
 * Revision 1.5  2001/09/21 20:21:54  pgs
 * tried to make it resizable but it didn't work.
 *
 * Revision 1.4  2001/09/17 03:38:06  pgs
 * updated render to honour use_alpha setting and modified the set_ routines
 * to only render/draw if the widget is realized (prevent Gtk-CRITICAL errors)
 *
 * Revision 1.3  2001/09/16 03:27:32  pgs
 * Modified render code to render transparency.  TODO: make it render not
 * using transparency if this option is not set.
 *
 * Revision 1.2  2000/06/20 13:26:54  warmerda
 * added standard headers
 *
 */

#include <gtk/gtkalignment.h>
#include <gtk/gtkcolorsel.h>
#include <gtk/gtkdrawingarea.h>
#include <gtk/gtkframe.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkcolorseldialog.h>
#include "gtkcolorwell.h"
#include <gdk/gdkkeysyms.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtk.h>

/* These are the dimensions of the color sample in the color picker */
#define COLOR_WELL_WIDTH  24
#define COLOR_WELL_HEIGHT 16
#define COLOR_WELL_PAD    1

/* Size of checks and gray levels for alpha compositing checkerboard*/
#define CHECK_SIZE  4
#define CHECK_DARK  (1.0 / 3.0)
#define CHECK_LIGHT (2.0 / 3.0)

enum {
    COLOR_SET,
    LAST_SIGNAL
};

static void gtk_color_well_class_init (GtkColorWellClass *class);
static void gtk_color_well_init       (GtkColorWell      *cw);
static void gtk_color_well_destroy    (GtkObject             *object);
static void gtk_color_well_clicked    (GtkButton             *button);
static void gtk_color_well_state_changed (GtkWidget *widget, GtkStateType previous_state);
static void gtk_color_well_realize (GtkWidget *widget);
static void gtk_color_well_style_set (GtkWidget *widget, GtkStyle *previous_style);


static guint color_well_signals[LAST_SIGNAL] = { 0 };

static GtkButtonClass *parent_class;


GtkType
gtk_color_well_get_type (void)
{
    static GtkType cp_type = 0;

    if (!cp_type) {
        GtkTypeInfo cp_info = {
            "GtkColorWell",
            sizeof (GtkColorWell),
            sizeof (GtkColorWellClass),
            (GtkClassInitFunc) gtk_color_well_class_init,
            (GtkObjectInitFunc) gtk_color_well_init,
            NULL, /* reserved_1 */
            NULL, /* reserved_2 */
            (GtkClassInitFunc) NULL
        };

        cp_type = gtk_type_unique (gtk_button_get_type (), &cp_info);
    }
    return cp_type;
}

/**
 * Draw color well using pixbuf
 */
static void
render(GtkColorWell *cw)
{
    int e;
    guchar alpha;

    gint x, y, f, n, width, height, rowstride, n_channels;
    guchar c[3], cc[3 * 2], *cp = c;
    gdouble o; //opacity ( 0 - 1 )
    GdkPixbuf *pixbuf;
    guchar *pixels, *p;

    g_return_if_fail (cw != NULL);
    g_return_if_fail (GTK_IS_COLOR_WELL (cw));

    pixbuf = GDK_PIXBUF(gtk_image_get_pixbuf(GTK_IMAGE(cw->preview)));
    rowstride = gdk_pixbuf_get_rowstride (pixbuf);
    n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    pixels = gdk_pixbuf_get_pixels (pixbuf);

    width = gdk_pixbuf_get_width (pixbuf);
    height = gdk_pixbuf_get_height (pixbuf);

    gtk_color_well_get_i8( cw, &c[0], &c[1], &c[2], &alpha );
    o = (gdouble)(alpha / 255.0);

    //for alpha blending to create the gray checkerboard.  The colors here are
    //for either the dark or light areas of the checkerboard.  A nifty
    //calc below chooses between them.
    if (cw->use_alpha) {
	for ( n=0; n < 3; n++ ) {
	    cc[n] = (guchar)((1.0 - o) * 255 + ( o * c[n] ));
	    cc[n+3] = (guchar)((1.0 - o) * 192 + ( o * c[n] ));
	}
	cp = cc;
    }

    for( y = 0; y < height; y++ ) {
	for( x = 0, e = 0; x < width; x++ ) {
	    p = pixels + y * rowstride + x * n_channels;

            //this determines the size of the checkerboard.  The result is
            //either 0 or 3.
            if (cw->use_alpha)
            	f = 3 * (((x % 12) < 6) ^ ((y % 12) < 6 ));
            else
            	f = 0;

            for ( n=0; n < 3; n++ ) {
		p[n] = cp[n+f];
	    }
        }
    }

    gtk_widget_queue_draw( cw->preview );
}

#ifdef REMOVED_GTK2_PORT
static void
render(GtkColorWell *cw)
{
    int e;
    guchar row[COLOR_WELL_WIDTH*3];
    guchar red, green, blue, alpha;

    gint x, y, f, n, width, height;
    guchar c[3], cc[3 * 2], *cp = c;
    gdouble o; //opacity ( 0 - 1 )

    g_return_if_fail (cw != NULL);
    g_return_if_fail (GTK_IS_COLOR_WELL (cw));

	//width = GTK_WIDGET(cw)->allocation.width;
	//height = GTK_WIDGET(cw)->allocation.height;

	width = COLOR_WELL_WIDTH;
	height = COLOR_WELL_HEIGHT;

    gtk_color_well_get_i8( cw, &red, &green, &blue, &alpha );

    //g_message( "red=%d, green=%d, blue=%d, alpha=%d", red, green, blue, alpha);
    c[0] = red;
    c[1] = green;
    c[2] = blue;
    o = (gdouble)(alpha / 255.0);

    //for alpha blending to create the gray checkerboard.  The colors here are
    //for either the dark or light areas of the checkerboard.  A nifty
    //calc below chooses between them.
    if (cw->use_alpha)
    {
		for ( n=0; n < 3; n++ )
		{
			cc[n] = (guchar)((1.0 - o) * 255 + ( o * c[n] ));
			cc[n+3] = (guchar)((1.0 - o) * 192 + ( o * c[n] ));
		}
		cp = cc;
	}

    for( y = 0; y < height; y++ )
    {

        for( x = 0, e = 0; x < width; x++ )
        {
            //this determines the size of the checkerboard.  The result is
            //either 0 or 3.
            if (cw->use_alpha)
            	f = 3 * (((x % 12) < 6) ^ ((y % 12) < 6 ));
            else
            	f = 0;

            for ( n=0; n < 3; n++ )
                row[e++] = cp[n+f];
        }
        gtk_preview_draw_row( GTK_PREVIEW( cw->preview ), row, 0, y, width );
    }

    gtk_widget_queue_draw( cw->preview );
}
#endif

static void
gtk_color_well_class_init (GtkColorWellClass *class)
{
    GtkObjectClass *object_class;
    GtkWidgetClass *widget_class;
    GtkButtonClass *button_class;


    object_class = (GtkObjectClass *) class;
    button_class = (GtkButtonClass *) class;
    widget_class = (GtkWidgetClass *) class;
    parent_class = gtk_type_class (gtk_button_get_type ());

    color_well_signals[COLOR_SET] =
      g_signal_new ("color_set",
		    G_TYPE_FROM_CLASS (class),
		    G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
		    G_STRUCT_OFFSET (GtkColorWellClass, color_set),
		    NULL, NULL,
		    g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

    /* GTK2 PORT...
    color_well_signals[COLOR_SET] =
        gtk_signal_new ("color_set",
                GTK_RUN_FIRST,
                object_class->type,
                GTK_SIGNAL_OFFSET (GtkColorWellClass, color_set),
                gtk_marshal_NONE__POINTER,
                GTK_TYPE_NONE, 1,
                GTK_TYPE_POINTER);
    gtk_object_class_add_signals (object_class, color_well_signals, LAST_SIGNAL);
    */

    object_class->destroy = gtk_color_well_destroy;
    widget_class->state_changed = gtk_color_well_state_changed;
    widget_class->realize = gtk_color_well_realize;
    widget_class->style_set = gtk_color_well_style_set;
    button_class->clicked = gtk_color_well_clicked;

    class->color_set = NULL;
}

static void
gtk_color_well_realize (GtkWidget *widget)
{
    if (GTK_WIDGET_CLASS(parent_class)->realize)
        GTK_WIDGET_CLASS (parent_class)->realize (widget);
    render (GTK_COLOR_WELL (widget));
}
static void
gtk_color_well_style_set (GtkWidget *widget, GtkStyle *previous_style)
{
    if (GTK_WIDGET_CLASS(parent_class)->style_set)
        GTK_WIDGET_CLASS (parent_class)->style_set (widget, previous_style);
    if (GTK_WIDGET_REALIZED (widget))
        render (GTK_COLOR_WELL (widget));
}

static void
gtk_color_well_state_changed (GtkWidget *widget, GtkStateType previous_state)
{
    if (widget->state == GTK_STATE_INSENSITIVE || previous_state == GTK_STATE_INSENSITIVE)
        render (GTK_COLOR_WELL (widget));
}
static void
gtk_color_well_init (GtkColorWell *cw)
{
    cw->continuous = 0;
}

GtkWidget *
gtk_color_well_new(const gchar *title)
{
    GtkWidget *alignment;
    GtkWidget *frame;
    GtkColorWell *color_well;
    GdkPixbuf *pixbuf;

    color_well = GTK_COLOR_WELL(gtk_type_new (gtk_color_well_get_type () ));

    /* ---- Create pixel buffer for rendering color well ---- */
    pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8,
			    COLOR_WELL_WIDTH, COLOR_WELL_HEIGHT);
    if ((color_well->preview = gtk_image_new_from_pixbuf(pixbuf)) == NULL) {
        /* We failed */
    	g_error( "failed to create preview image in gtk_color_well_new" );
    }

    alignment = gtk_alignment_new (0.5, 0.5, 0.0, 0.0 );
    gtk_container_set_border_width( GTK_CONTAINER(alignment), COLOR_WELL_PAD);
    gtk_container_add (GTK_CONTAINER (color_well), alignment );
    gtk_widget_show (alignment);

    frame = gtk_frame_new (NULL);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT );
    gtk_container_add (GTK_CONTAINER (alignment), frame );
    gtk_widget_show (frame);

    gtk_container_add (GTK_CONTAINER(frame), color_well->preview );
    gtk_widget_show(color_well->preview);

    return GTK_WIDGET( color_well );
}

#ifdef REMOVED_GTK2_PORT
GtkWidget *
gtk_color_well_new(const gchar *title)
{
    GtkWidget *alignment;
    GtkWidget *frame;
    GtkColorWell *color_well;

    color_well = GTK_COLOR_WELL(gtk_type_new (gtk_color_well_get_type () ));

    /* Check to make sure we can init a color_well */

    if( ( color_well->preview = gtk_preview_new( GTK_PREVIEW_COLOR ) ) == NULL )
    {
    /* We failed */
    	g_error( "failed to create preview in gtk_color_well_new" );

    }

    alignment = gtk_alignment_new (0.5, 0.5, 0.0, 0.0 );
    gtk_container_set_border_width( GTK_CONTAINER(alignment), COLOR_WELL_PAD);
    gtk_container_add (GTK_CONTAINER (color_well), alignment );
    gtk_widget_show (alignment);

    frame = gtk_frame_new (NULL);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_OUT );
    gtk_container_add (GTK_CONTAINER (alignment), frame );
    gtk_widget_show (frame);

    gtk_preview_size( GTK_PREVIEW( color_well->preview ), COLOR_WELL_WIDTH, COLOR_WELL_HEIGHT );
    gtk_container_add (GTK_CONTAINER(frame), color_well->preview );
    gtk_widget_show(color_well->preview);

    return GTK_WIDGET( color_well );
}
#endif

static void
gtk_color_well_destroy (GtkObject *object)
{
    GtkColorWell *cw;

    g_return_if_fail (object != NULL);
    g_return_if_fail (GTK_IS_COLOR_WELL (object));

    cw = GTK_COLOR_WELL (object);

    /* GTK2 PORT... test and nullify resources */
    if( cw->color_dialog ) {
      gtk_widget_destroy( cw->color_dialog );
      cw->color_dialog = NULL;
    }

    if( cw->preview ) {
        gtk_widget_destroy( cw->preview );
	cw->preview = NULL;
    }

    if (GTK_OBJECT_CLASS (parent_class)->destroy)
        (* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}


/* Callback used when the color selection dialog is destroyed */
static gboolean
color_wheel_destroy(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    GtkColorSelectionDialog *cs = GTK_COLOR_SELECTION_DIALOG (data);

    gtk_widget_hide( GTK_WIDGET( cs ) );

    return TRUE;
}

static void
cs_cancel_clicked (GtkWidget *widget, gpointer data)
{
    GtkColorSelectionDialog *cs = GTK_COLOR_SELECTION_DIALOG (data);

    gtk_widget_hide( GTK_WIDGET( cs ) );
}

/* Callback for when the OK button in the color selection dialog is clicked */
static void
cs_ok_clicked (GtkWidget *widget, gpointer data)
{
    GtkColorWell *cp;
    gdouble color[4];

    cp = GTK_COLOR_WELL (data);

    gtk_color_selection_get_color (GTK_COLOR_SELECTION (GTK_COLOR_SELECTION_DIALOG (cp->color_dialog)->colorsel),
                       color);
    gtk_widget_hide(cp->color_dialog);

    cp->r = color[0];
    cp->g = color[1];
    cp->b = color[2];
    cp->a = cp->use_alpha ? color[3] : 1.0;

    render (cp);

    gtk_signal_emit (GTK_OBJECT (cp), color_well_signals[COLOR_SET] );
}

static void
color_continuous_update( GtkWidget *widget, gpointer data )
{
    GtkColorWell *cp;
    gdouble color[4];

    cp = GTK_COLOR_WELL (data);

    gtk_color_selection_get_color (GTK_COLOR_SELECTION (GTK_COLOR_SELECTION_DIALOG (cp->color_dialog)->colorsel),
                       color);

    cp->r = color[0];
    cp->g = color[1];
    cp->b = color[2];
    cp->a = cp->use_alpha ? color[3] : 1.0;

    render (cp);

    /* Notify the world that the color was set */

    gtk_signal_emit (GTK_OBJECT (cp), color_well_signals[COLOR_SET] );

}


void
gtk_color_well_set_continuous( GtkColorWell *cw, gboolean continuous )
{
    g_return_if_fail( cw != NULL );
    g_return_if_fail( GTK_IS_COLOR_WELL( cw ) );

    if( continuous == cw->continuous )
    return;

    if( continuous )
    {
    if( cw->color_dialog )
        cw->cont_sig_id = gtk_signal_connect( GTK_OBJECT( GTK_COLOR_SELECTION_DIALOG( cw->color_dialog )->colorsel ),
                          "color-changed",
                          (GtkSignalFunc) color_continuous_update,
                          cw );
    cw->continuous = 1;
    } else {
    if( cw->color_dialog )
        gtk_signal_disconnect( GTK_OBJECT( GTK_COLOR_SELECTION_DIALOG( cw->color_dialog )->colorsel ), cw->cont_sig_id );

    cw->continuous = 0;
    }
}


static void
gtk_color_well_clicked (GtkButton *button)
{
    GtkColorWell *cw;
    GtkColorSelectionDialog *csd;
    gdouble color[4];

    g_return_if_fail (button != NULL);
    g_return_if_fail (GTK_IS_COLOR_WELL (button));

    cw = GTK_COLOR_WELL (button);

    /*if dialog already exists, make sure it's shown and raised*/
    if(cw->color_dialog) {
        csd = GTK_COLOR_SELECTION_DIALOG (cw->color_dialog);
        gtk_widget_show(cw->color_dialog);
        if(cw->color_dialog->window)
            gdk_window_raise(cw->color_dialog->window);
    } else {
        /* Create the dialog and connects its buttons */

        cw->color_dialog = gtk_color_selection_dialog_new (cw->title);
        csd = GTK_COLOR_SELECTION_DIALOG (cw->color_dialog);

        gtk_signal_connect (GTK_OBJECT (cw->color_dialog), "delete-event",
                    (GtkSignalFunc) color_wheel_destroy,
                    cw->color_dialog );

        gtk_signal_connect (GTK_OBJECT (csd->ok_button), "clicked",
                    (GtkSignalFunc) cs_ok_clicked,
                    cw);

        gtk_signal_connect (GTK_OBJECT (csd->cancel_button), "clicked",
                    (GtkSignalFunc) cs_cancel_clicked,
                    cw->color_dialog);

        if( cw->continuous )
            cw->cont_sig_id = gtk_signal_connect (GTK_OBJECT (csd->colorsel), "color-changed",
                              (GtkSignalFunc) color_continuous_update,
                              cw);

        /* FIXME: do something about the help button */

        gtk_widget_hide( csd->help_button );

        gtk_window_set_position (GTK_WINDOW (cw->color_dialog), GTK_WIN_POS_MOUSE);

        /* If there is a grabed window, set new dialog as modal */
        if (gtk_grab_get_current())
            gtk_window_set_modal(GTK_WINDOW(cw->color_dialog),TRUE);
    }
    gtk_color_selection_set_has_opacity_control
        (GTK_COLOR_SELECTION (csd->colorsel), cw->use_alpha);

    color[0] = cw->r;
    color[1] = cw->g;
    color[2] = cw->b;
    color[3] = cw->use_alpha ? cw->a : 1.0;

    /* Hack: we set the color twice so that GtkColorSelection will remember its history */
    gtk_color_selection_set_color (GTK_COLOR_SELECTION (csd->colorsel), color);
    gtk_color_selection_set_color (GTK_COLOR_SELECTION (csd->colorsel), color);

    gtk_widget_show (cw->color_dialog);
}


/**
 * gtk_color_well_set_d
 * @cp: Pointer to GNOME color picker widget.
 * @r: Red color component, values are in [0.0, 1.0]
 * @g: Green color component, values are in [0.0, 1.0]
 * @b: Blue color component, values are in [0.0, 1.0]
 * @a: Alpha component, values are in [0.0, 1.0]
 *
 * Description:
 * Set color shown in the color picker widget using floating point values.
 */

void
gtk_color_well_set_d (GtkColorWell *cw, gdouble r, gdouble g, gdouble b, gdouble a)
{
    g_return_if_fail (cw != NULL);
    g_return_if_fail (GTK_IS_COLOR_WELL (cw));
    g_return_if_fail ((r >= 0.0) && (r <= 1.0));
    g_return_if_fail ((g >= 0.0) && (g <= 1.0));
    g_return_if_fail ((b >= 0.0) && (b <= 1.0));
    g_return_if_fail ((a >= 0.0) && (a <= 1.0));

    cw->r = r;
    cw->g = g;
    cw->b = b;
    cw->a = a;

	if ( !GTK_WIDGET_REALIZED( cw ) ) return;

    render( cw );
}


/**
 * gtk_color_well_get_d
 * @cp: Pointer to GNOME color picker widget.
 * @r: Output location of red color component, values are in [0.0, 1.0]
 * @g: Output location of green color component, values are in [0.0, 1.0]
 * @b: Output location of blue color component, values are in [0.0, 1.0]
 * @a: Output location of alpha color component, values are in [0.0, 1.0]
 *
 * Description:
 * Retrieve color currently selected in the color picker widget in the form of floating point values.
 */

void gtk_color_well_get_d (GtkColorWell *cp, gdouble *r, gdouble *g, gdouble *b, gdouble *a)
{
    g_return_if_fail (cp != NULL);
    g_return_if_fail (GTK_IS_COLOR_WELL (cp));

    if (r)
        *r = cp->r;

    if (g)
        *g = cp->g;

    if (b)
        *b = cp->b;

    if (a)
        *a = cp->a;
}


/**
 * gtk_color_well_set_i8
 * @cp: Pointer to GNOME color picker widget.
 * @r: Red color component, values are in [0, 255]
 * @g: Green color component, values are in [0, 255]
 * @b: Blue color component, values are in [0, 255]
 * @a: Alpha component, values are in [0, 255]
 *
 * Description:
 * Set color shown in the color picker widget using 8-bit integer values.
 */

void
gtk_color_well_set_i8 (GtkColorWell *cp, guint8 r, guint8 g, guint8 b, guint8 a)
{
    g_return_if_fail (cp != NULL);
    g_return_if_fail (GTK_IS_COLOR_WELL (cp));
    /* Don't check range of r,g,b,a since it's a 8 bit unsigned type. */

    cp->r = r / 255.0;
    cp->g = g / 255.0;
    cp->b = b / 255.0;
    cp->a = a / 255.0;

	if ( !GTK_WIDGET_REALIZED( cp ) ) return;

	render( cp );
}


/**
 * gtk_color_well_get_i8
 * @cp: Pointer to GNOME color picker widget.
 * @r: Output location of red color component, values are in [0, 255]
 * @g: Output location of green color component, values are in [0, 255]
 * @b: Output location of blue color component, values are in [0, 255]
 * @a: Output location of alpha color component, values are in [0, 255]
 *
 * Description:
 * Retrieve color currently selected in the color picker widget in the form of 8-bit integer values.
 */

void
gtk_color_well_get_i8 (GtkColorWell *cp, guint8 *r, guint8 *g, guint8 *b, guint8 *a)
{
    g_return_if_fail (cp != NULL);
    g_return_if_fail (GTK_IS_COLOR_WELL (cp));

    if (r)
        *r = (guint8) (cp->r * 255.0 + 0.5);

    if (g)
        *g = (guint8) (cp->g * 255.0 + 0.5);

    if (b)
        *b = (guint8) (cp->b * 255.0 + 0.5);

    if (a)
        *a = (guint8) (cp->a * 255.0 + 0.5);
}


/**
 * gtk_color_well_set_i16
 * @cp: Pointer to GNOME color picker widget.
 * @r: Red color component, values are in [0, 65535]
 * @g: Green color component, values are in [0, 65535]
 * @b: Blue color component, values are in [0, 65535]
 * @a: Alpha component, values are in [0, 65535]
 *
 * Description:
 * Set color shown in the color picker widget using 16-bit integer values.
 */

void
gtk_color_well_set_i16 (GtkColorWell *cp, gushort r, gushort g, gushort b, gushort a)
{
    g_return_if_fail (cp != NULL);
    g_return_if_fail (GTK_IS_COLOR_WELL (cp));
    /* Don't check range of r,g,b,a since it's a 16 bit unsigned type. */

    cp->r = r / 65535.0;
    cp->g = g / 65535.0;
    cp->b = b / 65535.0;
    cp->a = a / 65535.0;

	if ( !GTK_WIDGET_REALIZED( cp ) ) return;

	render( cp );
}


/**
 * gtk_color_well_get_i16
 * @cp: Pointer to GNOME color picker widget.
 * @r: Output location of red color component, values are in [0, 65535]
 * @g: Output location of green color component, values are in [0, 65535]
 * @b: Output location of blue color component, values are in [0, 65535]
 * @a: Output location of alpha color component, values are in [0, 65535]
 *
 * Description:
 * Retrieve color currently selected in the color picker widget in the form of 16-bit integer values.
 */

void
gtk_color_well_get_i16 (GtkColorWell *cp, gushort *r, gushort *g, gushort *b, gushort *a)
{
    g_return_if_fail (cp != NULL);
    g_return_if_fail (GTK_IS_COLOR_WELL (cp));

    if (r)
        *r = (gushort) (cp->r * 65535.0 + 0.5);

    if (g)
        *g = (gushort) (cp->g * 65535.0 + 0.5);

    if (b)
        *b = (gushort) (cp->b * 65535.0 + 0.5);

    if (a)
        *a = (gushort) (cp->a * 65535.0 + 0.5);
}


/**
 * gtk_color_well_set_dither
 * @cp: Pointer to GNOME color picker widget.
 * @dither: %TRUE if color sample should be dithered, %FALSE if not.
 *
 * Description:
 * Sets whether the picker should dither the color sample or just paint
 * a solid rectangle.
 */

/**
 * gtk_color_well_set_use_alpha
 * @cp: Pointer to GNOME color picker widget.
 * @use_alpha: %TRUE if color sample should use alpha channel, %FALSE if not.
 *
 * Description:
 * Sets whether or not the picker should use the alpha channel.
 */

void
gtk_color_well_set_use_alpha (GtkColorWell *cp, gboolean use_alpha)
{
    g_return_if_fail (cp != NULL);
    g_return_if_fail (GTK_IS_COLOR_WELL (cp));

    cp->use_alpha = use_alpha ? TRUE : FALSE;

	if ( !GTK_WIDGET_REALIZED( cp ) ) return;

	render( cp );
}


/**
 * gtk_color_well_set_title
 * @cp: Pointer to GNOME color picker widget.
 * @title: String containing new window title.
 *
 * Description:
 * Sets the title for the color selection dialog.
 */

void
gtk_color_well_set_title (GtkColorWell *cw, const gchar *title)
{
    g_return_if_fail (cw != NULL);
    g_return_if_fail (GTK_IS_COLOR_WELL (cw));

    if (cw->title)
        g_free (cw->title);

    cw->title = g_strdup (title);

    if (cw->color_dialog)
        gtk_window_set_title (GTK_WINDOW (cw->color_dialog), cw->title);
}
