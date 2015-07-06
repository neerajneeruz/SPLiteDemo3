/**********************************************************************
 * $Id: IllegalArgumentException.h 1820 2006-09-06 16:54:23Z mloskot $
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

#ifndef GEOS_UTIL_ILLEGALARGUMENTEXCEPTION_H
#define GEOS_UTIL_ILLEGALARGUMENTEXCEPTION_H

#include <string>

#include <geos/util/GEOSException.h>

namespace geos {
namespace util { // geos::util

/** 
 * \brief Indicates one or more legal arguments.
 *
 * This exception is thrown - for example - when
 * trying to apply set-theoretic methods to a
 * GeometryCollection object.
 */
class IllegalArgumentException: public GEOSException {
public:
	IllegalArgumentException()
		:
		GEOSException("IllegalArgumentException", "")
	{}

	IllegalArgumentException(const std::string& msg)
		:
		GEOSException("IllegalArgumentException", msg)
	{}

	~IllegalArgumentException() throw() {};
};

} // namespace geos::util
} // namespace geos


#endif // GEOS_UTIL_ILLEGALARGUMENTEXCEPTION_H

/**********************************************************************
 * $Log$
 * Revision 1.2  2006/04/04 08:16:46  strk
 * Changed GEOSException hierarchy to be derived from std::runtime_exception.
 * Removed the GEOSException::toString redundant method (use ::what() instead)
 *
 * Revision 1.1  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/
