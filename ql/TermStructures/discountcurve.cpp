
/*
 Copyright (C) 2002, 2003 Decillion Pty(Ltd)

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file discountcurve.cpp
    \brief Term structure based on loglinear interpolation of discount factors
*/

#include <ql/TermStructures/discountcurve.hpp>

namespace QuantLib {

    namespace TermStructures {

        DiscountCurve::DiscountCurve(
	   const Date &todaysDate,
	   const std::vector<Date>& dates,
	   const std::vector<DiscountFactor>& discounts,
	   const Calendar& calendar,
	   const RollingConvention roll,
	   const DayCounter& dayCounter)
	: todaysDate_(todaysDate),
	  dayCounter_(dayCounter),
	  calendar_(calendar), roll_(roll),
          dates_(dates), discounts_(discounts) {
            QL_REQUIRE(dates_.size() > 0, "DiscountCurve::DiscountCurve : "
		       "No input Dates given");
            QL_REQUIRE(discounts_.size() > 0, "DiscountCurve::DiscountCurve : "
		       "No input Discount factors given");
            QL_REQUIRE(discounts_.size() == dates_.size(),
		       "DiscountCurve::DiscountCurve : "
		       "Dates/Discount factors count mismatch");
            QL_REQUIRE(discounts_[0] == 1.0,
                "DiscountCurve::DiscountCurve : "
                "the first discount must be == 1.0 "
                "to flag the corrsponding date as settlement date");

	    referenceDate_ = dates_[0];
	    calibrateNodes();
      }

      void DiscountCurve::calibrateNodes() const {
	 Size i,ci;

	 times_.resize(dates_.size());
	 times_[0] = 0.0;
	 for(i = 1; i < dates_.size(); i++) {
	    QL_REQUIRE(dates_[i] > dates_[i-1],
		       "DiscountCurve::DiscountCurve : invalid date ("+
		       DateFormatter::toString(dates_[i])+", vs "+
		       DateFormatter::toString(dates_[i-1])+")");
	    QL_REQUIRE(discounts_[i] > 0.0,
		       "DiscountCurve::DiscountCurve : invalid discount");
	    times_[i] = dayCounter_.yearFraction(referenceDate_, dates_[i]);
	 }
	 interpolation_ = Handle<DfInterpolation>
	    (new DfInterpolation(times_.begin(),
				 times_.end(),
				 discounts_.begin()));
	 
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
				(*interpolation_)(t,true));
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
	 
	 interpolation_ = Handle<DfInterpolation>
	    (new DfInterpolation(times_.begin(),
				 times_.end(),
				 discounts_.begin()));
      }

      Handle<TermStructure> DiscountCurve::reversebootstrap(
	 int compounding) const {
	 std::vector<Rate> forwards;
	 Date compoundDate = calendar_.advance(referenceDate_,
					       12/compounding,
					       Months, roll_);
	 Time compoundTime = dayCounter_.yearFraction(referenceDate_,
						      compoundDate);
	 double qFactor = 0.0;
	 Size i,ci;
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

	 return Handle<CompoundForward>
	    (new CompoundForward(todaysDate_,
				 referenceDate_,
				 dates_,forwards,
				 calendar_,roll_,
				 compounding,
				 dayCounter_));
      }

      DiscountFactor DiscountCurve::discountImpl(
	 Time t, bool extrapolate) const
      {
         QL_REQUIRE(t >= 0.0,
             "DiscountCurve::discountImpl "
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

      int DiscountCurve::referenceNode(
	 Time t, bool extrapolate) const {
	 QL_REQUIRE(t >= 0.0 && (t <= times_.back() || extrapolate),
                    "DiscountCurve: time (" +
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

      Rate DiscountCurve::compoundForwardImpl(
	 Time t, int compounding, bool extrapolate) const {
	 if (compounding == 0)
	    return DiscountStructure::compoundForwardImpl(t,
							  compounding,
							  extrapolate);
	 return forwardCurve(compounding)->compoundForward(t,
							   compounding,
							   extrapolate);
      }

      Handle<TermStructure> DiscountCurve::forwardCurve(
	 int compounding) const {
	 if (forwardCurveMap_.find(compounding) == forwardCurveMap_.end())
	    forwardCurveMap_[compounding] = reversebootstrap(compounding);
	 return forwardCurveMap_[compounding];
      }

    }

}
