
/*
 Copyright (C) 2002, 2003 Decillion Pty(Ltd)

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/TermStructures/extendeddiscountcurve.hpp>
#include <ql/TermStructures/compoundforward.hpp>

namespace QuantLib {

    ExtendedDiscountCurve::ExtendedDiscountCurve(
                                 const Date &todaysDate,
                                 const std::vector<Date>& dates,
                                 const std::vector<DiscountFactor>& discounts,
                                 const Calendar& calendar,
                                 const RollingConvention roll,
                                 const DayCounter& dayCounter)
    : DiscountCurve(todaysDate,dates,discounts,dayCounter),
      calendar_(calendar), roll_(roll) {
        calibrateNodes();
    }

    void ExtendedDiscountCurve::calibrateNodes() const {
        Size i;
        int ci;
	 
        std::vector<Date> dates = dates_;
        std::vector<Time> times = times_;
        std::vector<Rate> discounts = discounts_;
	 
        for (i = 0, ci = 1; i < dates.size(); i++) {
            Date rateDate = dates[i];
            Date tmpDate = calendar_.advance(referenceDate_,
                                             ci, Months, roll_);
            while (rateDate > tmpDate) {
                dates.insert(dates.begin() + i, tmpDate);
                Time t = dayCounter_.yearFraction(referenceDate_,tmpDate);
                times.insert(times.begin() + i, t);
                discounts.insert(discounts.begin() + i,
                                 interpolation_(t,true));
                i++;
                tmpDate = calendar_.advance(referenceDate_,
                                            ++ci, Months, roll_);
            }
            if (tmpDate == rateDate)
                ci++;
        }
        dates_ = dates;
        times_ = times;
        discounts_ = discounts;

        interpolation_ = LogLinearInterpolation(times_.begin(), times_.end(),
                                                discounts_.begin());
    }

    boost::shared_ptr<TermStructure> 
    ExtendedDiscountCurve::reversebootstrap(int compounding) const {
        std::vector<Rate> forwards;
        Date compoundDate = calendar_.advance(referenceDate_,
                                              12/compounding,
                                              Months, roll_);
        Time compoundTime = dayCounter_.yearFraction(referenceDate_,
                                                     compoundDate);
        double qFactor = 0.0;
        Size i;
        int ci;
        // Ignore first entry (SPOT with df=1.0)
        for (i = 1, ci = 1; i < dates_.size(); i++) {
            Rate fwd;
            Date rateDate = dates_[i];
            Time t = dayCounter_.yearFraction(referenceDate_,rateDate);
            DiscountFactor df = discount(t);
            if (t <= compoundTime) {
                fwd = ((1.0/df)-1.0)/t;
                qFactor = df*t;
            } else {
                Date tmpDate = calendar_.advance(referenceDate_,
                                                 (12/compounding) * (ci+1),
                                                 Months, roll_);
                Time tt = dayCounter_.yearFraction(compoundDate,
                                                   rateDate);
                fwd = (1.0-df)/(qFactor+df*tt);
                // Rates on non-compounding boundaries?
                if (rateDate >= tmpDate) {
                    ci++;
                    qFactor += df*tt;
                    compoundDate = tmpDate;
                }
            }
            forwards.push_back(fwd);
        }
        forwards.insert(forwards.begin(),forwards[0]);

        return boost::shared_ptr<CompoundForward>(
                        new CompoundForward(todaysDate_, referenceDate_,
                                            dates_, forwards, calendar_,roll_,
                                            compounding, dayCounter_));
    }

    Rate ExtendedDiscountCurve::compoundForwardImpl(Time t, int compounding, 
                                                    bool extrapolate) const {
        if (compounding == 0)
            return DiscountCurve::compoundForwardImpl(t, compounding,
                                                      extrapolate);
        return forwardCurve(compounding)->compoundForward(t, compounding,
                                                          extrapolate);
    }

    boost::shared_ptr<TermStructure> 
    ExtendedDiscountCurve::forwardCurve(int compounding) const {
        if (forwardCurveMap_.find(compounding) == forwardCurveMap_.end())
            forwardCurveMap_[compounding] = reversebootstrap(compounding);
        return forwardCurveMap_[compounding];
    }

}
