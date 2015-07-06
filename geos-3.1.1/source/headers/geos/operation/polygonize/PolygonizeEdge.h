/**********************************************************************
 * $Id: PolygonizeEdge.h 1820 2006-09-06 16:54:23Z mloskot $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/


#ifndef GEOS_OP_POLYGONIZE_POLYGONIZEEDGE_H
#define GEOS_OP_POLYGONIZE_POLYGONIZEEDGE_H

#include <geos/planargraph/Edge.h> // for inheritance

// Forward declarations
namespace geos {
	namespace geom { 
		class LineString;
	}
}

namespace geos {
namespace operation { // geos::operation
namespace polygonize { // geos::operation::polygonize

/* \brief
 * An edge of a polygonization graph.
 *
 * @version 1.4
 */
class PolygonizeEdge: public planargraph::Edge {
private:
	const geom::LineString *line;
public:
	PolygonizeEdge(const geom::LineString *newLine);
	const geom::LineString* getLine();
};

} // namespace geos::operation::polygonize
} // namespace geos::operation
} // namespace geos

#endif // GEOS_OP_POLYGONIZE_POLYGONIZEEDGE_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/22 11:19:06  strk
 * opPolygonize.h headers split.
 *
 **********************************************************************/
