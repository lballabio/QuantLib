/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006, 2007, 2009 StatPro Italia srl
 Copyright (C) 2006 Ferdinando Ametrano

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

/*! \file interestrateindex.hpp
    \brief base class for interest rate indexes
*/

#ifndef quantlib_interestrateindex_hpp
#define quantlib_interestrateindex_hpp

#include <ql/index.hpp>
#include <ql/time/calendar.hpp>
#include <ql/currency.hpp>
#include <ql/time/daycounter.hpp>
#include <ql/time/period.hpp>

namespace QuantLib {

    //! base class for interest rate indexes
    /*! \todo add methods returning InterestRate */
    class InterestRateIndex : public Index,
                              public Observer {
      public:
        InterestRateIndex(const std::string& familyName,
                          const Period& tenor,
                          Natural settlementDays,
                          const Currency& currency,
                          const Calendar& fixingCalendar,
                          const DayCounter& dayCounter);
        //! \name Index interface
        //@{
        std::string name() const;
        Calendar fixingCalendar() const;
        bool isValidFixingDate(const Date& fixingDate) const;
        Rate fixing(const Date& fixingDate,
                    bool forecastTodaysFixing = false) const;
        //@}
        //! \name Observer interface
        //@{
        void update();
        //@}
        //! \name Inspectors
        //@{
        std::string familyName() const;
        Period tenor() const;
        Natural fixingDays() const;
        Date fixingDate(const Date& valueDate) const;
        const Currency& currency() const;
        const DayCounter& dayCounter() const;
        //@}
        /*! \name Date calculations

            These method can be overridden to implement particular
            conventions (e.g. EurLibor)

            @{
        */
        virtual Date valueDate(const Date& fixingDate) const;
        virtual Date maturityDate(const Date& valueDate) const = 0;
        // @}
      protected:
        virtual Rate forecastFixing(const Date& fixingDate) const = 0;
        std::string familyName_;
        Period tenor_;
        Natural fixingDays_;
        Calendar fixingCalendar_;
        Currency currency_;
        DayCounter dayCounter_;
    };


    // inline definitions

    inline void InterestRateIndex::update() {
        notifyObservers();
    }

    inline std::string InterestRateIndex::familyName() const {
        return familyName_;
    }

    inline Period InterestRateIndex::tenor() const {
        return tenor_;
    }

    inline Natural InterestRateIndex::fixingDays() const {
        return fixingDays_;
    }

    inline const Currency& InterestRateIndex::currency() const {
        return currency_;
    }

    inline const DayCounter& InterestRateIndex::dayCounter() const {
        return dayCounter_;
    }

}

#endif
