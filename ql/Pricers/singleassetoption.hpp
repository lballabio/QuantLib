
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file singleassetoption.hpp
    \brief common code for option evaluation
*/

#ifndef BSM_option_pricer_h
#define BSM_option_pricer_h

#include <ql/Instruments/payoffs.hpp>

namespace QuantLib {

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
        virtual ~SingleAssetOption() {}
        // modifiers
        virtual void setVolatility(double newVolatility);
        virtual void setRiskFreeRate(Rate newRate);
        virtual void setDividendYield(Rate newDividendYield);
        // accessors
        virtual double value() const = 0;
        virtual double delta() const = 0;
        virtual double gamma() const = 0;
        virtual double theta() const;
        virtual double vega() const;
        virtual double rho() const;
        virtual double dividendRho() const;
        /*! \warning Options with a gamma that changes sign have
                     values that are <b>not</b> monotonic in the
                     volatility, e.g binary options. In these cases
                     impliedVolatility can fail and in any case is
                     meaningless.  Another possible source of failure
                     is to have a targetValue that is not attainable
                     with any volatility, e.g.  a targetValue lower
                     than the intrinsic value in the case of American
                     options. 
        */
        double impliedVolatility(double targetValue,
                                 double accuracy = 1e-4,
                                 Size maxEvaluations = 100,
                                 double minVol = QL_MIN_VOLATILITY,
                                 double maxVol = QL_MAX_VOLATILITY) const;
        double impliedDivYield(double targetValue,
                               double accuracy = 1e-4,
                               Size maxEvaluations = 100,
                               double minVol = QL_MIN_DIVYIELD,
                               double maxVol = QL_MAX_DIVYIELD) const;
        virtual boost::shared_ptr<SingleAssetOption> clone() const = 0;
      protected:
        double underlying_;
        PlainVanillaPayoff payoff_;
        Spread dividendYield_;
        Rate riskFreeRate_;
        Time residualTime_;
        double volatility_;
        mutable bool hasBeenCalculated_;
        mutable double rho_, dividendRho_, vega_, theta_;
        mutable bool rhoComputed_, dividendRhoComputed_, vegaComputed_,
            thetaComputed_;
        const static double dVolMultiplier_;
        const static double dRMultiplier_;
      private:
        class VolatilityFunction;
        friend class VolatilityFunction;
        class DivYieldFunction;
        friend class DivYieldFunction;
    };

    class SingleAssetOption::VolatilityFunction {
      public:
        VolatilityFunction(const boost::shared_ptr<SingleAssetOption>& tempBSM,
                           double targetPrice);
        double operator()(double x) const;
      private:
        mutable boost::shared_ptr<SingleAssetOption> bsm;
        double targetPrice_;
    };

    class SingleAssetOption::DivYieldFunction {
      public:
        DivYieldFunction(const boost::shared_ptr<SingleAssetOption>& tempBSM,
                         double targetPrice);
        double operator()(double x) const;
      private:
        mutable boost::shared_ptr<SingleAssetOption> bsm;
        double targetPrice_;
    };


    inline SingleAssetOption::VolatilityFunction::VolatilityFunction(
                          const boost::shared_ptr<SingleAssetOption>& tempBSM,
                          double targetPrice) {
        bsm = tempBSM;
        targetPrice_ = targetPrice;
    }

    inline double 
    SingleAssetOption::VolatilityFunction::operator()(double x) const {
        bsm -> setVolatility(x);
        return (bsm -> value() - targetPrice_);
    }


    inline SingleAssetOption::DivYieldFunction::DivYieldFunction(
                          const boost::shared_ptr<SingleAssetOption>& tempBSM,
                          double targetPrice) {
        bsm = tempBSM;
        targetPrice_ = targetPrice;
    }

    inline double 
    SingleAssetOption::DivYieldFunction::operator()(double x) const {
        bsm -> setDividendYield(x);
        return (bsm -> value() - targetPrice_);
    }

}


#endif
