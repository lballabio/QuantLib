/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 J. Erik Radmall

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

/*! \file commoditycashflow.hpp
    \brief Commodity cash flow
*/

#ifndef quantlib_commodity_cash_flow_hpp
#define quantlib_commodity_cash_flow_hpp

#include <ql/cashflow.hpp>
#include <ql/money.hpp>
#include <map>

namespace QuantLib {

    class CommodityCashFlow : public CashFlow {
      public:
        CommodityCashFlow(const Date& date,
                          Money discountedAmount,
                          Money undiscountedAmount,
                          Money discountedPaymentAmount,
                          Money undiscountedPaymentAmount,
                          Real discountFactor,
                          Real paymentDiscountFactor,
                          bool finalized)
        : date_(date), discountedAmount_(discountedAmount),
          undiscountedAmount_(undiscountedAmount),
          discountedPaymentAmount_(discountedPaymentAmount),
          undiscountedPaymentAmount_(undiscountedPaymentAmount),
          discountFactor_(discountFactor),
          paymentDiscountFactor_(paymentDiscountFactor),
          finalized_(finalized) {}
        //! \name Event interface
        //@{
        Date date() const { return date_; }
        //@}
        //! \name CashFlow interface
        //@{
        Real amount() const { return discountedAmount_.value(); }
        //@}
        const Currency& currency() const {
            return discountedAmount_.currency();
        }

        const Money& discountedAmount() const { return discountedAmount_; }
        const Money& undiscountedAmount() const { return undiscountedAmount_; }
        const Money& discountedPaymentAmount() const {
            return discountedPaymentAmount_;
        }
        const Money& undiscountedPaymentAmount() const {
            return undiscountedPaymentAmount_;
        }
        Real discountFactor() const { return discountFactor_; }
        Real paymentDiscountFactor() const { return paymentDiscountFactor_; }
        bool finalized() const { return finalized_; }

        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}
      private:
        Date date_;
        Money discountedAmount_, undiscountedAmount_,
              discountedPaymentAmount_, undiscountedPaymentAmount_;
        Real discountFactor_, paymentDiscountFactor_;
        bool finalized_;
    };

    typedef std::map<Date, boost::shared_ptr<CommodityCashFlow> >
                                                           CommodityCashFlows;

    #ifndef __DOXYGEN__
    std::ostream& operator<<(std::ostream& out,
                             const CommodityCashFlows& cashFlows);
    #endif

}

#endif
