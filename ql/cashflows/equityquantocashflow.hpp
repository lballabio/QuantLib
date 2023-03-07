/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2023 Marcin Rybacki

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

/*! \file equityquantocashflow.hpp
    \brief equity quanto cash flow
*/

#ifndef quantlib_equity_quanto_cash_flow_hpp
#define quantlib_equity_quanto_cash_flow_hpp

#include <ql/termstructures/volatility/equityfx/blackvoltermstructure.hpp>
#include <ql/termstructures/yield/zeroyieldstructure.hpp>
#include <ql/cashflow.hpp>
#include <ql/patterns/visitor.hpp>

namespace QuantLib {
    
    class EquityIndex;
    class EquityQuantoCashFlowPricer;
    
    class EquityQuantoCashFlow : public CashFlow, public Observer {
       public:
        EquityQuantoCashFlow(Real notional,
                             ext::shared_ptr<EquityIndex> equityIndex,
                             const Date& startDate,
                             const Date& endDate,
                             const Date& paymentDate);
        //! \name Inspectors
        //@{
        Real notional() const { return notional_; }
        const ext::shared_ptr<EquityIndex>& equityIndex() const { return equityIndex_; }
        Date startDate() const { return startDate_; }
        Date endDate() const { return endDate_; }
        Date paymentDate() const { return paymentDate_; }
        //@}
        //! \name CashFlow interface
        //@{
        Real amount() const override;
        //@}
        //! \name Observer interface
        //@{
        void update() override { notifyObservers(); }
        //@}
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&) override;
        //@}
        void setPricer(const ext::shared_ptr<EquityQuantoCashFlowPricer>&);
        const ext::shared_ptr<EquityQuantoCashFlowPricer>& pricer() const { return pricer_; };

      private:
        Real notional_;
        ext::shared_ptr<EquityIndex> equityIndex_;
        Date startDate_, endDate_, paymentDate_;
        ext::shared_ptr<EquityQuantoCashFlowPricer> pricer_;
    };

    inline void EquityQuantoCashFlow::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<EquityQuantoCashFlow>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            CashFlow::accept(v);
    }

    void setCouponPricer(const Leg& leg, const ext::shared_ptr<EquityQuantoCashFlowPricer>&);

    class EquityQuantoCashFlowPricer : public virtual Observer, public virtual Observable {
      public:
        EquityQuantoCashFlowPricer(Handle<YieldTermStructure> quantoCurrencyTermStructure,
                                   Handle<BlackVolTermStructure> equityVolatility,
                                   Handle<BlackVolTermStructure> fxVolatility,
                                   Handle<Quote> correlation);
        //! \name Interface
        //@{
        virtual Real quantoAmount() const;
        virtual void initialize(const EquityQuantoCashFlow&);
        //@}

        //! \name Observer interface
        //@{
        void update() override { notifyObservers(); }
        //@}
      private:
        Handle<YieldTermStructure> quantoCurrencyTermStructure_, quantoTermStructure;
        Handle<BlackVolTermStructure> equityVolatility_, fxVolatility_;
        Handle<Quote> correlation_;

        const EquityQuantoCashFlow* cashFlow_;
    };
}

#endif