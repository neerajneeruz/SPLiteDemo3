/**********************************************************************
 * $Id: Quadrant.cpp 1820 2006-09-06 16:54:23Z mloskot $
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

#include <sstream>

#include <geos/geomgraph/Quadrant.h>
#include <geos/util/IllegalArgumentException.h>

#include <geos/geom/Coordinate.h>

using namespace std;
using namespace geos::geom;

namespace geos {
namespace geomgraph { // geos.geomgraph

/**
* Returns the quadrant of a directed line segment (specified as x and y
* displacements, which cannot both be 0).
*/
int Quadrant::quadrant(double dx, double dy) {
	if (dx == 0.0 && dy == 0.0) {
		ostringstream s;
		s<<"Cannot compute the quadrant for point ";
		s<<"("<<dx<<","<<dy<<")"<<endl;
		throw util::IllegalArgumentException(s.str());
	}
	if (dx >= 0) {
		if (dy >= 0)
			return 0;
		else
			return 3;
	} else {
		if (dy >= 0)
			return 1;
		else
		return 2;
	}
}

/**
* Returns the quadrant of a directed line segment from p0 to p1.
*/
int Quadrant::quadrant(const Coordinate& p0, const Coordinate& p1) {
	double dx=p1.x-p0.x;
	double dy=p1.y-p0.y;
	if (dx==0.0 && dy==0.0)
	{
		throw util::IllegalArgumentException("Cannot compute the quadrant for two identical points " + p0.toString());
	}
	return quadrant(dx, dy);
}

/**
* Returns true if the quadrants are 1 and 3, or 2 and 4
*/
bool Quadrant::isOpposite(int quad1, int quad2){
	if (quad1==quad2) return false;
	int diff=(quad1-quad2+4)%4;
	// if quadrants are not adjacent, they are opposite
	if (diff==2) return true;
	return false;
}

/** 
* Returns the right-hand quadrant of the halfplane defined by the two quadrants,
* or -1 if the quadrants are opposite, or the quadrant if they are identical.
*/
int Quadrant::commonHalfPlane(int quad1, int quad2){
	// if quadrants are the same they do not determine a unique common halfplane.
	// Simply return one of the two possibilities
	if (quad1==quad2) return quad1;
	int diff=(quad1-quad2+4)%4;
	// if quadrants are not adjacent, they do not share a common halfplane
	if (diff==2) return -1;
	//
	int min=(quad1<quad2)? quad1:quad2;
	int max=(quad1>quad2)? quad1:quad2;
	// for this one case, the righthand plane is NOT the minimum index;
	if (min==0 && max==3) return 3;
	// in general, the halfplane index is the minimum of the two adjacent quadrants
	return min;
}

/**
* Returns whether the given quadrant lies within the given halfplane (specified
* by its right-hand quadrant).
*/
bool Quadrant::isInHalfPlane(int quad, int halfPlane){
	if (halfPlane==3) {
		return quad==3 || quad==0;
	}
	return quad==halfPlane || quad==halfPlane+1;
}

/**
* Returns true if the given quadrant is 0 or 1.
*/
bool Quadrant::isNorthern(int quad) {
	return quad==0 || quad==1;
}

} // namespace geos.geomgraph
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.10  2006/03/15 17:16:29  strk
 * streamlined headers inclusion
 *
 * Revision 1.9  2006/03/09 16:46:47  strk
 * geos::geom namespace definition, first pass at headers split
 *
 * Revision 1.8  2006/03/06 19:40:46  strk
 * geos::util namespace. New GeometryCollection::iterator interface, many cleanups.
 *
 * Revision 1.7  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.6  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.5  2006/02/09 15:52:47  strk
 * GEOSException derived from std::exception; always thrown and cought by const ref.
 *
 * Revision 1.4  2005/02/05 05:44:47  strk
 * Changed geomgraph nodeMap to use Coordinate pointers as keys, reduces
 * lots of other Coordinate copies.
 *
 * Revision 1.3  2005/01/28 09:47:51  strk
 * Replaced sprintf uses with ostringstream.
 *
 * Revision 1.2  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.1  2004/03/19 09:48:45  ybychkov
 * "geomgraph" and "geomgraph/indexl" upgraded to JTS 1.4
 *
 * Revision 1.10  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

