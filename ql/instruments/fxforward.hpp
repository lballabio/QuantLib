/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2024

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
        which computes the NPV by discounting the domestic and foreign
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
            \param nominal1         Amount to be exchanged (positive = pay, negative = receive)
            \param currency1        Currency of nominal1
            \param nominal2         Amount to be exchanged (positive = pay, negative = receive)
            \param currency2        Currency of nominal2
            \param maturityDate     Settlement date of the forward contract
            \param payCurrency1     If true, pay currency1 and receive currency2;
                                    if false, receive currency1 and pay currency2
        */
        FxForward(Real nominal1,
                  const Currency& currency1,
                  Real nominal2,
                  const Currency& currency2,
                  const Date& maturityDate,
                  bool payCurrency1);

        /*! Constructor for FX Forward using exchange rate.
            \param nominal          Notional amount in source currency
            \param sourceCurrency   Currency of the nominal amount
            \param targetCurrency   Currency to exchange into
            \param forwardRate      The forward exchange rate (target/source)
            \param maturityDate     Settlement date of the forward contract
            \param sellingSource    If true, sell source currency (pay source, receive target);
                                    if false, buy source currency (receive source, pay target)
        */
        FxForward(Real nominal,
                  const Currency& sourceCurrency,
                  const Currency& targetCurrency,
                  Real forwardRate,
                  const Date& maturityDate,
                  bool sellingSource);
        //@}

        //! \name Inspectors
        //@{
        //! First nominal amount
        Real nominal1() const { return nominal1_; }
        //! First currency
        const Currency& currency1() const { return currency1_; }
        //! Second nominal amount
        Real nominal2() const { return nominal2_; }
        //! Second currency
        const Currency& currency2() const { return currency2_; }
        //! Settlement date
        const Date& maturityDate() const { return maturityDate_; }
        //! True if paying currency1
        bool payCurrency1() const { return payCurrency1_; }
        //@}

        //! \name Instrument interface
        //@{
        bool isExpired() const override;
        void setupArguments(PricingEngine::arguments*) const override;
        void fetchResults(const PricingEngine::results*) const override;
        //@}

        //! \name Additional results
        //@{
        //! Fair forward rate (currency2/currency1)
        Real fairForwardRate() const;
        //! NPV in currency1 terms
        Real npvCurrency1() const;
        //! NPV in currency2 terms
        Real npvCurrency2() const;
        //@}

      private:
        Real nominal1_;
        Currency currency1_;
        Real nominal2_;
        Currency currency2_;
        Date maturityDate_;
        bool payCurrency1_;

        mutable Real fairForwardRate_;
        mutable Real npvCurrency1_;
        mutable Real npvCurrency2_;
    };

    //! Arguments for FX Forward pricing engine
    class FxForward::arguments : public virtual PricingEngine::arguments {
      public:
        Real nominal1 = Null<Real>();
        Currency currency1;
        Real nominal2 = Null<Real>();
        Currency currency2;
        Date maturityDate;
        bool payCurrency1 = true;
        void validate() const override;
    };

    //! Results for FX Forward pricing engine
    class FxForward::results : public Instrument::results {
      public:
        Real fairForwardRate = Null<Real>();
        Real npvCurrency1 = Null<Real>();
        Real npvCurrency2 = Null<Real>();
        void reset() override;
    };

    //! Base class for FX Forward pricing engines
    class FxForward::engine : public GenericEngine<FxForward::arguments, FxForward::results> {};

}

#endif
