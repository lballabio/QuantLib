
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

    #ifndef QL_DISABLE_DEPRECATED
    ExtendedDiscountCurve::ExtendedDiscountCurve(
                                 const Date &todaysDate,
                                 const std::vector<Date>& dates,
                                 const std::vector<DiscountFactor>& discounts,
                                 const Calendar& calendar,
                                 const BusinessDayConvention conv,
                                 const DayCounter& dayCounter)
    : DiscountCurve(todaysDate,dates,discounts,dayCounter),
      calendar_(calendar), conv_(conv) {
        calibrateNodes();
    }
    #endif

    ExtendedDiscountCurve::ExtendedDiscountCurve(
                                 const std::vector<Date>& dates,
                                 const std::vector<DiscountFactor>& discounts,
                                 const Calendar& calendar,
                                 const BusinessDayConvention conv)
    : DiscountCurve(dates,discounts),
      calendar_(calendar), conv_(conv) {
        calibrateNodes();
    }

    void ExtendedDiscountCurve::calibrateNodes() const {
        Size i;
        Integer ci;

        std::vector<Date> dates = dates_;
        std::vector<Time> times = times_;
        std::vector<Rate> discounts = discounts_;

        for (i = 0, ci = 1; i < dates.size(); i++) {
            Date rateDate = dates[i];
            Date tmpDate = calendar_.advance(referenceDate(),
                                             ci, Months, conv_);
            while (rateDate > tmpDate) {
                dates.insert(dates.begin() + i, tmpDate);
                Time t = Settings::instance().dayCounter().yearFraction(
                    referenceDate(),tmpDate);
                times.insert(times.begin() + i, t);
                discounts.insert(discounts.begin() + i,
                                 interpolation_(t,true));
                i++;
                tmpDate = calendar_.advance(referenceDate(),
                                            ++ci, Months, conv_);
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

    boost::shared_ptr<YieldTermStructure>
    ExtendedDiscountCurve::reversebootstrap(Integer compounding) const {
        std::vector<Rate> forwards;
        Date compoundDate = calendar_.advance(referenceDate(),
                                              12/compounding,
                                              Months, conv_);
        Time compoundTime = Settings::instance().dayCounter().yearFraction(
            referenceDate(), compoundDate);
        Real qFactor = 0.0;
        Size i;
        Integer ci;
        // Ignore first entry (SPOT with df=1.0)
        for (i = 1, ci = 1; i < dates_.size(); i++) {
            Rate fwd;
            Date rateDate = dates_[i];
            Time t = Settings::instance().dayCounter().yearFraction(
                referenceDate(),rateDate);
            DiscountFactor df = discount(t);
            if (t <= compoundTime) {
                fwd = ((1.0/df)-1.0)/t;
                qFactor = df*t;
            } else {
                Date tmpDate = calendar_.advance(referenceDate(),
                                                 (12/compounding) * (ci+1),
                                                 Months, conv_);
                Time tt = Settings::instance().dayCounter().yearFraction(
                    compoundDate, rateDate);
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
                        new CompoundForward(referenceDate(),
                                            dates_, forwards, calendar_,conv_,
                                            compounding,
                                            Settings::instance().dayCounter()));
    }

    Rate ExtendedDiscountCurve::compoundForwardImpl(Time t,
                                                    Integer compounding)
                                                                      const {
        if (compounding == 0)
            return DiscountCurve::compoundForwardImpl(t, compounding);
        return forwardCurve(compounding)->forwardRate(t, t,
            SimpleThenCompounded, Frequency(compounding), true);
    }

    boost::shared_ptr<YieldTermStructure>
    ExtendedDiscountCurve::forwardCurve(Integer compounding) const {
        if (forwardCurveMap_.find(compounding) == forwardCurveMap_.end())
            forwardCurveMap_[compounding] = reversebootstrap(compounding);
        return forwardCurveMap_[compounding];
    }

}
