
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

/*! \file cashflow.hpp
    \brief Base class for cash flows

    \fullpath
    ql/%cashflow.hpp
*/

// $Id$

#ifndef quantlib_cash_flow_hpp
#define quantlib_cash_flow_hpp

#include "ql/date.hpp"
#include "ql/Patterns/observable.hpp"

/*! \namespace QuantLib::CashFlows
    \brief Concrete implementations of the CashFlow interface

    /*! See sect. \ref cashflows */
*/

namespace QuantLib {

    //! Base class for cash flows
    /*! This class is purely virtual and acts as a base class for the actual 
        cash flow implementations. 
    */
    class CashFlow : public Patterns::Observable {
      public:
        virtual ~CashFlow() {}
        //! returns the amount of the cash flow
        /*! \note The amount is not discounted, i.e., it is the actual amount 
            paid at the cash flow date.
        */
        virtual double amount() const = 0;
        //! returns the date at which the cash flow is settled
        virtual Date date() const = 0;
    };

}


#endif
