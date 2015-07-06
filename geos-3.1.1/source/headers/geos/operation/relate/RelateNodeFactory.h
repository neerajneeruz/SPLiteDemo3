/**********************************************************************
 * $Id: RelateNodeFactory.h 1820 2006-09-06 16:54:23Z mloskot $
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

#ifndef GEOS_OP_RELATE_RELATENODEFACTORY_H
#define GEOS_OP_RELATE_RELATENODEFACTORY_H

#include <geos/geomgraph/NodeFactory.h> // for RelateNodeFactory inheritance

// Forward declarations
namespace geos {
	namespace geom {
		class Coordinate;
	}
	namespace geomgraph {
		class Node;
	}
}


namespace geos {
namespace operation { // geos::operation
namespace relate { // geos::operation::relate

/** \brief
 * Used by the geomgraph::NodeMap in a RelateNodeGraph to create RelateNode objects.
 */
class RelateNodeFactory: public geomgraph::NodeFactory {
public:
	geomgraph::Node* createNode(const geom::Coordinate &coord) const;
	static const geomgraph::NodeFactory &instance();
private:
	RelateNodeFactory() {};
};


} // namespace geos:operation:relate
} // namespace geos:operation
} // namespace geos

#endif // GEOS_OP_RELATE_RELATENODEFACTORY_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/21 13:11:29  strk
 * opRelate.h header split
 *
 **********************************************************************/

