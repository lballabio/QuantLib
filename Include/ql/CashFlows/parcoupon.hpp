
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

/*! \file parcoupon.hpp
    \brief Coupon at par on a term structure

    $Id$
    $Source$
    $Log$
    Revision 1.1  2001/06/01 16:50:16  lballabio
    Term structure on deposits and swaps

*/

#ifndef quantlib_par_coupon_hpp
#define quantlib_par_coupon_hpp

#include "ql/CashFlows/accruingcoupon.hpp"
#include "ql/index.hpp"
#include "ql/Indexes/xibor.hpp"
#include "ql/spread.hpp"

namespace QuantLib {

    namespace CashFlows {
        
        //! Coupon at par on a term structure
        class ParCoupon : public AccruingCoupon {
          public:
            ParCoupon(double nominal, const Indexes::Xibor& index, 
                int n, TimeUnit unit, Spread spread, 
                const Handle<Calendar>& calendar, 
                const Handle<DayCounter>& dayCounter,
                const RelinkableHandle<TermStructure>& termStructure,
                const Date& startDate, const Date& endDate, 
                const Date& refPeriodStart = Date(),
                const Date& refPeriodEnd = Date());
            // CashFlow interface
            double amount() const;
          private:
            double nominal_;
            Spread spread_;
            Indexes::Xibor index_;
            int n_;
            TimeUnit unit_;
            RelinkableHandle<TermStructure> termStructure_;
        };

    }

}


#endif
