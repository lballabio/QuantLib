/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

/*! \file lattice1d.hpp
    \brief One-dimensional lattice class
*/

#ifndef quantlib_tree_lattice_1d_hpp
#define quantlib_tree_lattice_1d_hpp

#include <ql/methods/lattices/lattice.hpp>

namespace QuantLib {

    //! One-dimensional tree-based lattice.
    /*! Derived classes must implement the following interface:
        \code
        Real underlying(Size i, Size index) const;
        \endcode

        \ingroup lattices */
    template <class Impl>
    class TreeLattice1D : public TreeLattice<Impl> {
      public:
        TreeLattice1D(const TimeGrid& timeGrid, Size n)
        : TreeLattice<Impl>(timeGrid,n) {}
        Array grid(Time t) const override {
            Size i = this->timeGrid().index(t);
            Array grid(this->impl().size(i));
            for (Size j=0; j<grid.size(); j++)
                grid[j] = this->impl().underlying(i,j);
            return grid;
        }
        Real underlying(Size i, Size index) const {
            return this->impl().underlying(i,index);
        }
    };

}


#endif
