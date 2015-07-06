// $Id: DoubleBitsTest.cpp 1820 2006-09-06 16:54:23Z mloskot $
// 
// Test Suite for geos::index::quadtree::DoubleBits class.

// TUT
#include <tut.h>
// GEOS
#include <geos/index/quadtree/DoubleBits.h>

using namespace geos::index::quadtree;

namespace tut
{
	// dummy data, not used
	struct test_doublebits_data {};

	typedef test_group<test_doublebits_data> group;
	typedef group::object object;

	group test_doublebits_group("geos::index::quadtree::DoubleBits");

	//
	// Test Cases
	//

	// 1 - Test exponent()
	template<>
	template<>
	void object::test<1>()
	{
		ensure_equals(DoubleBits::exponent(-1), 0);
		ensure_equals(DoubleBits::exponent(8.0), 3);
		ensure_equals(DoubleBits::exponent(128.0), 7);
	}


} // namespace tut

