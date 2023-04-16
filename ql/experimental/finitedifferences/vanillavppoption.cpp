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

/*! \file vanillavppoption.cpp
*/


#include <ql/event.hpp>
#include <ql/instruments/basketoption.hpp>
#include <ql/instruments/vanillaswingoption.hpp>
#include <ql/experimental/finitedifferences/vanillavppoption.hpp>

namespace QuantLib {
    namespace {

        class IdenticalPayoff : public Payoff {
          public:
            std::string name() const override { return "IdenticalPayoff"; }
            std::string description() const override { return name(); }
            Real operator()(Real price) const override { return price; }
        };
    }

    VanillaVPPOption::VanillaVPPOption(
        Real heatRate,
        Real pMin, Real pMax,
        Size tMinUp, Size tMinDown,
        Real startUpFuel, Real startUpFixCost,
        const std::shared_ptr<SwingExercise>& exercise,
        Size nStarts, Size nRunningHours)
    : MultiAssetOption(std::shared_ptr<Payoff>(), exercise),
      heatRate_(heatRate),
      pMin_(pMin), pMax_(pMax),
      tMinUp_(tMinUp), tMinDown_(tMinDown),
      startUpFuel_(startUpFuel),
      startUpFixCost_(startUpFixCost),
      nStarts_(nStarts),
      nRunningHours_(nRunningHours) {
        Array weigths(2);
        weigths[0] = 1.0; weigths[1] = -heatRate;

        payoff_ = std::shared_ptr<Payoff>(new AverageBasketPayoff(
            std::shared_ptr<Payoff>(new IdenticalPayoff()), weigths));
    }

    void VanillaVPPOption::arguments::validate() const {
        QL_REQUIRE(exercise, "no exercise given");
        QL_REQUIRE(nStarts == Null<Size>() || nRunningHours == Null<Size>(),
                "either a start limit or fuel limit is supported");
    }

    void VanillaVPPOption::setupArguments(
                                PricingEngine::arguments* args) const {

        MultiAssetOption::setupArguments(args);

        auto* arguments = dynamic_cast<VanillaVPPOption::arguments*>(args);
        QL_REQUIRE(arguments != nullptr, "wrong argument type");

        arguments->heatRate       = heatRate_;
        arguments->pMin           = pMin_;
        arguments->pMax           = pMax_;
        arguments->tMinUp         = tMinUp_;
        arguments->tMinDown       = tMinDown_;
        arguments->startUpFuel    = startUpFuel_;
        arguments->startUpFixCost = startUpFixCost_;
        arguments->nStarts        = nStarts_;
        arguments->nRunningHours  = nRunningHours_;
    }

    bool VanillaVPPOption::isExpired() const {
        return detail::simple_event(exercise_->lastDate()).hasOccurred();
    }
}

