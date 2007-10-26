/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003 Decillion Pty(Ltd)

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

#include <ql/termstructures/yield/compoundforward.hpp>
#include <ql/termstructures/yield/extendeddiscountcurve.hpp>

namespace QuantLib {

    CompoundForward::CompoundForward(const Date & referenceDate,
                                     const std::vector<Date> &dates,
                                     const std::vector<Rate> &forwards,
                                     const Calendar & calendar,
                                     const BusinessDayConvention conv,
                                     const Integer compounding,
                                     const DayCounter & dayCounter)
    : ForwardRateStructure(referenceDate, calendar, dayCounter),
      conv_(conv), compounding_(compounding),
      needsBootstrap_(true), dates_(dates), forwards_(forwards) {
        QL_REQUIRE(!dates_.empty(), "no input dates given");
        QL_REQUIRE(!forwards_.empty(), "no input rates given");
        QL_REQUIRE(dates_.size() == forwards_.size(),
                   "inconsistent number of dates/forward rates");
        calibrateNodes();
    }

    void CompoundForward::calibrateNodes() const {
        Size i;
        Integer ci;

        times_.resize(dates_.size());
        for (i = 0; i < dates_.size(); i++)
            times_[i] = dayCounter().yearFraction(referenceDate(),dates_[i]);
        fwdinterp_ = LinearInterpolation(times_.begin(), times_.end(),
                                         forwards_.begin());
        fwdinterp_.update();

        std::vector<Date> dates = dates_;
        std::vector<Time> times = times_;
        std::vector<Rate> forwards = forwards_;

        for (i = 0, ci = 1; i < dates.size(); i++) {
            Date rateDate = dates[i];
            Date tmpDate = calendar().advance(referenceDate(),
                                             ci, Months, conv_);
            while (rateDate > tmpDate) {
                dates.insert(dates.begin() + i, tmpDate);
                Time t = dayCounter().yearFraction(referenceDate(),tmpDate);
                times.insert(times.begin() + i, t);
                forwards.insert(forwards.begin() + i,
                                fwdinterp_(t,true));
                i++;
                tmpDate = calendar().advance(referenceDate(),
                                            ++ci, Months, conv_);
            }
            if (tmpDate == rateDate)
                ci++;
        }
        dates_ = dates;
        times_ = times;
        forwards_ = forwards;

        if (dates_[0] != referenceDate()) {
            dates_.insert(dates_.begin(),referenceDate());
            times_.insert(times_.begin(), 0.0);
            forwards_.insert(forwards_.begin(), forwards_[0]);
        }

        fwdinterp_ = LinearInterpolation(times_.begin(), times_.end(),
                                         forwards_.begin());
        fwdinterp_.update();
    }

    boost::shared_ptr<YieldTermStructure> CompoundForward::bootstrap() const {
        needsBootstrap_ = false;
        QL_REQUIRE(compounding_ > 0,
                   "continuous compounding needs no bootstrap.");
        try {
            std::vector<DiscountFactor> discounts;
            Date compoundDate = calendar().advance(referenceDate(),
                                                  12/compounding_,
                                                  Months, conv_);
            Time compoundTime = dayCounter().yearFraction(referenceDate(),
                                                         compoundDate);
            Real qFactor = 0.0;
            Size i;
            Integer ci;
            for (i = 0, ci = 1; i < dates_.size(); i++) {
                DiscountFactor df;
                Date rateDate = dates_[i];
                Time t = dayCounter().yearFraction(referenceDate(),rateDate);
                Rate r = forwardImpl(t);
                if (t <= compoundTime) {
                    df = 1.0/(1.0+r*t);
                    qFactor = df*t;
                } else {
                    Date tmpDate =
                        calendar().advance(referenceDate(),
                                          (12/compounding_) * (ci+1),
                                          Months, conv_);
                    Time tt = dayCounter().yearFraction(compoundDate,
                                                       rateDate);
                    df = (1.0-qFactor*r)/(1.0+r*tt);
                    if (rateDate >= tmpDate) {
                        ci++;
                        qFactor += df*tt;
                        compoundDate = tmpDate;
                    }
                }
                discounts.push_back(df);
            }
            discountCurve_ = boost::shared_ptr<ExtendedDiscountCurve>(
                     new ExtendedDiscountCurve(dates_, discounts,
                                               calendar(), conv_,
                                               dayCounter()));
        }
        catch (std::exception& ) {
            // signal incomplete state
            needsBootstrap_ = true;
            // rethrow
            throw;
        }
        return discountCurve_;
    }

    Rate CompoundForward::zeroYieldImpl(Time t) const {
        if (compounding_ == 0)
            return ForwardRateStructure::zeroYieldImpl(t);
        if (needsBootstrap_)
            bootstrap();
        //return discountCurve()->zeroYield(t,true);
        return discountCurve()->zeroRate(t, Continuous, NoFrequency, true);
    }

    DiscountFactor CompoundForward::discountImpl(Time t) const {
        if (compounding_ == 0)
            return ForwardRateStructure::discountImpl(t);
        if (needsBootstrap_)
            bootstrap();
        return discountCurve()->discount(t,true);
    }

    Size CompoundForward::referenceNode(Time t) const {
        if (t >= times_.back())
            return times_.size()-1;
        std::vector<Time>::const_iterator i=times_.begin(),
            j=times_.end(), k;
        while (j-i > 1) {
            k = i+(j-i)/2;
            if (t <= *k)
                j = k;
            else
                i = k;
        }
        return (j-times_.begin());
    }

    Rate CompoundForward::forwardImpl(Time t) const {
        if (t == 0.0) {
            return forwards_[0];
        } else {
            Size n = referenceNode(t);
            if (t == times_[n]) {
                return forwards_[n];
            } else {
                return fwdinterp_(t,true);
            }
        }
    }

    Rate CompoundForward::compoundForwardImpl(Time t, Integer f) const {
        if (f == compounding_)
            return forwardImpl(t);
        if (needsBootstrap_)
            bootstrap();
        return discountCurve()->compoundForward(t,f,true);
    }

    boost::shared_ptr<ExtendedDiscountCurve>
    CompoundForward::discountCurve() const {
        QL_REQUIRE(compounding_ > 0,
                   "continuous compounding needs no bootstrap.");
        if (needsBootstrap_)
            bootstrap();
        return discountCurve_;
    }

}
