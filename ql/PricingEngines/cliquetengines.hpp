
/*
 Copyright (C) 2003 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file cliquetengines.hpp
    \brief Cliquet option engines

    \fullpath
    ql/PricingEngines/%cliquetengines.hpp
*/

// $Id$

#ifndef quantlib_cliquet_engines_h
#define quantlib_cliquet_engines_h

#include <ql/PricingEngines/vanillaengines.hpp>
#include <ql/PricingEngines/mcengine.hpp>

namespace QuantLib {

    namespace PricingEngines {

        //! arguments for cliquet option calculation
        // should inherit from a strikeless version of VanillaOptionArguments
        class CliquetOptionArguments : public VanillaOptionArguments {
          public:
            CliquetOptionArguments() : moneyness(Null<double>()),
                                       accruedCoupon(Null<double>()),
                                       localCap(Null<double>()),
                                       localFloor(Null<double>()),
                                       globalCap(Null<double>()),
                                       globalFloor(Null<double>()),
                                       resetDates() {}
            void validate() const;
            double moneyness, accruedCoupon;
            double localCap, localFloor, globalCap, globalFloor;
            std::vector<Date> resetDates;
        };

        inline void CliquetOptionArguments::validate() const {
            VanillaOptionArguments::validate();
            QL_REQUIRE(moneyness != Null<double>(),
                       "CliquetOptionArguments::validate() : "
                       "null moneyness given");
            QL_REQUIRE(moneyness > 0.0,
                       "CliquetOptionArguments::validate() : "
                       "negative or zero moneyness given");
            QL_REQUIRE(accruedCoupon == Null<double>() || localCap >= 0.0,
                       "CliquetOptionArguments::validate() : "
                       "negative accrued coupon");
            QL_REQUIRE(localCap == Null<double>() || localCap >= 0.0,
                       "CliquetOptionArguments::validate() : "
                       "negative local cap");
            QL_REQUIRE(localFloor == Null<double>() || localCap >= 0.0,
                       "CliquetOptionArguments::validate() : "
                       "negative local floor");
            QL_REQUIRE(globalCap == Null<double>() || localCap >= 0.0,
                       "CliquetOptionArguments::validate() : "
                       "negative global cap");
            QL_REQUIRE(globalFloor == Null<double>() || localCap >= 0.0,
                       "CliquetOptionArguments::validate() : "
                       "negative global floor");
            QL_REQUIRE(resetDates.size()>0,
                       "CliquetOptionArguments::validate() : "
                       "no reset dates given");
            // sort resetDates here
            for (Size i = 0; i < resetDates.size(); i++) {
                Time resetTime = riskFreeTS->dayCounter().yearFraction(
                    riskFreeTS->referenceDate(), resetDates[i]);
                QL_REQUIRE(maturity >= resetTime,
                           "CliquetOptionArguments::validate() : "
                           "reset time greater than maturity");
            }
        }

        //! Cliquet engine base class
        class CliquetEngine : public GenericEngine<CliquetOptionArguments,
                                                   VanillaOptionResults> {};

        //! Monte Carlo cliquet engine
        template<class S, class SG, class PG, class PP>
        class MCCliquetEngine : public CliquetEngine,
                                public McEngine<S, PG, PP> {
          public:
            MCCliquetEngine(bool antitheticVariance,
                            bool controlVariate,
                            const TimeGrid& timeGrid,
                            SG sequenceGenerator)
            : antitheticVariance_(antitheticVariance),
              controlVariate_(controlVariate), timeGrid_(timeGrid),
              sequenceGenerator_(sequenceGenerator) {}
            void calculate() const;
          protected:
            Handle<PG> pathGenerator() const;
            Handle<PP> pathPricer() const;
            bool antitheticVariance_, controlVariate_;
          private:
            TimeGrid timeGrid_;
            SG sequenceGenerator_;
        };

        template<class S, class SG, class PG, class PP>
        inline
        Handle<PP> MCCliquetEngine<S, SG, PG, PP>::pathPricer() const {
            //! Initialize the path pricer
            return Handle<MonteCarlo::PathPricer<MonteCarlo::Path> >(
                new MonteCarlo::CliquetOptionPathPricer(arguments_.type,
                arguments_.underlying, arguments_.moneyness,
                arguments_.accruedCoupon, arguments_.localCap,
                arguments_.localFloor, arguments_.globalCap,
                arguments_.globalFloor, arguments_.riskFreeTS));
        }

    }

}

#endif
