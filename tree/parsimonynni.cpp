//
//  parsimonynni.cpp
//  Parsimony NNI implementation.
//  Created by James Barbetti on 30-Nov-2020.
//

#include <placement/blockallocator.h>
#include <placement/targetbranch.h>
#include <placement/parallelparsimonycalculator.h>
#include <utils/timekeeper.h>
#include "phylonode.h" //for GET_OTHER_ADJACENT_PHYLO_NODES
#include "phylotreethreadingcontext.h"
#include "parsimonymove.h"
#include "parsimonysearch.h"

class ParsimonyNNIMove : public ParsimonyMove {
public:
    typedef ParsimonyMove super;
    PhyloNode*  left;
    PhyloBranch middle;
    PhyloNode*  right;
    ParsimonyNNIMove(): super(), left(nullptr),
                        middle(nullptr, nullptr), right(nullptr) {
    }
    ParsimonyNNIMove(const ParsimonyNNIMove& rhs)
       : super(rhs), left(rhs.left),
         middle(rhs.middle), right(rhs.right) {
    }
    ParsimonyNNIMove& operator=(const ParsimonyNNIMove& rhs) {
        super::operator=(rhs);
        left   = rhs.left;
        middle = rhs.middle;
        right  = rhs.right;
        return *this;
    }
    virtual ~ParsimonyNNIMove() {
    }
    virtual void initialize(intptr_t source_branch, bool beLazy) {
        source_branch_id = source_branch;
        lazy             = beLazy;
        benefit          = -1.0;
    }
    virtual std::string getDescription() const {
        std::stringstream s;
        s << " exchanging neighbours of branch " << source_branch_id;
        return s.str();
    }
    void consider(int branch_id, PhyloNode* leftNode,
                  const PhyloBranch& middleBranch,
                  PhyloNode* rightNode, double move_benefit) {
        if (benefit < move_benefit) {
            source_branch_id = branch_id;
            left             = leftNode;
            middle           = middleBranch;
            right            = rightNode;
            benefit          = move_benefit;
        }
    }
    PhyloNode* getOtherLeftNode() const {
        FOR_EACH_ADJACENT_PHYLO_NODE(middle.first, left, it, node) {
            if (node!=middle.second) {
                return node;
            }
        }
        return nullptr;
    }
    PhyloNode* getOtherRightNode() const {
        FOR_EACH_ADJACENT_PHYLO_NODE(middle.second, right, it, node) {
            if (node!=middle.first) {
                return node;
            }
        }
        return nullptr;
    }
    static intptr_t getParsimonyVectorSize(intptr_t radius) {
        return 2;
    }
    static intptr_t getMinimumPathVectorCount() {
        return 2;
    }
    virtual void   findMove(const PhyloTree& tree, /* problem, not const */
                            const TargetBranchRange& branches, /* not TargetBranchRange */
                            int /*radius*/ /* ignored; just part of signature */,
                            std::vector<UINT*> &path_parsimony,
                            double parsimony_score) {
        const PhyloBranch& tb = branches[source_branch_id];
        if ( tb.first->degree()!=3) {
            return;
        }
        if (tb.second->degree()!=3 ) {
            return;
        }
        PhyloNode* left1;
        PhyloNode* left2;
        GET_OTHER_ADJACENT_PHYLO_NODES(tb.first, tb.second,
                                       left1, left2);
        PhyloNode* right1;
        PhyloNode* right2;
        GET_OTHER_ADJACENT_PHYLO_NODES(tb.second, tb.first,
                                       right1, right2);
        double cost1 = ParallelParsimonyCalculator::parsimonyLink4CostOutOfTree
                       ( tree, left1, right1, tb.first, tb.second,
                         left2, right2,
                         path_parsimony[0], path_parsimony[1]);
        TREE_LOG_LINE(tree, VB_DEBUG, "for " << source_branch_id
                      << " cost1 " << cost1 << ","
                      << " oldcost " << parsimony_score );
        consider(source_branch_id, left1, tb, right2, parsimony_score - cost1);
        
        double cost2 = ParallelParsimonyCalculator::parsimonyLink4CostOutOfTree
                       ( tree, left1, right2, tb.first,
                         tb.second, left2, right1,
                         path_parsimony[0], path_parsimony[1]);
        TREE_LOG_LINE(tree, VB_DEBUG, "for " << source_branch_id
                      << " cost2 " << cost2 << ","
                      << " oldcost " << parsimony_score );
        consider(source_branch_id, left1, tb, right1, parsimony_score - cost2);
        positions_considered += 2;
    }
    virtual void finalize(PhyloTree& tree,
                  const TargetBranchRange& branches) {
    }
    virtual bool isStillPossible(const TargetBranchRange& branches,
                                 PhyloBranchVector& path) const {
        path.clear();
        if (middle.first->hasNeighbor(left)
            && middle.first->hasNeighbor(middle.second)
            && middle.second->hasNeighbor(right)) {
            path.emplace_back(left, middle.first);
            path.emplace_back(middle.first, middle.second);
            path.emplace_back(middle.second, right);
            return true;
        }
        return false;
    }
    virtual double recalculateBenefit
                   ( PhyloTree& tree, double tree_parsimony_score,
                     TargetBranchRange& branches,
                     LikelihoodBlockPairs &blocks,
                     std::vector< std::vector<UINT*> >& parsimony_path_vectors) const {
        ParallelParsimonyCalculator ppc(tree, false);
        
        PhyloNode* left2  = getOtherLeftNode();
        PhyloNode* right2 = getOtherRightNode();
        std::vector<UINT*>& buffer1 = parsimony_path_vectors[0];
        std::vector<UINT*>& buffer2 = parsimony_path_vectors[1];
        
        double swap_cost;
        swap_cost = ppc.parsimonyLink4Cost(right, left2, middle.first,
                                           middle.second, left, right2,
                                           buffer1[0], buffer2[0]);
        double noswap_cost;
        noswap_cost = ppc.parsimonyLink4Cost(right, left, middle.first,
                                             middle.second, left2, right2,
                                             buffer1[1], buffer2[1]);
        TREE_LOG_LINE(tree, VB_DEBUG, "branch " << source_branch_id << ","
                 << " swap cost "    << swap_cost << ","
                 << " no-swap cost " << noswap_cost );
        double improvement = noswap_cost - swap_cost;
        return improvement;

    }
    virtual double apply(PhyloTree& tree,
                         double parsimony_score,
                         TargetBranchRange& branches,
                         LikelihoodBlockPairs blocks,
                         std::vector< std::vector<UINT*> >& parsimony_path_vectors) {
        double improvement = recalculateBenefit(tree, parsimony_score,
                                                branches, blocks,
                                                parsimony_path_vectors);
        TREE_LOG_LINE(tree, VB_MAX, "branch " << source_branch_id
                 << " had predicted benefit " << benefit
                 << ", and actual improvement  " << improvement);
        if (improvement<0)
        {
            return parsimony_score;
        }

        //Swap inward neighbours
        left->updateNeighbor (middle.first,  middle.second);
        right->updateNeighbor(middle.second, middle.first);

        //Swap outward neighbours and views (still up-to-date!)
        middle.first->updateNeighbor (left,  right);
        middle.second->updateNeighbor(right, left);
        std::swap(middle.first->findNeighbor (right)->partial_pars,
                  middle.second->findNeighbor(left )->partial_pars);
        std::swap(middle.first->findNeighbor (right)->id,
                  middle.second->findNeighbor(left )->id);
        //Swap branch ids too.

        //Replace views on the middle branch with those we calculated
        //when figuring out swap_cost.
        std::vector<UINT*>& buffer1 = parsimony_path_vectors[0];
        PhyloNeighbor* nei_to_left = middle.second->findNeighbor(middle.first);
        std::swap(nei_to_left->partial_pars, buffer1[0]);
        nei_to_left->setParsimonyComputed(true);
        
        std::vector<UINT*>& buffer2 = parsimony_path_vectors[1];
        PhyloNeighbor* nei_to_right = middle.first->findNeighbor(middle.second);
        std::swap(nei_to_right->partial_pars, buffer2[0]);
        nei_to_right->setParsimonyComputed(true);
        
        parsimony_score -= improvement;
        
        //Mark inward views as out of date
        middle.first->clearReversePartialParsimony (middle.second);
        middle.second->clearReversePartialParsimony(middle.first);
        
        
        struct Adjuster {
            void setParsimonyLength(PhyloTree& tree, double parsimony, PhyloNode* node1) {
                FOR_EACH_ADJACENT_PHYLO_NODE(node1, nullptr, it, node2) {
                    PhyloNeighbor* leftNei  = node1->findNeighbor(node2);
                    PhyloNeighbor* rightNei = node2->findNeighbor(node1);
                    double branch_cost = parsimony
                                       - tree.getSubTreeParsimony(leftNei)
                                       - tree.getSubTreeParsimony(rightNei);
                    if (branch_cost<1) {
                        branch_cost = 1;
                    }
                    double branch_length = branch_cost / tree.getAlnNSite();
                    leftNei->length = rightNei->length = branch_length;
                }
            }
        } adj;
        adj.setParsimonyLength(tree, parsimony_score, middle.first);
        adj.setParsimonyLength(tree, parsimony_score, middle.second);
        
        std::swap(left, right);
        
        auto id_1             = left->findNeighbor(middle.first)->id;
        branches[id_1].first  = left;
        branches[id_1].second = middle.first;

        auto id_2             = right->findNeighbor(middle.second)->id;
        branches[id_2].first  = right;
        branches[id_2].second = middle.second;
        
        return parsimony_score;
    }
};

void PhyloTree::doParsimonyNNI() {
    ParsimonySearchParameters s;
        
    s.name                       = "NNI";
    s.iterations                 = params->parsimony_nni_iterations;
    s.lazy_mode                  = false;
    s.radius                     = -1;
    s.calculate_connection_costs = false;
    doParsimonySearch<ParsimonyNNIMove>(s);
}
