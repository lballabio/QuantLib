/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2007 Chiara Fornarola
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006 StatPro Italia srl

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

/*! \file index.hpp
    \brief virtual base class for indexes
*/

#ifndef quantlib_index_hpp
#define quantlib_index_hpp

#include <ql/settings.hpp>
#include <ql/time/calendar.hpp>
#include <ql/indexes/indexmanager.hpp>

namespace QuantLib {

    //! purely virtual base class for indexes
    class Index : public Observable {
      public:
        virtual ~Index() {}
        //! Returns the name of the index.
        /*! \warning This method is used for output and comparison
                     between indexes. It is <b>not</b> meant to be
                     used for writing switch-on-type code.
        */
        virtual std::string name() const = 0;
        //! returns the calendar defining valid fixing dates
        const Calendar& fixingCalendar() const;
        //! returns TRUE if the fixing date is a valid one
        bool isValidFixingDate(const Date& fixingDate) const;
        //! returns the fixing at the given date
        /*! the date passed as arguments must be the actual calendar
            date of the fixing; no settlement days must be used.
        */
        virtual Real fixing(const Date& fixingDate,
                            bool forecastTodaysFixing = false) const = 0;
        //! stores the historical fixing at the given date
        /*! the date passed as arguments must be the actual calendar
            date of the fixing; no settlement days must be used.
        */
        void addFixing(const Date& fixingDate,
                       Real fixing);
        //! stores historical fixings at the given dates
        /*! the dates passed as arguments must be the actual calendar
            dates of the fixings; no settlement days must be used.
        */
        void addFixings(const std::vector<Date>& dates,
                        const std::vector<Real>& values);
        //! stores historical fixings at the given dates
        /*! the dates passed as arguments must be the actual calendar
            dates of the fixings; no settlement days must be used.
        */
        template <class DateIterator, class ValueIterator>
        void addFixings(DateIterator dBegin, DateIterator dEnd,
                        ValueIterator vBegin) {
            std::string tag = name();
            TimeSeries<Real> h = IndexManager::instance().getHistory(tag);
            bool allValidFixings = true;
            Date invalidDate, refDate=Settings::instance().evaluationDate();
            Real invalidValue;
            while (dBegin != dEnd) {
                if (isValidFixingDate(*dBegin) && *dBegin<=refDate)
                    h[*(dBegin++)] = *(vBegin++);
                else {
                    allValidFixings = false;
                    invalidDate = *(dBegin++);
                    invalidValue = *(vBegin++);
                }
            }
            IndexManager::instance().setHistory(tag, h);
            QL_REQUIRE(allValidFixings,
                       "At least one invalid fixing provided: " <<
                       invalidDate.weekday() << " " << invalidDate <<
                       ", " << invalidValue <<
                       ", evaluation date being " << refDate);
        }
        //! clears all stored historical fixings
        void clearFixings();
      protected:
        Calendar fixingCalendar_;
    };

    inline const Calendar& Index::fixingCalendar() const {
        return fixingCalendar_;
    }

    inline void Index::addFixings(const std::vector<Date>& dates,
                                  const std::vector<Real>& values) {
        QL_REQUIRE(dates.size()==values.size(),
                   "size mismatch between dates (" << dates.size() <<
                   ") and values  (" << values.size() << ")");
        addFixings(dates.begin(), dates.end(), values.begin());
    }

}

#endif
