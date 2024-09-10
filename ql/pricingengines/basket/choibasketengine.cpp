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

/*! \file choibasketengine.cpp
*/

#include <ql/pricingengines/basket/choibasketengine.hpp>

namespace QuantLib {

    ChoiBasketEngine::ChoiBasketEngine(
        std::vector<ext::shared_ptr<GeneralizedBlackScholesProcess> > processes,
        Matrix rho)
    : n_(processes.size()),
      processes_(std::move(processes)),
      rho_(std::move(rho)) {

        QL_REQUIRE(n_ > 0, "No Black-Scholes process is given.");
        QL_REQUIRE(n_ == rho_.size1() && rho_.size1() == rho_.size2(),
            "process and correlation matrix must have the same size.");

        std::for_each(processes_.begin(), processes_.end(),
            [this](const auto& p) { registerWith(p); });
    }

    void ChoiBasketEngine::calculate() const {

    }
}
