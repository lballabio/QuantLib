

/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

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
/*! \file singleassetoption.hpp
    \brief common code for option evaluation

    \fullpath
    ql/Pricers/%singleassetoption.hpp
*/

// $Id$

#ifndef BSM_option_pricer_h
#define BSM_option_pricer_h

#include <ql/option.hpp>
#include <ql/solver1d.hpp>

#define QL_MIN_VOLATILITY 0.0001
#define QL_MAX_VOLATILITY 4.0

namespace QuantLib {

    //! Pricing models for options
    /*! See sect. \ref pricers */
    namespace Pricers {


        double ExercisePayoff(Option::Type type, double price, double strike);

        //! Black-Scholes-Merton option
        class SingleAssetOption {
          public:
            SingleAssetOption(Option::Type type,
                              double underlying,
                              double strike,
                              Spread dividendYield,
                              Rate riskFreeRate,
                              Time residualTime,
                              double volatility);
            virtual ~SingleAssetOption() {}    // just in case
            // modifiers
            virtual void setVolatility(double newVolatility) ;
            virtual void setRiskFreeRate(Rate newRate) ;
            virtual void setDividendYield(Rate newDividendYield) ;
            // accessors
            virtual double value() const = 0;
            virtual double delta() const = 0;
            virtual double gamma() const = 0;
            virtual double theta() const = 0;
            virtual double vega() const;
            virtual double rho() const;
            virtual double dividendRho() const;
            double impliedVolatility(double targetValue,
                                     double accuracy = 1e-4,
                                     Size maxEvaluations = 100,
                                     double minVol = QL_MIN_VOLATILITY,
                                     double maxVol = QL_MAX_VOLATILITY) const ;
            virtual Handle<SingleAssetOption> clone() const = 0;
          protected:
            // results declared as mutable to preserve the logical
            Option::Type type_;
            double underlying_;
            double strike_;
            Spread dividendYield_;
            Rate riskFreeRate_;
            Time residualTime_;
            double volatility_;
            mutable bool hasBeenCalculated_;
            mutable double rho_, dividendRho_, vega_;
            mutable bool rhoComputed_, dividendRhoComputed_, vegaComputed_;
            const static double dVolMultiplier_;
            const static double dRMultiplier_;
          private:
            class VolatilityFunction;
            friend class VolatilityFunction;
        };

        class SingleAssetOption::VolatilityFunction : public ObjectiveFunction {
          public:
            VolatilityFunction(const Handle<SingleAssetOption>& tempBSM,
                               double targetPrice);
            double operator()(double x) const;
          private:
            mutable Handle<SingleAssetOption> bsm;
            double targetPrice_;
        };

        inline SingleAssetOption::VolatilityFunction::VolatilityFunction(
                const Handle<SingleAssetOption>& tempBSM,
                double targetPrice) {
            bsm = tempBSM;
            targetPrice_ = targetPrice;
        }

        inline double SingleAssetOption::VolatilityFunction::operator()(double x) const {
            bsm -> setVolatility(x);
            return (bsm -> value() - targetPrice_);
        }

    }

}


#endif
