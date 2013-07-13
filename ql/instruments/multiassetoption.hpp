/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2004 Neil Firth
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

/*! \file multiassetoption.hpp
    \brief Option on multiple assets
*/

#ifndef quantlib_multiasset_option_hpp
#define quantlib_multiasset_option_hpp

#include <ql/option.hpp>

namespace QuantLib {

    //! Base class for options on multiple assets
    class MultiAssetOption : public Option {
      public:
        class engine;
        class results;
        MultiAssetOption(const boost::shared_ptr<Payoff>&,
                         const boost::shared_ptr<Exercise>&);
        //! \name Instrument interface
        //@{
        bool isExpired() const;
        //@}
        //! \name greeks
        //@{
        Real delta() const;
        Real gamma() const;
        Real theta() const;
        Real vega() const;
        Real rho() const;
        Real dividendRho() const;
        //@}
        void setupArguments(PricingEngine::arguments*) const;
        void fetchResults(const PricingEngine::results*) const;
      protected:
        void setupExpired() const;
        // results
        mutable Real delta_,  gamma_, theta_,
            vega_, rho_, dividendRho_;
    };

    //! %Results from multi-asset option calculation
    class MultiAssetOption::results : public Instrument::results,
                                      public Greeks {
      public:
        void reset() {
            Instrument::results::reset();
            Greeks::reset();
        }
    };

    class MultiAssetOption::engine :
        public GenericEngine<MultiAssetOption::arguments,
                             MultiAssetOption::results> {};

}


#endif

