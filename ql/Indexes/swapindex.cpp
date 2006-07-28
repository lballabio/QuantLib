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

#include <ql/Indexes/swapindex.hpp>
#include <ql/Instruments/all.hpp>

#include <sstream>


namespace QuantLib {

    Rate SwapIndex::forecastFixing(const Date& fixingDate) const {
        QL_REQUIRE(iborIndex_, "no index set");
        QL_REQUIRE(iborIndex_->termStructure(), "no term structure set");
		boost::shared_ptr<VanillaSwap> swap(underlyingSwap(fixingDate));
        return swap->fairRate();
    }

	boost::shared_ptr<VanillaSwap> SwapIndex::underlyingSwap(const Date& fixingDate) const {

		Date start = calendar_.advance(fixingDate, settlementDays_,Days);
        Date end = calendar_.advance(start,years_,Years);
        Schedule fixedLegSchedule(calendar_, start, end,
                                  fixedLegFrequency_, fixedLegConvention_);
        Schedule floatingLegSchedule(calendar_, start, end,
                                     floatingLegFrequency_,
                                     floatingLegConvention_);

		return boost::shared_ptr<VanillaSwap>(new VanillaSwap (true, 100.0,
                        fixedLegSchedule, 0.0, dayCounter_,
                        floatingLegSchedule,
                        iborIndex_, iborIndex_->settlementDays(),
                        0.0,
						dayCounter_,
                        Handle<YieldTermStructure>(iborIndex_->termStructure())));
	}

	boost::shared_ptr<Schedule> SwapIndex::fixedRateSchedule(const Date& fixingDate) const {
	
		Date start = calendar_.advance(fixingDate, settlementDays_,Days);
        Date end = calendar_.advance(start,years_,Years);
        return boost::shared_ptr<Schedule>(new Schedule(calendar_, start, end,
                                  fixedLegFrequency_, fixedLegConvention_));
	}
}
