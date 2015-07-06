// $Id: SegmentStringTest.cpp 2229 2008-11-26 19:41:44Z pramsey $
// 
// Test Suite for geos::noding::SegmentString class.

// TUT
#include <tut.h>
// GEOS
#include <geos/noding/SegmentString.h>
#include <geos/noding/Octant.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateArraySequenceFactory.h>
#include <memory>

namespace tut
{
    //
    // Test Group
    //

    // Common data used by tests
    struct test_segmentstring_data
    {

    	typedef std::auto_ptr<geos::geom::CoordinateSequence> \
		CoordinateSequenceAutoPtr;

    	typedef std::auto_ptr<geos::noding::SegmentString> \
		SegmentStringAutoPtr;
	
	const geos::geom::CoordinateSequenceFactory* csFactory;

	SegmentStringAutoPtr
	makeSegmentString(geos::geom::CoordinateSequence* cs, void *d = 0)
	{
		return SegmentStringAutoPtr(
			new geos::noding::SegmentString(cs, d)
			);
	}

	test_segmentstring_data()
		:
		csFactory(geos::geom::CoordinateArraySequenceFactory::instance())
	{
	}
	
	~test_segmentstring_data()
	{
	}

    };

    typedef test_group<test_segmentstring_data> group;
    typedef group::object object;

    group test_segmentstring_group("geos::noding::SegmentString");

    //
    // Test Cases
    //

    // test constructor with 2 equal points
    template<>
    template<>
    void object::test<1>()
    {
    	CoordinateSequenceAutoPtr cs(csFactory->create(0, 2));

	ensure(cs.get());

	geos::geom::Coordinate c0(0, 0);
	geos::geom::Coordinate c1(0, 0);

	cs->add(c0);
	cs->add(c1);

	ensure_equals(cs->size(), 2u);

	SegmentStringAutoPtr ss(makeSegmentString(cs.get()));
	ensure(ss.get());

	ensure_equals(ss->size(), 2u);

	ensure_equals(ss->getData(), (void*)0);

	ensure_equals(ss->getCoordinates(), cs.get());

	ensure_equals(ss->getCoordinate(0), c0);

	ensure_equals(ss->getCoordinate(1), c1);

	ensure_equals(ss->isIsolated(), false);

	ensure_equals(ss->isClosed(), true);

	// this would throw an exception
	bool octant_failed=false;
	try {
		ss->getSegmentOctant(0);
	} catch (...) {
		octant_failed=true;
	}
	ensure(octant_failed);

	ensure_equals(ss->getNodeList().size(), 0u);

	ss->testInvariant();
    }

    // test constructor with 2 different points
    template<>
    template<>
    void object::test<2>()
    {
    	CoordinateSequenceAutoPtr cs(csFactory->create(0, 2));

	ensure(cs.get());

	geos::geom::Coordinate c0(0, 0);
	geos::geom::Coordinate c1(1, 0);

	cs->add(c0);
	cs->add(c1);

	ensure_equals(cs->size(), 2u);

	SegmentStringAutoPtr ss(makeSegmentString(cs.get()));
	ensure(ss.get());

	ensure_equals(ss->size(), 2u);

	ensure_equals(ss->getData(), (void*)0);

	ensure_equals(ss->getCoordinates(), cs.get());

	ensure_equals(ss->getCoordinate(0), c0);

	ensure_equals(ss->getCoordinate(1), c1);

	ensure_equals(ss->isIsolated(), false);

	ensure_equals(ss->isClosed(), false);

	ensure_equals(ss->getSegmentOctant(0), 0);

	ensure_equals(ss->getNodeList().size(), 0u);

	ss->testInvariant();
    }

    // test constructor with 4 different points forming a ring
    template<>
    template<>
    void object::test<3>()
    {
    	CoordinateSequenceAutoPtr cs(csFactory->create(0, 2));

	ensure(cs.get());

	geos::geom::Coordinate c0(0, 0);
	geos::geom::Coordinate c1(1, 0);
	geos::geom::Coordinate c2(1, 1);

	cs->add(c0);
	cs->add(c1);
	cs->add(c2);
	cs->add(c0);

	ensure_equals(cs->size(), 4u);

	SegmentStringAutoPtr ss(makeSegmentString(cs.get()));
	ensure(ss.get());

	ensure_equals(ss->size(), 4u);

	ensure_equals(ss->getData(), (void*)0);

	ensure_equals(ss->getCoordinates(), cs.get());

	ensure_equals(ss->getCoordinate(0), c0);

	ensure_equals(ss->getCoordinate(1), c1);

	ensure_equals(ss->getCoordinate(2), c2);

	ensure_equals(ss->getCoordinate(3), c0);

	ensure_equals(ss->isIsolated(), false);

	ensure_equals(ss->isClosed(), true);

	ensure_equals(ss->getSegmentOctant(2), 4);

	ensure_equals(ss->getSegmentOctant(1), 1);

	ensure_equals(ss->getSegmentOctant(0), 0);

	ensure_equals(ss->getNodeList().size(), 0u);

	ss->testInvariant();
    }

    // test adding intersections 
    template<>
    template<>
    void object::test<4>()
    {
	geos::geom::Coordinate p0(0, 0);
	geos::geom::Coordinate p1(10, 0);


    	CoordinateSequenceAutoPtr cs(csFactory->create(0, 2));
	cs->add(p0);
	cs->add(p1);

	SegmentStringAutoPtr ss(makeSegmentString(cs.get()));

	ensure_equals(ss->getNodeList().size(), 0u);

	// the intersection is invalid, but SegmentString trusts us
	ss->addIntersection(p0, 0);
	ensure_equals(ss->getNodeList().size(), 1u);

	// This node is already present, so shouldn't be
	// accepted as a new one
	ss->addIntersection(p0, 0);
	ensure_equals(ss->getNodeList().size(), 1u);

	ss->addIntersection(p1, 0);
	ensure_equals(ss->getNodeList().size(), 2u);

	ss->addIntersection(p1, 0);
	ensure_equals(ss->getNodeList().size(), 2u);

	ss->addIntersection(p0, 0);
	ensure_equals(ss->getNodeList().size(), 2u);

    }

	// test Octant class
	template<>
	template<>
	void object::test<5>()
	{
		geos::geom::Coordinate p0(0, 0);
		geos::geom::Coordinate p1(5,-5);
		int octant_rc1 = 0;
		int octant_rc2 = 0;
		int testPassed = true;
		try {
			octant_rc1 = geos::noding::Octant::octant(p0,p1);
			octant_rc2 = geos::noding::Octant::octant(&p0,&p1);
			testPassed = (octant_rc1 == octant_rc2);
		}
		catch (...) {
			testPassed = false;
		}
		ensure(testPassed);
	}


} // namespace tut

