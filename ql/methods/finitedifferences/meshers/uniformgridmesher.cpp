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
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/meshers/uniformgridmesher.hpp>

namespace QuantLib {

    UniformGridMesher::UniformGridMesher(
        const ext::shared_ptr<FdmLinearOpLayout> & layout,
        const std::vector<std::pair<Real, Real> > & boundaries)
    : FdmMesher (layout),
      dx_       (new Real[layout->dim().size()]),
      locations_(layout->dim().size()) {

        QL_REQUIRE(boundaries.size() == layout->dim().size(),
                   "inconsistent boundaries given");

        for (Size i=0; i < layout->dim().size(); ++i) {
            dx_[i] = (boundaries[i].second - boundaries[i].first)
                / (layout->dim()[i]-1);

            locations_[i] = std::vector<Real>(layout->dim()[i]);
            for (Size j=0; j < layout->dim()[i]; ++j) {
                locations_[i][j] = boundaries[i].first + j*dx_[i];
            }
        }
    }

    Array UniformGridMesher::locations(Size d) const {
        Array retVal(layout_->size());

        for (const auto& iter : *layout_) {
            retVal[iter.index()] = locations_[d][iter.coordinates()[d]];
        }

        return retVal;
    }
}
