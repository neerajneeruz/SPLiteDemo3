/**********************************************************************
 * $Id: SimpleWKTTester.cpp 1820 2006-09-06 16:54:23Z mloskot $
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

// CTS.cpp : Testing class
//

#include <iostream>
#include <fstream>

#include <geos/io.h>
#include <geos/geom.h>
#include <geos/util/GEOSException.h>

using namespace std;
using namespace geos::io;
using namespace geos::geom;
using namespace geos::util;

int main(int /*argc*/, char** /*argv*/)
{
    
	try {
		ofstream out("WKTOut");
		ifstream in("WKTIn");
		string instr;
		string outstr;
		WKTReader *r = new WKTReader(new GeometryFactory(new PrecisionModel(),10));
		WKTWriter *w = new WKTWriter();
		Geometry *g;

		cout << "Start Testing:" << endl;
		while(!in.eof()) {
			getline(in,instr);
			if (instr!="") {
				g=r->read(instr);
				outstr=w->write(g);
				out << "----------" << endl;
				out << instr << endl;
				out << outstr << endl;
				out << "----------" << endl << endl;
			}
		}
		out.flush();
		out.close();
		cout << "End of Testing" << endl;

	} catch (const GEOSException& ge) {
		cout << ge.what() << endl;
	}

	return 0;
}

/**********************************************************************
 * $Log$
 * Revision 1.7  2006/06/12 16:59:04  strk
 * Removed unused parameter warnings
 *
 * Revision 1.6  2006/06/09 08:28:38  strk
 * Removed compiler warning
 *
 * Revision 1.5  2006/03/09 16:46:49  strk
 * geos::geom namespace definition, first pass at headers split
 *
 **********************************************************************/

