/**********************************************************************
 * $Id: Assert.h 1820 2006-09-06 16:54:23Z mloskot $
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

#ifndef GEOS_UTIL_ASSERT_H
#define GEOS_UTIL_ASSERT_H

#include <string>

// Forward declarations
namespace geos {
	namespace geom {
		class Coordinate;
	}
}

namespace geos {
namespace util { // geos.util

class Assert {
public:

	static void isTrue(bool assertion, const std::string& message);

	static void isTrue(bool assertion) {
		isTrue(assertion, std::string());
	}


	static void equals(const geom::Coordinate& expectedValue,
			const geom::Coordinate& actualValue,
			const std::string& message);

	static void equals(const geom::Coordinate& expectedValue,
			const geom::Coordinate& actualValue)
	{
		equals(expectedValue, actualValue, std::string());
	}


	static void shouldNeverReachHere(const std::string& message);

	static void shouldNeverReachHere() { shouldNeverReachHere(std::string()); }
};

} // namespace geos.util
} // namespace geos


#endif // GEOS_UTIL_ASSERT_H

/**********************************************************************
 * $Log$
 * Revision 1.2  2006/03/14 17:46:26  strk
 * Removed unrequired include
 *
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/
