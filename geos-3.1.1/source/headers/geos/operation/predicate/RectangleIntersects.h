/**********************************************************************
 * $Id: RectangleIntersects.h 1820 2006-09-06 16:54:23Z mloskot $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_OP_PREDICATE_RECTANGLEINTERSECTS_H
#define GEOS_OP_PREDICATE_RECTANGLEINTERSECTS_H

#include <geos/geom/Polygon.h> // for inlines

// Forward declarations
namespace geos {
	namespace geom {
		class Envelope;
		//class Polygon;
	}
}

namespace geos {
namespace operation { // geos::operation
namespace predicate { // geos::operation::predicate

/** \brief
 * Optimized implementation of spatial predicate "intersects"
 * for cases where the first Geometry is a rectangle.
 * 
 * As a further optimization,
 * this class can be used directly to test many geometries against a single
 * rectangle.
 *
 * Last port: operation/predicate/RectangleIntersects.java rev. 1.3 (JTS-1.7)
 */
class RectangleIntersects {

private:

	const geom::Polygon &rectangle;

 	const geom::Envelope &rectEnv;

public:

	/** \brief
	 * Crossover size at which brute-force intersection scanning
	 * is slower than indexed intersection detection.
	 *
	 * Must be determined empirically.  Should err on the
	 * safe side by making value smaller rather than larger.
	 */
	static const size_t MAXIMUM_SCAN_SEGMENT_COUNT;

	/** \brief
	 * Create a new intersects computer for a rectangle.
	 *
	 * @param newRect a rectangular geometry
	 */
	RectangleIntersects(const geom::Polygon &newRect)
		:
		rectangle(newRect),
		rectEnv(*(newRect.getEnvelopeInternal()))
	{}

	bool intersects(const geom::Geometry& geom);

	static bool intersects(const geom::Polygon &rectangle,
			const geom::Geometry &b)
	{
		RectangleIntersects rp(rectangle); 
		return rp.intersects(b);
	}

};


} // namespace geos::operation::predicate
} // namespace geos::operation
} // namespace geos

#endif // ifndef GEOS_OP_PREDICATE_RECTANGLEINTERSECTS_H
