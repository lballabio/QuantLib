
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
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
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file floatingratecoupon.hpp
    \brief Coupon at par on a term structure

    \fullpath
    ql/CashFlows/%floatingratecoupon.hpp
*/

// $Id$

#ifndef quantlib_floating_rate_coupon_hpp
#define quantlib_floating_rate_coupon_hpp

#include "ql/cashflow.hpp"
#include "ql/handle.hpp"
#include "ql/Indexes/xibor.hpp"

namespace QuantLib {

    namespace CashFlows {

        //! Coupon at par on a term structure
        /*! \warning This class does not perform any date adjustment,
            i.e., the start and end date passed upon construction
            should be already rolled to a business day.
	    */
        class FloatingRateCoupon : public CashFlow,
                                   public Patterns::Observer {
          public:
            FloatingRateCoupon(double nominal,
                const RelinkableHandle<TermStructure>& termStructure,
                const Date& startDate, const Date& endDate,
                const Date& refPeriodStart = Date(),
                const Date& refPeriodEnd = Date(),
                const Handle<Index>& index = Handle<Index>(),
                Spread spread = 0.0);
            ~FloatingRateCoupon();
            //! \name CashFlow interface
            //@{
            Date date() const { return endDate_; }
            double amount() const;
            //@}
            //! \name Observer interface
            //@{
            void update();
            //@}
          private:
            double accrualPeriod() const;
            double nominal_;
            RelinkableHandle<TermStructure> termStructure_;
            Date startDate_, endDate_;
            Date refPeriodStart_, refPeriodEnd_;
            Handle<Indexes::Xibor> index_;
            Spread spread_;
        };

    }

}


#endif
