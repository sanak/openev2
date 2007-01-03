/******************************************************************************
 * $Id: gview.h,v 1.1.1.1 2005/04/18 16:38:33 uid1026 Exp $
 *
 * Project:  OpenEV
 * Purpose:  Primary OpenEV include file.
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
 * $Log: gview.h,v $
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
 * Revision 1.15  2003/06/25 16:43:24  warmerda
 * added gvrotatetool.h
 *
 * Revision 1.14  2003/05/23 16:18:17  warmerda
 * added GvRecords for CIETMap
 *
 * Revision 1.13  2003/02/28 16:47:48  warmerda
 * added gvsymbolmanager.h
 *
 * Revision 1.12  2002/02/28 18:52:22  gmwalter
 * Added a point-of-interest tool similar to the region-of-interest
 * tool (allows a user to select a temporary point without having to add a
 * new layer).  Added a mechanism to allow some customization of openev
 * via a textfile defining external modules.
 *
 * Revision 1.11  2000/08/25 20:05:34  warmerda
 * added appcurlayer
 *
 * Revision 1.10  2000/07/25 23:33:04  warmerda
 * added rectangle tool
 *
 * Revision 1.9  2000/07/14 18:25:53  warmerda
 * added ipgcplayer
 *
 * Revision 1.8  2000/06/20 13:27:08  warmerda
 * added standard headers
 *
 */

#ifndef __GVIEW_H__
#define __GVIEW_H__

#include "gvtypes.h"
#include "gvmanager.h"
#include "gvdata.h"
#include "gvundo.h"
#include "gvviewarea.h"
#include "gvviewlink.h"
#include "gvpointlayer.h"
#include "gvlinelayer.h"
#include "gvarealayer.h"
#include "gvpquerylayer.h"
#include "ipgcplayer.h"
#include "appcurlayer.h"
#include "gvrecords.h"
#include "gvrasterlayer.h"
#include "gvrasterlut.h"
#include "gvselecttool.h"
#include "gvpointtool.h"
#include "gvlinetool.h"
#include "gvrecttool.h"
#include "gvrotatetool.h"
#include "gvareatool.h"
#include "gvnodetool.h"
#include "gvroitool.h"
#include "gvpoitool.h"
#include "gvtracktool.h"
#include "gvzoompantool.h"
#include "gvtoolbox.h"
#include "gvsymbolmanager.h"

#endif /*__GVIEW_H__*/
