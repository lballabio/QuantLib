/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
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
#include <boost/optional.hpp>

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

        /*! This flag specifies whether or not cashflows occurring on
            the NPV date should, by default, enter the NPV.  It can be
            overridden locally when calling the NPV-related
            methods.
        */
        bool& includeReferenceDateCashFlows();
        bool includeReferenceDateCashFlows() const;

        /*! If set, this flag specifies whether or not cashflows
            occurring on today's date should enter the NPV.  When the
            NPV date (i.e., the date at which the cash flows are
            discounted) equals today's date, this flag overrides the
            behavior chosen for the NPV date.
        */
        boost::optional<bool>& includeTodaysCashFlows();
        boost::optional<bool> includeTodaysCashFlows() const;

        bool& enforcesTodaysHistoricFixings();
        bool enforcesTodaysHistoricFixings() const;
      private:
        DateProxy evaluationDate_;
        bool includeReferenceDateCashFlows_;
        boost::optional<bool> includeTodaysCashFlows_;
        bool enforcesTodaysHistoricFixings_;
    };


    // helper class to temporarily and safely change the settings
    class SavedSettings {
      public:
        SavedSettings();
        ~SavedSettings();
      private:
        Date evaluationDate_;
        bool includeReferenceDateCashFlows_;
        boost::optional<bool> includeTodaysCashFlows_;
        bool enforcesTodaysHistoricFixings_;
    };

}


#endif

