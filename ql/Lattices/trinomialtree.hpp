
/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file trinomialtree.hpp
    \brief Trinomial tree class
*/

#ifndef quantlib_lattices_trinomial_tree_h
#define quantlib_lattices_trinomial_tree_h

#include <ql/stochasticprocess.hpp>
#include <ql/Lattices/tree.hpp>

namespace QuantLib {

    class TrinomialBranching;

    //! Recombining trinomial tree class
    /*! This class defines a recombining trinomial tree approximating a
        a diffusion.
        \warning The diffusion term of the SDE must be independent of the
                 underlying process.
    */
    class TrinomialTree : public Tree {
      public:
        TrinomialTree(const boost::shared_ptr<StochasticProcess>& process,
                      const TimeGrid& timeGrid,
                      bool isPositive = false);
        double dx(Size i) const { return dx_[i]; }
        Size size(Size i) const;
        double underlying(Size i, Size index) const;
        const TimeGrid& timeGrid() const { return timeGrid_; }

        Size descendant(Size i, Size index, Size branch) const;
        double probability(Size i, Size index, Size branch) const;

      protected:
        std::vector<boost::shared_ptr<TrinomialBranching> > branchings_;
        double x0_;
        std::vector<double> dx_;
        TimeGrid timeGrid_;
    };

    //! Branching scheme for a trinomial node
    /*! Each node has three descendants, with the middle branch linked
        to the node which is closest to the expectation of the variable.
    */
    class TrinomialBranching {
      public:
        TrinomialBranching() : probs_(3) {}
        virtual ~TrinomialBranching() {}

        Size descendant(Size index, Size branch) const;
        double probability(Size index, Size branch) const;
        int jMin() const;
      private:
        friend class TrinomialTree;

        std::vector<int> k_;
        std::vector<std::vector<double> > probs_;
    };

    inline 
    Size TrinomialTree::descendant(Size i, Size index, Size branch) const {
        return branchings_[i]->descendant(index, branch);
    }

    inline
    double TrinomialTree::probability(Size i, Size j, Size b) const {
        return branchings_[i]->probability(j, b);
    }

    inline 
    Size TrinomialBranching::descendant(Size index, Size branch) const {
        return (k_[index] - jMin()) - 1 + branch;
    }

    inline 
    double TrinomialBranching::probability(Size index, Size branch) const {
        return probs_[branch][index];
    }

    inline int TrinomialBranching::jMin() const {
        return *std::min_element(k_.begin(), k_.end()) - 1;
    }

}


#endif
