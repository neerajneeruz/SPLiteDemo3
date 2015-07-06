/**********************************************************************
 * $Id: ConnectedElementPointFilter.cpp 2131 2008-07-15 22:04:51Z mloskot $
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

#include <geos/operation/distance/ConnectedElementPointFilter.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/Point.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Polygon.h>

#include <vector>
#include <typeinfo>

using namespace std;
using namespace geos::geom;

namespace geos {
namespace operation { // geos.operation
namespace distance { // geos.operation.distance

/**
* Returns a list containing a Coordinate from each Polygon, LineString, and Point
* found inside the specified geometry. Thus, if the specified geometry is
* not a GeometryCollection, an empty list will be returned.
*/
vector<const Coordinate*>*
ConnectedElementPointFilter::getCoordinates(const Geometry *geom)
{
	vector<const Coordinate*> *points=new vector<const Coordinate*>();
	ConnectedElementPointFilter c(points);
	geom->apply_ro(&c);
	return points;
}

void ConnectedElementPointFilter::filter_ro(const Geometry *geom) {
	if ((typeid(*geom)==typeid(Point)) ||
		(typeid(*geom)==typeid(LineString)) ||
		(typeid(*geom)==typeid(Polygon)))
			pts->push_back(geom->getCoordinate());
}

} // namespace geos.operation.distance
} // namespace geos.operation
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.11  2006/03/21 17:55:01  strk
 * opDistance.h header split
 *
 **********************************************************************/

