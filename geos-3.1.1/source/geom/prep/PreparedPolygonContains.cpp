/**********************************************************************
 * $Id: PreparedPolygonContains.cpp 2194 2008-09-23 23:01:00Z mloskot $
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


#include <geos/geom/prep/PreparedPolygonContains.h>
#include <geos/geom/prep/PreparedPolygon.h>
#include <geos/geom/Geometry.h>

namespace geos {
namespace geom { // geos.geom
namespace prep { // geos.geom.prep

//
// public:
//
PreparedPolygonContains::PreparedPolygonContains(const PreparedPolygon * const prepPoly)
    : AbstractPreparedPolygonContains( prepPoly)
{
}

//
// protected:
//
bool 
PreparedPolygonContains::fullTopologicalPredicate( const geom::Geometry * geom)
{
	bool isContained = prepPoly->getGeometry().contains( geom);
	return isContained;
}

//
// private:
//
} // namespace geos.geom.prep
} // namespace geos.geom
} // namespace geos

/**********************************************************************
 * $Log$
 *
 **********************************************************************/
