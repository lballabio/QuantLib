/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis

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

/*! \file twoassetcorrelationoption.hpp
    \brief Two-asset correlation option
*/

#ifndef quantlib_two_asset_correlation_option_hpp
#define quantlib_two_asset_correlation_option_hpp

#include <ql/instruments/multiassetoption.hpp>
#include <ql/instruments/payoffs.hpp>

namespace QuantLib {

    class TwoAssetCorrelationOption : public MultiAssetOption {
      public:
        class arguments;
        class engine;
        TwoAssetCorrelationOption(Option::Type type,
                                  Real strike1,
                                  Real strike2,
                                  const ext::shared_ptr<Exercise>&);
        void setupArguments(PricingEngine::arguments*) const;
      protected:
        Real X2_;
    };

    class TwoAssetCorrelationOption::arguments
        : public MultiAssetOption::arguments {
      public:
        arguments() : X2(Null<Real>()) {}
        void validate() const {
            MultiAssetOption::arguments::validate();
            QL_REQUIRE(X2 != Null<Real>(), "no X2 given");
        }
        Real X2;
    };

    class TwoAssetCorrelationOption::engine
        : public GenericEngine<TwoAssetCorrelationOption::arguments,
                               TwoAssetCorrelationOption::results> {};

}


#endif
