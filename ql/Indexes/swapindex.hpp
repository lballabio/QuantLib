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
/*! \file swaprate.hpp
    \brief swap-rate indexes
*/

#ifndef quantlib_swapindex_hpp
#define quantlib_swapindex_hpp

#include <ql/Indexes/interestrateindex.hpp>
#include <ql/Instruments/vanillaswap.hpp>

namespace QuantLib {

    //! base class for swap-rate indexes
    class SwapIndex : public InterestRateIndex {
      public:
        SwapIndex(const std::string& familyName,
                  Integer years,
                  Integer settlementDays,
                  Currency currency,
                  const Calendar& calendar,
                  Frequency fixedLegFrequency,
                  BusinessDayConvention fixedLegConvention,
                  const DayCounter& fixedLegDayCounter,
                  const boost::shared_ptr<Xibor>& iborIndex)
        : InterestRateIndex(familyName, Period(years, Years), settlementDays,
                            currency, calendar, fixedLegDayCounter),
          years_(years), iborIndex_(iborIndex),
          fixedLegFrequency_(fixedLegFrequency),
          fixedLegConvention_(fixedLegConvention)
        {
            registerWith(iborIndex_);
        }
        //! \name InterestRateIndex interface
        //@{
        Rate forecastFixing(const Date& fixingDate) const;
        //@}
        //! \name Inspectors
        //@{
        Frequency fixedLegFrequency() const { return fixedLegFrequency_; }
        BusinessDayConvention fixedLegConvention() const {
            return fixedLegConvention_;
        }
        boost::shared_ptr<Xibor> iborIndex() const { return iborIndex_; }
		boost::shared_ptr<Schedule> fixedRateSchedule(const Date& fixingDate) const;
		boost::shared_ptr<VanillaSwap> underlyingSwap(const Date& fixingDate) const;
      private:
        Integer years_;
        boost::shared_ptr<Xibor> iborIndex_;
        Frequency fixedLegFrequency_;
        BusinessDayConvention fixedLegConvention_;
    };

}

#endif
