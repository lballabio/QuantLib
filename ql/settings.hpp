/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007, 2011 Ferdinando Ametrano
 Copyright (C) 2007 François du Vignaud
 Copyright (C) 2004, 2005, 2007, 2009 StatPro Italia srl

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

/*! \file settings.hpp
    \brief global repository for run-time library settings
*/

#ifndef quantlib_settings_hpp
#define quantlib_settings_hpp

#include <ql/patterns/singleton.hpp>
#include <ql/time/date.hpp>
#include <ql/utilities/observablevalue.hpp>
#include <ql/optional.hpp>

namespace QuantLib {

    //! global repository for run-time library settings
    class Settings : public Singleton<Settings> {
        friend class Singleton<Settings>;
      private:
        Settings();
        class DateProxy : public ObservableValue<Date> {
          public:
            DateProxy();
            DateProxy& operator=(const Date&);
            operator Date() const;
        };
        friend std::ostream& operator<<(std::ostream&, const DateProxy&);
      public:
        //! the date at which pricing is to be performed.
        /*! Client code can inspect the evaluation date, as in:
            \code
            Date d = Settings::instance().evaluationDate();
            \endcode
            where today's date is returned if the evaluation date is
            set to the null date (its default value;) can set it to a
            new value, as in:
            \code
            Settings::instance().evaluationDate() = d;
            \endcode
            and can register with it, as in:
            \code
            registerWith(Settings::instance().evaluationDate());
            \endcode
            to be notified when it is set to a new value.
            \warning a notification is not sent when the evaluation
                     date changes for natural causes---i.e., a date
                     was not explicitly set (which results in today's
                     date being used for pricing) and the current date
                     changes as the clock strikes midnight.
        */
        DateProxy& evaluationDate();
        const DateProxy& evaluationDate() const;

        /*! Call this to prevent the evaluation date to change at
            midnight (and, incidentally, to gain quite a bit of
            performance.)  If no evaluation date was previously set,
            it is equivalent to setting the evaluation date to
            Date::todaysDate(); if an evaluation date other than
            Date() was already set, it has no effect.
        */
        void anchorEvaluationDate();
        /*! Call this to reset the evaluation date to
            Date::todaysDate() and allow it to change at midnight.  It
            is equivalent to setting the evaluation date to Date().
            This comes at the price of losing some performance, since
            the evaluation date is re-evaluated each time it is read.
        */
        void resetEvaluationDate();

        /*! This flag specifies whether or not Events occurring on the reference
            date should, by default, be taken into account as not happened yet.
            It can be overridden locally when calling the Event::hasOccurred
            method.
        */
        bool& includeReferenceDateEvents();
        bool includeReferenceDateEvents() const;

        /*! If set, this flag specifies whether or not CashFlows
            occurring on today's date should enter the NPV.  When the
            NPV date (i.e., the date at which the cash flows are
            discounted) equals today's date, this flag overrides the
            behavior chosen for includeReferenceDate. It cannot be overridden
            locally when calling the CashFlow::hasOccurred method.
        */
        ext::optional<bool>& includeTodaysCashFlows();
        ext::optional<bool> includeTodaysCashFlows() const;

        bool& enforcesTodaysHistoricFixings();
        bool enforcesTodaysHistoricFixings() const;

      private:
        DateProxy evaluationDate_;
        bool includeReferenceDateEvents_ = false;
        ext::optional<bool> includeTodaysCashFlows_;
        bool enforcesTodaysHistoricFixings_ = false;
    };


    // helper class to temporarily and safely change the settings
    class SavedSettings {
      public:
        SavedSettings();
        ~SavedSettings();
      private:
        Date evaluationDate_;
        bool includeReferenceDateEvents_;
        ext::optional<bool> includeTodaysCashFlows_;
        bool enforcesTodaysHistoricFixings_;
    };


    // inline

    inline Settings::DateProxy::operator Date() const {
        if (value() == Date())
            return Date::todaysDate();
        else
            return value();
    }

    inline Settings::DateProxy& Settings::DateProxy::operator=(const Date& d) {
        if (value() != d) // avoid notifications if the date doesn't actually change
            ObservableValue<Date>::operator=(d);
        return *this;
    }

    inline Settings::DateProxy& Settings::evaluationDate() {
        return evaluationDate_;
    }

    inline const Settings::DateProxy& Settings::evaluationDate() const {
        return evaluationDate_;
    }

    inline bool& Settings::includeReferenceDateEvents() {
        return includeReferenceDateEvents_;
    }

    inline bool Settings::includeReferenceDateEvents() const {
        return includeReferenceDateEvents_;
    }

    inline ext::optional<bool>& Settings::includeTodaysCashFlows() {
        return includeTodaysCashFlows_;
    }

    inline ext::optional<bool> Settings::includeTodaysCashFlows() const {
        return includeTodaysCashFlows_;
    }

    inline bool& Settings::enforcesTodaysHistoricFixings() {
        return enforcesTodaysHistoricFixings_;
    }

    inline bool Settings::enforcesTodaysHistoricFixings() const {
        return enforcesTodaysHistoricFixings_;
    }

}

#endif
