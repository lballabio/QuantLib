
/*
  Copyright (C) 2002, 2003 Decillion Pty(Ltd).

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

#include <ql/TermStructures/compoundforward.hpp>
#include <ql/TermStructures/extendeddiscountcurve.hpp>

namespace QuantLib {

    CompoundForward::CompoundForward(const Date & todaysDate,
                                     const Date & referenceDate,
                                     const std::vector<Date> &dates,
                                     const std::vector<Rate> &forwards,
                                     const Calendar & calendar,
                                     const RollingConvention roll,
                                     const int compounding,
                                     const DayCounter & dayCounter)
    : todaysDate_(todaysDate), referenceDate_(referenceDate),
      dayCounter_(dayCounter), calendar_(calendar), roll_(roll),
      compounding_(compounding), needsBootstrap_(true),
      dates_(dates), forwards_(forwards) {
        QL_REQUIRE(dates_.size() > 0, 
                   "CompoundForward::CompoundForward : "
                   "no input dates given");
        QL_REQUIRE(forwards_.size() > 0, 
                   "CompoundForward::CompoundForward : "
                   "no input rates given");
        QL_REQUIRE(dates_.size() == forwards_.size(),
                   "CompoundForward::CompoundForward : "
                   "inconsistent number of dates/forward rates");
        calibrateNodes();
    }

    void CompoundForward::calibrateNodes() const {
        Size i;
        int ci;

        times_.resize(dates_.size());
        for (i = 0; i < dates_.size(); i++)
            times_[i] = dayCounter_.yearFraction(referenceDate_,dates_[i]);
        fwdinterp_ = LinearInterpolation(times_.begin(), times_.end(),
                                         forwards_.begin());
	 
        std::vector<Date> dates = dates_;
        std::vector<Time> times = times_;
        std::vector<Rate> forwards = forwards_;
	 
        for (i = 0, ci = 1; i < dates.size(); i++) {
            Date rateDate = dates[i];
            Date tmpDate = calendar_.advance(referenceDate_,
                                             ci, Months, roll_);
            while (rateDate > tmpDate) {
                dates.insert(dates.begin() + i, tmpDate);
                Time t = dayCounter_.yearFraction(referenceDate_,tmpDate);
                times.insert(times.begin() + i, t);
                forwards.insert(forwards.begin() + i,
                                fwdinterp_(t,true));
                i++;
                tmpDate = calendar_.advance(referenceDate_,
                                            ++ci, Months, roll_);
            }
            if (tmpDate == rateDate)
                ci++;
        }
        dates_ = dates;
        times_ = times;
        forwards_ = forwards;

        if (dates_[0] != referenceDate_) {
            dates_.insert(dates_.begin(),referenceDate_);
            times_.insert(times_.begin(), 0.0);
            forwards_.insert(forwards_.begin(), forwards_[0]);
        }
	 
        fwdinterp_ = LinearInterpolation(times_.begin(), times_.end(),
                                         forwards_.begin());
    }

    boost::shared_ptr<TermStructure> CompoundForward::bootstrap() const {
        needsBootstrap_ = false;
        QL_REQUIRE(compounding_ > 0,
                   "Continuous compounding needs no bootstrap.");
        try {
            std::vector<DiscountFactor> discounts;
            Date compoundDate = calendar_.advance(referenceDate_,
                                                  12/compounding_,
                                                  Months, roll_);
            Time compoundTime = dayCounter_.yearFraction(referenceDate_,
                                                         compoundDate);
            double qFactor = 0.0;
            Size i;
            int ci;
            for (i = 0, ci = 1; i < dates_.size(); i++) {
                DiscountFactor df;
                Date rateDate = dates_[i];
                Time t = dayCounter_.yearFraction(referenceDate_,rateDate);
                Rate r = instantaneousForward(t);
                if (t <= compoundTime) {
                    df = 1.0/(1.0+r*t);
                    qFactor = df*t;
                } else {
                    Date tmpDate = 
                        calendar_.advance(referenceDate_,
                                          (12/compounding_) * (ci+1),
                                          Months, roll_);
                    Time tt = dayCounter_.yearFraction(compoundDate,
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
            discountCurve_ = boost::shared_ptr<DiscountCurve>(
                     new ExtendedDiscountCurve(todaysDate_, dates_, discounts,
                                               calendar_, roll_, dayCounter_));
        }
        catch(std::exception& e) {
            // signal incomplete state
            needsBootstrap_ = true;
            // rethrow
            QL_FAIL("Could not bootstrap curve." +
                    std::string(e.what()));
        }
        return discountCurve_;
    }

    Rate CompoundForward::zeroYieldImpl(Time t, bool extrapolate) const {
        if (compounding_ == 0)
            return ForwardRateStructure::zeroYieldImpl(t,extrapolate);
        if (needsBootstrap_)
            bootstrap();
        return discountCurve()->zeroYield(t,extrapolate);
    }

    DiscountFactor CompoundForward::discountImpl(Time t,
                                                 bool extrapolate) const {
        if (compounding_ == 0)
            return ForwardRateStructure::discountImpl(t,extrapolate);
        if (needsBootstrap_)
            bootstrap();
        return discountCurve()->discount(t,extrapolate);
    }

    Size CompoundForward::referenceNode(Time t, bool extrapolate) const {
        QL_REQUIRE(t >= 0.0 && (t <= times_.back() || extrapolate),
                   "CompoundForward: time (" +
                   DoubleFormatter::toString(t) +
                   ") outside curve definition [" +
                   DoubleFormatter::toString(0.0) + ", " +
                   DoubleFormatter::toString(times_.back()) + "]");
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

    Rate CompoundForward::forwardImpl(Time t, bool extrapolate) const {
        if (t == 0.0) {
            return forwards_[0];
        } else {
            Size n = referenceNode(t, extrapolate);
            if (t == times_[n]) {
                return forwards_[n];
            } else {
                return fwdinterp_(t,extrapolate);
            }
        }
        QL_DUMMY_RETURN(Rate());
    }

    Rate CompoundForward::compoundForwardImpl(Time t, int f, 
                                              bool extrapolate) const {
        if (f == compounding_)
            return forwardImpl(t,extrapolate);
        if (needsBootstrap_)
            bootstrap();
        return discountCurve()->compoundForward(t,f,extrapolate);
    }

    boost::shared_ptr<TermStructure> CompoundForward::discountCurve() const {
        QL_REQUIRE(compounding_ > 0,
                   "Continuous compounding needs no bootstrap.");
        if (needsBootstrap_)
            bootstrap();
        return discountCurve_;
    }

}
