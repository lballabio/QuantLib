
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

/*! \file lattice2d.hpp
    \brief Two-dimensional lattice class
*/

#ifndef quantlib_lattices_lattice2d_h
#define quantlib_lattices_lattice2d_h

#include <ql/Lattices/lattice.hpp>
#include <ql/Lattices/trinomialtree.hpp>
#include <ql/Math/matrix.hpp>

namespace QuantLib {

    //! Two-dimensional lattice.
    /*! This lattice is based on two trinomial trees and primarly used
        for the G2 short-rate model.

        \ingroup lattices
    */
    class Lattice2D : public Lattice {
      public:
        Lattice2D(const boost::shared_ptr<TrinomialTree>& tree1,
                  const boost::shared_ptr<TrinomialTree>& tree2,
                  double correlation);

        Size size(Size i) const { return tree1_->size(i)*tree2_->size(i); }
      protected:
        Size descendant(Size i, Size index, Size branch) const;
        double probability(Size i, Size index, Size branch) const;

        boost::shared_ptr<Tree> tree1_, tree2_;
      private:
        Matrix m_;
        double rho_;
    };

}


#endif
