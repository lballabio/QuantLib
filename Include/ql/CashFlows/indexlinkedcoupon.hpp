
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, contact ferdinando@ametrano.net
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/

/*! \file indexlinkedcoupon.hpp
    \brief Coupon paying an index fixing

    $Id$
    $Source$
    $Log$
    Revision 1.1  2001/05/31 08:56:40  lballabio
    Cash flows, scheduler, and generic swap added - the latter should be specialized and tested

*/

#ifndef quantlib_index_linked_coupon_hpp
#define quantlib_index_linked_coupon_hpp

#include "ql/CashFlows/accruingcoupon.hpp"
#include "ql/index.hpp"
#include "ql/spread.hpp"

namespace QuantLib {

    namespace CashFlows {
        
        //! Coupon paying an index fixing
        class IndexLinkedCoupon : public AccruingCoupon {
          public:
            IndexLinkedCoupon(double nominal, const Handle<Index>& index, 
                int n, TimeUnit unit, Spread spread, 
                const Handle<Calendar>& calendar, 
                RollingConvention rollingConvention, 
                const Handle<DayCounter>& dayCounter,
                const Date& startDate, const Date& endDate, 
                int fixingDays, const Date& refPeriodStart = Date(),
                const Date& refPeriodEnd = Date())
            : AccruingCoupon(calendar, rollingConvention, dayCounter, 
              startDate, endDate, refPeriodStart, refPeriodEnd), 
              nominal_(nominal), index_(index), n_(n), unit_(unit), 
              spread_(spread), fixingDays_(fixingDays) {}
            // CashFlow interface
            double amount() const;
            // other inspectors
            Rate fixing() const;
            Date fixingDate() const;
          private:
            double nominal_;
            Handle<Index> index_;
            int n_;
            TimeUnit unit_;
            Spread spread_;
            int fixingDays_;
        };

        inline double IndexLinkedCoupon::amount() const {
            return nominal_*(fixing()+spread_)*accrualPeriod();
        }

        inline Rate IndexLinkedCoupon::fixing() const {
            return index_->fixing(fixingDate(),n_,unit_);
        }
        
        inline Date IndexLinkedCoupon::fixingDate() const {
            return calendar_->advance(
                calendar_->roll(startDate_,rollingConvention_),
                    -fixingDays_,Days);
        }
        
    }

}


#endif
