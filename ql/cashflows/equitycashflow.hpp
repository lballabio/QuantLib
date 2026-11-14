/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2023 Marcin Rybacki

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file equitycashflow.hpp
    \brief equity cash flow
*/

#ifndef quantlib_equity_cash_flow_hpp
#define quantlib_equity_cash_flow_hpp

#include <ql/termstructures/volatility/equityfx/blackvoltermstructure.hpp>
#include <ql/termstructures/yield/zeroyieldstructure.hpp>
#include <ql/cashflows/indexedcashflow.hpp>
#include <ql/patterns/visitor.hpp>

namespace QuantLib {
    
    class EquityIndex;
    class EquityCashFlowPricer;
    
    class EquityCashFlow : public IndexedCashFlow {
       public:
        EquityCashFlow(Real notional,
                       ext::shared_ptr<EquityIndex> index,
                       const Date& baseDate,
                       const Date& fixingDate,
                       const Date& paymentDate,
                       bool growthOnly = true);
        //! \name CashFlow interface
        //@{
        Real amount() const override;
        //@}
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&) override;
        //@}
        void setPricer(const ext::shared_ptr<EquityCashFlowPricer>&);
        const ext::shared_ptr<EquityCashFlowPricer>& pricer() const { return pricer_; };

      private:
        ext::shared_ptr<EquityCashFlowPricer> pricer_;
    };

    inline void EquityCashFlow::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<EquityCashFlow>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            IndexedCashFlow::accept(v);
    }

    //! Equity cash flow whose notional is reset to the index level
    /*! The notional of the cash flow tracks a fixed number of index
        units: given an initial notional \f$ N \f$ set on the initial
        date \f$ T_0 \f$, the notional for the period between the
        base date \f$ T_{i-1} \f$ and the fixing date \f$ T_i \f$ is
        \f[
        N_i = N \frac{I(T_{i-1})}{I(T_0)},
        \f]
        so that the amount is
        \f[
        N_i \left[ \frac{I(T_i)}{I(T_{i-1})} - 1 \right]
        = \frac{N}{I(T_0)} \left[ I(T_i) - I(T_{i-1}) \right].
        \f]

        \warning the notional is always calculated from the fixings
                 of the index as given.  If a pricer is set, e.g.,
                 a quanto pricer, its adjustment applies to the
                 return over the period but not to the notional.
    */
    class EquityNotionalResetCashFlow : public EquityCashFlow {
      public:
        EquityNotionalResetCashFlow(Real initialNotional,
                                    ext::shared_ptr<EquityIndex> index,
                                    const Date& initialDate,
                                    const Date& baseDate,
                                    const Date& fixingDate,
                                    const Date& paymentDate);
        //! \name IndexedCashFlow interface
        //@{
        //! the notional reset on the base date
        Real notional() const override;
        //@}
        //! \name Inspectors
        //@{
        //! the notional on the initial date
        Real initialNotional() const { return IndexedCashFlow::notional(); }
        //! the date on which the initial notional was set
        const Date& initialDate() const { return initialDate_; }
        //@}
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&) override;
        //@}
      private:
        Date initialDate_;
    };

    inline void EquityNotionalResetCashFlow::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<EquityNotionalResetCashFlow>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            EquityCashFlow::accept(v);
    }

    void setCouponPricer(const Leg& leg, const ext::shared_ptr<EquityCashFlowPricer>&);

    class EquityCashFlowPricer : public virtual Observer, public virtual Observable {
      public:
        EquityCashFlowPricer() = default;
        //! \name Interface
        //@{
        virtual Real price() const = 0;
        virtual void initialize(const EquityCashFlow&) = 0;
        //@}

        //! \name Observer interface
        //@{
        void update() override { notifyObservers(); }
        //@}
      protected:
        ext::shared_ptr<EquityIndex> index_;
        Date baseDate_, fixingDate_;
        bool growthOnlyPayoff_;
    };

    class EquityQuantoCashFlowPricer : public EquityCashFlowPricer {
      public:
        EquityQuantoCashFlowPricer(Handle<YieldTermStructure> quantoCurrencyTermStructure,
                                   Handle<BlackVolTermStructure> equityVolatility,
                                   Handle<BlackVolTermStructure> fxVolatility,
                                   Handle<Quote> correlation);
        //! \name Interface
        //@{
        Real price() const override;
        void initialize(const EquityCashFlow&) override;
        //@}
      private:
        Handle<YieldTermStructure> quantoCurrencyTermStructure_, quantoTermStructure;
        Handle<BlackVolTermStructure> equityVolatility_, fxVolatility_;
        Handle<Quote> correlation_;
    };
}

#endif