/**********************************************************************
 * $Id: LineMergeDirectedEdge.h 1820 2006-09-06 16:54:23Z mloskot $
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

#ifndef GEOS_OP_LINEMERGE_LINEMERGEDIRECTEDEDGE_H
#define GEOS_OP_LINEMERGE_LINEMERGEDIRECTEDEDGE_H

#include <geos/planargraph/DirectedEdge.h> // for inheritance

// Forward declarations 
namespace geos {
	namespace geom { 
		class Coordinate;
		//class LineString;
	}
	namespace planargraph { 
		class Node;
	}
}


namespace geos {
namespace operation { // geos::operation
namespace linemerge { // geos::operation::linemerge

/**
 * \brief
 * A planargraph::DirectedEdge of a LineMergeGraph. 
 *
 */
class LineMergeDirectedEdge: public planargraph::DirectedEdge {
public:
	/**
	 * Constructs a LineMergeDirectedEdge connecting the <code>from</code>
	 * node to the <code>to</code> node.
	 *
	 * @param directionPt
	 *        specifies this DirectedEdge's direction (given by an
	 *	  imaginary line from the <code>from</code> node to
	 *	  <code>directionPt</code>)
	 *
	 * @param edgeDirection
	 *        whether this DirectedEdge's direction is the same as or
	 *        opposite to that of the parent Edge (if any)
	 */  
	LineMergeDirectedEdge(planargraph::Node *from,
			planargraph::Node *to,
			const geom::Coordinate& directionPt,
			bool edgeDirection);

	/**
	* Returns the directed edge that starts at this directed edge's end point, or null
	* if there are zero or multiple directed edges starting there.  
	* @return
	*/
	LineMergeDirectedEdge* getNext();
};

} // namespace geos::operation::linemerge
} // namespace geos::operation
} // namespace geos

#endif // GEOS_OP_LINEMERGE_LINEMERGEDIRECTEDEDGE_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/22 10:13:53  strk
 * opLinemerge.h split
 *
 **********************************************************************/
