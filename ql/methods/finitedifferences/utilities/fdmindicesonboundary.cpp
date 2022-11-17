/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Andreas Gaida
 Copyright (C) 2008 Ralph Schreyer
 Copyright (C) 2008 Klaus Spanderen

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


#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/utilities/fdmindicesonboundary.hpp>

namespace QuantLib {
    FdmIndicesOnBoundary::FdmIndicesOnBoundary(
        const ext::shared_ptr<FdmLinearOpLayout>& layout,
        Size direction, FdmDirichletBoundary::Side side) {

        std::vector<Size> newDim(layout->dim());
        newDim[direction] = 1;
        const Size hyperSize
            = std::accumulate(newDim.begin(), newDim.end(), Size(1), std::multiplies<>());
        indices_.resize(hyperSize);

        Size i=0;
        const FdmLinearOpIterator endIter = layout->end();
        for (FdmLinearOpIterator iter = layout->begin(); iter != endIter;
            ++iter) {
            if (   (   side == FdmDirichletBoundary::Lower
                    && iter.coordinates()[direction] == 0)
                || (   side == FdmDirichletBoundary::Upper
                    && iter.coordinates()[direction]
                                            == layout->dim()[direction]-1)) {
                QL_REQUIRE(hyperSize > i, "index missmatch");
                indices_[i++] = iter.index();
            }
        }


    }

    const std::vector<Size>& FdmIndicesOnBoundary::getIndices() const {
        return indices_;
    }
}
