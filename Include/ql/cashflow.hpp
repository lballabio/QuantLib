
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
 * if not, contact ferdinando@ametrano.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/Authors.txt
*/

/*! \file cashflow.hpp
    \brief Base class for cash flows

    $Id$
*/

//  $Source$
//  $Log$
//  Revision 1.5  2001/07/25 15:47:27  sigmud
//  Change from quantlib.sourceforge.net to quantlib.org
//
//  Revision 1.4  2001/07/19 16:40:10  lballabio
//  Improved docs a bit
//
//  Revision 1.3  2001/07/16 16:07:42  lballabio
//  Market elements and stuff
//
//  Revision 1.2  2001/06/15 13:52:06  lballabio
//  Reworked indexes
//
//  Revision 1.1  2001/05/31 08:56:40  lballabio
//  Cash flows, scheduler, and generic swap added - the latter should be specialized and tested
//

#ifndef quantlib_cash_flow_hpp
#define quantlib_cash_flow_hpp

#include "ql/date.hpp"
#include "ql/Patterns/observable.hpp"

/*! \namespace QuantLib::CashFlows
    \brief Concrete implementations of the CashFlow interface
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
