

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
/*! \file barrieroption.hpp
    \brief barrier option

    \fullpath
    ql/Pricers/%barrieroption.hpp
*/

// $Id$

#ifndef quantlib_barrier_option_pricer_h
#define quantlib_barrier_option_pricer_h

#include <ql/Pricers/singleassetoption.hpp>
#include <ql/Math/normaldistribution.hpp>

namespace QuantLib {

    namespace Pricers {

        /*! \class BarrierOption ql/Pricers/barrieroption.hpp
            \brief Barrier option

            The analytical calculation are taken from
            "Option pricing formulas", E.G. Haug, McGraw-Hill,
            p.69 and following.
        */
        class BarrierOption : public SingleAssetOption {
          public:
            // constructor
            enum BarrierType { DownIn, UpIn, DownOut, UpOut };
            BarrierOption(BarrierType barrType,
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
            void initialize_() const;
            BarrierType barrType_;
            double barrier_, rebate_;
            mutable double value_, sigmaSqrtT_, mu_, muSigma_;
            mutable double dividendDiscount_, riskFreeDiscount_;
            Math::CumulativeNormalDistribution f_;
            double A_(double eta, double phi) const;
            double B_(double eta, double phi) const;
            double C_(double eta, double phi) const;
            double D_(double eta, double phi) const;
            double E_(double eta, double phi) const;
            double F_(double eta, double phi) const;
        };


        // inline definitions

        inline double BarrierOption::A_(double eta, double phi) const {
            double x1 = QL_LOG(underlying_/payoff_.strike())/sigmaSqrtT_ + muSigma_;
            double N1 = f_(phi*x1);
            double N2 = f_(phi*(x1-sigmaSqrtT_));
            return phi*(underlying_ * dividendDiscount_ * N1
                        - payoff_.strike() * riskFreeDiscount_ * N2);
        }

        inline double BarrierOption::B_(double eta, double phi) const {
            double x2 = QL_LOG(underlying_/barrier_)/sigmaSqrtT_ + muSigma_;
            double N1 = f_(phi*x2);
            double N2 = f_(phi*(x2-sigmaSqrtT_));
            return phi*(underlying_ * dividendDiscount_ * N1
                      - payoff_.strike() * riskFreeDiscount_ * N2);
        }

        inline double BarrierOption::C_(double eta, double phi) const {
            double HS = barrier_/underlying_;
            double powHS0 = QL_POW(HS, 2 * mu_);
            double powHS1 = powHS0 * HS * HS;
            double y1 = QL_LOG(barrier_*HS/payoff_.strike())/sigmaSqrtT_ + muSigma_;
            double N1 = f_(eta*y1);
            double N2 = f_(eta*(y1-sigmaSqrtT_));
            return phi*(underlying_ * dividendDiscount_ * powHS1 * N1
                          - payoff_.strike() * riskFreeDiscount_ * powHS0 * N2);
        }

        inline double BarrierOption::D_(double eta, double phi) const {
            double HS = barrier_/underlying_;
            double powHS0 = QL_POW(HS, 2 * mu_);
            double powHS1 = powHS0 * HS * HS;
            double y2 = QL_LOG(barrier_/underlying_)/sigmaSqrtT_ + muSigma_;
            double N1 = f_(eta*y2);
            double N2 = f_(eta*(y2-sigmaSqrtT_));
            return phi*(underlying_ * dividendDiscount_ * powHS1 * N1
                          - payoff_.strike() * riskFreeDiscount_ * powHS0 * N2);
        }

        inline double BarrierOption::E_(double eta, double phi) const {
            if (rebate_ > 0) {
                double powHS0 = QL_POW(barrier_/underlying_, 2 * mu_);
                double x2 = QL_LOG(underlying_/barrier_)/sigmaSqrtT_ + muSigma_;
                double y2 = QL_LOG(barrier_/underlying_)/sigmaSqrtT_ + muSigma_;
                double N1 = f_(eta*(x2 - sigmaSqrtT_));
                double N2 = f_(eta*(y2 - sigmaSqrtT_));
                return rebate_ * riskFreeDiscount_ * (N1 - powHS0 * N2);
            } else {
                return 0.0;
            }
        }

        inline double BarrierOption::F_(double eta, double phi) const {
            if (rebate_ > 0) {
                double lambda = QL_SQRT(mu_*mu_ + 2.0*riskFreeRate_/
                                            (volatility_ * volatility_));
                double HS = barrier_/underlying_;
                double powHSplus = QL_POW(HS, mu_ + lambda);
                double powHSminus = QL_POW(HS, mu_ - lambda);

                double z = QL_LOG(barrier_/underlying_)/sigmaSqrtT_
                    + lambda * sigmaSqrtT_;

                double N1 = f_(eta * z);
                double N2 = f_(eta * (z - 2.0 * lambda * sigmaSqrtT_));
                return rebate_ * (powHSplus * N1 + powHSminus * N2);
            } else {
                return 0.0;
            }
        }

    }

}


#endif
