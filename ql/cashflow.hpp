
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software developed by the QuantLib Group; you can
 redistribute it and/or modify it under the terms of the QuantLib License;
 either version 1.0, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 QuantLib License for more details.

 You should have received a copy of the QuantLib License along with this
 program; if not, please email ferdinando@ametrano.net

 The QuantLib License is also available at http://quantlib.org/license.html
 The members of the QuantLib Group are listed in the QuantLib License
*/
/*! \file cashflow.hpp
    \brief Base class for cash flows

    \fullpath
    ql/%cashflow.hpp
*/

// $Id$

#ifndef quantlib_cash_flow_hpp
#define quantlib_cash_flow_hpp

#include <ql/date.hpp>
#include <ql/Patterns/observable.hpp>

/*! \namespace QuantLib::CashFlows
    \brief Concrete implementations of the CashFlow interface

    See sect. \ref cashflows
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
