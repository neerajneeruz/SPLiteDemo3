/**********************************************************************
 * $Id: SweepLineInterval.h 1820 2006-09-06 16:54:23Z mloskot $
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

#ifndef GEOS_INDEX_SWEEPLINE_SWEEPLINEINTERVAL_H
#define GEOS_INDEX_SWEEPLINE_SWEEPLINEINTERVAL_H

namespace geos {
namespace index { // geos.index
namespace sweepline { // geos:index:sweepline

class SweepLineInterval {
public:
	SweepLineInterval(double newMin, double newMax, void* newItem=0);
	double getMin();
	double getMax();
	void* getItem();
private:
	double min, max;
	void* item;
};

} // namespace geos:index:sweepline
} // namespace geos:index
} // namespace geos

#endif // GEOS_INDEX_SWEEPLINE_SWEEPLINEINTERVAL_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/21 10:01:30  strk
 * indexSweepline.h header split
 *
 **********************************************************************/

