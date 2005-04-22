/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 Neil Firth
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

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
#include <ql/stochasticprocess.hpp>
#include <ql/Math/matrix.hpp>

namespace QuantLib {

    //! Base class for options on multiple assets
    class MultiAssetOption : public Option {
      public:
        MultiAssetOption(
               const std::vector<boost::shared_ptr<StochasticProcess1D> >&
                                                                   stochProcs,
               const boost::shared_ptr<Payoff>& payoff,
               const boost::shared_ptr<Exercise>& exercise,
               const Matrix& correlation,
               const boost::shared_ptr<PricingEngine>& engine =
                                           boost::shared_ptr<PricingEngine>());
        //! \name Instrument interface
        //@{
        class arguments;
        class results;
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
        void setupArguments(Arguments*) const;
      protected:
        void setupExpired() const;
        void performCalculations() const;
        // results
        mutable Real delta_,  gamma_, theta_,
            vega_, rho_, dividendRho_;
        // arguments
        std::vector<boost::shared_ptr<StochasticProcess1D> >
            stochasticProcesses_;
        Matrix correlation_;
    };

    //! %Arguments for multi-asset option calculation
    class MultiAssetOption::arguments : public Option::arguments {
      public:
        arguments() {}
        void validate() const;
        std::vector<boost::shared_ptr<StochasticProcess1D> >
            stochasticProcesses;
        Matrix correlation;
    };

    //! %Results from multi-asset option calculation
    class MultiAssetOption::results : public Value,
                                      public Greeks {
      public:
        void reset() {
            Value::reset();
            Greeks::reset();
        }
    };

}


#endif

