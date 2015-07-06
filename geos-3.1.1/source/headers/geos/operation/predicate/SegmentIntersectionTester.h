/**********************************************************************
 * $Id: SegmentIntersectionTester.h 1820 2006-09-06 16:54:23Z mloskot $
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

#ifndef GEOS_OP_PREDICATE_SEGMENTINTERSECTIONTESTER_H
#define GEOS_OP_PREDICATE_SEGMENTINTERSECTIONTESTER_H

#include <geos/algorithm/LineIntersector.h> // for composition

// Forward declarations
namespace geos {
	namespace geom {
		class LineString;
		class CoordinateSequence;
	}
}

namespace geos {
namespace operation { // geos::operation
namespace predicate { // geos::operation::predicate

/** \brief
 * Tests if any line segments in two sets of CoordinateSequences intersect.
 * Optimized for small geometry size.
 *
 * Short-circuited to return as soon an intersection is found.
 *
 * Last port: operation/predicate/SegmentIntersectionTester.java rev. 1.6
 * (JTS-1.7)
 *
 */
class SegmentIntersectionTester {

private:

	/// \brief
	/// For purposes of intersection testing,
	/// don't need to set precision model
	///
	algorithm::LineIntersector li; // Robust

	bool hasIntersectionVar;

public:

	SegmentIntersectionTester(): hasIntersectionVar(false) {}

	bool hasIntersectionWithLineStrings(const geom::CoordinateSequence &seq,
		const std::vector<const geom::LineString *>& lines);

	bool hasIntersection(const geom::CoordinateSequence &seq0,
		const geom::CoordinateSequence &seq1);


};

} // namespace geos::operation::predicate
} // namespace geos::operation
} // namespace geos

#endif // ifndef GEOS_OP_PREDICATE_SEGMENTINTERSECTIONTESTER_H
