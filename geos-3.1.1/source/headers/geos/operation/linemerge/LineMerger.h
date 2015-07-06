/**********************************************************************
 * $Id: LineMerger.h 1820 2006-09-06 16:54:23Z mloskot $
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

#ifndef GEOS_OP_LINEMERGE_LINEMERGER_H
#define GEOS_OP_LINEMERGE_LINEMERGER_H

#include <geos/operation/linemerge/LineMergeGraph.h> // for composition

#include <vector>

// Forward declarations 
namespace geos {
	namespace geom { 
		class LineString;
		class GeometryFactory;
		class Geometry;
	}
	namespace planargraph {
		class Node;
	}
	namespace operation { 
		namespace linemerge { 
			class EdgeString;
			class LineMergeDirectedEdge;
		}
	}
}


namespace geos {
namespace operation { // geos::operation
namespace linemerge { // geos::operation::linemerge

/**
 *
 * \brief
 * Sews together a set of fully noded LineStrings.
 *
 * Sewing stops at nodes of degree 1 or 3 or more.
 * The exception is an isolated loop, which only has degree-2 nodes,
 * in which case a node is simply chosen as a starting point.
 * The direction of each merged LineString will be that of the majority
 * of the LineStrings from which it was derived.
 * 
 * Any dimension of Geometry is handled.
 * The constituent linework is extracted to form the edges.
 * The edges must be correctly noded; that is, they must only meet
 * at their endpoints. 
 *
 * The LineMerger will still run on incorrectly noded input
 * but will not form polygons from incorrected noded edges.
 *
 */
class LineMerger {

private:

	LineMergeGraph graph;

	std::vector<geom::LineString*> *mergedLineStrings;

	std::vector<EdgeString*> edgeStrings;

	const geom::GeometryFactory *factory;

	void merge();

	void buildEdgeStringsForObviousStartNodes();

	void buildEdgeStringsForIsolatedLoops();

	void buildEdgeStringsForUnprocessedNodes();

	void buildEdgeStringsForNonDegree2Nodes();

	void buildEdgeStringsStartingAt(planargraph::Node *node);

	EdgeString* buildEdgeStringStartingWith(LineMergeDirectedEdge *start);

public:
	LineMerger();
	~LineMerger();

	/**
	 * \brief
	 * Adds a collection of Geometries to be processed.
	 * May be called multiple times.
	 *
	 * Any dimension of Geometry may be added; the constituent
	 * linework will be extracted.
	 */
	void add(std::vector<geom::Geometry*> *geometries);

	/**
	 * \brief
	 * Adds a Geometry to be processed.
	 * May be called multiple times.
	 *
	 * Any dimension of Geometry may be added; the constituent
	 * linework will be extracted.
	 */  
	void add(const geom::Geometry *geometry);

	/**
	 * \brief
	 * Returns the LineStrings built by the merging process.
	 */
	std::vector<geom::LineString*>* getMergedLineStrings();

	void add(const geom::LineString *lineString);

};

} // namespace geos::operation::linemerge
} // namespace geos::operation
} // namespace geos

#endif // GEOS_OP_LINEMERGE_LINEMERGER_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/22 10:13:53  strk
 * opLinemerge.h split
 *
 **********************************************************************/
