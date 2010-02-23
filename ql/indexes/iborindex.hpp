/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009 StatPro Italia srl
 Copyright (C) 2009 Ferdinando Ametrano

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

/*! \file iborindex.hpp
    \brief base class for Inter-Bank-Offered-Rate indexes
*/

#ifndef quantlib_ibor_index_hpp
#define quantlib_ibor_index_hpp

#include <ql/indexes/interestrateindex.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>

namespace QuantLib {

    //! base class for Inter-Bank-Offered-Rate indexes (e.g. %Libor, etc.)
    class IborIndex : public InterestRateIndex {
      public:
        IborIndex(const std::string& familyName,
                  const Period& tenor,
                  Natural settlementDays,
                  const Currency& currency,
                  const Calendar& fixingCalendar,
                  BusinessDayConvention convention,
                  bool endOfMonth,
                  const DayCounter& dayCounter,
                  const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>());
        //! \name Inspectors
        //@{
        BusinessDayConvention businessDayConvention() const;
        bool endOfMonth() const { return endOfMonth_; }
        //! the curve used to forecast fixings
        Handle<YieldTermStructure> forwardingTermStructure() const;
        //@}
        //! \name Date calculations
        //@{
        Date maturityDate(const Date& valueDate) const;
        // @}
        //! \name Other methods
        //@{
        //! returns a copy of itself linked to a different forwarding curve
        virtual boost::shared_ptr<IborIndex> clone(
                        const Handle<YieldTermStructure>& forwarding) const;
        // @}
      protected:
        Rate forecastFixing(const Date& fixingDate) const;
        BusinessDayConvention convention_;
        Handle<YieldTermStructure> termStructure_;
        bool endOfMonth_;
    };


    class OvernightIndex : public IborIndex {
      public:
        OvernightIndex(const std::string& familyName,
                       Natural settlementDays,
                       const Currency& currency,
                       const Calendar& fixingCalendar,
                       const DayCounter& dayCounter,
                       const Handle<YieldTermStructure>& h =
                                    Handle<YieldTermStructure>());
        //! returns a copy of itself linked to a different forwarding curve
        boost::shared_ptr<IborIndex> clone(
                                   const Handle<YieldTermStructure>& h) const;
    };

}

#endif
