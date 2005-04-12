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
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file bsmlattice.hpp
    \brief Binomial trees under the BSM model
*/

#ifndef quantlib_bsm_lattice_hpp
#define quantlib_bsm_lattice_hpp

#include <ql/Lattices/binomialtree.hpp>
#include <ql/Lattices/lattice1d.hpp>

namespace QuantLib {

    //! Simple binomial lattice approximating the Black-Scholes model
    /*! \ingroup lattices */
    template <class T>
    class BlackScholesLattice : public Lattice1D<BlackScholesLattice<T> > {
      public:
        BlackScholesLattice(const boost::shared_ptr<T>& tree,
                            Rate riskFreeRate,
                            Time end,
                            Size steps);

        Size size(Size i) const { return tree_->size(i); }
        DiscountFactor discount(Size, Size) const { return discount_; }

        void stepback(Size i, const Array& values, Array& newValues) const;

        Real underlying(Size i, Size index) const {
            return tree_->underlying(i, index);
        }
        Size descendant(Size i, Size index, Size branch) const {
            return tree_->descendant(i, index, branch);
        }
        Real probability(Size i, Size index, Size branch) const {
            return tree_->probability(i, index, branch);
        }
      private:
        boost::shared_ptr<T> tree_;
        DiscountFactor discount_;
        Real pd_, pu_;
    };


    // template definitions

    template <class T>
    BlackScholesLattice<T>::BlackScholesLattice(
                              const boost::shared_ptr<T>& tree,
                              Rate riskFreeRate, Time end, Size steps)
    : Lattice1D<BlackScholesLattice<T> >(TimeGrid(end, steps), 2),
      tree_(tree), discount_(std::exp(-riskFreeRate*(end/steps))) {
        pd_ = tree->probability(0,0,0);
        pu_ = tree->probability(0,0,1);
    }

    template <class T>
    void BlackScholesLattice<T>::stepback(Size i, const Array& values,
                                          Array& newValues) const {
        for (Size j=0; j<size(i); j++)
            newValues[j] = (pd_*values[j] + pu_*values[j+1])*discount_;
    }

}


#endif
