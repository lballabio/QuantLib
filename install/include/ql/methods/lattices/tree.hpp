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

/*! \file tree.hpp
    \brief Tree class
*/

#ifndef quantlib_tree_hpp
#define quantlib_tree_hpp

#include <ql/types.hpp>
#include <ql/patterns/curiouslyrecurring.hpp>

namespace QuantLib {

    //! %Tree approximating a single-factor diffusion
    /*! Derived classes must implement the following interface:
        \code
        public:
          Real underlying(Size i, Size index) const;
          Size size(Size i) const;
          Size descendant(Size i, Size index, Size branch) const;
          Real probability(Size i, Size index, Size branch) const;
        \endcode
        and provide a public enumeration
        \code
        enum { branches = N };
        \endcode
        where N is a suitable constant (2 for binomial, 3 for trinomial...)

        \ingroup lattices
    */
    template <class T>
    class Tree : public CuriouslyRecurringTemplate<T> {
      public:
        explicit Tree(Size columns) : columns_(columns) {}
        Size columns() const { return columns_; }
      private:
        Size columns_;
    };

}


#endif
