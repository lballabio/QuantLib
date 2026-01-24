/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Chirag Desai

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

/*! \file fxforward.hpp
    \brief FX Forward instrument
*/

#ifndef quantlib_fx_forward_hpp
#define quantlib_fx_forward_hpp

#include <ql/currency.hpp>
#include <ql/instrument.hpp>
#include <ql/time/date.hpp>

namespace QuantLib {

    //! %FX Forward instrument
    /*! This class represents a foreign exchange forward contract,
        which is an agreement to exchange a specified amount of one
        currency for another currency at a future date at a
        predetermined exchange rate.

        The instrument can be valued using DiscountingFxForwardEngine,
        which computes the NPV by discounting the source and target
        legs using their respective yield curves.

        \ingroup instruments
    */
    class FxForward : public Instrument {
      public:
        class arguments;
        class results;
        class engine;
        //! \name Constructors
        //@{
        /*! Constructor for FX Forward using nominal amounts.
            \param sourceNominal    Notional amount in source(domestic) currency
            \param sourceCurrency   Currency of sourceNominal (source currency)
            \param targetNominal    Notional amount in target(foreign) currency
            \param targetCurrency   Currency of targetNominal (target currency)
            \param maturityDate     Settlement date of the forward contract
            \param paySourceCurrency If true, pay source currency and receive target currency;
                                    if false, receive source currency and pay target currency
        */
        FxForward(Real sourceNominal,
                  const Currency& sourceCurrency,
                  Real targetNominal,
                  const Currency& targetCurrency,
                  const Date& maturityDate,
                  bool paySourceCurrency);

        /*! Constructor for FX Forward using exchange rate.
            \param sourceNominal    Notional amount in source currency
            \param sourceCurrency   Currency of nominal amount
            \param targetCurrency   Currency to exchange into
            \param forwardRate      The forward exchange rate (target/source)
            \param maturityDate     Settlement date of the forward contract
            \param sellingSource    If true, sell source currency (pay source, receive target);
                                    if false, buy source currency (receive source, pay target)
        */
        FxForward(Real sourceNominal,
                  const Currency& sourceCurrency,
                  const Currency& targetCurrency,
                  Real forwardRate,
                  const Date& maturityDate,
                  bool sellingSource);
        //@}

        //! \name Inspectors
        //@{
        //! Source nominal amount
        Real sourceNominal() const { return sourceNominal_; }
        //! Source currency
        const Currency& sourceCurrency() const { return sourceCurrency_; }
        //! Target nominal amount
        Real targetNominal() const { return targetNominal_; }
        //! Target currency
        const Currency& targetCurrency() const { return targetCurrency_; }
        //! Settlement date
        const Date& maturityDate() const { return maturityDate_; }
        //! True if paying source currency
        bool paySourceCurrency() const { return paySourceCurrency_; }
        //! Contracted forward rate (target currency per unit of source currency)
        Real forwardRate() const { return targetNominal_ / sourceNominal_; }
        //@}

        //! \name Instrument interface
        //@{
        bool isExpired() const override;
        void setupArguments(PricingEngine::arguments*) const override;
        void fetchResults(const PricingEngine::results*) const override;
        //@}

        //! \name Additional results
        //@{
        //! Fair forward rate (targetCurrency/sourceCurrency),  the market-implied fair rate
        //! computed by the engine
        Real fairForwardRate() const;
        //! NPV in source currency terms
        Real npvSourceCurrency() const;
        //! NPV in target currency terms
        Real npvTargetCurrency() const;
        //@}

      private:
        Real sourceNominal_;
        Currency sourceCurrency_;
        Real targetNominal_;
        Currency targetCurrency_;
        Date maturityDate_;
        bool paySourceCurrency_;

        mutable Real fairForwardRate_;
        mutable Real npvSourceCurrency_;
        mutable Real npvTargetCurrency_;
    };

    //! Arguments for FX Forward pricing engine
    class FxForward::arguments : public virtual PricingEngine::arguments {
      public:
        Real sourceNominal = Null<Real>();
        Currency sourceCurrency;
        Real targetNominal = Null<Real>();
        Currency targetCurrency;
        Date maturityDate;
        bool paySourceCurrency = true;
        void validate() const override;
    };

    //! Results for FX Forward pricing engine
    class FxForward::results : public Instrument::results {
      public:
        Real fairForwardRate = Null<Real>();
        Real npvSourceCurrency = Null<Real>();
        Real npvTargetCurrency = Null<Real>();
        void reset() override;
    };

    //! Base class for FX Forward pricing engines
    class FxForward::engine : public GenericEngine<FxForward::arguments, FxForward::results> {};

}

#endif
