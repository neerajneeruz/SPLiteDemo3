/**********************************************************************
 * $Id: SimpleSnapRounder.h 1820 2006-09-06 16:54:23Z mloskot $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006      Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_NODING_SNAPROUND_SIMPLESNAPROUNDER_H
#define GEOS_NODING_SNAPROUND_SIMPLESNAPROUNDER_H

#include <vector>

#include <geos/inline.h>

#include <geos/noding/Noder.h> // for inheritance
#include <geos/algorithm/LineIntersector.h> // for composition
#include <geos/geom/Coordinate.h> // for use in vector
#include <geos/geom/PrecisionModel.h> // for inlines (should drop)

// Forward declarations
namespace geos {
	namespace geom {
		//class PrecisionModel;
	}
	namespace algorithm {
		class LineIntersector;
	}
	namespace noding {
		class SegmentString;
		namespace snapround {
			class HotPixel;
		}
	}
}

namespace geos {
namespace noding { // geos::noding
namespace snapround { // geos::noding::snapround

/** \brief
 * Uses Snap Rounding to compute a rounded,
 * fully noded arrangement from a set of {@link SegmentString}s.
 *
 * Implements the Snap Rounding technique described in
 * Hobby, Guibas & Marimont, and Goodrich et al.
 * Snap Rounding assumes that all vertices lie on a uniform grid
 * (hence the precision model of the input must be fixed precision,
 * and all the input vertices must be rounded to that precision).
 *
 * This implementation uses simple iteration over the line segments.
 *
 * This implementation appears to be fully robust using an integer
 * precision model.
 * It will function with non-integer precision models, but the
 * results are not 100% guaranteed to be correctly noded.
 *
 * Last port: noding/snapround/SimpleSnapRounder.java rev. 1.2 (JTS-1.7)
 *
 */
class SimpleSnapRounder: public Noder { // implements NoderIface

private:
	const geom::PrecisionModel& pm;
	algorithm::LineIntersector li;
	double scaleFactor;
	std::vector<SegmentString*>* nodedSegStrings;

	void checkCorrectness(std::vector<SegmentString*>& inputSegmentStrings);

	void snapRound(std::vector<SegmentString*>* segStrings,
			algorithm::LineIntersector& li);

	/**
	 * Computes all interior intersections in the vector
	 * of SegmentString, and fill the given vector
	 * with their Coordinates.
	 *
	 * Does NOT node the segStrings.
	 *
	 * @param segStrings a vector of const Coordinates for the intersections
	 * @param li the algorithm::LineIntersector to use
	 * @param ret the vector to push intersection Coordinates to
	 */
	void findInteriorIntersections(std::vector<SegmentString*>& segStrings,
			algorithm::LineIntersector& li, std::vector<geom::Coordinate>& ret);

	/** \brief
	 * Computes nodes introduced as a result of snapping segments to
	 * snap points (hot pixels)
	 * @param li
	 */
	void computeSnaps(const std::vector<SegmentString*>& segStrings,
		std::vector<geom::Coordinate>& snapPts);

	void computeSnaps(SegmentString* ss, std::vector<geom::Coordinate>& snapPts);

	/** \brief
	 * Performs a brute-force comparison of every segment in each
	 * {@link SegmentString}.
	 * This has n^2 performance.
	 */
	void computeVertexSnaps(SegmentString* e0, SegmentString* e1);

public:

	SimpleSnapRounder(const geom::PrecisionModel& newPm);

	std::vector<SegmentString*>* getNodedSubstrings() const;

	void computeNodes(std::vector<SegmentString*>* inputSegmentStrings);

	void add(const SegmentString* segStr);

	/**
	 * Adds a new node (equal to the snap pt) to the segment if the segment
	 * passes through the hot pixel.
	 *
	 * @param hotPix
	 * @param segStr
	 * @param segIndex
	 * @return <code>true</code> if a node was added
	 */
	static bool addSnappedNode(const HotPixel& hotPix,
			SegmentString& segStr,
			unsigned int segIndex);

	/**
	 * Computes nodes introduced as a result of
	 * snapping segments to vertices of other segments
	 *
	 * @param segStrings the list of segment strings to snap together
	 */
	void computeVertexSnaps(const std::vector<SegmentString*>& edges);
};

} // namespace geos::noding::snapround
} // namespace geos::noding
} // namespace geos

//#ifdef GEOS_INLINE
//# include "geos/noding/snapround/SimpleSnapRounder.inl"
//#endif

#endif // GEOS_NODING_SNAPROUND_SIMPLESNAPROUNDER_H

/**********************************************************************
 * $Log$
 * Revision 1.3  2006/05/03 15:02:49  strk
 * moved some implementations from header to .cpp file (taken out of inline)
 *
 * Revision 1.2  2006/03/24 09:52:41  strk
 * USE_INLINE => GEOS_INLINE
 *
 * Revision 1.1  2006/03/14 12:55:56  strk
 * Headers split: geomgraphindex.h, nodingSnapround.h
 *
 **********************************************************************/

