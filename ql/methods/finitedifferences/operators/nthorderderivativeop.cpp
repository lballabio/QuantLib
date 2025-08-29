/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2018 Klaus Spanderen

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

/*! \file NthOrderDerivativeOp.cpp
    \brief n-th order derivative linear operator
*/

#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/operators/numericaldifferentiation.hpp>
#include <ql/methods/finitedifferences/operators/nthorderderivativeop.hpp>

#include <set>

namespace QuantLib {

    NthOrderDerivativeOp::NthOrderDerivativeOp(
        Size direction, Size order, Integer nPoints,
        const ext::shared_ptr<FdmMesher>& mesher)
    : m_(mesher->layout()->size(), mesher->layout()->size()) {

        const Integer hPoints = nPoints/2;
        const bool isEven = (nPoints == 2*hPoints);

        Array xValues = mesher->locations(direction);
        std::set<Real> tmp(xValues.begin(), xValues.end());
        xValues = Array(tmp.begin(), tmp.end()); //unique vector

        const Integer nx(mesher->layout()->dim()[direction]);

        QL_REQUIRE(Integer(xValues.size()) == nx,
            "inconsistent set of grid values in direction " << direction);

        QL_REQUIRE(nPoints > 1 && Integer(nPoints) <= nx,
             "inconsistent number of points");

        Array xOffsets(nPoints);
        const std::function<Real(Real)> emptyFct;

        for (const auto& iter : *mesher->layout()) {
            const auto ix = Integer(iter.coordinates()[direction]);
            const Integer offset = std::max(0, hPoints - ix)
                - std::max(0, hPoints - (nx-((isEven)? 0 : 1) - ix));

            const Integer ilx = ix - hPoints + offset;

            for (Integer j=0; j < nPoints; ++j) {
                const Integer idx = ilx + j;
                xOffsets[j] = xValues[idx] - xValues[ix];
            }

            const Array weights =
                NumericalDifferentiation(emptyFct, order, xOffsets).weights();

            const Size i = iter.index();
            for (Integer j=0; j < nPoints; ++j) {
                const Size k = mesher->layout()->neighbourhood(iter, direction, ilx - ix + j);

                m_(i, k) = weights[j];
            }
        }
    }

    NthOrderDerivativeOp::array_type NthOrderDerivativeOp::apply(const array_type& r) const {
        return prod(m_, r);
    }


    SparseMatrix NthOrderDerivativeOp::toMatrix() const {
        return m_;
    }

}

