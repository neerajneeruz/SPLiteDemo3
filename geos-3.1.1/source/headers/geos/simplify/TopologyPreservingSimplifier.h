/**********************************************************************
 * $Id: TopologyPreservingSimplifier.h 1820 2006-09-06 16:54:23Z mloskot $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: simplify/TopologyPreservingSimplifier.java rev. 1.4 (JTS-1.7)
 *
 **********************************************************************
 *
 * NOTES:
 *
 **********************************************************************/

#ifndef _GEOS_SIMPLIFY_TOPOLOGYPRESERVINGSIMPLIFIER_H_
#define _GEOS_SIMPLIFY_TOPOLOGYPRESERVINGSIMPLIFIER_H_ 

#include <memory> // for auto_ptr
#include <map> 

// Forward declarations
namespace geos {
	namespace simplify {
		class TaggedLinesSimplifier;
	}
	namespace geom {
		class Geometry;
	}
}

namespace geos {
namespace simplify { // geos::simplify

/** \brief
 *
 * Simplifies a geometry, ensuring that
 * the result is a valid geometry having the
 * same dimension and number of components as the input.
 *
 * The simplification uses a maximum distance difference algorithm
 * similar to the one used in the Douglas-Peucker algorithm.
 * 
 * In particular, if the input is an areal geometry
 * ( Polygon or MultiPolygon )
 * 
 *  -  The result has the same number of shells and holes (rings) as the input,
 *     in the same order
 *  -  The result rings touch at <b>no more</b> than the number of touching point in the input
 *     (although they may touch at fewer points)
 *
 */
class TopologyPreservingSimplifier
{

public:

	static std::auto_ptr<geom::Geometry> simplify(
			const geom::Geometry* geom,
			double tolerance);

	TopologyPreservingSimplifier(const geom::Geometry* geom);

	/** \brief
	 * Sets the distance tolerance for the simplification.
	 *
	 * All vertices in the simplified geometry will be within this
	 * distance of the original geometry.
	 * The tolerance value must be non-negative.  A tolerance value
	 * of zero is effectively a no-op.
	 *
	 * @param distanceTolerance the approximation tolerance to use
	 */
	void setDistanceTolerance(double tolerance);

	std::auto_ptr<geom::Geometry> getResultGeometry();

private:

	const geom::Geometry* inputGeom;

	std::auto_ptr<TaggedLinesSimplifier> lineSimplifier;

};



} // namespace geos::simplify
} // namespace geos

#endif // _GEOS_SIMPLIFY_TOPOLOGYPRESERVINGSIMPLIFIER_H_ 

/**********************************************************************
 * $Log$
 * Revision 1.3  2006/04/13 16:04:10  strk
 * Made TopologyPreservingSimplifier implementation successfully build
 *
 * Revision 1.2  2006/04/13 14:25:17  strk
 * TopologyPreservingSimplifier initial port
 *
 **********************************************************************/
