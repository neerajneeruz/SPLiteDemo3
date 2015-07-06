/**********************************************************************
 * $Id: GeometricShapeFactory.h 1820 2006-09-06 16:54:23Z mloskot $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_UTIL_GEOMETRICSHAPEFACTORY_H
#define GEOS_UTIL_GEOMETRICSHAPEFACTORY_H

#include <cassert>

#include <geos/geom/Coordinate.h>

// Forward declarations
namespace geos {
	namespace geom { 
		class Envelope;
		class Polygon;
		class GeometryFactory;
		class LineString;
	}
}

namespace geos {
namespace util { // geos::util


/**
 * \class GeometricShapeFactory util.h geos.h
 *
 * \brief
 * Computes various kinds of common geometric shapes.
 * Allows various ways of specifying the location and extent of the shapes,
 * as well as number of line segments used to form them.
 *
 */
class GeometricShapeFactory {
private:
	class Dimensions {
	public:
		Dimensions();
		geom::Coordinate base;
		geom::Coordinate centre;
		double width;
		double height;
		void setBase(const geom::Coordinate& newBase);
		void setCentre(const geom::Coordinate& newCentre);
		void setSize(double size);
		void setWidth(double nWidth);
		void setHeight(double nHeight);
		geom::Envelope* getEnvelope();
	};
	const geom::GeometryFactory* geomFact;
	Dimensions dim;
	int nPts;
public:
	/**
	 * \brief
	 * Create a shape factory which will create shapes using the given
	 * GeometryFactory.
	 *
	 * @param factory the factory to use. You need to keep the
	 *	factory alive for the whole GeometricShapeFactory
	 *	life time.
	 * 
	 */
	GeometricShapeFactory(const geom::GeometryFactory *factory);

	~GeometricShapeFactory();

	/**
	 * \brief Creates a elliptical arc, as a LineString.
	 *
	 * @return an elliptical arc
	 */
	geom::LineString* createArc(double startAng, double endAng);

	/**
	 * \brief Creates a circular Polygon.
	 *
	 * @return a circle
	 */
	geom::Polygon* createCircle();

	/**
	 * \brief Creates a rectangular Polygon.
	 *
	 * @return a rectangular Polygon
	 */
	geom::Polygon* createRectangle();

	/**
	 * \brief
	 * Sets the location of the shape by specifying the base coordinate
	 * (which in most cases is the * lower left point of the envelope
	 * containing the shape).
	 *
	 * @param base the base coordinate of the shape
	 */
	void setBase(const geom::Coordinate& base);

	/**
	 * \brief
	 * Sets the location of the shape by specifying the centre of
	 * the shape's bounding box
	 *
	 * @param centre the centre coordinate of the shape
	 */
	void setCentre(const geom::Coordinate& centre);

	/**
	 * \brief Sets the height of the shape.
	 *
	 * @param height the height of the shape
	 */
	void setHeight(double height);

	/**
	 * \brief Sets the total number of points in the created Geometry
	 */
	void setNumPoints(int nNPts);

	/**
	 * \brief
	 * Sets the size of the extent of the shape in both x and y directions.
	 *
	 * @param size the size of the shape's extent
	 */
	void setSize(double size);

	/**
	 * \brief Sets the width of the shape.
	 *
	 * @param width the width of the shape
	 */
	void setWidth(double width);

};

} // namespace geos::util
} // namespace geos

#endif // GEOS_UTIL_GEOMETRICSHAPEFACTORY_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/
