/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Ralph Schreyer

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

/*! \file fdmarithmeticaveragecondition.cpp
    \brief step condition to handle arithmetic average
*/

#include <ql/math/interpolations/cubicinterpolation.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmarithmeticaveragecondition.hpp>
#include <utility>

namespace QuantLib {

    FdmArithmeticAverageCondition::FdmArithmeticAverageCondition(
        std::vector<Time> averageTimes,
        Real,
        Size pastFixings,
        const ext::shared_ptr<FdmMesher>& mesher,
        Size equityDirection)
    : x_(mesher->layout()->dim()[equityDirection]),
      a_(mesher->layout()->dim()[equityDirection == 0 ? 1 : 0]),
      averageTimes_(std::move(averageTimes)), pastFixings_(pastFixings), mesher_(mesher),
      equityDirection_(equityDirection) {

        QL_REQUIRE(mesher->layout()->dim().size()==2, "2D allowed only");
        QL_REQUIRE(equityDirection == 0 || equityDirection == 1,
                   "equityDirection has to be 0 or 1");

        const Size xSpacing = mesher_->layout()->spacing()[equityDirection];
        Array tmp = mesher_->locations(equityDirection);
        for (Size i = 0; i < x_.size(); ++i) {
            x_[i] = std::exp(tmp[i*xSpacing]);
        }
        const Size averageDirection = equityDirection == 0 ? 1 : 0;
        const Size aSpacing = mesher_->layout()->spacing()[averageDirection];
        tmp = mesher_->locations(averageDirection);
        for (Size i = 0; i < a_.size(); ++i) {
            a_[i] = std::exp(tmp[i*aSpacing]);
        }
    }

    void FdmArithmeticAverageCondition::applyTo(Array& a, Time t) const {
        QL_REQUIRE(mesher_->layout()->size() == a.size(),
                   "inconsistent array dimensions");

        const auto iter
            = std::find(averageTimes_.begin(), averageTimes_.end(), t);
        const Size nTimes
            = std::count(averageTimes_.begin(), averageTimes_.end(), t);
        if (nTimes > 0) {
            Array aCopy(a);
            const Size iT = iter - averageTimes_.begin() + 1 + pastFixings_;
            const Size averageDirection = equityDirection_ == 0 ? 1 : 0;
            const Size xSpacing = mesher_->layout()->spacing()[equityDirection_];
            const Size aSpacing = mesher_->layout()->spacing()[averageDirection];
            Array tmp(a_.size());

            for (Size i=0; i<x_.size(); ++i) {
                for (Size j=0; j<a_.size(); ++j) {
                    Size index = i*xSpacing + j*aSpacing;
                    tmp[j] = aCopy[index];
                }
                MonotonicCubicNaturalSpline interp(a_.begin(), a_.end(),
                                           tmp.begin());
                for (Size j=0; j<a_.size(); ++j) {
                    Size index = i*xSpacing + j*aSpacing;
                    a[index] = interp((iT-nTimes)/(double)(iT)*a_[j] +
                                      nTimes/(double)(iT)*x_[i], true);
                }
            }
        }
    }
}
