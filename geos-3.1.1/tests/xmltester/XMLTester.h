/**********************************************************************
 * $Id: XMLTester.h 1960 2007-01-16 14:12:14Z strk $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2005 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef __XMLTESTER_H__
#define __XMLTESTER_H__

#include <geos/geom/GeometryFactory.h>
#include <geos/geom/PrecisionModel.h>

using namespace geos;

class XMLTester {

private:
	enum {
		SHOW_RUN_INFO=1,
		SHOW_CASE,
		SHOW_TEST,
		SHOW_RESULT,
		SHOW_GEOMS,
		SHOW_GEOMS_FULL,
		PRED
	};

	void parsePrecisionModel();
	void parseCase();
	void parseTest();
	void runPredicates(const geom::Geometry *a, const geom::Geometry *b);
	geom::Geometry *parseGeometry(const std::string &in, const char* label="parsed");
	static std::string trimBlanks(const std::string &in);
	void printGeom(std::ostream& os, geom::Geometry *g);
	std::string printGeom(geom::Geometry *g);
	void printTest(bool success, const std::string& expected_result, const std::string& actual_result);

	geom::Geometry *gA;
	geom::Geometry *gB;
	geom::Geometry *gT;

	std::auto_ptr<geom::PrecisionModel> pm;
	std::auto_ptr<geom::GeometryFactory> factory;
	std::auto_ptr<io::WKTReader> wktreader;
	std::auto_ptr<io::WKTWriter> wktwriter;
	std::auto_ptr<io::WKBReader> wkbreader;
	std::auto_ptr<io::WKBWriter> wkbwriter;
	CMarkupSTL xml;

	int verbose;
	int test_predicates;

	int failed;
	int succeeded;
	int caseCount;
	int testCount;
	std::string opSignature;

	int testFileCount;
	int totalTestCount;

	const std::string *curr_file;
	std::string curr_case_desc;

	bool testValidOutput;
	bool testValidInput;
	bool sqlOutput;
	bool HEXWKB_output;

	void testValid(const geom::Geometry* g, const std::string& label);

public:
	XMLTester();
	~XMLTester();
	void run(const std::string &testFile);
	void resultSummary(std::ostream &os) const;
	void resetCounters();

	/*
	 * Values:
	 *	0: Show case description, run tests, show result
	 *	1: Show parsed geometry values
	 *	2: Run predicates
	 *
	 * Return previously set verbosity level
	 */
	int setVerbosityLevel(int val);

	int getFailuresCount() { return failed; }

	void testOutputValidity(bool val) { testValidOutput=val; }
	void testInputValidity(bool val) { testValidInput=val; }
	void setSQLOutput(bool val) { sqlOutput=val; }
	void setHEXWKBOutput(bool val) { HEXWKB_output=val; }

};


#endif 
