/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2012 Peter Caspers

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


/*! \file fdmtimedepdirichletboundary.cpp
*/

#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearop.hpp>
#include <ql/methods/finitedifferences/utilities/fdmindicesonboundary.hpp>
#include <ql/methods/finitedifferences/utilities/fdmtimedepdirichletboundary.hpp>
#include <algorithm>
#include <utility>

namespace QuantLib {

    FdmTimeDepDirichletBoundary::FdmTimeDepDirichletBoundary(
        const std::shared_ptr<FdmMesher>& mesher,
        std::function<Real(Real)> valueOnBoundary,
        Size direction,
        Side side)
    : indices_(FdmIndicesOnBoundary(mesher->layout(), direction, side).getIndices()),
      valueOnBoundary_(std::move(valueOnBoundary)), values_(indices_.size()) {}

    FdmTimeDepDirichletBoundary::FdmTimeDepDirichletBoundary(
        const std::shared_ptr<FdmMesher>& mesher,
        std::function<Array(Real)> valuesOnBoundary,
        Size direction,
        Side side)
    : indices_(FdmIndicesOnBoundary(mesher->layout(), direction, side).getIndices()),
      valuesOnBoundary_(std::move(valuesOnBoundary)), values_(indices_.size()) {}

    void FdmTimeDepDirichletBoundary::setTime(Time t) {
        if (!(valueOnBoundary_ == nullptr)) {
            std::fill(values_.begin(), values_.end(), valueOnBoundary_(t));
        } else if (!(valuesOnBoundary_ == nullptr)) {
            values_ = valuesOnBoundary_(t);
        } else {
            QL_FAIL("no boundary values defined");
        }
    }

    void FdmTimeDepDirichletBoundary::applyAfterApplying(array_type& a) const {
        QL_REQUIRE(indices_.size() == values_.size(),
                   "values on boundary size (" << values_.size()
                   << ") does not match hypersurface size ("
                   << indices_.size() << ")");
        for (auto iter = indices_.begin(); iter != indices_.end(); ++iter) {
            a[*iter] = values_[iter - indices_.begin()];
        }
    }

    void FdmTimeDepDirichletBoundary::applyAfterSolving(array_type& a) const {
        this->applyAfterApplying(a);
    }
}
