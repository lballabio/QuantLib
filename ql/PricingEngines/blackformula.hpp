
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

#ifndef quantlib_blackformula_hpp
#define quantlib_blackformula_hpp

#include <ql/Instruments/payoffs.hpp>

namespace QuantLib {

    class BlackFormula {
      public:
        BlackFormula(double forward,
                     double discount,
                     double variance,
                     const boost::shared_ptr<StrikedTypePayoff>& payoff);
        double value() const;
        double delta(double spot) const;
        //! Sensitivity in percent to a percent movement in the underlying
        double elasticity(double spot) const;
        double gamma(double spot) const;
        double deltaForward() const;
        //! Sensitivity in percent to a percent movement in the forward price
        double elasticityForward() const;
        double gammaForward() const;
        double theta(double spot, double maturity) const;
        double thetaPerDay(double spot, double maturity) const;
        double vega(double maturity) const;
        double rho(double maturity) const;
        double dividendRho(double maturity) const;
        /*! Probability of being in the money in the bond martingale measure.
            It is a risk-neutral probability, not the real world probability.
        */
        double itmCashProbability() const;
        /*! Probability of being in the money in the asset martingale measure.
            It is a risk-neutral probability, not the real world probability.
        */
        double itmAssetProbability() const;
        double strikeSensitivity() const;
      private:
        double forward_, discount_, variance_;
        double stdDev_, strike_;
        double D1_, D2_, alpha_, beta_, DalphaDd1_, DbetaDd2_;
        double cum_d1_, cum_d2_;
        double X_, DXDs_, DXDstrike_;
    };


    // inline definitions

    inline double BlackFormula::thetaPerDay(double spot, 
                                            double maturity) const {
        return theta(spot, maturity)/365.0;
    }

    inline double BlackFormula::itmCashProbability() const {
        return cum_d2_;
    }

    inline double BlackFormula::itmAssetProbability() const {
        return cum_d1_;
    }

}


#endif
