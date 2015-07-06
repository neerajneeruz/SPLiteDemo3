/**********************************************************************
 * $Id: StringTokenizer.h 1820 2006-09-06 16:54:23Z mloskot $
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

#ifndef GEOS_IO_STRINGTOKENIZER_H
#define GEOS_IO_STRINGTOKENIZER_H

#include <string>

namespace geos {
namespace io {

class StringTokenizer {
public:
	enum {
		TT_EOF,
		TT_EOL,
		TT_NUMBER,
		TT_WORD
	};
	//StringTokenizer();
	StringTokenizer(const std::string& txt);
	~StringTokenizer() {};
	int nextToken();
	int peekNextToken();
	double getNVal();
	std::string getSVal();
private:
	const std::string &str;
	std::string stok;
	double ntok;
	std::string::const_iterator iter;
};

} // namespace io
} // namespace geos

#endif // #ifndef GEOS_IO_STRINGTOKENIZER_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/20 18:18:14  strk
 * io.h header split
 *
 **********************************************************************/
