

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file simplecashflow.hpp
    \brief Predetermined cash flow

    \fullpath
    ql/CashFlows/%simplecashflow.hpp
*/

// $Id$

#ifndef quantlib_simple_cash_flow_hpp
#define quantlib_simple_cash_flow_hpp

#include <ql/cashflow.hpp>

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
