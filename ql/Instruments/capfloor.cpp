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

/*! \file capfloor.cpp
    \brief European cap and floor class

    \fullpath
    ql/Instruments/%capfloor.cpp
*/

// $Id$

#include "ql/Instruments/capfloor.hpp"
#include "ql/CashFlows/floatingratecoupon.hpp"
#include "ql/Math/normaldistribution.hpp"

#include "ql/InterestRateModelling/onefactormodel.hpp"
#include "ql/Lattices/tree.hpp"


namespace QuantLib {

    namespace Instruments {

        using CashFlows::FloatingRateCoupon;
        using InterestRateModelling::OneFactorModel;
        using Lattices::TimeGrid;
        using Lattices::Tree;

        EuropeanCapFloor::EuropeanCapFloor( Type type, 
          const Handle<SimpleSwap>& swap, 
          std::vector<Rate> exerciseRates,
          RelinkableHandle<TermStructure> termStructure) 
        : type_(type), swap_(swap), exerciseRates_(exerciseRates), 
          termStructure_(termStructure) {
            std::vector<Handle<CashFlow> > floatingLeg = swap_->floatingLeg();
            std::vector<Handle<CashFlow> >::const_iterator begin, end;
            begin = floatingLeg.begin();
            end   = floatingLeg.end();
            NPV_ = 0.0;
            Date today = termStructure_->minDate();
            DayCounter counter = termStructure_->dayCounter();
            Size i=0;
            for (; begin != end; ++begin) {
                Handle<FloatingRateCoupon> coupon = *begin;
                QL_ENSURE(!coupon.isNull(), "not a floating rate coupon");
                Date beginDate = coupon->accrualStartDate();
                Date endDate = coupon->date();
                startTimes_.resize(i+1);
                endTimes_.resize(i+1);
                nominals_.resize(i+1);
                tenors_.resize(i+1);
                startTimes_[i] = counter.yearFraction(today, beginDate);
                endTimes_[i] = counter.yearFraction(today, endDate);

                times_.push_back(startTimes_[i]);
                times_.push_back(endTimes_[i]);
                                                         
                nominals_[i] = coupon->nominal();
                tenors_[i] = counter.yearFraction(beginDate, endDate);
                i++;
            }
            times_.unique();
            times_.sort();
            nPeriods_ = i;
        }

        void EuropeanCapFloor::performCalculations() const {
            QL_REQUIRE(!model_.isNull(), "Cannot price without model!");

            double temp = 
                model_->discountBondOption(Option::Call, 0.9, 1.0, 2.0);

            bool hasAnalyticalFormula;
            Handle<Tree> tree;
            if (temp==Null<double>()) {
                QL_REQUIRE(
                    model_->type() == InterestRateModelling::Model::OneFactor,
                    "Analytical formulas required for n-factor models n>1");
                Handle<OneFactorModel> model(model_);
                TimeGrid timeGrid(times_, 100);
                tree = model->tree(timeGrid);
                hasAnalyticalFormula = false;
            } else
                hasAnalyticalFormula = true;

            Option::Type optionType;
            if (type_==Cap)
                optionType = Option::Put;
            else
                optionType = Option::Call;

            NPV_ = 0.0;

            for (unsigned i=0; i<nPeriods_; i++) {
                Rate exerciseRate;
                if (i<exerciseRates_.size())
                    exerciseRate = exerciseRates_[i];
                else
                    exerciseRate = exerciseRates_.back();
                double optionStrike = 1.0/(1.0+exerciseRate*tenors_[i]);
                double optionValue = 0.0;

                Time maturity = startTimes_[i];
                Time bond = endTimes_[i];

                if (hasAnalyticalFormula)
                    optionValue = model_->discountBondOption( optionType, 
                        optionStrike, maturity, bond);
                else {
                    unsigned int iMid = tree->timeGrid().findIndex(maturity);
                    unsigned int iEnd = tree->timeGrid().findIndex(bond);

                    int j;
                    for (j=tree->jMin(iEnd); j<=tree->jMax(iEnd); j++)
                        tree->node(iEnd, j).setValue(1.0);
                    tree->rollback(iEnd, iMid);
                    for (j=tree->jMin(iMid); j<=tree->jMax(iMid); j++) {
                        double value = tree->node(iMid, j).value();
                        switch(optionType) {
                          case Option::Call:
                              value = QL_MAX(value - optionStrike, 0.0);
                              break;
                          case Option::Put:
                              value = QL_MAX(optionStrike - value, 0.0);
                              break;
                        }
                        optionValue += value*tree->node(iMid, j).statePrice();
                    }
                }

                double capletValue = nominals_[i]*
                    (1.0+exerciseRate*tenors_[i])*optionValue;
                NPV_ += capletValue;
            }
        }

    }

}
