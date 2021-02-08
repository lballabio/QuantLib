/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006 StatPro Italia srl
 Copyright (C) 2007, 2008 Ferdinando Ametrano
 Copyright (C) 2007 Chiara Fornarola

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

#include <ql/indexes/indexmanager.hpp>
#include <ql/math/comparison.hpp>
#include <ql/time/calendar.hpp>

namespace QuantLib {

    //! purely virtual base class for indexes
    /*! \warning this class performs no check that the
                 provided/requested fixings are for dates in the past,
                 i.e. for dates less than or equal to the evaluation
                 date. It is up to the client code to take care of
                 possible inconsistencies due to "seeing in the
                 future"
    */
    class Index : public Observable {
      public:
        ~Index() override = default;
        //! Returns the name of the index.
        /*! \warning This method is used for output and comparison
                     between indexes. It is <b>not</b> meant to be
                     used for writing switch-on-type code.
        */
        virtual std::string name() const = 0;
        //! returns the calendar defining valid fixing dates
        virtual Calendar fixingCalendar() const = 0;
        //! returns TRUE if the fixing date is a valid one
        virtual bool isValidFixingDate(const Date& fixingDate) const = 0;
        //! returns whether a historical fixing was stored for the given date
        bool hasHistoricalFixing(const Date& fixingDate) const;
        //! returns the fixing at the given date
        /*! the date passed as arguments must be the actual calendar
            date of the fixing; no settlement days must be used.
        */
        virtual Real fixing(const Date& fixingDate, bool forecastTodaysFixing = false) const = 0;
        //! returns the fixing TimeSeries
        const TimeSeries<Real>& timeSeries() const {
            return IndexManager::instance().getHistory(name());
        }
        //! check if index allows for native fixings.
        /*! If this returns false, calls to addFixing and similar
            methods will raise an exception.
        */
        virtual bool allowsNativeFixings() { return true; }
        //! stores the historical fixing at the given date
        /*! the date passed as arguments must be the actual calendar
            date of the fixing; no settlement days must be used.
        */
        virtual void addFixing(const Date& fixingDate, Real fixing, bool forceOverwrite = false);
        //! stores historical fixings from a TimeSeries
        /*! the dates in the TimeSeries must be the actual calendar
            dates of the fixings; no settlement days must be used.
        */
        void addFixings(const TimeSeries<Real>& t, bool forceOverwrite = false);
        //! stores historical fixings at the given dates
        /*! the dates passed as arguments must be the actual calendar
            dates of the fixings; no settlement days must be used.
        */
        template <class DateIterator, class ValueIterator>
        void addFixings(DateIterator dBegin,
                        DateIterator dEnd,
                        ValueIterator vBegin,
                        bool forceOverwrite = false) {
            checkNativeFixingsAllowed();
            std::string tag = name();
            TimeSeries<Real> h = IndexManager::instance().getHistory(tag);
            bool noInvalidFixing = true, noDuplicatedFixing = true;
            Date invalidDate, duplicatedDate;
            Real nullValue = Null<Real>();
            Real invalidValue = Null<Real>();
            Real duplicatedValue = Null<Real>();
            while (dBegin != dEnd) {
                bool validFixing = isValidFixingDate(*dBegin);
                Real currentValue = h[*dBegin];
                bool missingFixing = forceOverwrite || currentValue == nullValue;
                if (validFixing) {
                    if (missingFixing)
                        h[*(dBegin++)] = *(vBegin++);
                    else if (close(currentValue, *(vBegin))) {
                        ++dBegin;
                        ++vBegin;
                    } else {
                        noDuplicatedFixing = false;
                        duplicatedDate = *(dBegin++);
                        duplicatedValue = *(vBegin++);
                    }
                } else {
                    noInvalidFixing = false;
                    invalidDate = *(dBegin++);
                    invalidValue = *(vBegin++);
                }
            }
            IndexManager::instance().setHistory(tag, h);
            QL_REQUIRE(noInvalidFixing, "At least one invalid fixing provided: "
                                            << invalidDate.weekday() << " " << invalidDate << ", "
                                            << invalidValue);
            QL_REQUIRE(noDuplicatedFixing, "At least one duplicated fixing provided: "
                                               << duplicatedDate << ", " << duplicatedValue
                                               << " while " << h[duplicatedDate]
                                               << " value is already present");
        }
        //! clears all stored historical fixings
        void clearFixings();

      private:
        //! check if index allows for native fixings
        void checkNativeFixingsAllowed();
    };

    inline bool Index::hasHistoricalFixing(const Date& fixingDate) const {
        return IndexManager::instance().hasHistoricalFixing(name(), fixingDate);
    }

}

#endif
