/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Joseph Wang
 Copyright (C) 2009 StatPro Italia srl

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

/*! \file fddividendamericanengine.hpp
    \brief american engine with discrete deterministic dividends
*/

#ifndef quantlib_fd_dividend_american_engine_hpp
#define quantlib_fd_dividend_american_engine_hpp

#include <ql/instruments/dividendvanillaoption.hpp>
#include <ql/pricingengines/vanilla/fddividendengine.hpp>
#include <ql/pricingengines/vanilla/fdconditions.hpp>

namespace QuantLib {

    //! Finite-differences pricing engine for dividend American options
    /*! \ingroup vanillaengines

        \test
        - the correctness of the returned greeks is tested by
          reproducing numerical derivatives.
        - the invariance of the results upon addition of null
          dividends is tested.
    */
    template <template <class> class Scheme = CrankNicolson>
    class FDDividendAmericanEngine
        : public FDEngineAdapter<FDAmericanCondition<FDDividendEngine<Scheme> >,
                                 DividendVanillaOption::engine> {
        typedef FDEngineAdapter<FDAmericanCondition<FDDividendEngine<Scheme> >,
                                DividendVanillaOption::engine> super;
      public:
        FDDividendAmericanEngine(
             const boost::shared_ptr<GeneralizedBlackScholesProcess>& process,
             Size timeSteps=100, Size gridPoints=100,
             bool timeDependent = false)
        : super(process, timeSteps, gridPoints,timeDependent) {}
    };


    template <template <class> class Scheme = CrankNicolson>
    class FDDividendAmericanEngineMerton73
        : public FDEngineAdapter<FDAmericanCondition<
                                     FDDividendEngineMerton73<Scheme> >,
                                 DividendVanillaOption::engine> {
        typedef FDEngineAdapter<FDAmericanCondition<
                                    FDDividendEngineMerton73<Scheme> >,
                                DividendVanillaOption::engine> super;
      public:
        FDDividendAmericanEngineMerton73(
             const boost::shared_ptr<GeneralizedBlackScholesProcess>& process,
             Size timeSteps=100, Size gridPoints=100,
             bool timeDependent = false)
        : super(process, timeSteps, gridPoints,timeDependent) {}
    };

    template <template <class> class Scheme = CrankNicolson>
    class FDDividendAmericanEngineShiftScale
        : public FDEngineAdapter<FDAmericanCondition<
                                     FDDividendEngineShiftScale<Scheme> >,
                                 DividendVanillaOption::engine> {
        typedef FDEngineAdapter<FDAmericanCondition<
                                    FDDividendEngineShiftScale<Scheme> >,
                                DividendVanillaOption::engine> super;
      public:
        FDDividendAmericanEngineShiftScale(
             const boost::shared_ptr<GeneralizedBlackScholesProcess>& process,
             Size timeSteps=100, Size gridPoints=100,
             bool timeDependent = false)
        : super(process, timeSteps, gridPoints,timeDependent) {}
    };

}


#endif
