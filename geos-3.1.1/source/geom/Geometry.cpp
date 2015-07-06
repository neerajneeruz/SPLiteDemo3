/**********************************************************************
 * $Id: Geometry.cpp 2252 2009-01-21 22:18:37Z pramsey $
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
 **********************************************************************
 *
 * Last port: geom/Geometry.java rev. 1.100
 *
 **********************************************************************/

#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/GeometryComponentFilter.h>
#include <geos/geom/GeometryFilter.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/Point.h>
#include <geos/geom/MultiPoint.h>
#include <geos/geom/LineString.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/MultiLineString.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/geom/IntersectionMatrix.h>
#include <geos/util/IllegalArgumentException.h>
#include <geos/algorithm/CentroidPoint.h>
#include <geos/algorithm/CentroidLine.h>
#include <geos/algorithm/CentroidArea.h>
#include <geos/algorithm/InteriorPointPoint.h>
#include <geos/algorithm/InteriorPointLine.h>
#include <geos/algorithm/InteriorPointArea.h>
#include <geos/algorithm/ConvexHull.h>
#include <geos/operation/predicate/RectangleContains.h>
#include <geos/operation/predicate/RectangleIntersects.h>
#include <geos/operation/relate/RelateOp.h>
#include <geos/operation/valid/IsValidOp.h>
#include <geos/operation/overlay/OverlayOp.h>
#include <geos/operation/buffer/BufferOp.h>
#include <geos/operation/distance/DistanceOp.h>
#include <geos/io/WKBWriter.h>
#include <geos/io/WKTWriter.h>
#include <geos/version.h>

#include <algorithm>
#include <string>
#include <typeinfo>
#include <vector>
#include <cassert>
#include <memory>

#define SHORTCIRCUIT_PREDICATES 1

#ifndef GEOS_INLINE
# include <geos/geom/Geometry.inl>
#endif

using namespace std;
using namespace geos::algorithm;
using namespace geos::operation::valid;
using namespace geos::operation::relate;
using namespace geos::operation::buffer;
using namespace geos::operation::overlay;
using namespace geos::operation::distance;
using namespace geos::operation;

namespace geos {
namespace geom { // geos::geom


/*
 * Return current GEOS version 
 */
string
geosversion()
{
	return GEOS_VERSION;
}

/*
 * Return the version of JTS this GEOS
 * release has been ported from.
 */
string
jtsport()
{
	return GEOS_JTS_PORT;
}

GeometryComponentFilter Geometry::geometryChangedFilter;

// REMOVE THIS, use GeometryFactory::getDefaultInstance() directly
const GeometryFactory* Geometry::INTERNAL_GEOMETRY_FACTORY=GeometryFactory::getDefaultInstance();

Geometry::Geometry(const GeometryFactory *newFactory)
	:
	envelope(NULL),
	factory(newFactory),
	userData(NULL)
{
	if ( factory == NULL ) {
		factory = INTERNAL_GEOMETRY_FACTORY;
	} 
	SRID=factory->getSRID();
}

Geometry::Geometry(const Geometry &geom)
	:
	SRID(geom.getSRID()),
	factory(geom.factory),
	userData(NULL)
{
	if ( geom.envelope.get() )
	{
		envelope.reset(new Envelope(*(geom.envelope)));
	}
	//factory=geom.factory; 
	//envelope(new Envelope(*(geom.envelope.get())));
	//SRID=geom.getSRID();
	//userData=NULL;
}

bool
Geometry::hasNonEmptyElements(const vector<Geometry *>* geometries) 
{
	for (size_t i=0; i<geometries->size(); i++) {
		if (!(*geometries)[i]->isEmpty()) {
			return true;
		}
	}
	return false;
}

bool
Geometry::hasNullElements(const CoordinateSequence* list) 
{
	size_t npts=list->getSize();
	for (size_t i=0; i<npts; ++i) {
		if (list->getAt(i)==Coordinate::getNull()) {
			return true;
		}
	}
	return false;
}

bool
Geometry::hasNullElements(const vector<Geometry *>* lrs) 
{
	size_t n=lrs->size();
	for (size_t i=0; i<n; ++i) {
		if ((*lrs)[i]==NULL) {
			return true;
		}
	}
	return false;
}
	
/**
 * Tests whether the distance from this <code>Geometry</code>
 * to another is less than or equal to a specified value.
 *
 * @param geom the Geometry to check the distance to
 * @param cDistance the distance value to compare
 * @return <code>true</code> if the geometries are less than
 *  <code>distance</code> apart.
 */
bool
Geometry::isWithinDistance(const Geometry *geom,double cDistance)
{
	const Envelope *env0=getEnvelopeInternal();
	const Envelope *env1=geom->getEnvelopeInternal();
	double envDist=env0->distance(env1);
	//delete env0;
	//delete env1;
	if (envDist>cDistance)
	{
		return false;
	}
	// NOTE: this could be implemented more efficiently
	double geomDist=distance(geom);
	if (geomDist>cDistance)
	{
		return false;
	}
	return true;
}

/*public*/
Point*
Geometry::getCentroid() const
{
	Coordinate centPt;
	if ( ! getCentroid(centPt) ) return NULL;

	// We don't use createPointFromInternalCoord here
	// because ::getCentroid() takes care about rounding
	Point *pt=getFactory()->createPoint(centPt);
	return pt;
}

/*public*/
bool
Geometry::getCentroid(Coordinate& ret) const
{
	if ( isEmpty() ) { return false; }

	Coordinate c;

	int dim=getDimension();
	if(dim==0) {
		CentroidPoint cent; 
		cent.add(this);
		if ( ! cent.getCentroid(c) )
				return false;
	} else if (dim==1) {
		CentroidLine cent;
		cent.add(this);
		if ( ! cent.getCentroid(c) ) 
			return false;
	} else {
		CentroidArea cent;
		cent.add(this);
		if ( ! cent.getCentroid(c) )
			return false;
	}

	getPrecisionModel()->makePrecise(c);
	ret=c;

	return true;
}

Point*
Geometry::getInteriorPoint() const
{
	Coordinate interiorPt;
	int dim=getDimension();
	if (dim==0) {
		InteriorPointPoint intPt(this);
		if ( ! intPt.getInteriorPoint(interiorPt) ) return NULL;
	} else if (dim==1) {
		InteriorPointLine intPt(this);
		if ( ! intPt.getInteriorPoint(interiorPt) ) return NULL;
	} else {
		InteriorPointArea intPt(this);
		if ( ! intPt.getInteriorPoint(interiorPt) ) return NULL;
	}
	Point *p=getFactory()->createPointFromInternalCoord(&interiorPt, this);
	return p;
}

/**
 * Notifies this Geometry that its Coordinates have been changed by an external
 * party (using a CoordinateFilter, for example). The Geometry will flush
 * and/or update any information it has cached (such as its {@link Envelope} ).
 */
void
Geometry::geometryChanged()
{
	apply_rw(&geometryChangedFilter);
}

/**
 * Notifies this Geometry that its Coordinates have been changed by an external
 * party. When geometryChanged is called, this method will be called for
 * this Geometry and its component Geometries.
 * @see apply(GeometryComponentFilter *)
 */
void
Geometry::geometryChangedAction()
{
	//delete envelope;
	envelope.reset(NULL);
}

bool
Geometry::isValid() const
{
	return IsValidOp(this).isValid();
}

Geometry*
Geometry::getEnvelope() const
{
	return getFactory()->toGeometry(getEnvelopeInternal());
}

const Envelope *
Geometry::getEnvelopeInternal() const
{
	if (!envelope.get()) {
		envelope = computeEnvelopeInternal();
	}
	return envelope.get();
}

bool
Geometry::disjoint(const Geometry *g) const
{
#ifdef SHORTCIRCUIT_PREDICATES
	// short-circuit test
	if (! getEnvelopeInternal()->intersects(g->getEnvelopeInternal()))
		return true;
#endif
	IntersectionMatrix *im=relate(g);
	bool res=im->isDisjoint();
	delete im;
	return res;
}

bool
Geometry::touches(const Geometry *g) const
{
#ifdef SHORTCIRCUIT_PREDICATES
	// short-circuit test
	if (! getEnvelopeInternal()->intersects(g->getEnvelopeInternal()))
		return false;
#endif
	IntersectionMatrix *im=relate(g);
	bool res=im->isTouches(getDimension(), g->getDimension());
	delete im;
	return res;
}

bool
Geometry::intersects(const Geometry *g) const
{
#ifdef SHORTCIRCUIT_PREDICATES
	// short-circuit test
	if (! getEnvelopeInternal()->intersects(g->getEnvelopeInternal()))
		return false;
#endif

	/**
	 * TODO: (MD) Add optimizations:
	 *
	 * - for P-A case:
	 * If P is in env(A), test for point-in-poly
	 *
	 * - for A-A case:
	 * If env(A1).overlaps(env(A2))
	 * test for overlaps via point-in-poly first (both ways)
	 * Possibly optimize selection of point to test by finding point of A1
	 * closest to centre of env(A2).
	 * (Is there a test where we shouldn't bother - e.g. if env A
	 * is much smaller than env B, maybe there's no point in testing
	 * pt(B) in env(A)?
	 */

	// optimization for rectangle arguments
	if (isRectangle()) {
		return predicate::RectangleIntersects::intersects((Polygon&)*this, *g);
	}
	if (g->isRectangle()) {
		return predicate::RectangleIntersects::intersects((const Polygon&)*g, *this);
	}

	IntersectionMatrix *im=relate(g);
	bool res=im->isIntersects();
	delete im;
	return res;
}

/*public*/
bool
Geometry::covers(const Geometry* g) const
{
#ifdef SHORTCIRCUIT_PREDICATES
	// short-circuit test
	if (! getEnvelopeInternal()->contains(g->getEnvelopeInternal()))
		return false;
#endif
	// optimization for rectangle arguments
	if (isRectangle()) {
		return getEnvelopeInternal()->contains(g->getEnvelopeInternal());
	}

	auto_ptr<IntersectionMatrix> im(relate(g));
	return im->isCovers();
}


bool
Geometry::crosses(const Geometry *g) const
{
#ifdef SHORTCIRCUIT_PREDICATES
	// short-circuit test
	if (! getEnvelopeInternal()->intersects(g->getEnvelopeInternal()))
		return false;
#endif
	IntersectionMatrix *im=relate(g);
	bool res=im->isCrosses(getDimension(), g->getDimension());
	delete im;
	return res;
}

bool
Geometry::within(const Geometry *g) const
{
	return g->contains(this);
}

bool
Geometry::contains(const Geometry *g) const
{
#ifdef SHORTCIRCUIT_PREDICATES
	// short-circuit test
	if (! getEnvelopeInternal()->contains(g->getEnvelopeInternal()))
		return false;
#endif

	// optimization for rectangle arguments
	if (isRectangle()) {
		return predicate::RectangleContains::contains((Polygon&)*this, *g);
	}
	// Incorrect: contains is not commutative
	//if (g->isRectangle()) {
	//	return predicate::RectangleContains::contains((const Polygon&)*g, *this);
	//}

	IntersectionMatrix *im=relate(g);
	bool res=im->isContains();
	delete im;
	return res;
}

bool
Geometry::overlaps(const Geometry *g) const
{
#ifdef SHORTCIRCUIT_PREDICATES
	// short-circuit test
	if (! getEnvelopeInternal()->intersects(g->getEnvelopeInternal()))
		return false;
#endif
	IntersectionMatrix *im=relate(g);
	bool res=im->isOverlaps(getDimension(), g->getDimension());
	delete im;
	return res;
}

bool
Geometry::relate(const Geometry *g, const string &intersectionPattern) const
{
	IntersectionMatrix *im=relate(g);
	bool res=im->matches(intersectionPattern);
	delete im;
	return res;
}

bool
Geometry::equals(const Geometry *g) const
{
#ifdef SHORTCIRCUIT_PREDICATES
	// short-circuit test
	if (! getEnvelopeInternal()->equals(g->getEnvelopeInternal()))
		return false;
#endif
	IntersectionMatrix *im=relate(g);
	bool res=im->isEquals(getDimension(), g->getDimension());
	delete im;
	return res;
}

IntersectionMatrix*
Geometry::relate(const Geometry *other) const
	//throw(IllegalArgumentException *)
{
	return RelateOp::relate(this, other);
}

string
Geometry::toString() const
{
	return toText();
}

std::ostream&
operator<< (std::ostream& os, const Geometry& geom)
{
	io::WKBWriter writer;
	writer.writeHEX(geom, os);
	return os;
}

string
Geometry::toText() const
{
	io::WKTWriter writer;
	return writer.write(this);
}

Geometry*
Geometry::buffer(double distance) const
{
	return BufferOp::bufferOp(this, distance);
}

Geometry*
Geometry::buffer(double distance,int quadrantSegments) const
{
	return BufferOp::bufferOp(this, distance, quadrantSegments);
}

Geometry*
Geometry::buffer(double distance, int quadrantSegments, int endCapStyle) const
{
	return BufferOp::bufferOp(this, distance, quadrantSegments, endCapStyle);
}

Geometry*
Geometry::convexHull() const
{
	return ConvexHull(this).getConvexHull();
}

Geometry*
Geometry::intersection(const Geometry *other) const
{
	return OverlayOp::overlayOp(this, other, OverlayOp::opINTERSECTION);
}

Geometry*
Geometry::Union(const Geometry *other) const
	//throw(TopologyException *, IllegalArgumentException *)
{
	Geometry *out = NULL;

#ifdef SHORTCIRCUIT_PREDICATES
	// if envelopes are disjoint return a MULTI geom or
	// a geometrycollection
	if ( ! getEnvelopeInternal()->intersects(other->getEnvelopeInternal()) )
	{
//cerr<<"SHORTCIRCUITED-UNION engaged"<<endl;
		const GeometryCollection *coll;
		size_t ngeoms, i;
		vector<Geometry *> *v = new vector<Geometry *>();

		if ( NULL != (coll = dynamic_cast<const GeometryCollection *>(this)) )
		{
			ngeoms = coll->getNumGeometries();
			for (i=0; i<ngeoms; i++)
				v->push_back(coll->getGeometryN(i)->clone());
		} else {
			v->push_back(this->clone());
		}

		if ( NULL != (coll = dynamic_cast<const GeometryCollection *>(other)) )
		{
			ngeoms = coll->getNumGeometries();
			for (i=0; i<ngeoms; i++)
				v->push_back(coll->getGeometryN(i)->clone());
		} else {
			v->push_back(other->clone());
		}

		out = factory->buildGeometry(v);
		return out;
	}
#endif

	return OverlayOp::overlayOp(this, other, OverlayOp::opUNION);
}

Geometry*
Geometry::difference(const Geometry *other) const
	//throw(IllegalArgumentException *)
{
	return OverlayOp::overlayOp(this, other, OverlayOp::opDIFFERENCE);
}

Geometry*
Geometry::symDifference(const Geometry *other) const
{
	return OverlayOp::overlayOp(this, other, OverlayOp::opSYMDIFFERENCE);
}

int
Geometry::compareTo(const Geometry *geom) const
{
	// compare to self
	if ( this == geom ) return 0;

	if (getClassSortIndex()!=geom->getClassSortIndex()) {
		return getClassSortIndex()-geom->getClassSortIndex();
	}
	if (isEmpty() && geom->isEmpty()) {
		return 0;
	}
	if (isEmpty()) {
		return -1;
	}
	if (geom->isEmpty()) {
		return 1;
	}
	return compareToSameClass(geom);
}

bool
Geometry::isEquivalentClass(const Geometry *other) const
{
	if (typeid(*this)==typeid(*other))
		return true;
	else
		return false;
}

void
Geometry::checkNotGeometryCollection(const Geometry *g)
	//throw(IllegalArgumentException *)
{
	if ((typeid(*g)==typeid(GeometryCollection))) {
		throw  util::IllegalArgumentException("This method does not support GeometryCollection arguments\n");
	}
}

int
Geometry::getClassSortIndex() const
{
	//const type_info &t=typeid(*this);

	     if ( typeid(*this) == typeid(Point)              ) return 0;
	else if ( typeid(*this) == typeid(MultiPoint)         ) return 1;
	else if ( typeid(*this) == typeid(LineString)         ) return 2;
	else if ( typeid(*this) == typeid(LinearRing)         ) return 3;
	else if ( typeid(*this) == typeid(MultiLineString)    ) return 4;
	else if ( typeid(*this) == typeid(Polygon)            ) return 5;
	else if ( typeid(*this) == typeid(MultiPolygon)       ) return 6;
	else 
	{
		assert(typeid(*this) == typeid(GeometryCollection)); // unsupported class
		return 7;
	}

#if 0
	string str="Class not supported: ";
	str.append(typeid(*this).name());
	str.append("");
	Assert::shouldNeverReachHere(str);
	return -1;
#endif
}

int
Geometry::compare(vector<Coordinate> a, vector<Coordinate> b) const
{
	size_t i=0;
	size_t j=0;
	while (i<a.size() && j<b.size()) {
		Coordinate& aCoord=a[i];
		Coordinate& bCoord=b[j];
		int comparison=aCoord.compareTo(bCoord);
		if (comparison!=0) {
			return comparison;
		}
		i++;
		j++;
	}
	if (i<a.size()) {
		return 1;
	}
	if (j<b.size()) {
		return -1;
	}
	return 0;
}

int
Geometry::compare(vector<Geometry *> a, vector<Geometry *> b) const
{
	size_t i=0;
	size_t j=0;
	while (i<a.size() && j<b.size()) {
		Geometry *aGeom=a[i];
		Geometry *bGeom=b[j];
		int comparison=aGeom->compareTo(bGeom);
		if (comparison!=0) {
			return comparison;
		}
		i++;
		j++;
	}
	if (i<a.size()) {
		return 1;
	}
	if (j<b.size()) {
		return -1;
	}
	return 0;
}

/**
 *  Returns the minimum distance between this Geometry
 *  and the other Geometry 
 *
 * @param  other  the Geometry from which to compute the distance
 */
double
Geometry::distance(const Geometry *other) const
{
	return DistanceOp::distance(this, other);
}

/**
 *  Returns the area of this <code>Geometry</code>.
 *  Areal Geometries have a non-zero area.
 *  They override this function to compute the area.
 *  Others return 0.0
 *
 * @return the area of the Geometry
 */
double
Geometry::getArea() const
{
	return 0.0;
}

/**
 *  Returns the length of this <code>Geometry</code>.
 *  Linear geometries return their length.
 *  Areal geometries return their perimeter.
 *  They override this function to compute the area.
 *  Others return 0.0
 *
 * @return the length of the Geometry
 */
double
Geometry::getLength() const
{
	return 0.0;
}

Geometry::~Geometry()
{
	//delete envelope;
}

bool
GeometryGreaterThen::operator()(const Geometry *first, const Geometry *second)
{
	if (first->compareTo(second)>0)
		return true;
	else
		return false;
}

bool
Geometry::equal(const Coordinate& a, const Coordinate& b,
	double tolerance) const
{
	if (tolerance==0)
	{
		return a == b; // 2D only !!!
	}
	//double dist=a.distance(b);
	return a.distance(b)<=tolerance;
}

void
Geometry::apply_ro(GeometryFilter *filter) const
{
	filter->filter_ro(this);
}

void
Geometry::apply_rw(GeometryFilter *filter)
{
	filter->filter_rw(this);
}

void
Geometry::apply_ro(GeometryComponentFilter *filter) const
{
	filter->filter_ro(this);
}

void
Geometry::apply_rw(GeometryComponentFilter *filter)
{
	filter->filter_rw(this);
}

} // namespace geos::geom
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.119  2006/07/08 00:33:54  strk
 *         * configure.in: incremented CAPI minor version, to avoid                        falling behind any future version from the 2.2. branch.
 *         * source/geom/Geometry.cpp, source/geom/GeometryFactory.cpp,
 *         source/geomgraph/EdgeRing.cpp,
 *         source/headers/geos/geom/Geometry.h,
 *         source/headers/geos/geom/GeometryFactory.h,
 *         source/headers/geos/geom/GeometryFactory.inl,
 *         source/headers/geos/geomgraph/EdgeRing.h:
 *         updated doxygen comments (sync with JTS head).
 *         * source/headers/geos/platform.h.in: include <inttypes.h>
 *         rather then <stdint.h>
 *
 * Revision 1.118  2006/06/13 21:38:41  strk
 * Added self comparison check in Geometry::compareTo().
 *
 * Revision 1.117  2006/06/12 10:10:39  strk
 * Fixed getGeometryN() to take size_t rather then int, changed unsigned int parameters to size_t.
 *
 * Revision 1.116  2006/06/05 15:36:34  strk
 * Given OverlayOp funx code enum a name and renamed values to have a lowercase prefix. Drop all of noding headers from installed header set.
 *
 * Revision 1.115  2006/05/18 08:56:50  strk
 *         * source/geom/Geometry.cpp,
 *         source/headers/geos/geom/Geometry.h: added
 *         covers() and isCoveredBy() predicates.
 *         * tests/unit/Makefile.am,
 *         tests/unit/geom/Geometry/coversTest.cpp:
 *         added test for covers() predicates.
 *
 * Revision 1.114  2006/04/11 09:29:42  strk
 * Fixed initialization list (removed compiler warning)
 *
 * Revision 1.113  2006/04/10 18:15:09  strk
 * Changed Geometry::envelope member to be of type auto_ptr<Envelope>.
 * Changed computeEnvelopeInternal() signater to return auto_ptr<Envelope>
 *
 * Revision 1.112  2006/04/10 15:05:15  strk
 * Fixed a bug introduced by previous commit in getCentroid()
 *
 * Revision 1.111  2006/04/10 14:18:51  strk
 * Fixed getCentroid(Coordinate&) to round using PrecisionModel
 * all unit tests succeed.
 *
 * Revision 1.110  2006/04/10 13:09:47  strk
 * Added GeometryFactory::defaultInstance()
 * Made Geometry::INTERNAL_GEOMETRY_FACTORY an alias for it
 * removed last deletion from Unload::Release class
 *
 * Revision 1.109  2006/04/07 09:54:30  strk
 * Geometry::getNumGeometries() changed to return 'unsigned int'
 * rather then 'int'
 *
 * Revision 1.108  2006/03/31 11:03:39  strk
 * Fixed NULL-GeometryFactory constructor to use INTERNAL_GEOMETRY_FACTORY
 * (should fix bug #81)
 *
 * Revision 1.107  2006/03/24 09:52:41  strk
 * USE_INLINE => GEOS_INLINE
 *
 * Revision 1.106  2006/03/23 12:12:00  strk
 * Fixes to allow build with -DGEOS_INLINE
 *
 * Revision 1.105  2006/03/21 17:55:01  strk
 * opDistance.h header split
 *
 * Revision 1.104  2006/03/21 17:11:56  strk
 * opPredicate.h header split
 *
 * Revision 1.103  2006/03/21 13:11:28  strk
 * opRelate.h header split
 *
 * Revision 1.102  2006/03/20 16:57:43  strk
 * spatialindex.h and opValid.h headers split
 *
 * Revision 1.101  2006/03/20 12:03:25  strk
 * Added operator<< for Geometry, writing HEXWKB
 *
 * Revision 1.100  2006/03/17 13:24:58  strk
 * opOverlay.h header splitted. Reduced header inclusions in operation/overlay implementation files. ElevationMatrixFilter code moved from own file to ElevationMatrix.cpp (ideally a class-private).
 *
 * Revision 1.99  2006/03/16 10:42:43  strk
 * Bug #64 - Not all control paths return a value in geos::geom::Geometry::getClassSortIndex
 *
 * Revision 1.98  2006/03/14 12:55:55  strk
 * Headers split: geomgraphindex.h, nodingSnapround.h
 *
 * Revision 1.97  2006/03/13 21:54:56  strk
 * Streamlined headers inclusion.
 *
 * Revision 1.96  2006/03/09 16:46:47  strk
 * geos::geom namespace definition, first pass at headers split
 *
 * Revision 1.95  2006/03/06 19:40:46  strk
 * geos::util namespace. New GeometryCollection::iterator interface, many cleanups.
 *
 * Revision 1.94  2006/03/06 15:23:14  strk
 * geos::io namespace
 *
 * Revision 1.93  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.92  2006/03/01 18:37:07  strk
 * Geometry::createPointFromInternalCoord dropped (it's a duplication of GeometryFactory::createPointFromInternalCoord).
 * Fixed bugs in InteriorPoint* and getCentroid() inserted by previous commits.
 *
 * Revision 1.91  2006/03/01 17:16:38  strk
 * LineSegment class made final and optionally (compile-time) inlined.
 * Reduced heap allocations in Centroid{Area,Line,Point} and InteriorPoint{Area,Line,Point}.
 *
 * Revision 1.90  2006/02/20 10:14:18  strk
 * - namespaces geos::index::*
 * - Doxygen documentation cleanup
 *
 * Revision 1.89  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.88  2006/02/18 21:08:09  strk
 * - new CoordinateSequence::applyCoordinateFilter method (slow but useful)
 * - SegmentString::getCoordinates() doesn't return a clone anymore.
 * - SegmentString::getCoordinatesRO() obsoleted.
 * - SegmentString constructor does not promises constness of passed
 *   CoordinateSequence anymore.
 * - NEW ScaledNoder class
 * - Stubs for MCIndexPointSnapper and  MCIndexSnapRounder
 * - Simplified internal interaces of OffsetCurveBuilder and OffsetCurveSetBuilder
 *
 * Revision 1.87  2006/02/09 15:52:47  strk
 * GEOSException derived from std::exception; always thrown and cought by const ref.
 *
 * Revision 1.86  2006/02/09 13:44:57  strk
 * Added support for SRID in input WKB, undeprecated Geometry::setSRID
 * and Geometry::getSRID
 *
 * Revision 1.85  2006/02/08 17:18:28  strk
 * - New WKTWriter::toLineString and ::toPoint convenience methods
 * - New IsValidOp::setSelfTouchingRingFormingHoleValid method
 * - New Envelope::centre()
 * - New Envelope::intersection(Envelope)
 * - New Envelope::expandBy(distance, [ydistance])
 * - New LineString::reverse()
 * - New MultiLineString::reverse()
 * - New Geometry::buffer(distance, quadSeg, endCapStyle)
 * - Obsoleted toInternalGeometry/fromInternalGeometry
 * - More const-correctness in Buffer "package"
 *
 * Revision 1.84  2006/02/01 22:21:29  strk
 * - Added rectangle-based optimizations of intersects() and contains() ops
 * - Inlined all planarGraphComponent class
 *
 * Revision 1.83  2006/01/31 19:07:33  strk
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
 * Revision 1.82  2005/11/24 23:09:15  strk
 * CoordinateSequence indexes switched from int to the more
 * the correct unsigned int. Optimizations here and there
 * to avoid calling getSize() in loops.
 * Update of all callers is not complete yet.
 *
 * Revision 1.81  2005/11/21 16:03:20  strk
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
 * Revision 1.80  2005/11/08 10:03:28  strk
 * Set library version to 2.2.0.
 * Cleaned up Doxygen warnings.
 * Inlined more Envelope methods.
 * Dropped deprecated Envelope::overlaps methods.
 *
 * Revision 1.79  2005/08/22 13:31:16  strk
 * Fixed comparator functions used with STL sort() algorithm to
 * implement StrictWeakOrdering semantic.
 *
 * Revision 1.78  2005/07/11 10:50:19  strk
 * Added parens suggested by compiler
 *
 * Revision 1.77  2005/07/11 10:23:17  strk
 * removed useless assignment
 *
 * Revision 1.76  2005/06/22 00:46:57  strk
 * Shortcircuit tests for Union
 *
 * Revision 1.75  2005/05/13 17:14:54  strk
 * Added comment about 2D-only comparison of ::equal(Coordinate, Coordinate, double)
 *
 * Revision 1.74  2005/04/29 17:40:35  strk
 * Updated Doxygen documentation and some Copyright headers.
 *
 * Revision 1.73  2004/12/30 13:31:20  strk
 * Fixed a segfault on EMPTYGEOM->getCeontroid()
 *
 * Revision 1.72  2004/11/17 08:13:16  strk
 * Indentation changes.
 * Some Z_COMPUTATION activated by default.
 *
 * Revision 1.71  2004/09/16 09:48:40  strk
 * Finer short-circuit tests for equals, within, contains.
 *
 * Revision 1.70  2004/09/16 07:32:15  strk
 * Added short-circuit tests. Can be disabled at compile-time
 *
 * Revision 1.69  2004/07/27 16:35:46  strk
 * Geometry::getEnvelopeInternal() changed to return a const Envelope *.
 * This should reduce object copies as once computed the envelope of a
 * geometry remains the same.
 *
 * Revision 1.68  2004/07/22 16:58:01  strk
 * runtime version extractor functions split. geos::version() is now
 * geos::geosversion() and geos::jtsport()
 *
 * Revision 1.67  2004/07/21 09:55:24  strk
 * CoordinateSequence::atLeastNCoordinatesOrNothing definition fix.
 * Documentation fixes.
 *
 * Revision 1.66  2004/07/20 08:34:18  strk
 * Fixed a bug in opDistance.h.
 * Removed doxygen tags from obsoleted CoordinateList.cpp.
 * Got doxygen to run with no warnings.
 *
 * Revision 1.65  2004/07/19 13:19:30  strk
 * Documentation fixes
 *
 * Revision 1.64  2004/07/17 10:48:04  strk
 * fixed typo in documentation
 *
 * Revision 1.63  2004/07/17 09:18:54  strk
 * Added geos::version()
 *
 * Revision 1.62  2004/07/14 21:20:58  strk
 * Added GeometricShapeFactory note on doxygen mainpage
 *
 * Revision 1.61  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added CoordinateArraySequenceFactory::instance() function.
 *
 * Revision 1.60  2004/07/07 09:38:12  strk
 * Dropped WKTWriter::stringOfChars (implemented by std::string).
 * Dropped WKTWriter default constructor (internally created GeometryFactory).
 * Updated XMLTester to respect the changes.
 * Main documentation page made nicer.
 *
 * Revision 1.59  2004/07/06 17:58:21  strk
 * Removed deprecated Geometry constructors based on PrecisionModel and
 * SRID specification. Removed SimpleGeometryPrecisionReducer capability
 * of changing Geometry's factory. Reverted Geometry::factory member
 * to be a reference to external factory.
 *
 * Revision 1.58  2004/07/05 19:40:48  strk
 * Added GeometryFactory::destroyGeometry(Geometry *)
 *
 * Revision 1.57  2004/07/05 15:20:18  strk
 * Documentation again.
 *
 * Revision 1.56  2004/07/05 10:50:20  strk
 * deep-dopy construction taken out of Geometry and implemented only
 * in GeometryFactory.
 * Deep-copy geometry construction takes care of cleaning up copies
 * on exception.
 * Implemented clone() method for CoordinateSequence
 * Changed createMultiPoint(CoordinateSequence) signature to reflect
 * copy semantic (by-ref instead of by-pointer).
 * Cleaned up documentation.
 *
 * Revision 1.55  2004/07/03 12:51:37  strk
 * Documentation cleanups for DoxyGen.
 *
 * Revision 1.54  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.53  2004/07/01 17:34:07  strk
 * GeometryFactory argument in Geometry constructor reverted
 * to its copy-and-destroy semantic.
 *
 * Revision 1.52  2004/07/01 14:12:44  strk
 *
 * Geometry constructors come now in two flavors:
 * 	- deep-copy args (pass-by-reference)
 * 	- take-ownership of args (pass-by-pointer)
 * Same functionality is available through GeometryFactory,
 * including buildGeometry().
 *
 * Revision 1.51  2004/06/30 20:59:12  strk
 * Removed GeoemtryFactory copy from geometry constructors.
 * Enforced const-correctness on GeometryFactory arguments.
 *
 * Revision 1.50  2004/05/21 13:58:47  strk
 * ::intersection missed to invalidate geometryCollection inputs
 *
 * Revision 1.49  2004/05/17 07:23:05  strk
 * ::getCeontroid(): reduced dynamic allocations, added missing check for isEmpty
 *
 * Revision 1.48  2004/05/14 12:14:08  strk
 * const correctness
 *
 * Revision 1.47  2004/05/07 09:05:13  strk
 * Some const correctness added. Fixed bug in GeometryFactory::createMultiPoint
 * to handle NULL CoordinateSequence.
 *
 * Revision 1.46  2004/05/05 16:51:29  strk
 * avoided copy constructor in Geometry::geometryChangedFilter initializzazion
 *
 * Revision 1.45  2004/05/05 10:54:48  strk
 * Removed some private static heap explicit allocation, less cleanup done by
 * the unloader.
 *
 * Revision 1.44  2004/04/30 09:15:28  strk
 * Enlarged exception specifications to allow for AssertionFailedException.
 * Added missing initializers.
 *
 * Revision 1.43  2004/04/20 10:14:20  strk
 * Memory leaks removed.
 *
 * Revision 1.42  2004/04/14 13:56:26  strk
 * All geometries returned by {from,to}InternalGeometry calls are
 * now deleted after use (unless NOT new).
 * Some 'commented' throw specifications in geom.h
 *
 * Revision 1.41  2004/04/14 07:29:43  strk
 * Fixed GeometryFactory constructors to copy given PrecisionModel. Added GeometryFactory copy constructor. Fixed Geometry constructors to copy GeometryFactory.
 *
 * Revision 1.40  2004/04/01 10:44:33  ybychkov
 * All "geom" classes from JTS 1.3 upgraded to JTS 1.4
 *
 * Revision 1.39  2004/03/17 02:00:33  ybychkov
 * "Algorithm" upgraded to JTS 1.4
 *
 * Revision 1.38  2004/03/01 22:04:59  strk
 * applied const correctness changes by Manuel Prieto Villegas <ManuelPrietoVillegas@telefonica.net>
 *
 * Revision 1.37  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.36  2003/10/20 15:41:34  strk
 * Geometry::checkNotGeometryCollection made static and non-distructive.
 *
 * Revision 1.35  2003/10/13 12:51:28  strk
 * removed sortedClasses strings array from all geometries.
 *
 **********************************************************************/

