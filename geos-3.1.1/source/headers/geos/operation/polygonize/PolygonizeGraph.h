/**********************************************************************
 * $Id: PolygonizeGraph.h 1820 2006-09-06 16:54:23Z mloskot $
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


#ifndef GEOS_OP_POLYGONIZE_POLYGONIZEGRAPH_H
#define GEOS_OP_POLYGONIZE_POLYGONIZEGRAPH_H

#include <geos/planargraph/PlanarGraph.h> // for inheritance

#include <vector>

// Forward declarations
namespace geos {
	namespace geom { 
		class LineString;
		class GeometryFactory;
		class Coordinate;
		class CoordinateSequence;
	}
	namespace planargraph { 
		class Node;
		class Edge;
		class DirectedEdge;
	}
	namespace operation { 
		namespace polygonize {
			class EdgeRing;
			class PolygonizeDirectedEdge;
		}
	}
}

namespace geos {
namespace operation { // geos::operation
namespace polygonize { // geos::operation::polygonize


/** \brief
 * Represents a planar graph of edges that can be used to compute a
 * polygonization, and implements the algorithms to compute the
 * EdgeRings formed by the graph.
 * 
 * The marked flag on DirectedEdge is used to indicate that a directed edge
 * has be logically deleted from the graph.
 *
 */
class PolygonizeGraph: public planargraph::PlanarGraph {

public:

	/**
	 * \brief
	 * Deletes all edges at a node
	 */
	static void deleteAllEdges(planargraph::Node *node);

	/**
	 * \brief
	 * Create a new polygonization graph.
	 */
	PolygonizeGraph(const geom::GeometryFactory *newFactory);

	/**
	 * \brief
	 * Destroy a polygonization graph.
	 */
	~PolygonizeGraph();

	/**
	 * \brief
	 * Add a LineString forming an edge of the polygon graph.
	 * @param line the line to add
	 */
	void addEdge(const geom::LineString *line);

	/**
	 * \brief
	 * Computes the EdgeRings formed by the edges in this graph.
	 *
	 * @return a list of the EdgeRing found by the
	 * 	polygonization process.
	 */
	std::vector<EdgeRing*>* getEdgeRings();

	/**
	 * \brief
	 * Finds and removes all cut edges from the graph.
	 *
	 * @return a list of the LineString forming the removed cut edges
	 */
	std::vector<const geom::LineString*>* deleteCutEdges();

	/** \brief
	 * Marks all edges from the graph which are "dangles".
	 *
	 * Dangles are which are incident on a node with degree 1.
	 * This process is recursive, since removing a dangling edge
	 * may result in another edge becoming a dangle.
	 * In order to handle large recursion depths efficiently,
	 * an explicit recursion stack is used
	 *
	 * @return a List containing the LineStrings that formed dangles
	 */
	std::vector<const geom::LineString*>* deleteDangles();

private:

	static int getDegreeNonDeleted(planargraph::Node *node);

	static int getDegree(planargraph::Node *node, long label);

	const geom::GeometryFactory *factory;

	planargraph::Node* getNode(const geom::Coordinate& pt);

	void computeNextCWEdges();

	/**
	 * \brief
	 * Convert the maximal edge rings found by the initial graph traversal
	 * into the minimal edge rings required by JTS polygon topology rules.
	 *
	 * @param ringEdges
	 * 	the list of start edges for the edgeRings to convert.
	 */
	void convertMaximalToMinimalEdgeRings(
			std::vector<PolygonizeDirectedEdge*> *ringEdges);

	/**
	 * \brief
	 * Finds all nodes in a maximal edgering which are self-intersection
	 * nodes
	 *
	 * @param startDE
	 * @param label
	 * @return the list of intersection nodes found,
	 * or <code>null</code> if no intersection nodes were found.
	 * Ownership of returned vector goes to caller.
	 */
	static std::vector<planargraph::Node*>* findIntersectionNodes(
			PolygonizeDirectedEdge *startDE,
			long label);

	/**
	 * @param dirEdges a List of the DirectedEdges in the graph
	 * @return a List of DirectedEdges, one for each edge ring found
	 */
	static std::vector<PolygonizeDirectedEdge*>* findLabeledEdgeRings(
			std::vector<planargraph::DirectedEdge*> &dirEdges);

	static void label(std::vector<planargraph::DirectedEdge*> &dirEdges, long label);

	static void computeNextCWEdges(planargraph::Node *node);

	/**
	 * \brief
	 * Computes the next edge pointers going CCW around the given node,
	 * for the given edgering label.
	 * This algorithm has the effect of converting maximal edgerings
	 * into minimal edgerings
	 */
	static void computeNextCCWEdges(planargraph::Node *node, long label);

	/**
	 * \brief
	 * Traverse a ring of DirectedEdges, accumulating them into a list.
	 * This assumes that all dangling directed edges have been removed
	 * from the graph, so that there is always a next dirEdge.
	 *
	 * @param startDE the DirectedEdge to start traversing at
	 * @return a List of DirectedEdges that form a ring
	 */
	static std::vector<planargraph::DirectedEdge*>* findDirEdgesInRing(
			PolygonizeDirectedEdge *startDE);

	EdgeRing* findEdgeRing(PolygonizeDirectedEdge *startDE);

	/* Tese are for memory management */
	std::vector<planargraph::Edge *>newEdges;
	std::vector<planargraph::DirectedEdge *>newDirEdges;
	std::vector<planargraph::Node *>newNodes;
	std::vector<EdgeRing *>newEdgeRings;
	std::vector<geom::CoordinateSequence *>newCoords;
};

} // namespace geos::operation::polygonize
} // namespace geos::operation
} // namespace geos

#endif // GEOS_OP_POLYGONIZE_POLYGONIZEGRAPH_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/22 11:19:06  strk
 * opPolygonize.h headers split.
 *
 **********************************************************************/
