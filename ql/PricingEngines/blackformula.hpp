
/*
 Copyright (C) 2003 Ferdinando Ametrano

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

/*! \file blackformula.hpp
    \brief Black formula
*/

#ifndef quantlib_blackformula_h
#define quantlib_blackformula_h

#include <ql/Instruments/payoffs.hpp>
#include <ql/Math/normaldistribution.hpp>

namespace QuantLib {

    class BlackFormula {
    public:
        BlackFormula(double spot,
                     double forward,
                     double discount,
                     double variance,
                     Handle<StrikedTypePayoff> payoff);
        double value() const;
        double delta() const;
        double deltaForward() const;
        double elasticity() const;
        double gamma() const;
        double theta(double maturity) const;
        double thetaPerDay(double maturity) const;
        double vega(double maturity) const;
        double rho(double maturity) const;
        double dividendRho(double maturity) const;
        double itmProbability() const;
        double strikeSensitivity() const;
    private:
        double spot_, forward_, discount_, variance_;
        Handle<StrikedTypePayoff> payoff_;

        double stdDev_, strike_, dividendDiscount_;
        double fD1_, fD2_;
        double alpha_, beta_, NID1_, NID2_;
        bool cashOrNothing_;
    };


    inline BlackFormula::BlackFormula(double spot, double forward,
        double discount, double variance, Handle<StrikedTypePayoff> payoff)
    : spot_(spot), forward_(forward), discount_(discount),
      variance_(variance), payoff_(payoff),
      cashOrNothing_(false) {

        QL_REQUIRE(spot>0.0,
            "BlackFormula::BlackFormula : "
            "positive spot value required");

        QL_REQUIRE(forward>0.0,
            "BlackFormula::BlackFormula : "
            "positive forward value required");

        QL_REQUIRE(discount>0.0,
            "BlackFormula::BlackFormula : "
            "positive discount required");

        QL_REQUIRE(variance>=0.0,
            "BlackFormula::BlackFormula : "
            "negative variance not allowed");

        stdDev_ = QL_SQRT(variance);
        strike_ = payoff->strike();
        dividendDiscount_ = forward / spot * discount;


        double fderD1, fderD2;
        if (variance>=QL_EPSILON) {
            if (strike_==0.0) {
                fderD1 = 0.0;
                fderD2 = 0.0;
                fD1_ = 1.0;
                fD2_ = 1.0;
            } else {
                CumulativeNormalDistribution f;
                double D1 = (QL_LOG(forward/strike_) +
                             0.5 * variance) / stdDev_;
                double D2 = D1-stdDev_;
                fD1_ = f(D1);
                fD2_ = f(D2);
                fderD1 = f.derivative(D1);
                fderD2 = f.derivative(D2);
            }
        } else {
            fderD1 = 0.0;
            fderD2 = 0.0;
            if (forward>strike_) {
                fD1_ = 1.0;
                fD2_ = 1.0;
            } else {
                fD1_ = 0.0;
                fD2_ = 0.0;
            }
        }



        switch (payoff->optionType()) {
          case Option::Call:
            alpha_ = fD1_;
            beta_  = fD2_;
            NID1_  = fderD1;
            NID2_  = fderD2;
            break;
          case Option::Put:
            alpha_ = fD1_-1.0;
            beta_  = fD2_-1.0;
            NID1_  = fderD1;
            NID2_  = fderD2;
            break;
          case Option::Straddle:
            alpha_ = 2.0*fD1_-1.0;
            beta_  = 2.0*fD2_-1.0;
            NID1_  = 2.0*fderD1;
            NID2_  = 2.0*fderD2;
            break;
          default:
            throw IllegalArgumentError("BlackFormula::BlackFormula : "
                                       "invalid option type");
        }


        // Binary Cash-Or-Nothing payoff?
        Handle<CashOrNothingPayoff> coo;
        #if defined(HAVE_BOOST)
        coo = boost::dynamic_pointer_cast<CashOrNothingPayoff>(payoff_);
        #else
        try {
            coo = payoff_;
        } catch (...) {}
        #endif
        if (!IsNull(coo)) {
            // ok, the payoff is Binary Cash-Or-Nothing
            alpha_ = 0.0;
            strike_ = - coo->cashPayoff();
            cashOrNothing_ = true;
        }

        // Binary Asset-Or-Nothing payoff?
        Handle<AssetOrNothingPayoff> aoo;
        #if defined(HAVE_BOOST)
        aoo = boost::dynamic_pointer_cast<AssetOrNothingPayoff>(payoff_);
        #else
        try {
            aoo = payoff_;
        } catch (...) {}
        #endif
        if (!IsNull(aoo)) {
            // ok, the payoff is Binary Asset-Or-Nothing
            beta_ = 0.0;
        }

        // Binary Super-Share payoff?
        Handle<SuperSharePayoff> ss;
        #if defined(HAVE_BOOST)
        ss = boost::dynamic_pointer_cast<SuperSharePayoff>(payoff_);
        #else
        try {
            ss = payoff_;
        } catch (...) {}
        #endif
        if (!IsNull(ss)) {
            // ok, the payoff is Binary Super-Share
            throw Error("Binary Super-Share payoff not handled yet");

        }

    }

    inline double BlackFormula::value() const {
        return discount_ * (forward_ * alpha_ - strike_ * beta_);
    }

    inline double BlackFormula::delta() const {
        return dividendDiscount_ * alpha_;
    }

    inline double BlackFormula::deltaForward() const {
        return discount_ * alpha_;
    }

    //! Sensitivity in percent to a percent movement in the underlying price
    inline double BlackFormula::elasticity() const {
        return delta()/value()*spot_;
    }

    inline double BlackFormula::gamma() const {
        return (NID1_==0.0 ? 0.0 : NID1_* dividendDiscount_ / (spot_ * stdDev_));
    }

    inline double BlackFormula::theta(double maturity) const {

        QL_REQUIRE(maturity>=0.0,
            "BlackFormula::theta : "
            "negative maturity not allowed");

        double rate = -QL_LOG(discount_)/maturity;
        double dividendRate = -QL_LOG(dividendDiscount_)/maturity;

        double vol;
        if (variance_>0.0) vol = QL_SQRT(variance_/maturity);
        else               vol = 0.0;

        return rate*value() - (rate-dividendRate)*spot_*delta()
            - 0.5*vol*vol*spot_*spot_*gamma();
    }

    inline double BlackFormula::thetaPerDay(double maturity) const {
        return theta(maturity)/365.0;
    }

    inline double BlackFormula::rho(double maturity) const {
        QL_REQUIRE(maturity>=0.0,
            "BlackFormula::rho : "
            "negative maturity not allowed");

        return maturity * discount_ * strike_ * beta_;
    }

    inline double BlackFormula::dividendRho(double maturity) const {
        QL_REQUIRE(maturity>=0.0,
            "BlackFormula::dividendRho : "
            "negative maturity not allowed");
        return - maturity * discount_ * forward_ * alpha_;
    }

    inline double BlackFormula::vega(double maturity) const {
        QL_REQUIRE(maturity>=0.0,
            "BlackFormula::vega : "
            "negative maturity not allowed");
        return NID1_ * discount_ * forward_ * QL_SQRT(maturity);
    }

    inline double BlackFormula::strikeSensitivity() const {

        if (cashOrNothing_) {
            // to be checked!!!!!!!!!!
            double strike = payoff_->strike();
            QL_REQUIRE(strike!=0.0,
                "BlackFormula::strikeSensitivity() : "
                "cannot be calculated for a cash-or-nothing with null strike");
            if (stdDev_>0.0) {
                return - discount_ * NID2_; // * payoff_->cashPayoff() / (stdDev_ * strike);
            } else {
                if (spot_!=strike)
                    return 0.0;
                else // should return infinity. what to do?
                    return 0.0;
            }
        } else {
            return - discount_ * beta_;
        }
    }

    inline double BlackFormula::itmProbability() const {
        return beta_;
    }

}


#endif
