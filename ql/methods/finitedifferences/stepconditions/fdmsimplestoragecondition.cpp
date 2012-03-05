/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011 Klaus Spanderen

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
            const boost::shared_ptr<FdmMesher>& mesher,
            const boost::shared_ptr<FdmInnerValueCalculator>& calculator,
            Real changeRate)
    : exerciseTimes_(exerciseTimes),
      mesher_       (mesher),
      calculator_   (calculator),
      changeRate_   (changeRate) {

        const boost::shared_ptr<FdmLinearOpLayout> layout = mesher_->layout();

        x_.reserve(layout->dim()[0]);
        y_.reserve(layout->dim()[1]);

        const FdmLinearOpIterator endIter = layout->end();
        for (FdmLinearOpIterator iter = layout->begin(); iter != endIter;
             ++iter) {
            if (!iter.coordinates()[1]) {
                x_.push_back(mesher_->location(iter, 0));
            }
            if (!iter.coordinates()[0]) {
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

            const boost::shared_ptr<FdmLinearOpLayout> layout=mesher_->layout();
            const FdmLinearOpIterator endIter = layout->end();

            for (FdmLinearOpIterator iter = layout->begin(); iter != endIter;
                 ++iter) {

                const std::vector<Size>& coor = iter.coordinates();
                const Real x = x_[coor[0]];
                const Real y = y_[coor[1]];

                const Real price = calculator_->innerValue(iter, t);
                Real currentValue = a[iter.index()];

                // sell
                if (coor[1] > 0) {
                    const Real sellPrice = interpl(x, y-changeRate_);
                    currentValue
                        = std::max(currentValue, sellPrice + price*changeRate_);
                }
                // buy
                if (coor[1] < y_.size()-1) {
                    const Real buyPrice = interpl(x, y+changeRate_);
                    currentValue
                        = std::max(currentValue, buyPrice - price*changeRate_);
                }
                retVal[iter.index()] = currentValue;
            }
            a = retVal;
        }
    }
}
