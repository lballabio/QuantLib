/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011 Klaus Spanderen

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

/*! \file vanillavppoption.hpp
    \brief vanilla virtual power plant option
*/

#ifndef quantlib_vanilla_vpp_option_hpp
#define quantlib_vanilla_vpp_option_hpp

#include <ql/instruments/multiassetoption.hpp>

namespace QuantLib {
    class BasketPayoff;
    class SwingExercise;

    class VanillaVPPOption : public MultiAssetOption {
      public:
        class arguments;
        VanillaVPPOption(Real heatRate,
                         Real pMin, Real pMax,
                         Size tMinUp, Size tMinDown,
                         Real startUpFuel, Real startUpFixCost,
                         const std::shared_ptr<SwingExercise>& exercise,
                         Size nStarts = Null<Size>(),
                         Size nRunningHours = Null<Size>());

        bool isExpired() const override;
        void setupArguments(PricingEngine::arguments*) const override;

      private:
        const Real heatRate_;
        const Real pMin_, pMax_;
        const Size tMinUp_, tMinDown_;
        const Real startUpFuel_, startUpFixCost_;
        const Size nStarts_, nRunningHours_;
    };

    class VanillaVPPOption::arguments
        : public virtual MultiAssetOption::arguments {
      public:
        arguments() = default;
        void validate() const override;

        Real heatRate;
        Real pMin, pMax;
        Size tMinUp, tMinDown;
        Real startUpFuel, startUpFixCost;
        Size nStarts, nRunningHours;
    };
}

#endif
