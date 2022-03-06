/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2012 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis

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

/*! \file twoassetbarrieroption.hpp
    \brief Barrier option on two assets
*/

#ifndef quantlib_two_asset_barrier_option_hpp
#define quantlib_two_asset_barrier_option_hpp

#include <ql/instruments/oneassetoption.hpp>
#include <ql/instruments/barriertype.hpp>
#include <ql/instruments/payoffs.hpp>

namespace QuantLib {

    class GeneralizedBlackScholesProcess;

    //! %Barrier option on two assets
    /*! \ingroup instruments */
    class TwoAssetBarrierOption : public Option {
      public:
        class arguments;
        class engine;
        TwoAssetBarrierOption(
                      Barrier::Type barrierType,
                      Real barrier,
                      const ext::shared_ptr<StrikedTypePayoff>& payoff,
                      const ext::shared_ptr<Exercise>& exercise);

        bool isExpired() const override;
        void setupArguments(PricingEngine::arguments*) const override;

      protected:
        // arguments
        Barrier::Type barrierType_;
        Real barrier_;
    };


    //! %Arguments for two-asset %barrier %option calculation
    class TwoAssetBarrierOption::arguments : public Option::arguments {
      public:
        arguments();
        Barrier::Type barrierType;
        Real barrier;
        void validate() const override;
    };

    //! %Two-asset barrier-option %engine base class
    class TwoAssetBarrierOption::engine
        : public GenericEngine<TwoAssetBarrierOption::arguments,
                               TwoAssetBarrierOption::results> {
      protected:
        bool triggered(Real underlying) const;
    };

}


#endif


#ifndef id_f4c22aa07065a4ab8b96f09a086524d0
#define id_f4c22aa07065a4ab8b96f09a086524d0
inline bool test_f4c22aa07065a4ab8b96f09a086524d0(const int* i) {
    return i != nullptr;
}
#endif
