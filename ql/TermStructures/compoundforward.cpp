
/*
 Copyright (C) 2002 Andre Louw.

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

/*! \file compoundforward.cpp
    \brief compounded forward term structure

    \fullpath
    ql/TermStructures/%compoundforward.cpp
*/

// $Id$

#include <ql/TermStructures/compoundforward.hpp>
#include <ql/dataformatters.hpp>

namespace QuantLib {

    namespace TermStructures {

        CompoundForward::CompoundForward(
            const std::vector<Date>& dates,
            const std::vector<Rate>& forwards,
            Currency currency,
            const DayCounter& dayCounter,
            const Date& todaysDate,
            const Calendar& calendar,
            int settlementDays,
            RollingConvention roll,
            int compoundFrequency)
        : currency_(currency), dayCounter_(dayCounter),
          todaysDate_(todaysDate),
          calendar_(calendar),settlementDays_(settlementDays),
          roll_(roll), compoundFrequency_(compoundFrequency),
          needsBootstrap_(true),dates_(dates), forwards_(forwards) {

            QL_REQUIRE(dates_.size()>0,"No input Dates given");
            QL_REQUIRE(forwards_.size()>0,"No input rates given");
            QL_REQUIRE(dates_.size()==forwards_.size(),
                       "Inconsistent number of Dates/Forward Rates");
            settlementDate_ = calendar.advance(todaysDate_,
                                               settlementDays_,Days);

            discounts_ = std::vector<DiscountFactor>();
            zeroYields_ = std::vector<Rate>();
   
            for (Size i=0; i<dates_.size(); i++)
                times_.push_back(dayCounter_.yearFraction(settlementDate_,
                                                          dates_[i]));
	 
            fwdinterp_ = Handle<FwdInterpolation>
                (new FwdInterpolation(times_.begin(), times_.end(),
                                      forwards_.begin()));

            dfinterp_ = Handle<DfInterpolation>
                (new DfInterpolation(times_.begin(), times_.end(),
                                     discounts_.begin()));
	 
            validateInputs();
        }

        CompoundForward::CompoundForward(
	        const std::vector<std::string>& identifiers,
            const std::vector<Rate>& forwards,
            Currency currency,
            const DayCounter& dayCounter,
            const Date& todaysDate,
            const Calendar& calendar,
            int settlementDays,
            RollingConvention roll,
            int compoundFrequency)
        : currency_(currency), dayCounter_(dayCounter),
          todaysDate_(todaysDate),
          calendar_(calendar),settlementDays_(settlementDays),
          roll_(roll), compoundFrequency_(compoundFrequency),
          needsBootstrap_(true),forwards_(forwards) {

            QL_REQUIRE(identifiers.size()>0,"No input Identifiers given");
            QL_REQUIRE(forwards_.size()>0,"No input rates given");
            QL_REQUIRE(identifiers.size()==forwards_.size(),
                       "Inconsistent number of Identifiers/Forward Rates");
            settlementDate_ = calendar.advance(todaysDate_,
                                               settlementDays_,Days);
            Size i;
            for (i=0; i<identifiers.size(); i++) {
                Date aDate = calendar.advance(settlementDate_,
                                              Period(identifiers[i]), roll);
                dates_.push_back(aDate);
            }
            discounts_ = std::vector<DiscountFactor>();
            zeroYields_ = std::vector<Rate>();
            
            for (i=0; i<dates_.size(); i++)
                times_[i] = dayCounter_.yearFraction(settlementDate_, 
                                                     dates_[i]);
            
            fwdinterp_ = Handle<FwdInterpolation>
                (new FwdInterpolation(times_.begin(), times_.end(),
                                      forwards_.begin()));
            
            dfinterp_ = Handle<DfInterpolation>
                (new DfInterpolation(times_.begin(), times_.end(),
                                     discounts_.begin()));
            
            validateInputs();
        }

        void CompoundForward::bootstrap() const {
            // prevent recursively calling bootstrap() when the
            // term structure methods are called by the rate helpers
            needsBootstrap_ = false;
            try {
                Date compoundDate = calendar_.advance(settlementDate_,
                                                      compoundFrequency_, 
                                                      Months, roll_);
	    
                for (Size i=0,ci=0; i<dates_.size(); i++) {
                    DiscountFactor df;
                    Time t;

                    Date rateDate = dates_[i];
                    Rate fwd = forwards_[i];
                    if (compoundDate >= rateDate) {
                        t = dayCounter_.yearFraction(settlementDate_,rateDate,
                                                     settlementDate_,rateDate);
                        df = 1.0/(1.0+fwd*t);
                        ci = i;
                    } else {
                        Size a;
                        Date aDate, pDate;
                        double tempD = 0.0, prev;
		     
                        QL_REQUIRE(discounts_.size()>0,
                                   "Needs forward on at least "
                                   "compounding start");
                        for (a=discounts_.size()-1; a>ci; a--) {
                            prev = discounts_.at(a);
                            aDate = dates_.at(a);
                            pDate = dates_.at(a-1);
                            t = dayCounter_.yearFraction(pDate,aDate,
                                                         pDate,aDate);
                            tempD += fwd*prev*t;
                        }
                        prev = discounts_.at(a);
                        aDate = dates_.at(a);
                        t = dayCounter_.yearFraction(settlementDate_,aDate,
                                                     settlementDate_,aDate);
                        tempD += fwd*prev*t;
		     
                        aDate = dates_.at(discounts_.size());
                        pDate = dates_.at(discounts_.size()-1);
                        t = dayCounter_.yearFraction(pDate,aDate,
                                                     pDate,aDate);
                        df = (1.0-tempD)/(1.0+fwd*t);
                    }
                    discounts_.push_back(df);
                    zeroYields_.push_back(-QL_LOG(df) / times_[i]);
                }
            } catch (...) {
                needsBootstrap_ = true;
                throw;
            }
        }

        void CompoundForward::validateInputs() const {
            Date compoundDate = calendar_.advance(settlementDate_,
                                                  compoundFrequency_,
                                                  Months, roll_);

            for (Size i=0,ci=1; i<dates_.size(); i++) {
                Date rateDate;

                rateDate = dates_.at(i);
                // Passed compounding?
                if (compoundDate < rateDate) {
                    Date tmpDate = calendar_.advance(settlementDate_,
                                                     compoundFrequency_*(++ci),
                                                     Months, roll_);
                    // Missed any forwards?
                    while (tmpDate < rateDate) {
                        Time t = dayCounter_.yearFraction(settlementDate_, 
                                                          tmpDate);
                        Rate r = (*fwdinterp_)(t, true);

                        dates_.insert(dates_.begin()+i,tmpDate);
                        forwards_.insert(forwards_.begin()+i,r);
                        i++;
                        tmpDate = calendar_.advance(settlementDate_,
                                                    compoundFrequency_*(++ci),
                                                    Months, roll_);
                    }
                }
            }
        }

        Rate CompoundForward::zeroYieldImpl(Time t, bool extrapolate) const {
            if (needsBootstrap_) 
                bootstrap();
            if (t == 0.0) {
                return zeroYields_[0];
            } else {
                int n = referenceNode(t, extrapolate);
                if (t == times_[n]) {
                    return zeroYields_[n];
                } else {
                    Time tn = times_[n-1];
                    return (zeroYields_[n-1]*tn+forwards_[n]*(t-tn))/t;
                }
            }
            QL_DUMMY_RETURN(Rate());
        }

        DiscountFactor CompoundForward::discountImpl(Time t,
                                                     bool extrapolate) const {
            if (needsBootstrap_) 
                bootstrap();
            if (t == 0.0) {
                return 1.0;
            } else {
                int n = referenceNode(t, extrapolate);
                if (t == times_[n])
                    return discounts_[n];
                else
                    return (*dfinterp_)(t, true);
            }
            QL_DUMMY_RETURN(DiscountFactor());
        }

        Rate CompoundForward::forwardImpl(Time t, bool extrapolate) const {
            if (needsBootstrap_) 
                bootstrap();
            if (t == 0.0) {
                return forwards_[0];
            } else {
                int n = referenceNode(t, extrapolate);
                if (t == times_[n])
                    return forwards_[n];
                else
                    return (*fwdinterp_)(t, true);
            }
            QL_DUMMY_RETURN(Rate());
        }

        int CompoundForward::referenceNode(Time t, bool extrapolate) const {
            QL_REQUIRE(t >= 0.0 && (t <= times_.back() || extrapolate),
                       "CompoundForward: time (" +
                       DoubleFormatter::toString(t) +
                       ") outside curve definition [" +
                       DoubleFormatter::toString(0.0) + ", " +
                       DoubleFormatter::toString(times_.back()) + "]");
            if (t>=times_.back())
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

    }

}
