
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

/*! \file cap.cpp
    \brief European cap and floor class

    \fullpath
    ql/InterestRateModelling/CalibrationHelpers/%cap.hpp
*/

// $Id$

#include "ql/InterestRateModelling/CalibrationHelpers/cap.hpp"
#include "ql/CashFlows/floatingratecoupon.hpp"
#include "ql/Math/normaldistribution.hpp"

namespace QuantLib {

    namespace InterestRateModelling {

        namespace CalibrationHelpers {

            using CashFlows::FloatingRateCoupon;

            Cap::Cap(
                const Period& wait,
                const Period& tenor,
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
                Rate fixedRate = 0.04;//dummy value
                SimpleSwap swap(
                  false, 
                  termStructure->settlementDate().plus(wait.length(), wait.units()),
                  tenor.length(),
                  tenor.units(),
                  index->calendar(),
                  index->rollingConvention(),
                  std::vector<double>(1, 1.0),
                  frequency,
                  std::vector<double>(1, fixedRate),
                  false,
                  index->dayCounter(),
                  frequency,
                  index,
                  0,//FIXME
                  std::vector<double>(1, 0.0),
                  termStructure);
                Rate fairFixedRate = fixedRate - swap.NPV()/swap.fixedLegBPS();
                swap_ = Handle<SimpleSwap>(new SimpleSwap(
                  false, 
                  termStructure->settlementDate().plus(wait.length(), wait.units()),
                  tenor.length(),
                  tenor.units(),
                  index->calendar(),
                  index->rollingConvention(),
                  std::vector<double>(1, 1.0),
                  frequency,
                  std::vector<double>(1, fairFixedRate),
                  false,
                  index->dayCounter(),
                  frequency,
                  index,
                  0,//FIXME
                  std::vector<double>(1, 0.0),
                  termStructure));
                cap_ = Handle<EuropeanCap>(new EuropeanCap(
                  swap_,
                  std::vector<Rate>(1, exerciseRate_),
                  termStructure));

                std::vector<Handle<CashFlow> > floatingLeg = swap_->floatingLeg();
                std::vector<Handle<CashFlow> >::const_iterator begin, end;
                begin = floatingLeg.begin();
                end   = floatingLeg.end();
                Date today = termStructure_->minDate();
                DayCounter counter = termStructure_->dayCounter();
                unsigned int i=0;
                for (; begin != end; ++begin) {
                    const FloatingRateCoupon* coupon = 
                    #if QL_ALLOW_TEMPLATE_METHOD_CALLS
                        begin->downcast<FloatingRateCoupon>();
                    #else
                        dynamic_cast<const FloatingRateCoupon*>(begin->pointer());
                    #endif
                    QL_ENSURE(coupon != 0, "not a floating rate coupon");
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

            double Cap::value(const Handle<Model>& model) {
                cap_->useModel(model);
                cap_->recalculate();
                return cap_->NPV();
            }

            double Cap::blackPrice(double sigma) const {
                Math::CumulativeNormalDistribution f;
                double value = 0.0;
                for (unsigned i=0; i<nbOfPeriods_; i++) {
                    Time start = startTimes_[i];
                    Time end = endTimes_[i];
                    double tenor = end - start;
                    double p = termStructure_->discount(start);
                    double forward = QL_LOG(p/termStructure_->discount(end))/tenor;
                    double capletValue;
                    if (start > QL_EPSILON) {
                        double d1 = (QL_LOG(forward/exerciseRate_)+0.5*sigma*sigma*start)/(sigma*QL_SQRT(start));
                        double d2 = d1 - sigma*QL_SQRT(start);
                        capletValue = p*tenor*(forward*f(d1) - exerciseRate_*f(d2));
                    } else {
                        capletValue = p*tenor*QL_MAX(forward - exerciseRate_, 0.0);
                    }
                    value += capletValue;
                }
                return value;
            }
        }
    }
}
