/*
 Copyright (C) 2006 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.


 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details. */

#include <ql/Indexes/swaprate.hpp>
#include <ql/Instruments/all.hpp>

#include <sstream>


namespace QuantLib {

    std::string SwapRate::name() const {
        std::ostringstream tenor;
        tenor << years_ << "y";
        return familyName_+" "+tenor.str()+"-swap rate";
    }

    Rate SwapRate::fixing(const Date& fixingDate,
                          bool forecastTodaysFixing) const {
        QL_REQUIRE(index_, "no index set");
        QL_REQUIRE(index_->termStructure(), "no term structure set");
        Date today = Settings::instance().evaluationDate();
        if (fixingDate < today) {
            // must have been fixed
            Rate pastFixing =
                IndexManager::instance().getHistory(name())[fixingDate];
            QL_REQUIRE(pastFixing != Null<Real>(),
                       "Missing " << name() << " fixing for " << fixingDate);
            return pastFixing;
        }
        if (fixingDate == today) {
            // might have been fixed
            try {
                Rate pastFixing =
                    IndexManager::instance().getHistory(name())[fixingDate];
                if (pastFixing != Null<Real>())
                    return pastFixing;
                else
                    ;   // fall through and forecast
            } catch (Error&) {
                ;       // fall through and forecast
            }
        }
		boost::shared_ptr<VanillaSwap> swap(underlyingSwap(fixingDate));
        return swap->fairRate();
    }

	boost::shared_ptr<VanillaSwap> SwapRate::underlyingSwap(const Date& fixingDate) const {

		Date start = calendar_.advance(fixingDate, settlementDays_,Days);
        Date end = calendar_.advance(start,years_,Years);
        Schedule fixedLegSchedule(calendar_, start, end,
                                  fixedLegFrequency_, fixedLegConvention_);
        Schedule floatingLegSchedule(calendar_, start, end,
                                     floatingLegFrequency_,
                                     floatingLegConvention_);

		return boost::shared_ptr<VanillaSwap>(new VanillaSwap (true, 100.0,
                        fixedLegSchedule, 0.0, fixedLegDayCounter_,
                        floatingLegSchedule, index_, indexFixingDays_, 0.0,
						fixedLegDayCounter_,
                        Handle<YieldTermStructure>(index_->termStructure())));
	}

	boost::shared_ptr<Schedule> SwapRate::fixedRateSchedule(const Date& fixingDate) const {
	
		Date start = calendar_.advance(fixingDate, settlementDays_,Days);
        Date end = calendar_.advance(start,years_,Years);
        return boost::shared_ptr<Schedule>(new Schedule(calendar_, start, end,
                                  fixedLegFrequency_, fixedLegConvention_));
	}
}
