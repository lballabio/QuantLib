/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2005 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file lattice2d.hpp
    \brief Two-dimensional lattice class
*/

#ifndef quantlib_tree_lattice_2d_hpp
#define quantlib_tree_lattice_2d_hpp

#include <ql/math/matrix.hpp>
#include <ql/methods/lattices/lattice.hpp>
#include <ql/methods/lattices/trinomialtree.hpp>
#include <utility>

namespace QuantLib {

    //! Two-dimensional tree-based lattice.
    /*! This lattice is based on two trinomial trees and primarily used
        for the G2 short-rate model.

        \ingroup lattices
    */
    template <class Impl, class T = TrinomialTree>
    class TreeLattice2D : public TreeLattice<Impl> {
      public:
        TreeLattice2D(const ext::shared_ptr<T>& tree1, ext::shared_ptr<T> tree2, Real correlation);

        Size size(Size i) const;
        Size descendant(Size i, Size index, Size branch) const;
        Real probability(Size i, Size index, Size branch) const;
      protected:
        ext::shared_ptr<T> tree1_, tree2_;
        // smelly
        Disposable<Array> grid(Time) const override { QL_FAIL("not implemented"); }

      private:
        Matrix m_;
        Real rho_;
    };


    // inline definitions

    template <class Impl, class T>
    inline Size TreeLattice2D<Impl,T>::size(Size i) const {
        return tree1_->size(i)*tree2_->size(i);
    }


    // template definitions

    template <class Impl, class T>
    TreeLattice2D<Impl, T>::TreeLattice2D(const ext::shared_ptr<T>& tree1,
                                          ext::shared_ptr<T> tree2,
                                          Real correlation)
    : TreeLattice<Impl>(tree1->timeGrid(), T::branches * T::branches), tree1_(tree1),
      tree2_(std::move(tree2)), m_(T::branches, T::branches), rho_(std::fabs(correlation)) {

        // what happens here?
        if (correlation < 0.0 && T::branches == 3) {
            m_[0][0] = -1.0;
            m_[0][1] = -4.0;
            m_[0][2] =  5.0;
            m_[1][0] = -4.0;
            m_[1][1] =  8.0;
            m_[1][2] = -4.0;
            m_[2][0] =  5.0;
            m_[2][1] = -4.0;
            m_[2][2] = -1.0;
        } else {
            m_[0][0] =  5.0;
            m_[0][1] = -4.0;
            m_[0][2] = -1.0;
            m_[1][0] = -4.0;
            m_[1][1] =  8.0;
            m_[1][2] = -4.0;
            m_[2][0] = -1.0;
            m_[2][1] = -4.0;
            m_[2][2] =  5.0;
        }
    }


    template <class Impl, class T>
    Size TreeLattice2D<Impl,T>::descendant(Size i, Size index,
                                           Size branch) const {
        Size modulo = tree1_->size(i);

        Size index1 = index % modulo;
        Size index2 = index / modulo;
        Size branch1 = branch % T::branches;
        Size branch2 = branch / T::branches;

        modulo = tree1_->size(i+1);
        return tree1_->descendant(i, index1, branch1) +
            tree2_->descendant(i, index2, branch2)*modulo;
    }

    template <class Impl, class T>
    Real TreeLattice2D<Impl,T>::probability(Size i, Size index,
                                            Size branch) const {
        Size modulo = tree1_->size(i);

        Size index1 = index % modulo;
        Size index2 = index / modulo;
        Size branch1 = branch % T::branches;
        Size branch2 = branch / T::branches;

        Real prob1 = tree1_->probability(i, index1, branch1);
        Real prob2 = tree2_->probability(i, index2, branch2);
        // does the 36 below depend on T::branches?
        return prob1*prob2 + rho_*(m_[branch1][branch2])/36.0;
    }

}


#endif


#ifndef id_bb09947f714338164de816203511551b
#define id_bb09947f714338164de816203511551b
inline bool test_bb09947f714338164de816203511551b(const int* i) {
    return i != nullptr;
}
#endif
