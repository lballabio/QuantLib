/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2007 StatPro Italia srl

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

/*! \file oneassetoption.hpp
    \brief Option on a single asset
*/

#ifndef quantlib_oneasset_option_hpp
#define quantlib_oneasset_option_hpp

#include <ql/option.hpp>

namespace QuantLib {

    //! Base class for options on a single asset
    class OneAssetOption : public Option {
      public:
        class engine;
        class results;
        OneAssetOption(const std::shared_ptr<Payoff>&,
                       const std::shared_ptr<Exercise>&);
        //! \name Instrument interface
        //@{
        bool isExpired() const override;
        //@}
        //! \name greeks
        //@{
        Real delta() const;
        Real deltaForward() const;
        Real elasticity() const;
        Real gamma() const;
        Real theta() const;
        Real thetaPerDay() const;
        Real vega() const;
        Real rho() const;
        Real dividendRho() const;
        Real strikeSensitivity() const;
        Real itmCashProbability() const;
        //@}
        void fetchResults(const PricingEngine::results*) const override;

      protected:
        void setupExpired() const override;
        // results
        mutable Real delta_, deltaForward_, elasticity_, gamma_, theta_,
            thetaPerDay_, vega_, rho_, dividendRho_, strikeSensitivity_,
            itmCashProbability_;
    };

    //! %Results from single-asset option calculation
    class OneAssetOption::results : public Instrument::results,
                                    public Greeks,
                                    public MoreGreeks {
      public:
        void reset() override {
            Instrument::results::reset();
            Greeks::reset();
            MoreGreeks::reset();
        }
    };

    class OneAssetOption::engine :
        public GenericEngine<OneAssetOption::arguments,
                             OneAssetOption::results> {};

}


#endif

