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

/*! \file analytictwoassetcorrelationengine.hpp
    \brief Analytic engine for two-asset correlation options
*/

#ifndef quantlib_analytic_two_asset_correlation_option_engine_hpp
#define quantlib_analytic_two_asset_correlation_option_engine_hpp

#include <ql/experimental/exoticoptions/twoassetcorrelationoption.hpp>
#include <ql/processes/blackscholesprocess.hpp>

namespace QuantLib {

    //! Analytic two-asset correlation option engine
    class AnalyticTwoAssetCorrelationEngine
        : public TwoAssetCorrelationOption::engine {
      public:
        AnalyticTwoAssetCorrelationEngine(ext::shared_ptr<GeneralizedBlackScholesProcess> p1,
                                          ext::shared_ptr<GeneralizedBlackScholesProcess> p2,
                                          Handle<Quote> correlation);
        void calculate() const override;

      private:
        ext::shared_ptr<GeneralizedBlackScholesProcess> p1_;
        ext::shared_ptr<GeneralizedBlackScholesProcess> p2_;
        Handle<Quote> correlation_;
    };

}


#endif


#ifndef id_776f8d18c9e9fddd6e2cb795cc47ceb8
#define id_776f8d18c9e9fddd6e2cb795cc47ceb8
inline bool test_776f8d18c9e9fddd6e2cb795cc47ceb8(const int* i) {
    return i != nullptr;
}
#endif
