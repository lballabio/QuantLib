/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2020 Lew Wei Hao

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

#include <ql/pricingengines/lookback/mclookbackengine.hpp>
#include <algorithm>

namespace QuantLib {

    class LookbackFixedPathPricer : public PathPricer<Path> {
      public:
        LookbackFixedPathPricer(Option::Type type,
                                Real strike,
                                DiscountFactor discount);
        Real operator()(const Path& path) const override;

      private:
        PlainVanillaPayoff payoff_;
        DiscountFactor discount_;
    };

    class LookbackPartialFixedPathPricer : public PathPricer<Path> {
      public:
        LookbackPartialFixedPathPricer(Time lookbackStart,
                                       Option::Type type,
                                       Real strike,
                                       DiscountFactor discount);
        Real operator()(const Path& path) const override;

      private:
        Time lookbackStart_;
        PlainVanillaPayoff payoff_;
        DiscountFactor discount_;
    };

    class LookbackFloatingPathPricer : public PathPricer<Path> {
      public:
        LookbackFloatingPathPricer(Option::Type type,
                                   DiscountFactor discount);
        Real operator()(const Path& path) const override;

      private:
        FloatingTypePayoff payoff_;
        DiscountFactor discount_;
    };

    class LookbackPartialFloatingPathPricer : public PathPricer<Path> {
      public:
        LookbackPartialFloatingPathPricer(Time lookbackEnd,
                                          Option::Type type,
                                          DiscountFactor discount);
        Real operator()(const Path& path) const override;

      private:
        Time lookbackEnd_;
        FloatingTypePayoff payoff_;
        DiscountFactor discount_;
    };

    namespace detail {

        ext::shared_ptr<PathPricer<Path> >
        mc_lookback_path_pricer(
               const ContinuousFixedLookbackOption::arguments& args,
               const GeneralizedBlackScholesProcess& process,
               DiscountFactor discount) {
            ext::shared_ptr<PlainVanillaPayoff> payoff =
                ext::dynamic_pointer_cast<PlainVanillaPayoff>(args.payoff);
            QL_REQUIRE(payoff, "non-plain payoff given");

            return ext::shared_ptr<PathPricer<Path> >(
                new LookbackFixedPathPricer(payoff->optionType(),
                                            payoff->strike(),
                                            discount));
        }

        ext::shared_ptr<PathPricer<Path> >
        mc_lookback_path_pricer(
               const ContinuousPartialFixedLookbackOption::arguments& args,
               const GeneralizedBlackScholesProcess& process,
               DiscountFactor discount) {
            ext::shared_ptr<PlainVanillaPayoff> payoff =
                ext::dynamic_pointer_cast<PlainVanillaPayoff>(args.payoff);
            QL_REQUIRE(payoff, "non-plain payoff given");

            Time lookbackStart = process.time(args.lookbackPeriodStart);

            return ext::shared_ptr<PathPricer<Path> >(
                new LookbackPartialFixedPathPricer(lookbackStart,
                                                   payoff->optionType(),
                                                   payoff->strike(),
                                                   discount));
        }

        ext::shared_ptr<PathPricer<Path> >
        mc_lookback_path_pricer(
               const ContinuousFloatingLookbackOption::arguments& args,
               const GeneralizedBlackScholesProcess& process,
               DiscountFactor discount) {
            ext::shared_ptr<FloatingTypePayoff> payoff =
                ext::dynamic_pointer_cast<FloatingTypePayoff>(args.payoff);
            QL_REQUIRE(payoff, "non-floating payoff given");

            return ext::shared_ptr<PathPricer<Path> >(
                new LookbackFloatingPathPricer(payoff->optionType(),
                                               discount));
        }

        ext::shared_ptr<PathPricer<Path> >
        mc_lookback_path_pricer(
               const ContinuousPartialFloatingLookbackOption::arguments& args,
               const GeneralizedBlackScholesProcess& process,
               DiscountFactor discount) {
            ext::shared_ptr<FloatingTypePayoff> payoff =
                ext::dynamic_pointer_cast<FloatingTypePayoff>(args.payoff);
            QL_REQUIRE(payoff, "non-floating payoff given");

            Time lookbackEnd = process.time(args.lookbackPeriodEnd);

            return ext::shared_ptr<PathPricer<Path> >(
                new LookbackPartialFloatingPathPricer(lookbackEnd,
                                                      payoff->optionType(),
                                                      discount));
        }

    }


    LookbackFixedPathPricer::LookbackFixedPathPricer(
        Option::Type type,
        Real strike,
        DiscountFactor discount)
    : payoff_(type, strike), discount_(discount) {
        QL_REQUIRE(strike>=0.0,
                   "strike less than zero not allowed");
    }

    Real LookbackFixedPathPricer::operator()(const Path& path) const {
        QL_REQUIRE(!path.empty(), "the path cannot be empty");

        Real underlying;
        switch (payoff_.optionType()) {
          case Option::Put:
            underlying = *std::min_element(path.begin()+1, path.end());
            break;
          case Option::Call:
            underlying = *std::max_element(path.begin()+1, path.end());
            break;
          default:
            QL_FAIL("unknown option type");
        }

        return payoff_(underlying) * discount_;
    }


    LookbackPartialFixedPathPricer::LookbackPartialFixedPathPricer(
        Time lookbackStart,
        Option::Type type,
        Real strike,
        const DiscountFactor discount)
    : lookbackStart_(lookbackStart), payoff_(type, strike), discount_(discount) {
        QL_REQUIRE(strike>=0.0,
                   "strike less than zero not allowed");
    }

    Real LookbackPartialFixedPathPricer::operator()(const Path& path) const {
        QL_REQUIRE(!path.empty(), "the path cannot be empty");

        const TimeGrid& timeGrid = path.timeGrid();
        Size startIndex = timeGrid.closestIndex(lookbackStart_);
        Real underlying;
        switch (payoff_.optionType()) {
          case Option::Put:
            underlying = *std::min_element(path.begin()+startIndex+1, path.end());
            break;
          case Option::Call:
            underlying = *std::max_element(path.begin()+startIndex+1, path.end());
            break;
          default:
            QL_FAIL("unknown option type");
        }

        return payoff_(underlying) * discount_;
    }


    LookbackFloatingPathPricer::LookbackFloatingPathPricer(
        Option::Type type,
        const DiscountFactor discount)
    : payoff_(type), discount_(discount) {}

    Real LookbackFloatingPathPricer::operator()(const Path& path) const {
        QL_REQUIRE(!path.empty(), "the path cannot be empty");

        Real terminalPrice = path.back();
        Real strike;
        switch (payoff_.optionType()) {
          case Option::Call:
            strike = *std::min_element(path.begin()+1, path.end());
            break;
          case Option::Put:
            strike = *std::max_element(path.begin()+1, path.end());
            break;
          default:
            QL_FAIL("unknown option type");
        }

        return payoff_(terminalPrice, strike) * discount_;
    }


    LookbackPartialFloatingPathPricer::LookbackPartialFloatingPathPricer(
        Time lookbackEnd,
        Option::Type type,
        DiscountFactor discount)
        : lookbackEnd_(lookbackEnd), payoff_(type), discount_(discount) {}

    Real LookbackPartialFloatingPathPricer::operator()(const Path& path) const {
        QL_REQUIRE(!path.empty(), "the path cannot be empty");

        const TimeGrid& timeGrid = path.timeGrid();
        Size endIndex = timeGrid.closestIndex(lookbackEnd_);
        Real terminalPrice = path.back();
        Real strike;

        switch (payoff_.optionType()) {
          case Option::Call:
            strike = *std::min_element(path.begin()+1, path.begin()+endIndex+1);
            break;
          case Option::Put:
            strike = *std::max_element(path.begin()+1, path.begin()+endIndex+1);
            break;
          default:
            QL_FAIL("unknown option type");
        }

        return payoff_(terminalPrice, strike) * discount_;
    }

}
