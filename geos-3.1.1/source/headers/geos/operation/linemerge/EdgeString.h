/**********************************************************************
 * $Id: EdgeString.h 1820 2006-09-06 16:54:23Z mloskot $
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

#ifndef GEOS_OP_LINEMERGE_EDGESTRING_H
#define GEOS_OP_LINEMERGE_EDGESTRING_H

#include <vector>

// Forward declarations 
namespace geos {
	namespace geom { 
		class GeometryFactory;
		class CoordinateSequence;
		class LineString;
	}
	namespace operation { 
		namespace linemerge { 
			class LineMergeDirectedEdge;
		}
	}
}


namespace geos {
namespace operation { // geos::operation
namespace linemerge { // geos::operation::linemerge

/**
 * \brief
 * A sequence of LineMergeDirectedEdge forming one of the lines that will
 * be output by the line-merging process.
 */
class EdgeString {
private:
	const geom::GeometryFactory *factory;
	std::vector<LineMergeDirectedEdge*> *directedEdges;
	geom::CoordinateSequence *coordinates;
	geom::CoordinateSequence* getCoordinates();
public:
	/*
	 * \brief
	 * Constructs an EdgeString with the given factory used to
	 * convert this EdgeString to a LineString
	 */
	EdgeString(const geom::GeometryFactory *newFactory);

	~EdgeString();

	/**
	* Adds a directed edge which is known to form part of this line.
	*/
	void add(LineMergeDirectedEdge *directedEdge);

	/*
	 * Converts this EdgeString into a LineString.
	 */
	geom::LineString* toLineString();
};

} // namespace geos::operation::linemerge
} // namespace geos::operation
} // namespace geos

#endif // GEOS_OP_LINEMERGE_EDGESTRING_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/22 10:13:53  strk
 * opLinemerge.h split
 *
 **********************************************************************/
