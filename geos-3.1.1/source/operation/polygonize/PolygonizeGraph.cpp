/**********************************************************************
 * $Id: PolygonizeGraph.cpp 1820 2006-09-06 16:54:23Z mloskot $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/operation/polygonize/PolygonizeGraph.h>
#include <geos/operation/polygonize/PolygonizeDirectedEdge.h>
#include <geos/operation/polygonize/PolygonizeEdge.h>
#include <geos/operation/polygonize/EdgeRing.h>
#include <geos/planargraph/Node.h>
#include <geos/planargraph/DirectedEdgeStar.h>
#include <geos/planargraph/DirectedEdge.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/LineString.h>

#include <cassert>
#include <vector>

//using namespace std;
using namespace geos::planargraph;
using namespace geos::geom;

namespace geos {
namespace operation { // geos.operation
namespace polygonize { // geos.operation.polygonize

int
PolygonizeGraph::getDegreeNonDeleted(Node *node)
{
	std::vector<DirectedEdge*> &edges=node->getOutEdges()->getEdges();
	int degree=0;
	for(unsigned int i=0; i<edges.size(); ++i) {
		PolygonizeDirectedEdge *de=(PolygonizeDirectedEdge*)edges[i];
		if (!de->isMarked()) ++degree;
	}
	return degree;
}

int
PolygonizeGraph::getDegree(Node *node, long label)
{
	std::vector<DirectedEdge*> &edges=node->getOutEdges()->getEdges();
	int degree=0;
	for(unsigned int i=0; i<edges.size(); ++i)
	{
		PolygonizeDirectedEdge *de=(PolygonizeDirectedEdge*)edges[i];
		if (de->getLabel()==label) ++degree;
	}
	return degree;
}

/**
 * Deletes all edges at a node
 */
void
PolygonizeGraph::deleteAllEdges(Node *node)
{
	std::vector<DirectedEdge*> &edges=node->getOutEdges()->getEdges();
	for(unsigned int i=0; i<edges.size(); ++i) {
		PolygonizeDirectedEdge *de=(PolygonizeDirectedEdge*)edges[i];
		de->setMarked(true);
		PolygonizeDirectedEdge *sym=(PolygonizeDirectedEdge*) de->getSym();
		if (sym!=NULL)
			sym->setMarked(true);
	}
}

/*
 * Create a new polygonization graph.
 */
PolygonizeGraph::PolygonizeGraph(const GeometryFactory *newFactory):
	factory(newFactory)
{
}

/*
 * Destroy a PolygonizeGraph
 */
PolygonizeGraph::~PolygonizeGraph()
{
	unsigned int i;
	for (i=0; i<newEdges.size(); i++)
		delete newEdges[i];
	for (i=0; i<newDirEdges.size(); i++)
		delete newDirEdges[i];
	for (i=0; i<newNodes.size(); i++)
		delete newNodes[i];
	for (i=0; i<newEdgeRings.size(); i++)
		delete newEdgeRings[i];
	for (i=0; i<newCoords.size(); i++) delete newCoords[i];
}

/*
 * Add a LineString forming an edge of the polygon graph.
 * @param line the line to add
 */
void
PolygonizeGraph::addEdge(const LineString *line)
{
	if (line->isEmpty()) return;

	CoordinateSequence *linePts=CoordinateSequence::removeRepeatedPoints(line->getCoordinatesRO());

	/*
	 * This would catch invalid linestrings
	 * (containing duplicated points only)
	 */
	if ( linePts->getSize() < 2 )
	{
		delete linePts;
		return;
	}

	const Coordinate& startPt=linePts->getAt(0);
	const Coordinate& endPt=linePts->getAt(linePts->getSize()-1);
	Node *nStart=getNode(startPt);
	Node *nEnd=getNode(endPt);
	DirectedEdge *de0=new PolygonizeDirectedEdge(nStart, nEnd, linePts->getAt(1), true);
	newDirEdges.push_back(de0);
	DirectedEdge *de1=new PolygonizeDirectedEdge(nEnd, nStart,
			linePts->getAt(linePts->getSize()-2), false);
	newDirEdges.push_back(de1);
	Edge *edge=new PolygonizeEdge(line);
	newEdges.push_back(edge);
	edge->setDirectedEdges(de0, de1);
	add(edge);

	newCoords.push_back(linePts);
}

Node *
PolygonizeGraph::getNode(const Coordinate& pt)
{
	Node *node=findNode(pt);
	if (node==NULL) {
		node=new Node(pt);
		newNodes.push_back(node);
		// ensure node is only added once to graph
		add(node);
	}
	return node;
}

void
PolygonizeGraph::computeNextCWEdges()
{
	std::vector<Node*> *pns=getNodes();
	// set the next pointers for the edges around each node
	for(int i=0;i<(int)pns->size();i++) {
		Node *node=(*pns)[i];
		computeNextCWEdges(node);
	}
	delete pns;
}

/*
 * Convert the maximal edge rings found by the initial graph traversal
 * into the minimal edge rings required by JTS polygon topology rules.
 *
 * @param ringEdges the list of start edges for the edgeRings to convert.
 */
void
PolygonizeGraph::convertMaximalToMinimalEdgeRings(std::vector<PolygonizeDirectedEdge*> *ringEdges)
{
	for(int i=0;i<(int)ringEdges->size();i++)
	{
		PolygonizeDirectedEdge *de=(*ringEdges)[i];
		long label=de->getLabel();
		std::vector<Node*> *intNodes=findIntersectionNodes(de, label);
		if (intNodes==NULL) continue;

		// flip the next pointers on the intersection nodes to
		// create minimal edge rings
		//std::vector<Node*> *pns=getNodes();

		// set the next pointers for the edges around each node
		for(int j=0;j<(int)intNodes->size();j++) {
			Node *node=(*intNodes)[j];
			computeNextCCWEdges(node, label);
		}

		delete intNodes;
	}
}

/*
 * Finds all nodes in a maximal edgering which are self-intersection nodes
 * @param startDE
 * @param label
 * @return the list of intersection nodes found,
 * or <code>NULL</code> if no intersection nodes were found.
 * Ownership of returned object goes to caller.
 */
std::vector<Node*>*
PolygonizeGraph::findIntersectionNodes(PolygonizeDirectedEdge *startDE, long label)
{
	PolygonizeDirectedEdge *de=startDE;
	std::vector<Node*> *intNodes=NULL;
	do {
		Node *node=de->getFromNode();
		if (getDegree(node, label) > 1) {
			if (intNodes==NULL)
				intNodes=new std::vector<Node*>();
			intNodes->push_back(node);
		}
		de=de->getNext();
		assert(de!=NULL); // found NULL DE in ring
		assert(de==startDE || !de->isInRing()); // found DE already in ring
	} while (de!=startDE);
	return intNodes;
}

/**
 * Computes the EdgeRings formed by the edges in this graph.
 * @return a list of the EdgeRing found by the polygonization process.
 */
std::vector<EdgeRing*>*
PolygonizeGraph::getEdgeRings()
{
	// maybe could optimize this, since most of these pointers should
	// be set correctly already
	// by deleteCutEdges()
	computeNextCWEdges();

	// clear labels of all edges in graph
	label(dirEdges, -1);
	std::vector<PolygonizeDirectedEdge*> *maximalRings=findLabeledEdgeRings(dirEdges);
	convertMaximalToMinimalEdgeRings(maximalRings);
	delete maximalRings;

	// find all edgerings
	std::vector<EdgeRing*> *edgeRingList=new std::vector<EdgeRing*>();
	for(unsigned int i=0; i<dirEdges.size(); ++i)
	{
		PolygonizeDirectedEdge *de=(PolygonizeDirectedEdge*)dirEdges[i];
		if (de->isMarked()) continue;
		if (de->isInRing()) continue;
		EdgeRing *er=findEdgeRing(de);
		edgeRingList->push_back(er);
	}
	return edgeRingList;
}

/**
*
* @param dirEdges a List of the DirectedEdges in the graph
* @return a List of DirectedEdges, one for each edge ring found
*/
std::vector<PolygonizeDirectedEdge*>*
PolygonizeGraph::findLabeledEdgeRings(std::vector<DirectedEdge*> &dirEdges)
{
	std::vector<PolygonizeDirectedEdge*> *edgeRingStarts=new std::vector<PolygonizeDirectedEdge*>();
	// label the edge rings formed
	long currLabel=1;
	for(unsigned int i=0; i<dirEdges.size(); ++i)
	{
		PolygonizeDirectedEdge *de=(PolygonizeDirectedEdge*)dirEdges[i];
		if (de->isMarked()) continue;
		if (de->getLabel() >= 0) continue;
		edgeRingStarts->push_back(de);
		std::vector<DirectedEdge*> *edges=findDirEdgesInRing(de);
		label(*edges, currLabel);
		delete edges;
		++currLabel;
	}
	return edgeRingStarts;
}

/*
 * Finds and removes all cut edges from the graph.
 * @return a list of the LineString forming the removed cut edges
 */
std::vector<const LineString*> *
PolygonizeGraph::deleteCutEdges()
{
	computeNextCWEdges();

	// label the current set of edgerings
	delete findLabeledEdgeRings(dirEdges);

	/*
	 * Cut Edges are edges where both dirEdges have the same label.
	 * Delete them, and record them
	 */
	std::vector<const LineString*> *cutLines=new std::vector<const LineString*>();
	for(unsigned int i=0; i<dirEdges.size(); ++i) {
		PolygonizeDirectedEdge *de=(PolygonizeDirectedEdge*)dirEdges[i];
		if (de->isMarked()) continue;
		PolygonizeDirectedEdge *sym=(PolygonizeDirectedEdge*) de->getSym();
		if (de->getLabel()==sym->getLabel()) {
			de->setMarked(true);
			sym->setMarked(true);
			// save the line as a cut edge
			PolygonizeEdge *e=(PolygonizeEdge*) de->getEdge();
			cutLines->push_back(e->getLine());
		}
	}
	return cutLines;
}

void
PolygonizeGraph::label(std::vector<DirectedEdge*> &dirEdges, long label)
{
	for(unsigned int i=0; i<dirEdges.size(); ++i)
	{
		PolygonizeDirectedEdge *de=(PolygonizeDirectedEdge*)dirEdges[i];
		de->setLabel(label);
	}
}

void
PolygonizeGraph::computeNextCWEdges(Node *node)
{
	DirectedEdgeStar *deStar=node->getOutEdges();
	PolygonizeDirectedEdge *startDE=NULL;
	PolygonizeDirectedEdge *prevDE=NULL;

	// the edges are stored in CCW order around the star
	std::vector<DirectedEdge*> &pde=deStar->getEdges();
	for(unsigned int i=0; i<pde.size(); ++i) {
		PolygonizeDirectedEdge *outDE=(PolygonizeDirectedEdge*)pde[i];
		if (outDE->isMarked()) continue;
		if (startDE==NULL)
			startDE=outDE;
		if (prevDE!=NULL) {
			PolygonizeDirectedEdge *sym=(PolygonizeDirectedEdge*) prevDE->getSym();
			sym->setNext(outDE);
		}
		prevDE=outDE;
	}
	if (prevDE!=NULL) {
		PolygonizeDirectedEdge *sym=(PolygonizeDirectedEdge*) prevDE->getSym();
		sym->setNext(startDE);
	}
}

/**
 * Computes the next edge pointers going CCW around the given node, for the
 * given edgering label.
 * This algorithm has the effect of converting maximal edgerings into
 * minimal edgerings
 */
void
PolygonizeGraph::computeNextCCWEdges(Node *node, long label)
{
	DirectedEdgeStar *deStar=node->getOutEdges();
	PolygonizeDirectedEdge *firstOutDE=NULL;
	PolygonizeDirectedEdge *prevInDE=NULL;

	// the edges are stored in CCW order around the star
	std::vector<DirectedEdge*> &edges=deStar->getEdges();

	/*
	 * Must use a SIGNED int here to allow for beak condition
	 * to be true.
	 */
	for(int i=edges.size()-1; i>=0; --i)
	{
		PolygonizeDirectedEdge *de=(PolygonizeDirectedEdge*)edges[i];
		PolygonizeDirectedEdge *sym=(PolygonizeDirectedEdge*) de->getSym();
		PolygonizeDirectedEdge *outDE=NULL;
		if (de->getLabel()==label) outDE=de;
		PolygonizeDirectedEdge *inDE=NULL;
		if (sym->getLabel()==label) inDE= sym;
		if (outDE==NULL && inDE==NULL) continue; // this edge is not in edgering
		if (inDE != NULL) {
			prevInDE=inDE;
		}
		if (outDE != NULL) {
			if (prevInDE != NULL) {
				prevInDE->setNext(outDE);
				prevInDE=NULL;
			}
			if (firstOutDE==NULL)
				firstOutDE=outDE;
		}
	}
	if (prevInDE != NULL) {
		assert(firstOutDE != NULL);
		prevInDE->setNext(firstOutDE);
	}
}

/*
 * Traverse a ring of DirectedEdges, accumulating them into a list.
 * This assumes that all dangling directed edges have been removed
 * from the graph, so that there is always a next dirEdge.
 *
 * @param startDE the DirectedEdge to start traversing at
 * @return a List of DirectedEdges that form a ring
 */
std::vector<DirectedEdge*>*
PolygonizeGraph::findDirEdgesInRing(PolygonizeDirectedEdge *startDE)
{
	PolygonizeDirectedEdge *de=startDE;
	std::vector<DirectedEdge*> *edges=new std::vector<DirectedEdge*>();
	do {
		edges->push_back(de);
		de=de->getNext();
		assert(de != NULL); // found NULL DE in ring
		assert(de==startDE || !de->isInRing()); // found DE already in ring
	} while (de != startDE);
	return edges;
}

EdgeRing *
PolygonizeGraph::findEdgeRing(PolygonizeDirectedEdge *startDE)
{
	PolygonizeDirectedEdge *de=startDE;
	EdgeRing *er=new EdgeRing(factory);
	// Now, when will we delete those EdgeRings ?
	newEdgeRings.push_back(er);
	do {
		er->add(de);
		de->setRing(er);
		de=de->getNext();
		assert(de != NULL); // found NULL DE in ring
		assert(de==startDE || ! de->isInRing()); // found DE already in ring
	} while (de != startDE);
	return er;
}

/**
 * Marks all edges from the graph which are "dangles".
 * Dangles are which are incident on a node with degree 1.
 * This process is recursive, since removing a dangling edge
 * may result in another edge becoming a dangle.
 * In order to handle large recursion depths efficiently,
 * an explicit recursion stack is used
 *
 * @return a List containing the LineStrings that formed dangles
 */
std::vector<const LineString*>*
PolygonizeGraph::deleteDangles()
{
	std::vector<Node*> *nodesToRemove=findNodesOfDegree(1);
	std::vector<const LineString*> *dangleLines=new std::vector<const LineString*>();
	std::vector<Node*> nodeStack;
	for(int i=0;i<(int)nodesToRemove->size();i++) {
		nodeStack.push_back((*nodesToRemove)[i]);
	}
	delete nodesToRemove;
	while (!nodeStack.empty()) {
		Node *node=nodeStack[nodeStack.size()-1];
		nodeStack.pop_back();
		deleteAllEdges(node);
		std::vector<DirectedEdge*> &nodeOutEdges=node->getOutEdges()->getEdges();
		for(unsigned int j=0; j<nodeOutEdges.size(); ++j)
		{
			PolygonizeDirectedEdge *de=(PolygonizeDirectedEdge*)nodeOutEdges[j];
			// delete this edge and its sym
			de->setMarked(true);
			PolygonizeDirectedEdge *sym=(PolygonizeDirectedEdge*) de->getSym();
			if (sym != NULL)
				sym->setMarked(true);
			// save the line as a dangle
			PolygonizeEdge *e=(PolygonizeEdge*) de->getEdge();
			dangleLines->push_back(e->getLine());
			Node *toNode=de->getToNode();
			// add the toNode to the list to be processed, if it is now a dangle
			if (getDegreeNonDeleted(toNode)==1)
				nodeStack.push_back(toNode);
		}
	}
	return dangleLines;
}

} // namespace geos.operation.polygonize
} // namespace geos.operation
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.18  2006/03/22 11:19:06  strk
 * opPolygonize.h headers split.
 *
 * Revision 1.17  2006/03/21 21:42:54  strk
 * planargraph.h header split, planargraph:: classes renamed to match JTS symbols
 *
 * Revision 1.16  2006/03/06 19:40:47  strk
 * geos::util namespace. New GeometryCollection::iterator interface, many cleanups.
 *
 * Revision 1.15  2006/03/03 10:46:22  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.14  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.13  2006/01/31 19:07:34  strk
 * - Renamed DefaultCoordinateSequence to CoordinateArraySequence.
 * - Moved GetNumGeometries() and GetGeometryN() interfaces
 *   from GeometryCollection to Geometry class.
 * - Added getAt(int pos, Coordinate &to) funtion to CoordinateSequence class.
 * - Reworked automake scripts to produce a static lib for each subdir and
 *   then link all subsystem's libs togheter
 * - Moved C-API in it's own top-level dir capi/
 * - Moved source/bigtest and source/test to tests/bigtest and test/xmltester
 * - Fixed PointLocator handling of LinearRings
 * - Changed CoordinateArrayFilter to reduce memory copies
 * - Changed UniqueCoordinateArrayFilter to reduce memory copies
 * - Added CGAlgorithms::isPointInRing() version working with
 *   Coordinate::ConstVect type (faster!)
 * - Ported JTS-1.7 version of ConvexHull with big attention to
 *   memory usage optimizations.
 * - Improved XMLTester output and user interface
 * - geos::geom::util namespace used for geom/util stuff
 * - Improved memory use in geos::geom::util::PolygonExtractor
 * - New ShortCircuitedGeometryVisitor class
 * - New operation/predicate package
 *
 * Revision 1.12  2005/12/09 11:36:38  strk
 * Small leak plugged in CAPI::GEOSHasZ() and in
 * invalid input to PolygonizeGraph (again)
 *
 * Revision 1.11  2005/12/09 10:03:46  strk
 * Fixed a bug making PolygonizeGraph choking on invalid LineStrings.
 * Minor optimizations in Polygonizer loops.
 *
 * Revision 1.10  2005/11/21 16:03:20  strk
 *
 * Coordinate interface change:
 *         Removed setCoordinate call, use assignment operator
 *         instead. Provided a compile-time switch to
 *         make copy ctor and assignment operators non-inline
 *         to allow for more accurate profiling.
 *
 * Coordinate copies removal:
 *         NodeFactory::createNode() takes now a Coordinate reference
 *         rather then real value. This brings coordinate copies
 *         in the testLeaksBig.xml test from 654818 to 645991
 *         (tested in 2.1 branch). In the head branch Coordinate
 *         copies are 222198.
 *         Removed useless coordinate copies in ConvexHull
 *         operations
 *
 * STL containers heap allocations reduction:
 *         Converted many containers element from
 *         pointers to real objects.
 *         Made some use of .reserve() or size
 *         initialization when final container size is known
 *         in advance.
 *
 * Stateless classes allocations reduction:
 *         Provided ::instance() function for
 *         NodeFactories, to avoid allocating
 *         more then one (they are all
 *         stateless).
 *
 * HCoordinate improvements:
 *         Changed HCoordinate constructor by HCoordinates
 *         take reference rather then real objects.
 *         Changed HCoordinate::intersection to avoid
 *         a new allocation but rather return into a provided
 *         storage. LineIntersector changed to reflect
 *         the above change.
 *
 * Revision 1.9  2005/11/15 12:14:05  strk
 * Reduced heap allocations, made use of references when appropriate,
 * small optimizations here and there.
 *
 * Revision 1.8  2004/12/14 10:35:44  strk
 * Comments cleanup. PolygonizeGraph keeps track of generated CoordinateSequence
 * for delayed destruction.
 *
 * Revision 1.7  2004/12/08 13:54:44  strk
 * gcc warnings checked and fixed, general cleanups.
 *
 * Revision 1.6  2004/10/26 16:09:21  strk
 * Some more intentation and envelope equality check fix.
 *
 * Revision 1.5  2004/10/19 19:51:14  strk
 * Fixed many leaks and bugs in Polygonizer.
 * Output still bogus.
 *
 * Revision 1.4  2004/10/13 10:03:02  strk
 * Added missing linemerge and polygonize operation.
 * Bug fixes and leaks removal from the newly added modules and
 * planargraph (used by them).
 * Some comments and indentation changes.
 *
 * Revision 1.3  2004/07/08 19:34:50  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added CoordinateArraySequenceFactory::instance() function.
 *
 * Revision 1.2  2004/07/02 13:28:29  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.1  2004/04/08 04:53:56  ybychkov
 * "operation/polygonize" ported from JTS 1.4
 *
 *
 **********************************************************************/
