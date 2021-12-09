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

/*! \file fddividendshoutengine.hpp
    \brief base class for shout engine with dividends
*/

#ifndef quantlib_fd_dividend_shout_engine_hpp
#define quantlib_fd_dividend_shout_engine_hpp

#include <ql/instruments/dividendvanillaoption.hpp>
#include <ql/pricingengines/vanilla/fddividendengine.hpp>
#include <ql/pricingengines/vanilla/fdconditions.hpp>
#include <ql/methods/finitedifferences/shoutcondition.hpp>

namespace QuantLib {

    //! Finite-differences shout engine with dividends
    /*! \ingroup vanillaengines

        \deprecated Use FdBlackScholesShoutEngine instead.
                    Deprecated in version 1.22.
    */
    template <template <class> class Scheme = CrankNicolson>
    class QL_DEPRECATED FDDividendShoutEngine
        : public FDEngineAdapter<FDShoutCondition<FDDividendEngine<Scheme> >,
                                 DividendVanillaOption::engine> {
        typedef FDEngineAdapter<FDShoutCondition<FDDividendEngine<Scheme> >,
                                DividendVanillaOption::engine> super;
      public:
        QL_DEPRECATED_DISABLE_WARNING
        FDDividendShoutEngine(
             const ext::shared_ptr<GeneralizedBlackScholesProcess>& process,
             Size timeSteps=100, Size gridPoints=100,
             bool timeDependent = false)
        : super(process, timeSteps, gridPoints,timeDependent) {}
        QL_DEPRECATED_ENABLE_WARNING
    };


    /*! \deprecated Use FdBlackScholesShoutEngine instead.
                    Deprecated in version 1.22.
    */
    template <template <class> class Scheme = CrankNicolson>
    class QL_DEPRECATED FDDividendShoutEngineMerton73
        : public FDEngineAdapter<FDShoutCondition<
                                     FDDividendEngineMerton73<Scheme> >,
                                 DividendVanillaOption::engine> {
        typedef FDEngineAdapter<FDShoutCondition<
                                    FDDividendEngineMerton73<Scheme> >,
                                DividendVanillaOption::engine> super;
      public:
        QL_DEPRECATED_DISABLE_WARNING
        FDDividendShoutEngineMerton73(
             const ext::shared_ptr<GeneralizedBlackScholesProcess>& process,
             Size timeSteps=100, Size gridPoints=100,
             bool timeDependent = false)
        : super(process, timeSteps, gridPoints,timeDependent) {}
        QL_DEPRECATED_ENABLE_WARNING
    };

    /*! \deprecated Use FdBlackScholesShoutEngine instead.
                    Deprecated in version 1.22.
    */
    template <template <class> class Scheme = CrankNicolson>
    class QL_DEPRECATED FDDividendShoutEngineShiftScale
        : public FDEngineAdapter<FDShoutCondition<
                                     FDDividendEngineShiftScale<Scheme> >,
                                 DividendVanillaOption::engine> {
        typedef FDEngineAdapter<FDShoutCondition<
                                    FDDividendEngineShiftScale<Scheme> >,
                                DividendVanillaOption::engine> super;
      public:
        QL_DEPRECATED_DISABLE_WARNING
        FDDividendShoutEngineShiftScale(
             const ext::shared_ptr<GeneralizedBlackScholesProcess>& process,
             Size timeSteps=100, Size gridPoints=100,
             bool timeDependent = false)
        : super(process, timeSteps, gridPoints,timeDependent) {}
        QL_DEPRECATED_ENABLE_WARNING
    };

}


#endif
