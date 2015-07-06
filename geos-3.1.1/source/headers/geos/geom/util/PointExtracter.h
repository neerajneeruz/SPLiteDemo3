/**********************************************************************
 * $Id: PointExtracter.h 1820 2006-09-06 16:54:23Z mloskot $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_GEOM_UTIL_POINTEXTRACTER_H
#define GEOS_GEOM_UTIL_POINTEXTRACTER_H

#include <geos/geom/GeometryFilter.h>
#include <geos/geom/Point.h>
#include <geos/platform.h>
#include <vector>

namespace geos {
namespace geom { // geos.geom
namespace util { // geos.geom.util

/**
 * Extracts all the 0-dimensional (Point) components from a Geometry.
 */
class PointExtracter: public GeometryFilter {

private:
	Point::ConstVect& comps;

public:
	/**
	 * Returns the Point components from a single geometry.
	 * If more than one geometry is to be processed, it is more
	 * efficient to create a single PointExtracter filter instance
	 * and pass it to multiple geometries.
	 */
	static void getPoints(const Geometry &geom, Point::ConstVect &ret)
	{
		PointExtracter pe(ret);
		geom.apply_ro(&pe);
	}

	/**
	 * Constructs a PointExtracterFilter with a list in which
	 * to store Points found.
	 */
	PointExtracter(Point::ConstVect& newComps)
		:
		comps(newComps)
		{}

	void filter_rw(Geometry *geom)
	{
if ( const Point *p=dynamic_cast<const Point *>(geom) )
		comps.push_back(p);
	}

	void filter_ro(const Geometry *geom)
	{
if ( const Point *p=dynamic_cast<const Point *>(geom) )
		comps.push_back(p);
	}

};

} // namespace geos.geom.util
} // namespace geos.geom
} // namespace geos

#endif

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/
