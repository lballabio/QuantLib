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
#include <ql/cashflows/dividend.hpp>
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
        //! \name LazyObject interface
        //@{
        void performCalculations() const override;
        //@}
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&) override;
        //@}
        void setPricer(const ext::shared_ptr<EquityCashFlowPricer>&);
        const ext::shared_ptr<EquityCashFlowPricer>& pricer() const { return pricer_; };

        //! \name Index levels
        //@{
        /*! the level of the index on the given date, i.e., its fixing
            if the date is in the past, or its forecast otherwise; if
            a pricer is set, the level is the one used by the pricer,
            e.g., including a quanto adjustment.
        */
        Real indexLevel(const Date& d) const;
        /*! the number of index units corresponding to the notional
            over the period, i.e., the notional divided by the index
            level on the base date.
        */
        Real indexUnits() const;
        //@}

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

        The index levels used for the notional are those returned by
        indexLevel(); therefore, if a pricer is set, e.g., a quanto
        pricer, its adjustment applies to the notional as well as to
        the return over the period.
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

    //! Dividend paid on the index units underlying an equity cash flow
    /*! The amount of the cash flow is
        \f[
        f \, D \, u_i,
        \f]
        where \f$ D \f$ is the dividend per index unit (that is, in
        index points, as returned by Dividend::amount()), \f$ f \f$ is
        the dividend factor, i.e., the share of the dividend passed on
        to the receiver (for instance, net of withholding taxes), and
        \f$ u_i \f$ is the number of index units underlying the given
        equity cash flow (see EquityCashFlow::indexUnits()).  The
        ex-dividend date must fall within the period of the equity
        cash flow, i.e., after its base date and no later than its
        fixing date.

        \note When the base date of the equity cash flow is in the
              future, the number of index units is forecast by
              dividing its notional by the forecast index level; no
              convexity adjustment is made.
    */
    class EquityDividendCashFlow : public CashFlow {
      public:
        EquityDividendCashFlow(ext::shared_ptr<Dividend> dividend,
                               ext::shared_ptr<EquityCashFlow> equityCashFlow,
                               Real dividendFactor,
                               const Date& paymentDate);
        //! \name Event interface
        //@{
        Date date() const override { return paymentDate_; }
        //@}
        //! \name CashFlow interface
        //@{
        Real amount() const override;
        //@}
        //! \name LazyObject interface
        //@{
        void performCalculations() const override {}
        //@}
        //! \name Inspectors
        //@{
        const ext::shared_ptr<Dividend>& dividend() const { return dividend_; }
        const ext::shared_ptr<EquityCashFlow>& equityCashFlow() const { return equityCashFlow_; }
        Real dividendFactor() const { return dividendFactor_; }
        Date exDividendDate() const { return dividend_->date(); }
        //@}
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&) override;
        //@}
      private:
        ext::shared_ptr<Dividend> dividend_;
        ext::shared_ptr<EquityCashFlow> equityCashFlow_;
        Real dividendFactor_;
        Date paymentDate_;
    };

    inline void EquityDividendCashFlow::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<EquityDividendCashFlow>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            CashFlow::accept(v);
    }

    void setCouponPricer(const Leg& leg, const ext::shared_ptr<EquityCashFlowPricer>&);

    class EquityCashFlowPricer : public virtual Observer, public virtual Observable {
      public:
        EquityCashFlowPricer() = default;
        //! \name Interface
        //@{
        virtual Real price() const = 0;
        virtual void initialize(const EquityCashFlow&) = 0;
        /*! the level of the index on the given date under the
            measure used by the pricer; by default, the fixing of the
            index (or its forecast for a future date).
        */
        virtual Real indexLevel(const ext::shared_ptr<EquityIndex>& index, const Date& d) const;
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
        //! the quanto-adjusted level of the index on the given date
        Real indexLevel(const ext::shared_ptr<EquityIndex>& index, const Date& d) const override;
        //@}
      private:
        void checkInputs() const;
        ext::shared_ptr<EquityIndex> quantoIndex(const ext::shared_ptr<EquityIndex>& index,
                                                 Real strike) const;

        Handle<YieldTermStructure> quantoCurrencyTermStructure_, quantoTermStructure;
        Handle<BlackVolTermStructure> equityVolatility_, fxVolatility_;
        Handle<Quote> correlation_;
    };
}

#endif