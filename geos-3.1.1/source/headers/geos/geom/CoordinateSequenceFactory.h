/**********************************************************************
 * $Id: CoordinateSequenceFactory.h 1820 2006-09-06 16:54:23Z mloskot $
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

#ifndef GEOS_GEOM_COORDINATESEQUENCEFACTORY_H
#define GEOS_GEOM_COORDINATESEQUENCEFACTORY_H


#include <vector>

//#include <geos/geom/Coordinate.h>
#include <geos/inline.h>

// Forward declarations
namespace geos {
	namespace geom { 
		class CoordinateSequence;
		class Coordinate;
	}
}

namespace geos {
namespace geom { // geos::geom

/**
 * \brief
 * An object that knows how to build a particular implementation of
 * CoordinateSequence from an array of Coordinates.
 */
class CoordinateSequenceFactory {
public:

	/** \brief
	 * Returns a CoordinateSequence based on the given array.
	 * Whether or not the vector is copied is implementation-dependent,
	 * for this reason caller does give up ownership of it.
	 * Implementations that will not copy it will need take care
	 * of deleting it.
	 * Note that a NULL value is allowed as coordinates, and will
	 * create an empty CoordinateSequence.
	 */
	virtual CoordinateSequence *create(
			std::vector<Coordinate> *coordinates) const=0;

	/** \brief
	 * Creates a CoordinateSequence of the specified size and dimension.
	 * For this to be useful, the CoordinateSequence implementation must
	 * be mutable.
	 *
	 * @param size the number of coordinates in the sequence
	 * @param dimension the dimension of the coordinates in the sequence
	 * 	(if user-specifiable, otherwise ignored)
	 */
	virtual CoordinateSequence *create(size_t size,
			size_t dimension) const=0;

	virtual ~CoordinateSequenceFactory() {}
};

} // namespace geos::geom
} // namespace geos

//#ifdef GEOS_INLINE
//# include "geos/geom/CoordinateSequenceFactory.inl"
//#endif

#endif // ndef GEOS_GEOM_COORDINATESEQUENCEFACTORY_H

/**********************************************************************
 * $Log$
 * Revision 1.4  2006/06/12 10:10:39  strk
 * Fixed getGeometryN() to take size_t rather then int, changed unsigned int parameters to size_t.
 *
 * Revision 1.3  2006/06/08 11:20:24  strk
 * Added missing virtual destructor to abstract classes.
 *
 * Revision 1.2  2006/03/24 09:52:41  strk
 * USE_INLINE => GEOS_INLINE
 *
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/
