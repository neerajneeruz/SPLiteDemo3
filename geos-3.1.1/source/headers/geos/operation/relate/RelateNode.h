/**********************************************************************
 * $Id: RelateNode.h 1820 2006-09-06 16:54:23Z mloskot $
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

#ifndef GEOS_OP_RELATE_RELATENODE_H
#define GEOS_OP_RELATE_RELATENODE_H

#include <geos/geomgraph/Node.h> // for inheritance

// Forward declarations
namespace geos {
	namespace geom {
		class IntersectionMatrix;
		class Coordinate;
	}
	namespace geomgraph {
		class EdgeEndStar;
	}
}


namespace geos {
namespace operation { // geos::operation
namespace relate { // geos::operation::relate

/** \brief
 * Represents a node in the topological graph used to compute spatial
 * relationships.
 */
class RelateNode: public geomgraph::Node {

public:

	RelateNode(const geom::Coordinate& coord, geomgraph::EdgeEndStar *edges);

	virtual ~RelateNode();

	void updateIMFromEdges(geom::IntersectionMatrix *im);

protected:

	void computeIM(geom::IntersectionMatrix *im);
};


} // namespace geos:operation:relate
} // namespace geos:operation
} // namespace geos

#endif // GEOS_OP_RELATE_RELATENODE_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/21 13:11:29  strk
 * opRelate.h header split
 *
 **********************************************************************/

