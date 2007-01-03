/******************************************************************************
 * $Id: ipgcplayer.h,v 1.1.1.1 2005/04/18 16:38:34 uid1026 Exp $
 *
 * Project:  InSAR Peppers
 * Purpose:  InSAR Peppers GCP Layer declarations.
 * Author:   Frank Warmerdam, warmerda@home.com
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
 * $Log: ipgcplayer.h,v $
 * Revision 1.1.1.1  2005/04/18 16:38:34  uid1026
 * Import reorganized openev tree with initial gtk2 port changes
 *
 * Revision 1.1.1.1  2005/03/07 21:16:36  uid1026
 * openev gtk2 port
 *
 * Revision 1.1.1.1  2005/02/08 00:50:27  uid1026
 *
 * Imported sources
 *
 * Revision 1.1  2000/07/14 18:24:57  warmerda
 * New
 *
 */

#ifndef __IPGCPLAYER_H__
#define __IPGCPLAYER_H__

#include "gvshapeslayer.h"

#define IP_TYPE_GCP_LAYER            (ip_gcp_layer_get_type ())
#define IP_GCP_LAYER(obj)            (GTK_CHECK_CAST ((obj), IP_TYPE_GCP_LAYER, IpGcpLayer))
#define IP_GCP_LAYER_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), IP_TYPE_GCP_LAYER, IpGcpLayerClass))
#define IP_IS_GCP_LAYER(obj)         (GTK_CHECK_TYPE ((obj),IP_TYPE_GCP_LAYER))
#define IP_IS_GCP_LAYER_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), \
                                                            IP_TYPE_GCP_LAYER))

typedef struct _IpGcpLayer       IpGcpLayer;
typedef struct _IpGcpLayerClass  IpGcpLayerClass;

struct _IpGcpLayer
{
    GvShapesLayer layer;
};

struct _IpGcpLayerClass
{
    GvShapesLayerClass parent_class;
};

GtkType ip_gcp_layer_get_type(void);
GtkObject* ip_gcp_layer_new(void);

#endif /* __IPGCPLAYER_H__ */
