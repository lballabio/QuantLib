
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

/*! \file floatingratecoupon.hpp
    \brief Coupon at par on a term structure

    $Id$
    $Source$
    $Log$
    Revision 1.3  2001/06/21 11:34:23  lballabio
    Ensured that floating rate coupon index stays alive

    Revision 1.2  2001/06/18 11:51:38  lballabio
    Fixed cash flow date

    Revision 1.1  2001/06/18 08:10:00  lballabio
    Reworked indexes and floating rate coupon

*/

#ifndef quantlib_floating_rate_coupon_hpp
#define quantlib_floating_rate_coupon_hpp

#include "ql/cashflow.hpp"
#include "ql/index.hpp"
#include "ql/termstructure.hpp"
#include "ql/handle.hpp"
#include "ql/Indexes/xibor.hpp"

namespace QuantLib {

    namespace CashFlows {
        
        //! Coupon at par on a term structure
        /*! \warning This class does not perform any date adjustment, 
            i.e., the start and end date passed upon construction
            should be already rolled to a business day.
	*/
        class FloatingRateCoupon : public CashFlow {
          public:
            FloatingRateCoupon(double nominal,  
                const RelinkableHandle<TermStructure>& termStructure,
                const Date& startDate, const Date& endDate, 
                const Date& refPeriodStart = Date(),
                const Date& refPeriodEnd = Date(),
                const Handle<Index>& index = Handle<Index>(),
                Spread spread = 0.0);
            //! \name CashFlow interface
            //@{
            Date date() const { return endDate_; }
            double amount() const;
            //@}
          private:
            double accrualPeriod() const;
            double nominal_;
            Date startDate_, endDate_;
            Date refPeriodStart_, refPeriodEnd_;
            Handle<Indexes::Xibor> index_;
            Spread spread_;
            RelinkableHandle<TermStructure> termStructure_;
        };

    }

}


#endif
