
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file treeswaption.cpp
    \brief European swaption calculated using finite differences

    \fullpath
    ql/Pricers/%treeswaption.cpp
*/

// $Id$

#include "ql/Pricers/treeswaption.hpp"
#include "ql/Lattices/tree.hpp"

namespace QuantLib {

    namespace Pricers {

        using namespace InterestRateModelling;
        using namespace Lattices;

        TreeSwaption::TreeSwaption(
            bool payFixed,
            Exercise::Type exerciseType,
            const std::vector<Time>& maturities,
            Time start,
            const std::vector<Time>& payTimes,
            const std::vector<double>& coupons,
            double nominal,
            size_t timeSteps)
        : payFixed_(payFixed), exerciseType_(exerciseType), 
          maturities_(maturities), start_(start), payTimes_(payTimes), 
          coupons_(coupons), nominal_(nominal) {
            
            std::list<Time> times(0);
            size_t i;
            for (i=0; i<maturities.size(); i++)
                times.push_back(maturities[i]);
            for (i=0; i<payTimes.size(); i++)
                times.push_back(payTimes[i]);
            times.unique();
            times.sort();

            timeGrid_ = TimeGrid(times, timeSteps);
        }

        void TreeSwaption::calculate() {
            QL_REQUIRE(!model_.isNull(), "You must first define a model");
            Handle<Tree> tree(model_->tree(timeGrid_));

            unsigned int iEnd = timeGrid_.findIndex(payTimes_.back());
            int j, i;
            for (j=tree->jMin(iEnd); j<=tree->jMax(iEnd); j++)
                tree->node(iEnd, j).setValue(nominal_);

            for (i=(payTimes_.size() - 1); i>=0; i--) {
                unsigned int iStart = timeGrid_.findIndex(payTimes_[i]);
                tree->rollback(iEnd,iStart);
                for (int j=tree->jMin(iEnd); j<=tree->jMax(iEnd); j++) {
                    double value = tree->node(iEnd, j).value();
                    tree->node(iEnd, j).setValue(value + coupons_[i]);
                }
                iEnd = iStart;
            }
            unsigned int iStart = timeGrid_.findIndex(start_);
            tree->rollback(iEnd, iStart);
            iEnd = iStart;
            for (j=tree->jMin(iEnd); j<=tree->jMax(iEnd); j++) {
                double value = tree->node(iEnd, j).value() - nominal_;
                if (payFixed_)
                    value = -value;
                tree->node(iEnd, j).setValue(value);
            }
            iStart = timeGrid_.findIndex(maturities_.back());
            tree->rollback(iEnd, iStart);
            iEnd = iStart;
            value_ = 0.0;
            QL_REQUIRE(exerciseType_==Exercise::European,
                "Exercise type not supported");
            for (j=tree->jMin(iEnd); j<=tree->jMax(iEnd); j++) {
                double value = QL_MAX(tree->node(iEnd, j).value(), 0.0);
                value_ += value*tree->node(iEnd, j).statePrice();
            }

        }

    }

}
