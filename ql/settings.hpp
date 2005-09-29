/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2005 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file settings.hpp
    \brief global repository for run-time library settings
*/

#ifndef quantlib_settings_hpp
#define quantlib_settings_hpp

#include <ql/date.hpp>
#include <ql/Patterns/singleton.hpp>
#include <ql/Utilities/observablevalue.hpp>

namespace QuantLib {

    //! global repository for run-time library settings
    class Settings : public Singleton<Settings> {
        friend class Singleton<Settings>;
      private:
        Settings() {}
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
      private:
        DateProxy evaluationDate_;
    };


    // inline definitions

    inline Settings::DateProxy& Settings::evaluationDate() {
        return evaluationDate_;
    }

    inline const Settings::DateProxy& Settings::evaluationDate() const {
        return evaluationDate_;
    }

    inline Settings::DateProxy::DateProxy() : ObservableValue<Date>(Date()) {}

    inline Settings::DateProxy::operator Date() const {
        if (value() == Date())
            return Date::todaysDate();
        else
            return value();
    }

    inline
    Settings::DateProxy& Settings::DateProxy::operator=(const Date& d) {
        ObservableValue<Date>::operator=(d);
        return *this;
    }

    inline std::ostream& operator<<(std::ostream& out,
                                    const Settings::DateProxy& p) {
        return out << Date(p);
    }

}


#endif

