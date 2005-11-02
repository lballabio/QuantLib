/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

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

#include <ql/FiniteDifferences/onefactoroperator.hpp>

#if !defined(QL_PATCH_BORLAND)

namespace QuantLib {

    OneFactorOperator::OneFactorOperator(
          const Array& grid,
          const boost::shared_ptr<OneFactorModel::ShortRateDynamics>& process)
    : TridiagonalOperator(grid.size()) {
        timeSetter_ = boost::shared_ptr<TimeSetter>(
                 new TimeSetter(grid, process));
    }

    OneFactorOperator::TimeSetter::TimeSetter(const Array &grid,
         const boost::shared_ptr<OneFactorModel::ShortRateDynamics>& dynamics)
    : grid_(grid), pde_(dynamics) {}

    void OneFactorOperator::TimeSetter::setTime(
                                            Time t,
                                            TridiagonalOperator& op) const {
        pde_.generateOperator(t, grid_, op);
    }
}

#endif

