/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Andreas Gaida
 Copyright (C) 2008, 2009 Ralph Schreyer
 Copyright (C) 2008, 2009 Klaus Spanderen

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
#include <ql/methods/finitedifferences/stepconditions/fdmamericanstepcondition.hpp>
#include <utility>

namespace QuantLib {

    FdmAmericanStepCondition::FdmAmericanStepCondition(
        ext::shared_ptr<FdmMesher> mesher, ext::shared_ptr<FdmInnerValueCalculator> calculator)
    : mesher_(std::move(mesher)), calculator_(std::move(calculator)) {}

    void FdmAmericanStepCondition::applyTo(Array& a, Time t) const {
        QL_REQUIRE(mesher_->layout()->size() == a.size(),
                   "inconsistent array dimensions");

        for (const auto& iter : *mesher_->layout()) {
            const Real innerValue = calculator_->innerValue(iter, t);
            if (innerValue > a[iter.index()]) {
                a[iter.index()] = innerValue;
            }
        }
    }
}
