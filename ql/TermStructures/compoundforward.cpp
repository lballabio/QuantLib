
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
				     const DayCounter & dayCounter,
				     const int granularity)
	:todaysDate_(todaysDate),
	 referenceDate_(referenceDate),
	 dayCounter_(dayCounter),
	 calendar_(calendar), roll_(roll),
	 compounding_(compounding),
	 granularity_(granularity),
	 needsBootstrap_(true),
	 dates_(dates), forwards_(forwards) {
	    QL_REQUIRE(dates_.size() > 0, "CompoundForward::CompoundForward : "
		       "No input Dates given");
	    QL_REQUIRE(forwards_.size() > 0, "CompoundForward::CompoundForward : "
		       "No input rates given");
	    QL_REQUIRE(dates_.size() == forwards_.size(),
		       "CompoundForward::CompoundForward : "
		       "Inconsistent number of Dates/Forward Rates");
	    times_.resize(dates_.size());
	    Size i;
	    for(i = 0; i < dates_.size(); i++)
		times_[i] = dayCounter_.yearFraction(referenceDate_,dates_[i]);
	    if (dates_[0] != referenceDate_) {
		dates_.insert(dates_.begin(),referenceDate_);
		times_.insert(times_.begin(), 0.0);
		forwards_.insert(forwards_.begin(), forwards_[0]);
	    }
		
	    fwdinterp_ = boost::shared_ptr<FwdInterpolation> (
		new FwdInterpolation(times_.begin(),
				     times_.end(),
				     forwards_.begin()));
	}

    std::vector<Date> CompoundForward::calibratedDateList(
	const std::vector<Date>& originalDates,
	const Date& referenceDate,
	const Calendar& calendar,
	const RollingConvention roll,
	int monthCount) {
	std::vector<Date> dates = originalDates;
	 
	Size i,ci;
	for (i = 0, ci = 1; i < dates.size(); i++) {
	    Date aDate = dates[i];
	    Date tmpDate = calendar.advance(referenceDate,
					    ci*monthCount,
					    Months, roll);
	    while (aDate > tmpDate) {
		dates.insert(dates.begin() + i, tmpDate);
		i++;
		tmpDate = calendar.advance(referenceDate,
					   ++ci*monthCount,
					   Months, roll);
	    }
	    if (tmpDate == aDate)
		ci++;
	}
	return dates;
    }
	
    boost::shared_ptr<TermStructure> CompoundForward::bootstrap() const {
	needsBootstrap_ = false;
	QL_REQUIRE(compounding_ > 0,
		   "Continuous compounding needs no bootstrap.");
	try {
	    int granularity = 12/((!granularity_) ? compounding_
				  : granularity_);
	    // First we need to calibrate the nodes to ensure we bootstrap
	    // all nodes relevant to the compounding frequency
	    std::vector<Date> dates = CompoundForward::calibratedDateList(
		dates_,referenceDate_,calendar_,roll_,granularity);
		
	    std::vector<DiscountFactor> discounts;
	    Date compoundDate = calendar_.advance(referenceDate_,
						  granularity,
						  Months, roll_);
	    Time compoundTime = dayCounter_.yearFraction(referenceDate_,
							 compoundDate);
	    double qFactor = 0.0;
	    Size i,ci;
	    for (i = 0, ci = 1; i < dates.size(); i++) {
		DiscountFactor df;
		Date rateDate = dates[i];
		Time t = dayCounter_.yearFraction(referenceDate_,
						  rateDate);
		Rate r = instantaneousForward(t);
		if (t <= compoundTime) {
		    df = 1.0/(1.0+r*t);
		    qFactor = df*t;
		} else {
		    Date tmpDate = calendar_.advance(referenceDate_,
						     (ci+1)*granularity,
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
	    discountCurve_ = boost::shared_ptr<DiscountCurve>
		(new ExtendedDiscountCurve(todaysDate_,
					   dates, discounts,
					   calendar_, roll_,
					   dayCounter_));
	}
	catch(std::exception& e) {
	    // signal incomplete state
	    needsBootstrap_ = true;
	    // rethrow
	    throw Error("Could not bootstrap curve." +
			std::string(e.what()));
	}
	return discountCurve_;
    }

    Rate CompoundForward::zeroYieldImpl(Time t) const {
        if (compounding_ == 0)
            return ForwardRateStructure::zeroYieldImpl(t);
        if (needsBootstrap_)
            bootstrap();
        return discountCurve()->zeroYield(t,true);
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
		return (*fwdinterp_) (t,true);
            }
        }
        QL_DUMMY_RETURN(Rate());
    }

    Rate CompoundForward::compoundForwardImpl(Time t, int f) const {
        if (f == compounding_)
            return forwardImpl(t);
        if (needsBootstrap_)
            bootstrap();
        return discountCurve()->compoundForward(t,f,true);
    }

    boost::shared_ptr<TermStructure> CompoundForward::discountCurve() const {
        QL_REQUIRE(compounding_ > 0,
                   "continuous compounding needs no bootstrap.");
        if (needsBootstrap_)
            bootstrap();
        return discountCurve_;
    }

}
