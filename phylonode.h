//
// C++ Interface: phylonode
//
// Description:
//
//
// Author: BUI Quang Minh, Steffen Klaere, Arndt von Haeseler <minh.bui@univie.ac.at>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef PHYLONODE_H
#define PHYLONODE_H

#include "node.h"

typedef unsigned short UBYTE;

/**
A neighbor in a phylogenetic tree

    @author BUI Quang Minh, Steffen Klaere, Arndt von Haeseler <minh.bui@univie.ac.at>
 */
class PhyloNeighbor : public Neighbor {
    friend class PhyloNode;
    friend class PhyloTree;
    friend class IQTree;
    friend class PhyloSuperTree;
    friend class MemSlotVector;

public:
    friend class TinaTree;
    friend class PhyloSuperTreePlen;

    /**
        construct class with a node and length		@param anode the other end of the branch

        @param alength length of branch
     */
    PhyloNeighbor(Node *anode, double alength) : Neighbor(anode, alength) {
        partial_lh = NULL;
        scale_num = NULL;
        partial_lh_computed = 0;
        lh_scale_factor = 0.0;
        partial_pars = NULL;
        size = 0;
    }

    /**
        construct class with a node and length
        @param anode the other end of the branch
        @param alength length of branch
        @param aid branch ID
     */
    PhyloNeighbor(Node *anode, double alength, int aid) : Neighbor(anode, alength, aid) {
        partial_lh = NULL;
        scale_num = NULL;
        partial_lh_computed = 0;
        lh_scale_factor = 0.0;
        partial_pars = NULL;
        size = 0;
    }

    /**
        tell that the partial likelihood vector is not computed
     */
    inline void clearPartialLh() {
        partial_lh_computed = 0;
    }

    /**
     *  tell that the partial likelihood vector is computed
     */
    inline void unclearPartialLh() {
        partial_lh_computed = 1;
    }

    /**
        clear all partial likelihood recursively in forward direction
        @param dad dad of this neighbor
     */
    void clearForwardPartialLh(Node *dad);

    /**
        DEPRECATED, moved to PhyloTree
        if partial_lh is NULL, reorient partial_lh (LM_PER_NODE technique)
        @param dad dad of this neighbor
    */
//    void reorientPartialLh(Node *dad);

	/**
	* For Upper Bounds analysis: get partial likelihood and lh scale factor
	*/
	double* get_partial_lh(){
	return partial_lh;
	}

	double get_lh_scale_factor(){
	return lh_scale_factor;
	}

	int get_partial_lh_computed(){
	return partial_lh_computed;
	}

    int getSize() {
        return size;
    }

private:

    /**
        true if the partial likelihood was computed
     */
    int partial_lh_computed;

    /**
        vector containing the partial likelihoods
     */
    double *partial_lh;

    /**
        likelihood scaling factor
     */
    double lh_scale_factor;

    /**
        vector containing number of scaling events per pattern // NEW!
     */
    UBYTE *scale_num;

    /**
        vector containing the partial parsimony scores
     */
    UINT *partial_pars;

    /** size of subtree below this neighbor in terms of number of taxa */
    int size;

};

/**
A node in a phylogenetic tree

    @author BUI Quang Minh, Steffen Klaere, Arndt von Haeseler <minh.bui@univie.ac.at>
 */
class PhyloNode : public Node {
    friend class PhyloTree;

public:
    /**
        constructor
     */
    PhyloNode();

    /**
        constructor
        @param aid id of this node
     */
    PhyloNode(int aid);

    /**
        constructor
        @param aid id of this node
        @param aname name of this node
     */
    PhyloNode(int aid, int aname);

    /**
        constructor
        @param aid id of this node
        @param aname name of this node
     */
    PhyloNode(int aid, const char *aname);

    /**
        initialization
     */
    void init();

    /**
        add a neighbor
        @param node the neighbor node
        @param length branch length
        @param id branch ID
     */
    virtual void addNeighbor(Node *node, double length, int id = -1);



    /**
        tell that all partial likelihood vectors below this node are not computed
     */
    void clearAllPartialLh(bool make_null, PhyloNode *dad);

    /**
        tell that all partial likelihood vectors (in reverse direction) below this node are not computed
     */
    void clearReversePartialLh(PhyloNode *dad);

    void computeReversePartialLh(PhyloNode *dad);

    /** 
        compute the size (#taxa) of the subtree rooted at this node
        using buffered 'size' attribute if computed beforehand
        @param dad dad of this node
    */
    int computeSize(Node *dad);

};


/**
    Node vector
 */
typedef vector<PhyloNode*> PhyloNodeVector;


#endif