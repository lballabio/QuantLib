
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

/*! \file simplecashflow.hpp
    \fullpath Include/ql/CashFlows/%simplecashflow.hpp
    \brief Predetermined cash flow

    $Id$
*/

//  $Source$
//  $Log$
//  Revision 1.5  2001/08/08 11:07:48  sigmud
//  inserting \fullpath for doxygen
//
//  Revision 1.4  2001/08/07 11:25:53  sigmud
//  copyright header maintenance
//
//  Revision 1.3  2001/07/25 15:47:27  sigmud
//  Change from quantlib.sourceforge.net to quantlib.org
//
//  Revision 1.2  2001/06/15 13:52:07  lballabio
//  Reworked indexes
//
//  Revision 1.1  2001/05/31 08:56:40  lballabio
//  Cash flows, scheduler, and generic swap added - the latter should be specialized and tested
//

#ifndef quantlib_simple_cash_flow_hpp
#define quantlib_simple_cash_flow_hpp

#include "ql/cashflow.hpp"

namespace QuantLib {

    namespace CashFlows {
        
        //! Predetermined cash flow
        /*! This cash flow pays a predetermined amount at a given date. */
        class SimpleCashFlow : public CashFlow {
          public:
            SimpleCashFlow(double amount, const Date& date)
            : amount_(amount), date_(date) {}
            double amount() const { return amount_; }
            Date date() const { return date_; }
          private:
            double amount_;
            Date date_;
        };

    }

}


#endif
