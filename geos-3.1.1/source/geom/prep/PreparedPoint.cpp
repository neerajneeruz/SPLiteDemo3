/**********************************************************************
 * $Id: PreparedPoint.cpp 2158 2008-08-18 16:24:18Z mloskot $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/


#include <geos/geom/prep/PreparedPoint.h>

namespace geos {
namespace geom { // geos.geom
namespace prep { // geos.geom.prep

bool 
PreparedPoint::intersects( geom::Geometry* g)
{
	if (! envelopesIntersect( g)) return false;

	// This avoids computing topology for the test geometry
	return isAnyTargetComponentInTest( g);
}

} // namespace geos.geom.prep
} // namespace geos.geom
} // namespace geos

/**********************************************************************
 * $Log$
 *
 **********************************************************************/
