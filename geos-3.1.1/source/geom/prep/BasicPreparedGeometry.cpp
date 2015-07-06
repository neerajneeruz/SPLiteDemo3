/**********************************************************************
 * $Id: BasicPreparedGeometry.cpp 2158 2008-08-18 16:24:18Z mloskot $
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


#include <geos/geom/prep/BasicPreparedGeometry.h>
#include <geos/geom/Coordinate.h> 
#include <geos/algorithm/PointLocator.h> 
#include <geos/geom/util/ComponentCoordinateExtracter.h> 

namespace geos {
namespace geom { // geos.geom
namespace prep { // geos.geom.prep

/*            *
 * protected: *
 *            */

void 
BasicPreparedGeometry::setGeometry( const geom::Geometry * geom ) 
{
	baseGeom = geom;
	geom::util::ComponentCoordinateExtracter::getCoordinates(*baseGeom, representativePts);
}

bool 
BasicPreparedGeometry::envelopesIntersect( const geom::Geometry* g) const
{
	return baseGeom->getEnvelopeInternal()->intersects(g->getEnvelopeInternal());
}

bool 
BasicPreparedGeometry::envelopeCovers( const geom::Geometry* g) const
{
	return baseGeom->getEnvelopeInternal()->covers(g->getEnvelopeInternal());
}

/*
 * public:
 */
BasicPreparedGeometry::BasicPreparedGeometry( const Geometry * geom)
{
	setGeometry( geom);
}

BasicPreparedGeometry::~BasicPreparedGeometry( )
{
}


bool 
BasicPreparedGeometry::isAnyTargetComponentInTest( const geom::Geometry * testGeom) const
{
	algorithm::PointLocator *locator = new algorithm::PointLocator();

	for (size_t i=0, n=representativePts.size(); i<n; i++)
	{
		geom::Coordinate c = *(representativePts[i]);
		if ( locator->intersects( c, testGeom) )
		return true;
	}
	return false;
}

bool 
BasicPreparedGeometry::contains(const geom::Geometry * g) const
{
	return baseGeom->contains(g);
}

bool 
BasicPreparedGeometry::containsProperly(const geom::Geometry * g)	const
{
	return baseGeom->relate(g, "T**FF*FF*");
}

bool 
BasicPreparedGeometry::coveredBy(const geom::Geometry * g) const
{
	return baseGeom->coveredBy(g);
}

bool 
BasicPreparedGeometry::covers(const geom::Geometry * g) const
{
	return baseGeom->covers(g);
}

bool 
BasicPreparedGeometry::crosses(const geom::Geometry * g) const
{
	return baseGeom->crosses(g);
}

bool 
BasicPreparedGeometry::disjoint(const geom::Geometry * g)	const
{
	return ! intersects(g);
}

bool 
BasicPreparedGeometry::intersects(const geom::Geometry * g) const
{
	return baseGeom->intersects(g);
}

bool 
BasicPreparedGeometry::overlaps(const geom::Geometry * g)	const
{
	return baseGeom->overlaps(g);
}

bool 
BasicPreparedGeometry::touches(const geom::Geometry * g) const
{
	return baseGeom->touches(g);
}

bool 
BasicPreparedGeometry::within(const geom::Geometry * g) const
{
	return baseGeom->within(g);
}

std::string 
BasicPreparedGeometry::toString()
{
	return baseGeom->toString();
}

} // namespace geos.geom.prep
} // namespace geos.geom
} // namespace geos

/**********************************************************************
 * $Log$
 *
 **********************************************************************/
