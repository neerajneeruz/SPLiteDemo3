/**********************************************************************
 * $Id: XMLTester.cpp 2131 2008-07-15 22:04:51Z mloskot $
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

//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
#ifdef _MSC_VER
#include <crtdbg.h>
#endif

#include <geos/geom/Point.h>
#include <geos/geom/LineString.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/IntersectionMatrix.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/BinaryOp.h>
#include <geos/operation/overlay/OverlayOp.h>
#include <geos/util.h>
//#include <geos/geomgraph.h>
#include <geos/io/WKBReader.h>
#include <geos/io/WKBWriter.h>
#include <geos/io/WKTReader.h>
#include <geos/io/WKTWriter.h>
#include <geos/opRelate.h>
#include <geos/opPolygonize.h>
#include <geos/opLinemerge.h>
#include <geos/profiler.h>
#include "markup/MarkupSTL.h"
#include <geos/unload.h>
#include <geos/opValid.h>
#include "XMLTester.h"

#include <cassert>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <memory>
#include <functional>
#include <stdexcept>
#include <cmath>

#ifdef _MSC_VER
#include <windows.h>
#include "Stackwalker.h"
#endif

using namespace geos;
using namespace geos::operation::polygonize;
using namespace geos::operation::linemerge;

void
tolower(std::string& str)
{
    std::transform(str.begin(), str.end(), str.begin(), (int(*)(int))std::tolower);
}

std::string
normalize_filename(const std::string& str)
{
	std::string newstring;

	std::string::size_type last_slash = str.find_last_of('/', str.size());
	if ( last_slash == std::string::npos ) newstring = str;
	else newstring = str.substr(last_slash+1);
	
	for (std::string::iterator i=newstring.begin(), e=newstring.end(); i!=e; ++i)
	{
		if ( *i == '.' ) *i = '_';
	}

	tolower(newstring);

	return newstring;
}

XMLTester::XMLTester()
	:
	gA(0),
	gB(0),
	gT(0),
	pm(0),
	factory(0),
	wktreader(0),
	wktwriter(0),
	wkbreader(0),
	wkbwriter(0),
	test_predicates(0),
	failed(0),
	succeeded(0),
	caseCount(0),
	testCount(0),
	testFileCount(0),
	totalTestCount(0),
	curr_file(NULL),
	testValidOutput(false),
	testValidInput(false),
	sqlOutput(false),
	HEXWKB_output(false)
{
	setVerbosityLevel(0);
}

int
XMLTester::setVerbosityLevel(int value)
{
	int old_value=verbose;

	verbose=value;

	return old_value;
}

/*private*/
void
XMLTester::printTest(bool success, const std::string& expected_result, const std::string& actual_result)
{
	if ( sqlOutput )
	{
		std::cout << "INSERT INTO \"" << normalize_filename(*curr_file) << "\" VALUES ("
		          << caseCount << ", "
		          << testCount << ", "
		          << "'" << opSignature << "', "
		          << "'" << curr_case_desc << "', ";

		std::string geomOut;

		if ( gA ) {
			std::cout << "'" << printGeom(gA) << "', ";
		} else {
			std::cout << "NULL, ";
		}

		if ( gB ) {
			std::cout << "'" << printGeom(gB) << "', ";
		} else {
			std::cout << "NULL, ";
		}

		std::cout << "'" << expected_result << "', "
		          << "'" << actual_result << "', ";

		if ( success ) std::cout << "'t'";
		else std::cout << "'f'";

		std::cout << ");" << std::endl;
	}

	else
	{
		std::cout << *curr_file <<":";
		std::cout << " case" << caseCount << ":";
		std::cout << " test" << testCount << ": "
			<< opSignature; 
		std::cout << ": " << (success?"ok.":"failed.")<<std::endl;

		std::cout << "\tDescription: " << curr_case_desc << std::endl;


		if ( gA ) {
			std::cout << "\tGeometry A: ";
			printGeom(std::cout, gA);
			std::cout << std::endl;
		}

		if ( gB ) {
			std::cout << "\tGeometry B: ";
			printGeom(std::cout, gB);
			std::cout << std::endl;
		}

		std::cout << "\tExpected result: "<<expected_result<<std::endl;
		std::cout << "\tObtained result: "<<actual_result<<std::endl;
		std::cout <<std::endl;
	}
}

void
XMLTester::run(const std::string &source)
{
	curr_file=&source;

	if ( sqlOutput )
	{
		std::cout << "CREATE TABLE \"" << normalize_filename(*curr_file) << "\"" 
		          << "( caseno integer, testno integer, " 
			  << " operation varchar, description varchar, "
			  << " a geometry, b geometry, expected geometry, "
			  << " obtained geometry, result bool )"

			  // NOTE: qgis 0.7.4 require oids for proper operations.
			  //       The 'WITH OIDS' parameter is supported back to
			  //       PostgreSQL 7.2, so if you run an older version
			  //       rebuild with the next line commented out.
			  << " WITH OIDS"

			  << ";" << std::endl;
	}

	++testFileCount;

	caseCount=0;

	if ( ! xml.Load(source.c_str()) )
	{
		std::cerr << "Error loading " << source << std::endl;
	}

	xml.ResetPos();
	xml.FindElem("run");
	xml.FindChildElem("precisionModel");
	parsePrecisionModel();
	while (xml.FindChildElem("case")) {
		try {
			parseCase();
		} catch (const std::exception& exc) {
			std::cerr<<exc.what()<<std::endl;
		}
	}
}

void 
XMLTester::resultSummary(std::ostream &os) const
{
	os<<"Files: "<<testFileCount<<std::endl;
	os<<"Tests: "<<totalTestCount<<std::endl;
	os<<"Failed: "<<failed<<std::endl;
	os<<"Succeeded: "<<succeeded<<std::endl;
}

void 
XMLTester::resetCounters() 
{
	testFileCount=totalTestCount=failed=succeeded=0;
}


void
XMLTester::parsePrecisionModel()
{
	using geos::geom::PrecisionModel;

	/* This does not seem to work... */
	std::string type=xml.GetChildAttrib("type");
	std::string scaleStr=xml.GetChildAttrib("scale");

	if ( scaleStr == "" ) {
		if ( type == "FLOATING_SINGLE" )
		{
			pm.reset(new PrecisionModel(PrecisionModel::FLOATING_SINGLE));
		}
		else
		{
			pm.reset(new PrecisionModel());
		}
	} else {
		char* stopstring;
		//string scaleStr=xml.GetChildAttrib("scale");
		std::string offsetXStr=xml.GetChildAttrib("offsetx");
		std::string offsetYStr=xml.GetChildAttrib("offsety");

		double scale=std::strtod(scaleStr.c_str(),&stopstring);
		double offsetX=std::strtod(offsetXStr.c_str(),&stopstring);
		double offsetY=std::strtod(offsetYStr.c_str(),&stopstring);
		pm.reset(new PrecisionModel(scale,offsetX,offsetY));
	}

	if (verbose > 1)
	{
		std::cerr << *curr_file <<": run: Precision Model: " << pm->toString() <<std::endl;
	}

	factory.reset(new geom::GeometryFactory(pm.get()));
	wktreader.reset(new io::WKTReader(factory.get()));
	wktwriter.reset(new io::WKTWriter());
	wkbreader.reset(new io::WKBReader(*factory));
	wkbwriter.reset(new io::WKBWriter());
}


void
XMLTester::testValid(const geom::Geometry* g, const std::string& label)
{
	operation::valid::IsValidOp ivo(g);
	bool result;
	result = ivo.isValid();
	if ( result == 0 )
	{
		operation::valid::TopologyValidationError *err = ivo.getValidationError();
		if ( err ) {
			std::cerr << *curr_file << ":"
			          << " case" << caseCount << ":"
			          << " test" << testCount << ": "
				  << opSignature << ": " 
			          << " invalid geometry (" << label 
			          << "): " << err->toString() << std::endl;
		}
	}
	//return result;
}

/**
 * Parse WKT or HEXWKB
 */
geom::Geometry *
XMLTester::parseGeometry(const std::string &in, const char* label)
{
	std::stringstream is(in, std::ios_base::in);
	char first_char;

	// Remove leading spaces
	while (is.get(first_char) && std::isspace(first_char));
	is.unget();

	geom::Geometry* ret;

	switch (first_char)
	{
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
			ret = wkbreader->readHEX(is);
			break;
		default:
			ret = wktreader->read(in);
			break;
	}

	if ( testValidInput ) testValid(ret, std::string(label));

	//ret->normalize();

	return ret;
}

std::string 
XMLTester::trimBlanks(const std::string &in)
{
	std::string out;
	std::string::size_type pos = in.find_first_not_of(" \t\n\r");
	if (pos!=std::string::npos) out=in.substr(pos);
	pos = out.find_last_not_of(" \t\n\r");
	if (pos!=std::string::npos) out=out.substr(0, pos+1);
	return out;
}

void
XMLTester::parseCase()
{
	std::string geomAin;
	std::string geomBin;
	std::string thrownException;

	gA=NULL;
	gB=NULL;

	xml.IntoElem();
	xml.FindChildElem("desc");
	curr_case_desc=trimBlanks(xml.GetChildData());

	try {
		xml.FindChildElem("a");
		geomAin=xml.GetChildData();
		geomAin=trimBlanks(geomAin);
		gA=parseGeometry(geomAin, "Geometry A");

		if ( xml.FindChildElem("b") )
		{
			geomBin=xml.GetChildData();
			geomBin=trimBlanks(geomBin);
			gB=parseGeometry(geomBin, "Geometry B");
		}
	}
	catch (const std::exception &e) {
		thrownException = e.what();
	}
	catch (...) {
		thrownException = "Unknown exception";
	}

	if ( thrownException != "" )
	{
		std::cout << *curr_file <<":";
		std::cout << " case" << caseCount << ":";
		std::cout << " skipped ("<<thrownException<<")."<<std::endl;
		xml.OutOfElem();
		return;
	}

	++caseCount;
	testCount=0;
	while(xml.FindChildElem("test")) {
		parseTest();
	}
	totalTestCount+=testCount;
		
	xml.OutOfElem();
	delete gA;
	delete gB;
}

/*private*/
void
XMLTester::printGeom(std::ostream& os, geom::Geometry *g)
{
	os << printGeom(g);
}

std::string
XMLTester::printGeom(geom::Geometry *g)
{
	if ( HEXWKB_output )
	{
		std::stringstream s(std::ios_base::binary|std::ios_base::in|std::ios_base::out);
		wkbwriter->write(*g, s);
		std::stringstream s2;
		wkbreader->printHEX(s, s2);
		return s2.str();
	}
	else
	{
		return wktwriter->write(g);
	}
}

void
XMLTester::parseTest()
{
	using namespace operation::overlay;

	typedef std::auto_ptr< geom::Geometry > GeomAutoPtr;

	int success=0; // no success by default
	std::string opName;
	std::string opArg1;
	std::string opArg2;
	std::string opArg3;
	std::string opRes;
	//string opSig;

	++testCount;

	xml.IntoElem();
	xml.FindChildElem("op");
	opName=xml.GetChildAttrib("name");
	opArg1=xml.GetChildAttrib("arg1");
	opArg2=xml.GetChildAttrib("arg2");
	opArg3=xml.GetChildAttrib("arg3");
	//opSig=xml.GetChildAttrib("arg3");
	opRes=xml.GetChildData();

	// trim blanks
	opRes=trimBlanks(opRes);
	opName=trimBlanks(opName);
	tolower(opName);

	std::string opSig="";

	if ( opArg1 != "" ) opSig=opArg1;
	if ( opArg2 != "" ) {
		if ( opSig != "" ) opSig += ", ";
		opSig += opArg2;
	}
	if ( opArg3 != "" ) {
		if ( opSig != "" ) opSig += ", ";
		opSig += opArg3;
	}

	opSignature = opName + "(" + opSig + ")";

	std::string actual_result="NONE";

	// expected_result will be modified by specific tests
	// if needed (geometry normalization, for example)
	std::string expected_result=opRes;

	try
	{

		util::Profile profile("op");

		if (opName=="relate")
		{
			std::auto_ptr<geom::IntersectionMatrix> im(gA->relate(gB));
			assert(im.get());

			if (im->matches(opArg3)) actual_result="true";
			else actual_result="false";

			if (actual_result==opRes) success=1;
		}

		else if (opName=="isvalid")
		{
			geom::Geometry *gT=gA;
			if ( ( opArg1 == "B" || opArg1 == "b" ) && gB ) {
				gT=gB;
			} 

			if (gT->isValid()) actual_result="true";
			else actual_result="false";

			if (actual_result==opRes) success=1;

		}

		else if (opName=="intersection")
		{

			GeomAutoPtr gRes(parseGeometry(opRes, "expected"));
			gRes->normalize();

			//GeomAutoPtr gRealRes(gA->intersection(gB));
			GeomAutoPtr gRealRes = BinaryOp(gA, gB, overlayOp(OverlayOp::opINTERSECTION));
			gRealRes->normalize();

			if (gRes->compareTo(gRealRes.get())==0) success=1;

			if ( testValidOutput ) testValid(gRes.get(), "result");

			actual_result=printGeom(gRealRes.get());
			expected_result=printGeom(gRes.get());

		}

		else if (opName=="union")
		{
			GeomAutoPtr gRes(parseGeometry(opRes, "expected"));
			gRes->normalize();

			//GeomAutoPtr gRealRes(gA->Union(gB));
			GeomAutoPtr gRealRes = BinaryOp(gA, gB, overlayOp(OverlayOp::opUNION));
			gRealRes->normalize();

			if (gRes->compareTo(gRealRes.get())==0) success=1;

			if ( testValidOutput ) testValid(gRes.get(), "result");

			actual_result=printGeom(gRealRes.get());
			expected_result=printGeom(gRes.get());
		}

		else if (opName=="difference")
		{

			GeomAutoPtr gRes(parseGeometry(opRes, "expected"));
			gRes->normalize();

			//GeomAutoPtr gRealRes(gA->difference(gB));
			GeomAutoPtr gRealRes = BinaryOp(gA, gB, overlayOp(OverlayOp::opDIFFERENCE));
			
			gRealRes->normalize();

			if (gRes->compareTo(gRealRes.get())==0) success=1;

			if ( testValidOutput ) testValid(gRes.get(), "result");

			actual_result=printGeom(gRealRes.get());
			expected_result=printGeom(gRes.get());

		}

		else if (opName=="symdifference")
		{
			GeomAutoPtr gRes(parseGeometry(opRes, "expected"));
			gRes->normalize();

			//GeomAutoPtr gRealRes(gA->symDifference(gB));
			GeomAutoPtr gRealRes = BinaryOp(gA, gB, overlayOp(OverlayOp::opSYMDIFFERENCE));
			gRealRes->normalize();

			if (gRes->compareTo(gRealRes.get())==0) success=1;

			if ( testValidOutput ) testValid(gRes.get(), "result");

			actual_result=printGeom(gRealRes.get());
			expected_result=printGeom(gRes.get());

		}

		else if (opName=="intersects")
		{
			if (gA->intersects(gB)) actual_result="true";
			else actual_result="false";
			
			if (actual_result==opRes) success=1;
		}

		else if (opName=="getboundary")
		{
			geom::Geometry *gT=gA;
			if ( ( opArg1 == "B" || opArg1 == "b" ) && gB ) gT=gB;

			GeomAutoPtr gRes(parseGeometry(opRes, "expected"));
			gRes->normalize();

			GeomAutoPtr gRealRes(gT->getBoundary());
			gRealRes->normalize();

			if (gRes->compareTo(gRealRes.get())==0) success=1;

			if ( testValidOutput ) testValid(gRes.get(), "result");

			actual_result=printGeom(gRealRes.get());
			expected_result=printGeom(gRes.get());

		}

		else if (opName=="getcentroid")
		{
			geom::Geometry *gT=gA;
			if ( ( opArg1 == "B" || opArg1 == "b" ) && gB ) gT=gB;

			GeomAutoPtr gRes(parseGeometry(opRes, "expected"));
			gRes->normalize();

			GeomAutoPtr gRealRes(gT->getCentroid());

			if ( gRealRes.get() ) gRealRes->normalize();
			else gRealRes.reset(factory->createGeometryCollection());
			gRealRes->normalize();

			if (gRes->compareTo(gRealRes.get())==0) success=1;

			if ( testValidOutput ) testValid(gRes.get(), "result");

			actual_result=printGeom(gRealRes.get());
			expected_result=printGeom(gRes.get());

		}

		else if (opName=="issimple")
		{
			geom::Geometry *gT=gA;
			if ( ( opArg1 == "B" || opArg1 == "b" ) && gB ) gT=gB;

			if (gT->isSimple()) actual_result="true";
			else actual_result="false";

			if (actual_result==opRes) success=1;

		}

		else if (opName=="convexhull")
		{
			geom::Geometry *gT=gA;
			if ( ( opArg1 == "B" || opArg1 == "b" ) && gB ) gT=gB;

			GeomAutoPtr gRes(parseGeometry(opRes, "expected"));
			gRes->normalize();

			GeomAutoPtr gRealRes(gT->convexHull());
			gRealRes->normalize();

			if (gRes->compareTo(gRealRes.get())==0) success=1;

			if ( testValidOutput ) testValid(gRes.get(), "result");

			actual_result=printGeom(gRealRes.get());
			expected_result=printGeom(gRes.get());

		}

		else if (opName=="buffer")
		{
			geom::Geometry *gT=gA;
			if ( ( opArg1 == "B" || opArg1 == "b" ) && gB ) gT=gB;

			GeomAutoPtr gRes(parseGeometry(opRes, "expected"));
			gRes->normalize();

			profile.start();

			GeomAutoPtr gRealRes;
			double dist = std::atof(opArg3.c_str());

			if ( opArg2 != "" ) {
				gRealRes.reset(gT->buffer(dist, std::atoi(opArg2.c_str())));
			} else {
				gRealRes.reset(gT->buffer(dist));
			}
			profile.stop();
			gRealRes->normalize();

			// Assume a success and check for obvious failures
			success=1;
			do
			{
				// TODO: Is a buffer always an area ?
				// 	 we might check geometry type..

				if ( gRes->getGeometryTypeId() != gRealRes->getGeometryTypeId() )
				{
					std::cerr << "Expected result is of type "
					        << gRes->getGeometryType()
						<< "; obtained result is of type "
						<< gRealRes->getGeometryType()
						<< std::endl;
					success=0;
					break;
				}

				if ( gRes->isEmpty() && gRealRes->isEmpty() )
				{
					// Success !
					break;
				}

				if ( gRes->getDimension() != 2 )
				{
					std::cerr << "Don't know how to validate "
						<< "result of buffer operation "
						<< "when expected result is not an "
						<< "areal type."
						<< std::endl;
				}
				
				double expectedArea = gRes->getArea();

				/// Allow area difference being at most
				/// 1/1000 of the area of the expected result.
				double areatol = expectedArea / 1e3;

				GeomAutoPtr gDiff = BinaryOp(gRes.get(), gRealRes.get(),
					overlayOp(OverlayOp::opDIFFERENCE));

				double areaDiff = gDiff->getArea();
				if ( areaDiff > areatol )
				{
					std::cerr << "Area of difference between "
						<< "obtained and expected: "
						<< areaDiff << " - Tolerated diff: "
						<< areatol << std::endl;
					success=0;
					break;
				}
				else
				{
					std::cerr << "Area of difference between "
						<< "obtained and expected: "
						<< areaDiff << " - Tolerated diff: "
						<< areatol << " (SUCCESS!)"
						<< std::endl;
				}

			}
			while (0);

			if ( testValidOutput ) testValid(gRes.get(), "result");

			actual_result=printGeom(gRealRes.get());
			expected_result=printGeom(gRes.get());
		}

		else if (opName=="getinteriorpoint")
		{
			geom::Geometry *gT=gA;
			if ( ( opArg1 == "B" || opArg1 == "b" ) && gB ) gT=gB;

			GeomAutoPtr gRes(parseGeometry(opRes, "expected"));
			gRes->normalize();

			GeomAutoPtr gRealRes(gT->getInteriorPoint());
			if ( gRealRes.get() ) gRealRes->normalize();
			else gRealRes.reset(factory->createGeometryCollection());

			if (gRes->compareTo(gRealRes.get())==0) success=1;

			if ( testValidOutput ) testValid(gRes.get(), "result");

			actual_result=printGeom(gRealRes.get());
			expected_result=printGeom(gRes.get());

		}

		else if (opName=="iswithindistance")
		{
			float dist=std::atof(opArg3.c_str());
			if (gA->isWithinDistance(gB, dist)) {
				actual_result="true";
			} else {
				actual_result="false";
			}

			if (actual_result==opRes) success=1;

		}

		else if (opName=="polygonize")
		{

			GeomAutoPtr gRes(wktreader->read(opRes));
			gRes->normalize();

			Polygonizer plgnzr;
			plgnzr.add(gA);


            std::vector<geos::geom::Polygon *>*polys = plgnzr.getPolygons();
			std::vector<geom::Geometry *>*newgeoms = new std::vector<geom::Geometry *>;
			for (unsigned int i=0; i<polys->size(); i++)
				newgeoms->push_back((*polys)[i]);
			delete polys;

			GeomAutoPtr gRealRes(factory->createGeometryCollection(newgeoms));
			gRealRes->normalize();


			if (gRes->compareTo(gRealRes.get())==0) success=1;

			if ( testValidOutput ) testValid(gRes.get(), "result");

			actual_result=printGeom(gRealRes.get());
			expected_result=printGeom(gRes.get());

		}

		else if (opName=="linemerge")
		{
			GeomAutoPtr gRes(wktreader->read(opRes));
			gRes->normalize();

			geom::Geometry *gT=gA;

			if ( ( opArg1 == "B" || opArg1 == "b" ) && gB ) gT=gB;

			LineMerger merger;
			merger.add(gT);
			std::auto_ptr< std::vector<geom::LineString *> > lines ( merger.getMergedLineStrings() );
			std::vector<geom::Geometry *>*newgeoms = new std::vector<geom::Geometry *>(lines->begin(),
					lines->end());

			GeomAutoPtr gRealRes(factory->createGeometryCollection(newgeoms));
			gRealRes->normalize();

			if (gRes->compareTo(gRealRes.get())==0) success=1;

			if ( testValidOutput ) testValid(gRes.get(), "result");

			actual_result=printGeom(gRealRes.get());
			expected_result=printGeom(gRes.get());

		}

		else if (opName=="areatest")
		{
			char* rest;
			double toleratedDiff = std::strtod(opRes.c_str(), &rest);
			if ( rest == opRes.c_str() )
			{
				throw std::runtime_error("malformed testcase: missing tolerated area difference in 'areatest' op");
			}

			if ( verbose > 1 )
			{
		std::cerr << "Running intersection for areatest" << std::endl;
			}
			GeomAutoPtr gI = BinaryOp(gA, gB,
					overlayOp(OverlayOp::opINTERSECTION));

			if ( testValidOutput )
			{
				testValid(gI.get(), "areatest intersection");
			}

			if ( verbose > 1 )
			{
		std::cerr << "Running difference(A,B) for areatest" << std::endl;
			}
			GeomAutoPtr gDab = BinaryOp(gA, gB,
					overlayOp(OverlayOp::opDIFFERENCE));

			if ( verbose > 1 )
			{
		std::cerr << "Running difference(B,A) for areatest" << std::endl;
			}
			GeomAutoPtr gDba = BinaryOp(gB, gA,
					overlayOp(OverlayOp::opDIFFERENCE));

			if ( testValidOutput )
			{
				testValid(gI.get(), "areatest difference");
			}

			if ( verbose > 1 )
			{
		std::cerr << "Running symdifference for areatest" << std::endl;
			}
			GeomAutoPtr gSD = BinaryOp(gA, gB,
					overlayOp(OverlayOp::opSYMDIFFERENCE));

			if ( verbose > 1 )
			{
		std::cerr << "Running union for areatest" << std::endl;
			}
			GeomAutoPtr gU = BinaryOp(gA, gB,
					overlayOp(OverlayOp::opUNION));

			double areaA = gA->getArea();
			double areaB = gB->getArea();
			double areaI = gI->getArea(); 
			double areaDab = gDab->getArea();
			double areaDba = gDba->getArea();
			double areaSD = gSD->getArea();
			double areaU = gU->getArea();

			double maxdiff = 0;
			std::string maxdiffop;

			// @ : symdifference
			// - : difference
			// + : union
			// ^ : intersection
		
			// A == ( A ^ B ) + ( A - B )
			double diff = std::fabs ( areaA - areaI - areaDab );
			if ( diff > maxdiff ) {
				maxdiffop = "A == ( A ^ B ) + ( A - B )";
				maxdiff = diff;
			}

			// B == ( A ^ B ) + ( B - A )
			diff = std::fabs ( areaB - areaI - areaDba );
			if ( diff > maxdiff ) {
				maxdiffop = "B == ( A ^ B ) + ( B - A )";
				maxdiff = diff;
			}

			//  ( A @ B ) == ( A - B ) + ( B - A )
			diff = std::fabs ( areaDab + areaDba - areaSD );
			if ( diff > maxdiff ) {
				maxdiffop = "( A @ B ) == ( A - B ) + ( B - A )";
				maxdiff = diff;
			}

			//  ( A u B ) == ( A ^ B ) + ( A @ B )
			diff = std::fabs ( areaI + areaSD - areaU );
			if ( diff > maxdiff ) {
				maxdiffop = "( A u B ) == ( A ^ B ) + ( A @ B )";
				maxdiff = diff;
			}

			if ( maxdiff <= toleratedDiff )
			{
				success=1;
			}

			std::stringstream tmp;
			tmp << maxdiffop << ": " << maxdiff;
			actual_result=tmp.str();
			expected_result=opRes;

		}

		else
		{
			std::cerr << *curr_file << ":";
			std::cerr << " case" << caseCount << ":";
			std::cerr << " test" << testCount << ": "
					  << opName << "(" << opSig << ")";
			std::cerr << ": skipped (unrecognized)." << std::endl;
			return;
		}

	}
	catch (const std::exception &e)
	{
		std::cerr<<"EXCEPTION on case "<<caseCount
			<<" test "<<testCount<<": "<<e.what()
			<<std::endl;
		actual_result = e.what();
	}
	catch (...)
	{
		std::cerr<<"EXEPTION"<<std::endl;
		actual_result = "Unknown exception thrown";
	}

	if ( success ) ++succeeded;
	else ++failed;

	if ((!success && verbose) || verbose > 1)
	{
		printTest(success, expected_result, actual_result);
	}

	if (test_predicates && gB && gA) {
		runPredicates(gA, gB);
	}

	xml.OutOfElem();
}

void
XMLTester::runPredicates(const geom::Geometry *gA, const geom::Geometry *gB)
{
	std::cout << "\t    Equals:\tAB=" << (gA->equals(gB)?"T":"F") << ", BA=" << (gB->equals(gA)?"T":"F") << std::endl;
	std::cout << "\t  Disjoint:\tAB=" << (gA->disjoint(gB)?"T":"F") << ", BA=" << (gB->disjoint(gA)?"T":"F") << std::endl;
	std::cout << "\tIntersects:\tAB=" << (gA->intersects(gB)?"T":"F") << ", BA=" << (gB->intersects(gA)?"T":"F") << std::endl;
	std::cout << "\t   Touches:\tAB=" << (gA->touches(gB)?"T":"F") << ", BA=" << (gB->touches(gA)?"T":"F") << std::endl;
	std::cout << "\t   Crosses:\tAB=" << (gA->crosses(gB)?"T":"F") << ", BA=" << (gB->crosses(gA)?"T":"F") << std::endl;
	std::cout << "\t    Within:\tAB=" << (gA->within(gB)?"T":"F") << ", BA=" << (gB->within(gA)?"T":"F") << std::endl;
	std::cout << "\t  Contains:\tAB=" << (gA->contains(gB)?"T":"F") << ", BA=" << (gB->contains(gA)?"T":"F") << std::endl;
	std::cout << "\t  Overlaps:\tAB=" << (gA->overlaps(gB)?"T":"F") << ", BA=" << (gB->overlaps(gA)?"T":"F") << std::endl;
}

XMLTester::~XMLTester()
{
}


static void
usage(char *me, int exitcode, std::ostream &os)
{
	os << "Usage: " << me << " [options] <test> [<test> ...]" << std::endl;
	os << "Options: " << std::endl;
	os << " -v                  Verbose mode "
	   << "(multiple -v increment verbosity)" << std::endl
	   << "--test-valid-output  Test output validity" << std::endl
	   << "--test-valid-input   Test input validity" << std::endl
	   << "--sql-output         Produce SQL output" << std::endl
	   << "--wkb-output         Print Geometries as HEXWKB" << std::endl;

    std::exit(exitcode);
}

int
main(int argC, char* argV[])
{
	int verbose=0;
	bool sql_output=false;

#ifdef _MSC_VER
	InitAllocCheck();
	{
#endif

	if ( argC < 2 ) usage(argV[0], 1, std::cerr);

	XMLTester tester;
	tester.setVerbosityLevel(verbose);

	for (int i=1; i<argC; ++i)
	{
		// increment verbosity level
		if ( ! std::strcmp(argV[i], "-v" ) )
		{
			++verbose;
			tester.setVerbosityLevel(verbose);
			continue;
		}
		if ( ! std::strcmp(argV[i], "--test-valid-output" ) )
		{
			tester.testOutputValidity(true);
			continue;
		}
		if ( ! std::strcmp(argV[i], "--sql-output" ) )
		{
			sql_output = true;
			tester.setSQLOutput(sql_output);
			continue;
		}
		if ( ! std::strcmp(argV[i], "--wkb-output" ) )
		{
			sql_output = true;
			tester.setHEXWKBOutput(sql_output);
			continue;
		}
		if ( ! std::strcmp(argV[i], "--test-valid-input" ) )
		{
			tester.testInputValidity(true);
			continue;
		}

		std::string source = argV[i];
		tester.run(source);
	}

	if ( ! sql_output ) tester.resultSummary(std::cout);
	else tester.resultSummary(std::cerr);

	io::Unload::Release();

	return tester.getFailuresCount();

#ifdef _MSC_VER
	}
	DeInitAllocCheck();
#endif

}

/**********************************************************************
 * $Log: XMLTester.cpp,v $
 * Revision 1.38  2006/07/13 03:59:10  csavage
 * Changes to compile on VC++ - fully qualified polygon name.  Should also work on MingW, will test next.
 *
 * Revision 1.37  2006/06/19 20:48:35  strk
 * parseCase(): make sure to exit the <case> tag before returning
 *
 * Revision 1.36  2006/06/14 19:19:10  strk
 * Added support for "AreaTest" operations.
 *
 * Revision 1.35  2006/06/12 10:39:29  strk
 * don't print test file precision model if verbosity level < 2.
 *
 * Revision 1.34  2006/06/05 15:36:34  strk
 * Given OverlayOp funx code enum a name and renamed values to have a lowercase prefix. Drop all of noding headers from installed header set.
 *
 * Revision 1.33  2006/05/19 16:38:22  strk
 *         * tests/xmltester/XMLTester.cpp: report
 *         error on load of requested tests.
 *
 * Revision 1.32  2006/04/14 14:57:15  strk
 * XMLTester binary ops invoked using the new BinaryOp template function.
 *
 * Revision 1.31  2006/04/07 13:26:38  strk
 * Use of auto_ptr<> to prevent confusing leaks in tester
 *
 * Revision 1.30  2006/03/22 16:01:33  strk
 * indexBintree.h header split, classes renamed to match JTS
 *
 * Revision 1.29  2006/03/17 14:56:39  strk
 * Fixed filename normalizer for sql output
 **********************************************************************/

