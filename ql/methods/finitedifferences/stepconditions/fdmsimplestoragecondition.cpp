/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011 Klaus Spanderen
 Copyright (C) 2014 Ralph Schreyer

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

#include <ql/math/interpolations/bilinearinterpolation.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmsimplestoragecondition.hpp>

namespace QuantLib {

    FdmSimpleStorageCondition::FdmSimpleStorageCondition(
            const std::vector<Time> & exerciseTimes,
            const ext::shared_ptr<FdmMesher>& mesher,
            const ext::shared_ptr<FdmInnerValueCalculator>& calculator,
            Real changeRate)
    : exerciseTimes_(exerciseTimes),
      mesher_       (mesher),
      calculator_   (calculator),
      changeRate_   (changeRate) {

        const ext::shared_ptr<FdmLinearOpLayout> layout = mesher_->layout();

        x_.reserve(layout->dim()[0]);
        y_.reserve(layout->dim()[1]);

        const FdmLinearOpIterator endIter = layout->end();
        for (FdmLinearOpIterator iter = layout->begin(); iter != endIter;
             ++iter) {
            if (iter.coordinates()[1] == 0U) {
                x_.push_back(mesher_->location(iter, 0));
            }
            if (iter.coordinates()[0] == 0U) {
                y_.push_back(mesher_->location(iter, 1));
            }
        }

    }

    void FdmSimpleStorageCondition::applyTo(Array& a, Time t) const {
        const std::vector<Time>::const_iterator iter
            = std::find(exerciseTimes_.begin(), exerciseTimes_.end(), t);

        if (iter != exerciseTimes_.end()) {
            Array retVal(a.size());

            Matrix m(y_.size(), x_.size());
            std::copy(a.begin(), a.end(), m.begin());
            BilinearInterpolation interpl(x_.begin(), x_.end(),
                                          y_.begin(), y_.end(), m);

            const ext::shared_ptr<FdmLinearOpLayout> layout=mesher_->layout();

            QL_REQUIRE(layout->size() == a.size(),
                       "inconsistent array dimensions");

            const FdmLinearOpIterator endIter = layout->end();

            for (FdmLinearOpIterator iter = layout->begin(); iter != endIter;
                 ++iter) {

                const std::vector<Size>& coor = iter.coordinates();
                const Real x = x_[coor[0]];
                const Real y = y_[coor[1]];

                const Real price = calculator_->innerValue(iter, t);

                const Real maxWithDraw = std::min(y-y_.front(), changeRate_);
                const Real sellPrice   = interpl(x, y-maxWithDraw);

                const Real maxInject = std::min(y_.back()-y, changeRate_);
                const Real buyPrice  = interpl(x, y+maxInject);

                // bang-bang-wait strategy
                Real currentValue = std::max(a[iter.index()],
                    std::max(buyPrice - price*maxInject,
                             sellPrice + price*maxWithDraw));

                // check if intermediate grid points give a better value
                std::vector<Real>::const_iterator yIter =
                    std::upper_bound(y_.begin(), y_.end(), y - maxWithDraw);

                while (yIter != y_.end() && *yIter < y + maxInject) {
                    if (*yIter != y) {
                        const Real change = *yIter - y;
                        const Real storagePrice(interpl(x, *yIter));

                        currentValue = std::max(currentValue,
                            storagePrice - change*price);
                    }
                    ++yIter;
                }

                retVal[iter.index()] = currentValue;
            }
            a = retVal;
        }
    }
}
