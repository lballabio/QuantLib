
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file barrieroptionpricer.hpp
    \brief barrier option pricer
*/

#ifndef quantlib_barrier_option_pricer_h
#define quantlib_barrier_option_pricer_h

#include <ql/PricingEngines/barrierengines.hpp>
#include <ql/Pricers/singleassetoption.hpp>
#include <ql/Math/normaldistribution.hpp>

namespace QuantLib {

    namespace Pricers {

        /*! \class BarrierOption ql/Pricers/barrieroption.hpp
            \brief Barrier option

            The analytical calculation are taken from
            "Option pricing formulas", E.G. Haug, McGraw-Hill,
            p.69 and following.

            \deprecated use Instruments::BarrierOption instead.
        */
        class BarrierOption : public SingleAssetOption {
          public:
            // constructor
            BarrierOption(Barrier::Type barrType,
                          Option::Type type,
                          double underlying,
                          double strike,
                          Spread dividendYield,
                          Rate riskFreeRate,
                          Time residualTime,
                          double volatility,
                          double barrier,
                          double rebate = 0.0);
            // accessors
            double value() const;
            double delta() const;
            double gamma() const;
            double theta() const;
            Handle<SingleAssetOption> clone() const {
                return Handle<SingleAssetOption>(new BarrierOption(*this));
            }
          protected:
            void calculate_() const;
            mutable double greeksCalculated_, delta_, gamma_, theta_;
          private:
            PricingEngines::AnalyticBarrierEngine engine_;
            Barrier::Type barrType_;
            double barrier_, rebate_;
            mutable double value_;
        };

    }

}


#endif
