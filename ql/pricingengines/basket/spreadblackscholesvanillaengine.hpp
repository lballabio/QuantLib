/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2024 Klaus Spanderen

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

/*! \file spreadblackscholesvanillaengine.hpp
    \brief base class for 2d spread pricing engines using the Black-Scholes model.
*/

#ifndef quantlib_spread_black_scholes_vanilla_engine_hpp
#define quantlib_spread_black_scholes_vanilla_engine_hpp

#include <ql/instruments/basketoption.hpp>
#include <ql/processes/blackscholesprocess.hpp>

namespace QuantLib {

    class SpreadBlackScholesVanillaEngine : public BasketOption::engine {
      public:
        SpreadBlackScholesVanillaEngine(
            ext::shared_ptr<GeneralizedBlackScholesProcess> process1,
            ext::shared_ptr<GeneralizedBlackScholesProcess> process2,
            Real correlation);

        void calculate() const override;

      protected:
        virtual Real calculate(
           Real f1, Real f2, Real strike, Option::Type optionType,
           Real variance1, Real variance2, DiscountFactor df) const = 0;

        const ext::shared_ptr<GeneralizedBlackScholesProcess> process1_;
        const ext::shared_ptr<GeneralizedBlackScholesProcess> process2_;
        const Real rho_;
    };
}

#endif
