
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

/*! \file bsmlattice.hpp
    \brief Binomial trees under the BSM model
*/

#ifndef quantlib_lattices_bsm_lattice_h
#define quantlib_lattices_bsm_lattice_h

#include <ql/Lattices/tree.hpp>
#include <ql/Lattices/lattice.hpp>

namespace QuantLib {

    //! Simple binomial lattice approximating the Black-Scholes model
    class BlackScholesLattice : public Lattice {
      public:
        BlackScholesLattice(const boost::shared_ptr<Tree>& tree, 
                            Rate riskFreeRate, 
                            Time end, 
                            Size steps);

        Size size(Size i) const { return tree_->size(i); }
        DiscountFactor discount(Size i, Size j) const { return discount_; }

        const boost::shared_ptr<Tree>& tree() const { return tree_; }
      protected:
        void stepback(Size i, const Array& values, Array& newValues) const;

        Size descendant(Size i, Size index, Size branch) const {
            return tree_->descendant(i, index, branch);
        }
        double probability(Size i, Size index, Size branch) const {
            return tree_->probability(i, index, branch);
        }
      private:
        boost::shared_ptr<Tree> tree_;
        DiscountFactor discount_;
        double pd_, pu_;
    };

}


#endif
