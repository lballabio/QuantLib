/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

/*! \file vanillaoption.hpp
    \brief Vanilla option on a single asset
*/

#ifndef quantlib_vanilla_option_hpp
#define quantlib_vanilla_option_hpp

#include <ql/instruments/oneassetoption.hpp>
#include <ql/instruments/payoffs.hpp>

namespace QuantLib {

    class GeneralizedBlackScholesProcess;

    //! Vanilla option (no discrete dividends, no barriers) on a single asset
    /*! \ingroup instruments */
    class VanillaOption : public OneAssetOption {
      public:
        VanillaOption(const ext::shared_ptr<StrikedTypePayoff>&,
                      const ext::shared_ptr<Exercise>&);

        /*! \warning currently, this method returns the Black-Scholes
                     implied volatility using analytic formulas for
                     European options and a finite-difference method
                     for American and Bermudan options. It will give
                     unconsistent results if the pricing was performed
                     with any other methods (such as jump-diffusion
                     models.)

            \warning options with a gamma that changes sign (e.g.,
                     binary options) have values that are <b>not</b>
                     monotonic in the volatility. In these cases, the
                     calculation can fail and the result (if any) is
                     almost meaningless.  Another possible source of
                     failure is to have a target value that is not
                     attainable with any volatility, e.g., a target
                     value lower than the intrinsic value in the case
                     of American options.
        */
        Volatility impliedVolatility(
             Real price,
             const ext::shared_ptr<GeneralizedBlackScholesProcess>& process,
             Real accuracy = 1.0e-4,
             Size maxEvaluations = 100,
             Volatility minVol = 1.0e-7,
             Volatility maxVol = 4.0) const;
    };

}


#endif



#ifndef id_204287a88d7745de4514f96537b59ccc
#define id_204287a88d7745de4514f96537b59ccc
inline bool test_204287a88d7745de4514f96537b59ccc(const int* i) {
    return i != nullptr;
}
#endif
