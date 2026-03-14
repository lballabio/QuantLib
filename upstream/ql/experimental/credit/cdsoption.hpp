/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Roland Stamm
 Copyright (C) 2009 Jose Aparicio

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

/*! \file cdsoption.hpp
    \brief CDS option
*/

#ifndef quantlib_cds_option_hpp
#define quantlib_cds_option_hpp

#include <ql/option.hpp>
#include <ql/instruments/creditdefaultswap.hpp>

namespace QuantLib {

    class Quote;
    class YieldTermStructure;

    //! CDS option
    /*! The side of the swaption is set by choosing the side of the CDS.
        A receiver CDS option is a right to buy an underlying CDS
        selling protection and receiving a coupon. A payer CDS option
        is a right to buy an underlying CDS buying protection and
        paying coupon.
    */
    class CdsOption : public Option {
      public:
        class arguments;
        class results;
        class engine;
        CdsOption(const ext::shared_ptr<CreditDefaultSwap>& swap,
                  const ext::shared_ptr<Exercise>& exercise,
                  bool knocksOut = true);

        //! \name Instrument interface
        //@{
        bool isExpired() const override;
        void setupArguments(PricingEngine::arguments*) const override;
        //@}
        //! \name Inspectors
        //@{
        const ext::shared_ptr<CreditDefaultSwap>& underlyingSwap() const {
            return swap_;
        }
        //@}
        //! \name Calculations
        //@{
        Rate atmRate() const;
        Real riskyAnnuity() const;
        Volatility impliedVolatility(
                              Real price,
                              const Handle<YieldTermStructure>& termStructure,
                              const Handle<DefaultProbabilityTermStructure>&,
                              Real recoveryRate,
                              Real accuracy = 1.e-4,
                              Size maxEvaluations = 100,
                              Volatility minVol = 1.0e-7,
                              Volatility maxVol = 4.0) const;
        //@}

    private:
        ext::shared_ptr<CreditDefaultSwap> swap_;
        bool knocksOut_;

        mutable Real riskyAnnuity_;
        void setupExpired() const override;
        void fetchResults(const PricingEngine::results*) const override;
    };


    //! %Arguments for CDS-option calculation
    class CdsOption::arguments : public CreditDefaultSwap::arguments,
                                 public Option::arguments {
      public:
        arguments() = default;

        ext::shared_ptr<CreditDefaultSwap> swap;
        bool knocksOut;
        void validate() const override;
    };

    //! %Results from CDS-option calculation
    class CdsOption::results : public Option::results {
      public:
        Real riskyAnnuity;
        void reset() override;
    };

    //! base class for swaption engines
    class CdsOption::engine
        : public GenericEngine<CdsOption::arguments, CdsOption::results> {};

}

#endif
