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
        Date date() const override { return date_; }
        //@}
        //! \name CashFlow interface
        //@{
        Real amount() const override { return amount_; }
        //@}
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&) override;
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
        void accept(AcyclicVisitor&) override;
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
        void accept(AcyclicVisitor&) override;
        //@}
    };


    // inline definitions

    inline void SimpleCashFlow::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<SimpleCashFlow>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            CashFlow::accept(v);
    }

    inline void Redemption::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<Redemption>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            SimpleCashFlow::accept(v);
    }

    inline void AmortizingPayment::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<AmortizingPayment>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            SimpleCashFlow::accept(v);
    }

}

#endif


#ifndef id_7386d6f69cdc8964653be5650a6df08c
#define id_7386d6f69cdc8964653be5650a6df08c
inline bool test_7386d6f69cdc8964653be5650a6df08c(int* i) { return i != 0; }
#endif
