
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

    typedef Math::DiscountFactorInterpolation <
	std::vector<Time>::const_iterator,
	std::vector<DiscountFactor>::const_iterator > DfInterpolation;
	
    ExtendedDiscountCurve::ExtendedDiscountCurve(
	const Date &todaysDate,
	const std::vector<Date>& dates,
	const std::vector<DiscountFactor>& discounts,
	const Calendar& calendar,
	const RollingConvention roll,
	const DayCounter& dayCounter,
	const int granularity)
        : DiscountCurve(todaysDate,dates,discounts,dayCounter),
	  calendar_(calendar), roll_(roll),
	  granularity_(granularity) {
	referenceDate_ = dates_[0];
	times_.resize(dates_.size());
	times_[0] = 0.0;
	Size i;
	for (i = 1; i < dates_.size(); i++) {
	    QL_REQUIRE(dates_[i] > dates_[i-1],
		       "ExtendedDiscountCurve::ExtendedDiscountCurve : invalid date ("+
		       DateFormatter::toString(dates_[i])+", vs "+
		       DateFormatter::toString(dates_[i-1])+")");
	    QL_REQUIRE(discounts_[i] > 0.0,
		       "ExtendedDiscountCurve::ExtendedDiscountCurve : invalid discount");
	    times_[i] = dayCounter_.yearFraction(referenceDate_,
						 dates_[i]);
	}
	interpolation_ = boost::shared_ptr<DfInterpolation>
	    (new DfInterpolation(times_.begin(),
				 times_.end(),
				     discounts_.begin()));
    }

    boost::shared_ptr<TermStructure> ExtendedDiscountCurve::reversebootstrap(
	int compounding) const {
	int granularity = 12/((!granularity_) ? compounding
			      : granularity_);

	// First we need to calibrate the nodes to ensure we bootstrap
	// all nodes relevant to the compounding frequency
	std::vector<Date> dates = CompoundForward::calibratedDateList(
	    dates_,referenceDate_,calendar_,roll_,granularity);

	std::vector<Rate> forwards;
	Date compoundDate = calendar_.advance(referenceDate_,
					      granularity,
					      Months, roll_);
	Time compoundTime = dayCounter_.yearFraction(referenceDate_,
						     compoundDate);
	double qFactor = 0.0;
	Size i,ci;
// 	printf("Reverse bootstrap, reference: %s, compound: %s (%.5f)\n",
// 	       DateFormatter::toString(referenceDate_).c_str(),
// 	       DateFormatter::toString(compoundDate).c_str(),
// 	       compoundTime);
	// Ignore first entry (SPOT with df=1.0)
	for (i = 1, ci = 1; i < dates.size(); i++) {
	    Rate fwd;
	    Date rateDate = dates[i];
	    Time t = dayCounter_.yearFraction(referenceDate_,
					      rateDate);
	    DiscountFactor df = discount(t);
	    if (t <= compoundTime) {
		fwd = ((1.0/df)-1.0)/t;
		qFactor = df*t;
// 		printf("BELOW %s => %.12f @ %.5f (%.12f) => %.12f\n",
// 		       DateFormatter::toString(rateDate).c_str(),
// 		       df,t,qFactor,fwd);
	    } else {
		Date tmpDate = calendar_.advance(referenceDate_,
						 (ci+1)*granularity,
						 Months, roll_);
		Time tt = dayCounter_.yearFraction(compoundDate,
						   rateDate);
		fwd = (1.0-df)/(qFactor+df*tt);
// 		printf("%s => %.12f @ %.5f (%.12f) => %.12f\n",
// 		       DateFormatter::toString(rateDate).c_str(),
// 		       df,t,qFactor,fwd);
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

	return boost::shared_ptr<CompoundForward>
	    (new CompoundForward(todaysDate_,
				 referenceDate_,
				 dates,forwards,
				 calendar_,roll_,
				 compounding,
				 dayCounter_));
    }

    DiscountFactor ExtendedDiscountCurve::discountImpl(
	Time t, bool extrapolate) const
    {
	QL_REQUIRE(t >= 0.0,
		   "ExtendedDiscountCurve::discountImpl "
		   "negative time (" + DoubleFormatter::toString(t) +
		   ") not allowed");
	if (t == 0.0) {
	    return discounts_[0];
	} else {
	    int n = referenceNode(t, extrapolate);
	    if (t == times_[n]) {
		return discounts_[n];
	    } else {
		return (*interpolation_) (t, extrapolate);
	    }
	}
	QL_DUMMY_RETURN(DiscountFactor());
    }

    int ExtendedDiscountCurve::referenceNode(
	Time t, bool extrapolate) const {
	QL_REQUIRE(t >= 0.0 && (t <= times_.back() || extrapolate),
		   "ExtendedDiscountCurve: time (" +
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

    Rate ExtendedDiscountCurve::compoundForwardImpl(
	Time t, int compounding, bool extrapolate) const {
	if (compounding == 0)
	    return DiscountStructure::compoundForwardImpl(t,
							  compounding,
							  extrapolate);
	return forwardCurve(compounding)->compoundForward(t,
							  compounding,
							  extrapolate);
    }

    boost::shared_ptr<TermStructure> ExtendedDiscountCurve::forwardCurve(
	int compounding) const {
	if (forwardCurveMap_.find(compounding) == forwardCurveMap_.end())
	    forwardCurveMap_[compounding] = reversebootstrap(compounding);
	return forwardCurveMap_[compounding];
    }

}
