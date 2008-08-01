/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008 StatPro Italia srl

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

#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/indexes/interestrateindex.hpp>

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
        //! \name InterestRateIndex interface
        //@{
        Handle<YieldTermStructure> termStructure() const;
        //@}
        //! \name Inspectors
        //@{
        BusinessDayConvention businessDayConvention() const;
        bool endOfMonth() const { return endOfMonth_; }
        //@}
        //! \name Date calculations
        //@{
        Date maturityDate(const Date& valueDate) const;
        // @}
        //! \name Other methods
        //@{
        //! returns a copy of itself linked to a different forecast curve
        virtual boost::shared_ptr<IborIndex> clone(
                                   const Handle<YieldTermStructure>& h) const;
        // @}
      protected:
        Rate forecastFixing(const Date& fixingDate) const;
        BusinessDayConvention convention_;
        Handle<YieldTermStructure> termStructure_;
        bool endOfMonth_;
    };


    // inline definitions

    inline BusinessDayConvention IborIndex::businessDayConvention() const {
        return convention_;
    }

    inline Handle<YieldTermStructure> IborIndex::termStructure() const {
        return termStructure_;
    }

}

#endif
