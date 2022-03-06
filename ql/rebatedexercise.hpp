/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013 Peter Caspers

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

/*! \file rebatedexercise.hpp
    \brief Option exercise with rebate payments
*/

#ifndef quantlib_rebatedexercise_hpp
#define quantlib_rebatedexercise_hpp

#include <ql/exercise.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/errors.hpp>

namespace QuantLib {

    //! Rebated exercise
    /*! in case of exercise the holder receives a rebate (if positive) or pays
       it (if negative)
        on the rebate settlement date
    */
    class RebatedExercise : public Exercise {
      public:
        // in case of exercise the holder receives the rebate
        // (if positive) or pays it (if negative) on the rebate
        // settlement date
        RebatedExercise(const Exercise& exercise,
                        Real rebate = 0.0,
                        Natural rebateSettlementDays = 0,
                        Calendar rebatePaymentCalendar = NullCalendar(),
                        BusinessDayConvention rebatePaymentConvention = Following);
        RebatedExercise(const Exercise& exercise,
                        const std::vector<Real>& rebates,
                        Natural rebateSettlementDays = 0,
                        Calendar rebatePaymentCalendar = NullCalendar(),
                        BusinessDayConvention rebatePaymentConvention = Following);
        Real rebate(Size index) const;
        Date rebatePaymentDate(Size index) const;
        const std::vector<Real> &rebates() const { return rebates_; }

      private:
        const std::vector<Real> rebates_;
        const Natural rebateSettlementDays_;
        const Calendar rebatePaymentCalendar_;
        const BusinessDayConvention rebatePaymentConvention_;
    };

    inline Real RebatedExercise::rebate(Size index) const {
        QL_REQUIRE(index < rebates_.size(),
                   "rebate with index " << index << " does not exist (0..."
                   << (rebates_.size()-1) << ")");
        return rebates_[index];
    }

    inline Date RebatedExercise::rebatePaymentDate(Size index) const {
        QL_REQUIRE(type_ == European || type_ == Bermudan,
                   "for american style exercises the rebate payment date "
                       << "has to be calculted in the client code");
        return rebatePaymentCalendar_.advance(dates_[index],
                                              rebateSettlementDays_, Days,
                                              rebatePaymentConvention_);
    }

}

#endif


#ifndef id_b47dc39377bd25ca31c0de728275c6d3
#define id_b47dc39377bd25ca31c0de728275c6d3
inline bool test_b47dc39377bd25ca31c0de728275c6d3(int* i) { return i != 0; }
#endif
