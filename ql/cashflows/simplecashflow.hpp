/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2010 StatPro Italia srl

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

/*! \file simplecashflow.hpp
    \brief Predetermined cash flow
*/

#ifndef quantlib_simple_cash_flow_hpp
#define quantlib_simple_cash_flow_hpp

#include <ql/patterns/visitor.hpp>
#include <ql/cashflow.hpp>

namespace QuantLib {

    //! Predetermined cash flow
    /*! This cash flow pays a predetermined amount at a given date. */
    class SimpleCashFlow : public CashFlow {
      public:
        SimpleCashFlow(Real amount,
                       const Date& date);
        //! \name Event interface
        //@{
        Date date() const { return date_; }
        //@}
        //! \name CashFlow interface
        //@{
        Real amount() const { return amount_; }
        //@}
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}
      private:
        Real amount_;
        Date date_;
    };


    //! Bond redemption
    /*! This class specializes SimpleCashFlow so that visitors
        can perform more detailed cash-flow analysis.
    */
    class Redemption : public SimpleCashFlow {
      public:
        Redemption(Real amount,
                   const Date& date)
        : SimpleCashFlow(amount, date) {}
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}
    };

    //! Amortizing payment
    /*! This class specializes SimpleCashFlow so that visitors
        can perform more detailed cash-flow analysis.
    */
    class AmortizingPayment : public SimpleCashFlow {
      public:
        AmortizingPayment(Real amount,
                          const Date& date)
        : SimpleCashFlow(amount, date) {}
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}
    };


    // inline definitions

    inline void SimpleCashFlow::accept(AcyclicVisitor& v) {
        Visitor<SimpleCashFlow>* v1 =
            dynamic_cast<Visitor<SimpleCashFlow>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            CashFlow::accept(v);
    }

    inline void Redemption::accept(AcyclicVisitor& v) {
        Visitor<Redemption>* v1 =
            dynamic_cast<Visitor<Redemption>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            SimpleCashFlow::accept(v);
    }

    inline void AmortizingPayment::accept(AcyclicVisitor& v) {
        Visitor<AmortizingPayment>* v1 =
            dynamic_cast<Visitor<AmortizingPayment>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            SimpleCashFlow::accept(v);
    }

}

#endif
