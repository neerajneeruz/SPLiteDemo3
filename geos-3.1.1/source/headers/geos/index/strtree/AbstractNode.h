/**********************************************************************
 * $Id: AbstractNode.h 1971 2007-02-07 00:34:26Z strk $
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

#ifndef GEOS_INDEX_STRTREE_ABSTRACTNODE_H
#define GEOS_INDEX_STRTREE_ABSTRACTNODE_H

#include <geos/index/strtree/Boundable.h> // for inheritance

#include <vector>

namespace geos {
namespace index { // geos::index
namespace strtree { // geos::index::strtree

/** \brief
 * A node of the STR tree.
 * 
 * The children of this node are either more nodes
 * (AbstractNodes) or real data (ItemBoundables).
 *
 * If this node contains real data (rather than nodes),
 * then we say that this node is a "leaf node".  
 *
 */
class AbstractNode: public Boundable {
private:
	std::vector<Boundable*> *childBoundables;
	int level;
public:
	AbstractNode(int newLevel, int capacity=10);
	virtual	~AbstractNode();
	inline std::vector<Boundable*>* getChildBoundables() {
		return childBoundables;
	}

	inline const std::vector<Boundable*>* getChildBoundables() const {
		return childBoundables;
	}

	/**
	 * Returns a representation of space that encloses this Boundable,
	 * preferably not much bigger than this Boundable's boundary yet fast to
	 * test for intersection with the bounds of other Boundables.
	 * The class of object returned depends on the subclass of
	 * AbstractSTRtree.
	 * 
	 * @return an Envelope (for STRtrees), an Interval (for SIRtrees),
	 *	or other object (for other subclasses of AbstractSTRtree)
	 *
	 * @see AbstractSTRtree::IntersectsOp
	 */  
	const void* getBounds() const;

	int getLevel();

	void addChildBoundable(Boundable *childBoundable);

protected:

	virtual void* computeBounds() const=0;

	mutable void* bounds;
};


} // namespace geos::index::strtree
} // namespace geos::index
} // namespace geos

#endif // GEOS_INDEX_STRTREE_ABSTRACTNODE_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/21 10:47:34  strk
 * indexStrtree.h split
 *
 **********************************************************************/

