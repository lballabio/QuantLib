/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file lattice1d.hpp
    \brief One-dimensional lattice class
*/

#ifndef quantlib_lattice1d_hpp
#define quantlib_lattice1d_hpp

#include <ql/Lattices/lattice.hpp>

namespace QuantLib {

    //! One-dimensional lattice.
    /*! Derived classes must implement the following interface:
        \code
        Real underlying(Size i, Size index) const;
        \endcode

        \ingroup lattices */
    template <class Impl>
    class Lattice1D : public Lattice<Impl> {
      public:
        Lattice1D(const TimeGrid& timeGrid, Size n)
        : Lattice<Impl>(timeGrid,n) {}
        Disposable<Array> grid(Time t) const {
            Size i = this->timeGrid().findIndex(t);
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
