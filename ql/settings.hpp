
/*
 Copyright (C) 2004 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

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
#include <ql/Patterns/observable.hpp>

namespace QuantLib {

    //! global repository for run-time library settings
    class Settings : public Singleton<Settings> {
        friend class Singleton<Settings>;
      private:
        Settings();
      public:
        //! \name Evaluation date
        //@{
        //! the date at which pricing is to be performed
        /*! If not set, the current date will be used */
        Date evaluationDate() const;
        //! change the evaluation date and notify registered instruments
        /*! \note settings the evaluation date to the null date will
                  cause evaluationDate() to return today's date.
        */
        void setEvaluationDate(const Date&);
        //! observable sending notification when the evaluation date changes
        /*! \warning this observable does not send a notification when
                     the evaluation date changes for natural
                     causes---i.e., a date was not explicitly set
                     (which results in today's date being used for
                     pricing) and the current date changes as the
                     clock strikes midnight.
        */
        boost::shared_ptr<Observable> evaluationDateGuard() const;
        //@}
      private:
        Date evaluationDate_;
        boost::shared_ptr<Observable> evaluationDateGuard_;
    };


    // inline definitions

    inline Settings::Settings() {
        evaluationDateGuard_ = boost::shared_ptr<Observable>(new Observable);
    }

    inline Date Settings::evaluationDate() const {
        if (evaluationDate_ == Date())
            return Date::todaysDate();
        else
            return evaluationDate_;
    }

    inline void Settings::setEvaluationDate(const Date& d) {
        evaluationDate_ = d;
        evaluationDateGuard_->notifyObservers();
    }

    inline
    boost::shared_ptr<Observable> Settings::evaluationDateGuard() const {
        return evaluationDateGuard_;
    }

}


#endif

