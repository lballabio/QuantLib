
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, contact ferdinando@ametrano.net
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/

/*! \file bsmoption.hpp
    \brief common code for option evaluation

    $Id$
*/

// $Source$
// $Log$
// Revision 1.8  2001/05/24 15:38:08  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef BSM_option_pricer_h
#define BSM_option_pricer_h

#include "ql/options.hpp"
#include "ql/rate.hpp"
#include "ql/handle.hpp"
#include "ql/solver1d.hpp"

#define QL_MIN_VOLATILITY 0.0001
#define QL_MAX_VOLATILITY 4.0

namespace QuantLib {

    namespace Pricers {

        class BSMOption : public Option {
          public:
            BSMOption(Type type, double underlying, double strike,
                Rate dividendYield, Rate riskFreeRate, Time residualTime,
                double volatility);
            virtual ~BSMOption() {}    // just in case
            // modifiers
            void setVolatility(double newVolatility) ;
            void setRiskFreeRate(Rate newRate) ;
            // accessors
            virtual double value() const = 0;
            virtual double delta() const = 0;
            virtual double gamma() const = 0;
            virtual double theta() const = 0;
            virtual double vega() const;
            virtual double rho() const;
            double impliedVolatility(double targetValue,
                double accuracy = 1e-4, int maxEvaluations = 100,
                double minVol = QL_MIN_VOLATILITY,
                double maxVol = QL_MAX_VOLATILITY) const ;
            virtual Handle<BSMOption> clone() const = 0;
          protected:
            // input data
            Type type_;
            double underlying_;
            double strike_;
            Rate dividendYield_, riskFreeRate_;
            Time residualTime_;
            double volatility_;
            // results
            // declared as mutable to preserve the logical
            mutable bool hasBeenCalculated_;
            mutable double value_;
            mutable double  rho_, vega_;
            mutable bool rhoComputed_, vegaComputed_;
            const static double dVolMultiplier_, dRMultiplier_;
          private:
            class BSMFunction;
            friend class BSMFunction;
        };

        class BSMOption::BSMFunction : public ObjectiveFunction {
          public:
            BSMFunction(const Handle<BSMOption>& tempBSM, double targetPrice);
            double operator()(double x) const;
          private:
            mutable Handle<BSMOption> bsm;
            double targetPrice_;
        };

        inline BSMOption::BSMFunction::BSMFunction(
                const Handle<BSMOption>& tempBSM, double targetPrice) {
            bsm = tempBSM;
            targetPrice_ = targetPrice;
        }

        inline double BSMOption::BSMFunction::operator()(double x) const {
            bsm -> setVolatility(x);
            return (bsm -> value() - targetPrice_);
        }

    }

}


#endif
