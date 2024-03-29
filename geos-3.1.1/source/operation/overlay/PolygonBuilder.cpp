/**********************************************************************
 * $Id: PolygonBuilder.cpp 2532 2009-05-27 19:11:05Z pramsey $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: operation/overlay/PolygonBuilder.java rev. 1.18 (JTS-1.7)
 *
 **********************************************************************/

#include <geos/operation/overlay/PolygonBuilder.h>
#include <geos/operation/overlay/OverlayOp.h>
#include <geos/operation/overlay/MaximalEdgeRing.h>
#include <geos/operation/overlay/MinimalEdgeRing.h>
#include <geos/geomgraph/Node.h>
#include <geos/geomgraph/NodeMap.h>
#include <geos/geomgraph/DirectedEdgeStar.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/Polygon.h>
#include <geos/algorithm/CGAlgorithms.h>
#include <geos/util/TopologyException.h>
#include <geos/util/GEOSException.h>

#include <vector>
#include <cassert>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

using namespace std;
using namespace geos::geomgraph;
using namespace geos::algorithm;
using namespace geos::geom;


namespace geos {
namespace operation { // geos.operation
namespace overlay { // geos.operation.overlay

PolygonBuilder::PolygonBuilder(const GeometryFactory *newGeometryFactory)
	:
	geometryFactory(newGeometryFactory)
{
}

PolygonBuilder::~PolygonBuilder()
{
	for(size_t i=0, n=shellList.size(); i<n; ++i)
	{
		delete shellList[i];
	}
}

/*public*/
void
PolygonBuilder::add(PlanarGraph *graph)
	//throw(TopologyException *)
{
	const vector<EdgeEnd*>* eeptr=graph->getEdgeEnds();
	assert(eeptr);
	const vector<EdgeEnd*>& ee = *eeptr;

	size_t eeSize=ee.size();

#if GEOS_DEBUG 
	cerr << __FUNCTION__ << ": PlanarGraph has " << eeSize << " EdgeEnds" << endl;
#endif

	vector<DirectedEdge*> dirEdges(eeSize);
	for(size_t i=0; i<eeSize; ++i)
	{
		assert(dynamic_cast<DirectedEdge*>(ee[i]));
		DirectedEdge* de = static_cast<DirectedEdge*>(ee[i]);
		dirEdges[i]=de;
	}

	NodeMap::container &nodeMap=graph->getNodeMap()->nodeMap;
	vector<Node*> nodes;
	nodes.reserve(nodeMap.size());
	for ( NodeMap::iterator it=nodeMap.begin(), itEnd=nodeMap.end();
		it != itEnd; ++it )
	{
		Node *node=it->second;
		nodes.push_back(node);
	}

	add(&dirEdges, &nodes); // might throw a TopologyException *
}

/*public*/
void
PolygonBuilder::add(const vector<DirectedEdge*> *dirEdges,
		const vector<Node*> *nodes)
		//throw(TopologyException *)
{
	//	PlanarGraph::linkResultDirectedEdgesS(nodes);

	typedef vector<Node*>::const_iterator NodeIt;
	
	for ( NodeIt nodeit = nodes->begin(), nodeEnd = nodes->end();
			nodeit != nodeEnd; ++nodeit)
	{
		Node *node=*nodeit;
		DirectedEdgeStar* des = dynamic_cast<DirectedEdgeStar*>(node->getEdges());
		assert(des);

		// This might throw a TopologyException
		des->linkResultDirectedEdges();
	}

#if GEOS_DEBUG > 1
	cerr << "CREATE TABLE diredges (g geometry);" << endl;
	for (size_t i=0, n=dirEdges->size(); i<n; i++)
	{
		DirectedEdge* de = (*dirEdges)[i];
		Edge* e = de->getEdge();
		const CoordinateSequence* pts = e->getCoordinates();
		cerr << "INSERT INTO diredges VALUES ('LINESTRING"
		     << pts->toString() << "');" << endl;
	}
#endif
	vector<MaximalEdgeRing*>* maxEdgeRings=buildMaximalEdgeRings(dirEdges);
#if GEOS_DEBUG > 1
	cerr << "CREATE TABLE maxedgerings (g geometry);" << endl;
	for (size_t i=0, n=maxEdgeRings->size(); i<n; i++)
	{
		EdgeRing* er = (*maxEdgeRings)[i];
		Polygon* poly = er->toPolygon(geometryFactory);
		cerr << "INSERT INTO maxedgerings VALUES ('"
		     << poly->toString() << "');" << endl;
		delete poly;
	}
#endif

	vector<EdgeRing*> freeHoleList;
	vector<MaximalEdgeRing*> *edgeRings=
		buildMinimalEdgeRings(maxEdgeRings,&shellList,&freeHoleList);

	sortShellsAndHoles(edgeRings,&shellList,&freeHoleList);
	placeFreeHoles(shellList, freeHoleList);
	delete maxEdgeRings;
	delete edgeRings;
	//Assert: every hole on freeHoleList has a shell assigned to it
}

/*public*/
vector<Geometry*>*
PolygonBuilder::getPolygons()
{
	vector<Geometry*> *resultPolyList=computePolygons(shellList);
	return resultPolyList;
}


/*private*/
vector<MaximalEdgeRing*> *
PolygonBuilder::buildMaximalEdgeRings(const vector<DirectedEdge*> *dirEdges)
{
#if GEOS_DEBUG
	cerr<<"PolygonBuilder::buildMaximalEdgeRings got "<<dirEdges->size()<<" dirEdges"<<endl;
#endif
	vector<MaximalEdgeRing*> *maxEdgeRings=new vector<MaximalEdgeRing*>();
	for(size_t i=0, n=dirEdges->size(); i<n; i++)
	{
		DirectedEdge *de=(*dirEdges)[i];
#if GEOS_DEBUG
	cerr << "  dirEdge " << i << endl
	     << de->printEdge() << endl
	     << " inResult:" << de->isInResult() << endl
	     << " isArea:" << de->getLabel()->isArea() << endl;
#endif
		if (de->isInResult() && de->getLabel()->isArea()) {
			// if this edge has not yet been processed
			if (de->getEdgeRing()==NULL) {
				MaximalEdgeRing *er;
				try
				{
					er=new MaximalEdgeRing(de,geometryFactory);
				}
				catch( util::GEOSException& e )
				{
					for( size_t i=0, n=maxEdgeRings->size(); i<n; i++)
						delete (*maxEdgeRings)[i];
					delete maxEdgeRings;
					throw;
				}
				maxEdgeRings->push_back(er);
				er->setInResult();
				//System.out.println("max node degree=" + er.getMaxDegree());
			}
		}
	}
#if GEOS_DEBUG
	cerr<<"  returning "<<maxEdgeRings->size()<<" maxEdgeRings"<<endl;
#endif
	return maxEdgeRings;
}

/*private*/
vector<MaximalEdgeRing*> *
PolygonBuilder::buildMinimalEdgeRings(vector<MaximalEdgeRing*> *maxEdgeRings,
	vector<EdgeRing*> *newShellList, vector<EdgeRing*> *freeHoleList)
{
	vector<MaximalEdgeRing*> *edgeRings=new vector<MaximalEdgeRing*>();
	for(size_t i=0, n=maxEdgeRings->size(); i<n; ++i)
	{
		MaximalEdgeRing *er=(*maxEdgeRings)[i];
#if GEOS_DEBUG
	cerr<<"buildMinimalEdgeRings: maxEdgeRing "<<i<<" has "<<er->getMaxNodeDegree()<<" maxNodeDegree"<<endl;
#endif
		if (er->getMaxNodeDegree()>2)
		{
			er->linkDirectedEdgesForMinimalEdgeRings();
			vector<MinimalEdgeRing*> *minEdgeRings=er->buildMinimalRings();
			// at this point we can go ahead and attempt to place
			// holes, if this EdgeRing is a polygon
			EdgeRing *shell=findShell(minEdgeRings);
			if(shell != NULL)
			{
				placePolygonHoles(shell, minEdgeRings);
				newShellList->push_back(shell);
			}
			else
			{
				freeHoleList->insert(
						freeHoleList->end(),
						minEdgeRings->begin(),
						minEdgeRings->end() );
			}
			delete er;
			delete minEdgeRings;
		}
		else
		{
			edgeRings->push_back(er);
		}
	}
	return edgeRings;
}

/*private*/
EdgeRing*
PolygonBuilder::findShell(vector<MinimalEdgeRing*> *minEdgeRings)
{
#ifndef NDEBUG
	int shellCount=0;
#endif
	EdgeRing *shell=NULL;

#if GEOS_DEBUG
	cerr<<"PolygonBuilder::findShell got "<<minEdgeRings->size()<<" minEdgeRings"<<endl;
#endif

	for (size_t i=0, n=minEdgeRings->size(); i<n; ++i)
	{
		EdgeRing *er=(*minEdgeRings)[i];
		if ( ! er->isHole() )
		{
			shell=er;
#ifndef NDEBUG
			++shellCount;
#endif
		}
	}
	assert(shellCount <= 1); // found two shells in MinimalEdgeRing list
	return shell;
}

/*private*/
void
PolygonBuilder::placePolygonHoles(EdgeRing *shell,
	vector<MinimalEdgeRing*> *minEdgeRings)
{
	for (size_t i=0, n=minEdgeRings->size(); i<n; ++i)
	{
		MinimalEdgeRing *er=(*minEdgeRings)[i];
		if ( er->isHole() )
		{
			er->setShell(shell);
		}
	}
}

/*private*/
void
PolygonBuilder::sortShellsAndHoles(vector<MaximalEdgeRing*> *edgeRings,
	vector<EdgeRing*> *newShellList, vector<EdgeRing*> *freeHoleList)
{
	for(size_t i=0, n=edgeRings->size(); i<n; i++)
	{
		EdgeRing *er=(*edgeRings)[i];
		//er->setInResult();
		if (er->isHole() )
		{
			freeHoleList->push_back(er);
		}
		else
		{
			newShellList->push_back(er);
		}
	}
}

/*private*/
void
PolygonBuilder::placeFreeHoles(std::vector<EdgeRing*>& newShellList,
	std::vector<EdgeRing*>& freeHoleList)
{
	for(std::vector<EdgeRing*>::iterator
			it=freeHoleList.begin(), itEnd=freeHoleList.end();
			it != itEnd;
			++it)
	{
		EdgeRing *hole=*it;
		// only place this hole if it doesn't yet have a shell
		if (hole->getShell()==NULL) {
			EdgeRing *shell=findEdgeRingContaining(hole, newShellList);
			if ( shell == NULL )
			{
#if GEOS_DEBUG
				Geometry* geom;
				std::cerr << "CREATE TABLE shells (g geometry);" << std::endl;
				std::cerr << "CREATE TABLE hole (g geometry);" << std::endl;
				for (std::vector<EdgeRing*>::iterator rIt=newShellList.begin(),
					rEnd=newShellList.end(); rIt!=rEnd; rIt++)
				{
					geom = (*rIt)->toPolygon(geometryFactory);
					std::cerr << "INSERT INTO shells VALUES ('"
					          << *geom
					          << "');" << std::endl;
					delete geom;
				}
				geom = hole->toPolygon(geometryFactory);
				std::cerr << "INSERT INTO hole VALUES ('"
				          << *geom
				          << "');" << std::endl;
				delete geom;
#endif
				//assert(shell!=NULL); // unable to assign hole to a shell
				throw util::TopologyException("unable to assign hole to a shell");
			}

			hole->setShell(shell);
		}
	}
}

/*private*/
EdgeRing*
PolygonBuilder::findEdgeRingContaining(EdgeRing *testEr,
	vector<EdgeRing*>& newShellList)
{
	LinearRing *testRing=testEr->getLinearRing();
	const Envelope *testEnv=testRing->getEnvelopeInternal();
	const Coordinate& testPt=testRing->getCoordinateN(0);
	EdgeRing *minShell=NULL;
	const Envelope *minEnv=NULL;
	for(size_t i=0, n=newShellList.size(); i<n; i++)
	{
		LinearRing *lr=NULL;
		EdgeRing *tryShell=newShellList[i];
		LinearRing *tryRing=tryShell->getLinearRing();
		const Envelope *tryEnv=tryRing->getEnvelopeInternal();
		if (minShell!=NULL) {
			lr=minShell->getLinearRing();
			minEnv=lr->getEnvelopeInternal();
		}
		bool isContained=false;
		const CoordinateSequence *rcl = tryRing->getCoordinatesRO();
		if (tryEnv->contains(testEnv)
			&& CGAlgorithms::isPointInRing(testPt,rcl))
				isContained=true;
		// check if this new containing ring is smaller than
		// the current minimum ring
		if (isContained) {
			if (minShell==NULL
				|| minEnv->contains(tryEnv)) {
					minShell=tryShell;
			}
		}
	}
	return minShell;
}

/*private*/
vector<Geometry*>*
PolygonBuilder::computePolygons(vector<EdgeRing*>& newShellList)
{
#if GEOS_DEBUG
	cerr<<"PolygonBuilder::computePolygons: got "<<newShellList.size()<<" shells"<<endl;
#endif
	vector<Geometry*> *resultPolyList=new vector<Geometry*>();

	// add Polygons for all shells
	for(size_t i=0, n=newShellList.size(); i<n; i++)
	{
		EdgeRing *er=newShellList[i];
		Polygon *poly=er->toPolygon(geometryFactory);
		resultPolyList->push_back(poly);
	}
	return resultPolyList;
}

/*public*/
bool
PolygonBuilder::containsPoint(const Coordinate& p)
{
	for(size_t i=0, size=shellList.size(); i<size; ++i)
	{
		EdgeRing *er=shellList[i];
		if ( er->containsPoint(p) )
		{
			return true;
		}
	}
	return false;
}

} // namespace geos.operation.overlay
} // namespace geos.operation
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.43  2006/06/14 19:17:29  strk
 * Fixed bug in findShell() needlessly erasing vector elements
 *
 * Revision 1.42  2006/06/14 13:59:24  strk
 * Fixed bug in PolygonBuilder::placePolygonHoles, performance improved as a side effect.
 *
 * Revision 1.41  2006/06/13 23:26:46  strk
 * cleanups
 *
 * Revision 1.40  2006/06/12 11:29:24  strk
 * unsigned int => size_t
 *
 * Revision 1.39  2006/03/20 16:57:44  strk
 * spatialindex.h and opValid.h headers split
 *
 * Revision 1.38  2006/03/20 13:20:29  strk
 * Changed assertion to TopologyException for the "orphaned" hole case
 * in order to allow for reduced precision ops to catch the case.
 *
 * Revision 1.37  2006/03/20 12:33:45  strk
 * Simplified some privat methods to use refs instead of pointers, added
 * debugging section for failiures of holes/shells associations
 *
 * Revision 1.36  2006/03/17 13:24:59  strk
 * opOverlay.h header splitted. Reduced header inclusions in operation/overlay implementation files. ElevationMatrixFilter code moved from own file to ElevationMatrix.cpp (ideally a class-private).
 *
 * Revision 1.35  2006/03/15 11:44:04  strk
 * debug blocks, dumping SQL when GEOS_DEBUG > 1
 *
 **********************************************************************/

