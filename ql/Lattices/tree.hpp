
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

/*! \file tree.hpp
    \brief Tree class
*/

#ifndef quantlib_lattices_tree_h
#define quantlib_lattices_tree_h

#include <ql/numericalmethod.hpp>

namespace QuantLib {

    //! %Tree approximating a single-factor diffusion
    class Tree {
      public:
        Tree(Size nColumns) : nColumns_(nColumns) {}
        virtual ~Tree() {}
        virtual double underlying(Size i,
                                  Size index) const = 0;
        virtual Size size(Size i) const = 0;
        virtual Size descendant(Size i,
                                Size index,
                                Size branch) const = 0;
        virtual double probability(Size i,
                                   Size index,
                                   Size branch) const = 0;

        Size nColumns() const { return nColumns_; }
      private:
        Size nColumns_;
    };

}


#endif
