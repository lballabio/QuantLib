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
      public:
        //! \name Evaluation date
        //@{
        //! the date at which pricing is to be performed
        /*! If not set, the current date will be used.
            \note setting the evaluation date to the null date will
                  actually cause it to be set to today's date.
        */
        DateProxy& evaluationDate();
        #ifndef QL_DISABLE_DEPRECATED
        //! change the evaluation date and notify registered instruments
        /*! \note settings the evaluation date to the null date will
                  cause evaluationDate() to return today's date.
            \deprecated assign the new value to evaluationDate() instead
        */
        void setEvaluationDate(const Date&);
        //! observable sending notification when the evaluation date changes
        /*! \warning this observable does not send a notification when
                     the evaluation date changes for natural
                     causes---i.e., a date was not explicitly set
                     (which results in today's date being used for
                     pricing) and the current date changes as the
                     clock strikes midnight.
            \deprecated register with evaluationDate() instead
        */
        boost::shared_ptr<Observable> evaluationDateGuard() const;
        //@}
        #endif
      private:
        // evaluation date
        DateProxy evaluationDate_;
    };


    // inline definitions

    inline Settings::DateProxy& Settings::evaluationDate() {
        return evaluationDate_;
    }

    #ifndef QL_DISABLE_DEPRECATED
    inline void Settings::setEvaluationDate(const Date& d) {
        evaluationDate_ = d;
    }

    inline
    boost::shared_ptr<Observable> Settings::evaluationDateGuard() const {
        return evaluationDate_;
    }
    #endif

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

}


#endif

