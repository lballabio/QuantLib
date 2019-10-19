/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2009 StatPro Italia srl

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

/*! \file fddividendeuropeanengine.hpp
    \brief finite-differences engine for European option with dividends
*/

#ifndef quantlib_fd_dividend_european_engine_hpp
#define quantlib_fd_dividend_european_engine_hpp

#include <ql/instruments/dividendvanillaoption.hpp>
#include <ql/pricingengines/vanilla/fddividendengine.hpp>

namespace QuantLib {

    //! Finite-differences pricing engine for dividend European options
    /*! \deprecated Use FDDividendEuropeanEngineMerton73 instead if you
                    want to use the Merton 73 escowed dividend model;
                    use FdBlackScholesVanillaEngine otherwise.
                    Deprecated in version 1.17.

        \ingroup vanillaengines

        \test
        - the correctness of the returned greeks is tested by
          reproducing numerical derivatives.
        - the invariance of the results upon addition of null
          dividends is tested.
    */
    template <template <class> class Scheme = CrankNicolson>
    class QL_DEPRECATED FDDividendEuropeanEngine
        : public FDEngineAdapter<FDDividendEngine<Scheme>,
                                 DividendVanillaOption::engine> {
        typedef FDEngineAdapter<FDDividendEngine<Scheme>,
                                DividendVanillaOption::engine> super;
      public:
        FDDividendEuropeanEngine(
             const ext::shared_ptr<GeneralizedBlackScholesProcess>& process,
             Size timeSteps=100, Size gridPoints=100,
             bool timeDependent = false)
        : super(process, timeSteps, gridPoints,timeDependent) {}
    };


    //! Finite-differences pricing engine for dividend European options
    /*! This engine uses the Merton 73 escowed dividend model. */
    template <template <class> class Scheme = CrankNicolson>
    class FDDividendEuropeanEngineMerton73
        : public FDEngineAdapter<FDDividendEngineMerton73<Scheme>,
                                 DividendVanillaOption::engine> {
        typedef FDEngineAdapter<FDDividendEngineMerton73<Scheme>,
                                DividendVanillaOption::engine> super;
      public:
        FDDividendEuropeanEngineMerton73(
             const ext::shared_ptr<GeneralizedBlackScholesProcess>& process,
             Size timeSteps=100, Size gridPoints=100,
             bool timeDependent = false)
        : super(process, timeSteps, gridPoints,timeDependent) {}
    };


    /*! \deprecated Use FdBlackScholesVanillaEngine instead.
                    Deprecated in version 1.17.
    */
    template <template <class> class Scheme = CrankNicolson>
    class QL_DEPRECATED FDDividendEuropeanEngineShiftScale
        : public FDEngineAdapter<FDDividendEngineShiftScale<Scheme>,
                                 DividendVanillaOption::engine> {
        typedef FDEngineAdapter<FDDividendEngineShiftScale<Scheme>,
                                DividendVanillaOption::engine> super;
      public:
        FDDividendEuropeanEngineShiftScale(
             const ext::shared_ptr<GeneralizedBlackScholesProcess>& process,
             Size timeSteps=100, Size gridPoints=100,
             bool timeDependent = false)
        : super(process, timeSteps, gridPoints,timeDependent) {}
    };

}


#endif
