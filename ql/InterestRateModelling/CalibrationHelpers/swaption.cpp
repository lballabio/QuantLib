
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

#include "ql/CashFlows/floatingratecoupon.hpp"
#include "ql/Math/normaldistribution.hpp"
#include "ql/InterestRateModelling/CalibrationHelpers/swaption.hpp"

namespace QuantLib {

    namespace InterestRateModelling {

        namespace CalibrationHelpers {

            using CashFlows::FloatingRateCoupon;
            using Instruments::EuropeanSwaption;
            using Instruments::SimpleSwap;

            Swaption::Swaption( 
                const Period& tenorPeriod,
                const Period& swapPeriod,
                const Handle<Indexes::Xibor>& index,
                Rate exerciseRate,
                const RelinkableHandle<TermStructure>& termStructure) :
                exerciseRate_(exerciseRate),
                termStructure_(termStructure) {

                Period indexTenor = index->tenor();
                int frequency;
                if (indexTenor.units() == Months)
                    frequency = 12/indexTenor.length();
                else if (indexTenor.units() == Years)
                    frequency = 1/indexTenor.length();
                else
                    throw Error("index tenor not valid!");
                Date startDate = termStructure->settlementDate().plus(
                    tenorPeriod.length(),
                    tenorPeriod.units());
                swap_ = Handle<SimpleSwap>(new SimpleSwap(
                  false, 
                  startDate,
                  swapPeriod.length(),
                  swapPeriod.units(),
                  index->calendar(),
                  index->rollingConvention(),
                  std::vector<double>(1, 1.0),
                  frequency,
                  std::vector<double>(1, exerciseRate_),
                  false,
                  index->dayCounter(),
                  frequency,
                  index,
                  0,//FIXME
                  std::vector<double>(1, 0.0),
                  termStructure));
                swaption_ = Handle<EuropeanSwaption>(new EuropeanSwaption(
                  swap_,
                  startDate,
                  termStructure));

                std::vector<Handle<CashFlow> > floatingLeg = 
                    swap_->floatingLeg();
                std::vector<Handle<CashFlow> >::const_iterator begin, end;
                begin = floatingLeg.begin();
                end   = floatingLeg.end();
                Date today = termStructure_->minDate();
                DayCounter counter = termStructure_->dayCounter();
                unsigned int i=0;
                for (; begin != end; ++begin) {
                    Handle<FloatingRateCoupon> coupon = *begin;
                    QL_ENSURE(!coupon.isNull(), 
                        "not a floating rate coupon");
                    Date beginDate = coupon->accrualStartDate();
                    Date endDate = coupon->date();
                    startTimes_.resize(i+1);
                    endTimes_.resize(i+1);
                    startTimes_[i] = counter.yearFraction(today, beginDate);
                    endTimes_[i] = counter.yearFraction(today, endDate);
                    i++;
                }
                nbOfPeriods_ = i;
            }

            double Swaption::value(const Handle<Model>& model) {
                swaption_->useModel(model);
                swaption_->recalculate();
                return swaption_->NPV();
            }

            double Swaption::blackPrice(double sigma) const {
                //FIXME: check
                double p = 0;
                for (unsigned i=0; i<nbOfPeriods_; i++)
                    p += termStructure_->discount(endTimes_[i]);
                Time start = startTimes_[0];
                double swapRate = exerciseRate_ - swap_->NPV()/swap_->fixedLegBPS();
                double value;
                if (start>0.0) {
                    Math::CumulativeNormalDistribution f;
                    double d1 = (QL_LOG(swapRate/exerciseRate_)+0.5*sigma*sigma*start)/(sigma*QL_SQRT(start));
                    double d2 = d1 - sigma*QL_SQRT(start);
                    value = p*(swapRate*f(d1) - exerciseRate_*f(d2));
                } else {
                    value = p*QL_MAX(swapRate - exerciseRate_, 0.0);
                }
                return value;
            }
        }
    }
}
