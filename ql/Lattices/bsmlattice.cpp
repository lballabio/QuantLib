
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

#include <ql/Lattices/bsmlattice.hpp>

namespace QuantLib {

    BlackScholesLattice::BlackScholesLattice(
                                  const boost::shared_ptr<Tree>& tree,
                                  Rate riskFreeRate,
                                  Time end, Size steps)
    : Lattice(TimeGrid(end, steps), 2),
      tree_(tree), discount_(std::exp(-riskFreeRate*(end/steps))) {
        pd_ = tree->probability(0,0,0);
        pu_ = tree->probability(0,0,1);
    }

    void BlackScholesLattice::stepback(Size i, const Array& values,
                                       Array& newValues) const {
        for (Size j=0; j<size(i); j++)
            newValues[j] = (pd_*values[j] + pu_*values[j+1])*discount_;
    }

}
