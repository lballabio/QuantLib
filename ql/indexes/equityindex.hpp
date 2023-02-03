/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2023 Marcin Rybacki

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

/*! \file equityindex.hpp
    \brief base class for equity indexes
*/

#ifndef quantlib_equityindex_hpp
#define quantlib_equityindex_hpp

#include <ql/index.hpp>
#include <ql/time/calendar.hpp>
#include <ql/currency.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>

namespace QuantLib {

    //! base class for equity indexes
    class EquityIndex : public Index, public Observer {
      public:
        EquityIndex(std::string name,
                    Natural settlementDays,
                    Currency currency,
                    Calendar fixingCalendar,
                    Handle<YieldTermStructure> rate = {},
                    Handle<YieldTermStructure> dividend = {});

        //! \name Index interface
        //@{
        std::string name() const override { return name_; }
        Calendar fixingCalendar() const override { return settlementCalendar_; }
        bool isValidFixingDate(const Date& fixingDate) const override;
        Rate fixing(const Date& fixingDate, bool forecastTodaysFixing = false) const override;
        //@}
        //! \name Observer interface
        //@{
        void update() override;
        //@}
        //! \name Inspectors
        //@{
        Natural settlementDays() const { return settlementDays_; }
        Date settlementDate(const Date& valueDate) const;
        const Currency& currency() const { return currency_; }
        //! the rate curve used to forecast fixings
        Handle<YieldTermStructure> equityInterestRateCurve() const { return rate_; }
        //! the dividend curve used to forecast fixings
        Handle<YieldTermStructure> equityDividendCurve() const { return dividend_; }
        //@}
        //! \name Fixing calculations
        //@{
        //! It can be overridden to implement particular conventions
        virtual Rate forecastFixing(const Date& fixingDate) const;
        virtual Rate pastFixing(const Date& fixingDate) const;
        // @}

      protected:
        std::string name_;
        Natural settlementDays_;
        Currency currency_;
        Calendar settlementCalendar_;
        Handle<YieldTermStructure> rate_;
        Handle<YieldTermStructure> dividend_;
    };

    inline bool EquityIndex::isValidFixingDate(const Date& d) const {
        return fixingCalendar().isBusinessDay(d);
    }

    inline void EquityIndex::update() { notifyObservers(); }

    inline Date EquityIndex::settlementDate(const Date& valueDate) const {
        Date fixingDate =
            fixingCalendar().advance(valueDate, +static_cast<Integer>(settlementDays_), Days);
        return fixingDate;
    }
}

#endif