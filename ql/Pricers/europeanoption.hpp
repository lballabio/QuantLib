
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

/*! \file europeanoption.hpp
    \brief european option
*/

#ifndef BSM_european_option_pricer_h
#define BSM_european_option_pricer_h

#include <ql/Pricers/singleassetoption.hpp>
#include <ql/Math/normaldistribution.hpp>

namespace QuantLib {

    //! Black-Scholes-Merton European option
    /*! \deprecated use VanillaOption with EuropeanAnalyticEngine */
    class EuropeanOption : public SingleAssetOption {
      public:
        // constructor
        EuropeanOption(Option::Type type, double underlying,
                       double strike, Spread dividendYield, Rate riskFreeRate,
                       Time residualTime, double volatility);
        // accessors
        double value() const;
        double delta() const;
        double gamma() const;
        double theta() const;
        double vega() const;
        double rho() const;
        double dividendRho() const;
        Handle<SingleAssetOption> clone() const {
            return Handle<SingleAssetOption>(new EuropeanOption(*this)); }
        // modifiers
        void setVolatility(double newVolatility);
        void setRiskFreeRate(Rate newRate);
        void setDividendYield(Rate newDividendYield);
        double beta() const;
      private:
        #if defined(QL_PATCH_SOLARIS)
        CumulativeNormalDistribution f_;
        #else
        static const CumulativeNormalDistribution f_;
        #endif
        double alpha() const;
        double standardDeviation() const;
        double D1() const;
        double D2() const;
        double NID1() const;
        DiscountFactor dividendDiscount() const;
        DiscountFactor riskFreeDiscount() const;
        // declared as mutable to preserve
        // the logical constness (does this word exist?) of value()
        mutable double alpha_, beta_, standardDeviation_,
            D1_, D2_, NID1_;
        mutable DiscountFactor dividendDiscount_, riskFreeDiscount_;
    };


    inline double EuropeanOption::alpha() const {
        if (alpha_==Null<double>()) {
            switch (payoff_.optionType()) {
              case Option::Call:
                alpha_ = f_(D1());
                break;
              case Option::Put:
                alpha_ = f_(D1())-1.0;
                break;
              case Option::Straddle:
                alpha_ = 2.0*f_(D1())-1.0;
                break;
              default:
                QL_FAIL("EuropeanOption: "
                        "invalid option type");
            }
        }
        return alpha_;
    }

    inline double EuropeanOption::beta() const {
        if (beta_==Null<double>()) {
            switch (payoff_.optionType()) {
              case Option::Call:
                beta_ = f_(D2());
                break;
              case Option::Put:
                beta_ = f_(D2())-1.0;
                break;
              case Option::Straddle:
                beta_ = 2.0*f_(D2())-1.0;
                break;
              default:
                QL_FAIL("EuropeanOption: "
                        "invalid option type");
            }
        }
        return beta_;
    }

    inline double EuropeanOption::standardDeviation() const {
        if (standardDeviation_==Null<double>())
            standardDeviation_ = volatility_*QL_SQRT(residualTime_);
        return standardDeviation_;
    }

    inline double EuropeanOption::D1() const {
        if (D1_==Null<double>())
            D1_ = QL_LOG(underlying_/payoff_.strike())/standardDeviation() +
                standardDeviation()/2.0 +
                (riskFreeRate_ - dividendYield_) *
                residualTime_/standardDeviation();
        return D1_;
    }

    inline double EuropeanOption::D2() const {
        if (D2_==Null<double>())
            D2_ = D1() - standardDeviation();
        return D2_;
    }

    inline double EuropeanOption::NID1() const {
        if (NID1_==Null<double>()) {
            switch (payoff_.optionType()) {
              case Option::Call:
              case Option::Put:
                NID1_ = f_.derivative(D1());
                break;
              case Option::Straddle:
                NID1_ = 2.0*f_.derivative(D1());
                break;
              default:
                QL_FAIL("EuropeanOption: "
                        "invalid option type");
            }
        }
        return NID1_;
    }

    inline DiscountFactor EuropeanOption::dividendDiscount() const {
        if (dividendDiscount_==Null<DiscountFactor>())
            dividendDiscount_ = (QL_EXP(-dividendYield_*residualTime_));
        return dividendDiscount_;
    }

    inline DiscountFactor EuropeanOption::riskFreeDiscount() const {
        if (riskFreeDiscount_==Null<DiscountFactor>())
            riskFreeDiscount_ = (QL_EXP(-riskFreeRate_*residualTime_));
        return riskFreeDiscount_;
    }

}


#endif
