
/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

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
/*! \file bsmlattice.cpp
    \brief Binomial trees under the BSM model

    \fullpath
    ql/Lattices/%bsmlattice.cpp
*/

// $Id$

#include "ql/Lattices/bsmlattice.hpp"

namespace QuantLib {

    namespace Lattices {

        BlackScholesLattice::BlackScholesLattice(
            const Handle<Tree>& tree, Rate riskFreeRate,
            Time end, Size steps)
        : Lattice(TimeGrid(end, steps), 2), 
          tree_(tree), discount_(QL_EXP(-riskFreeRate*(end/steps))) {
            pd_ = tree->probability(0,0,0);
            pu_ = tree->probability(0,0,1);
        }

        void BlackScholesLattice::stepback(
            Size i, const Array& values, Array& newValues) const {

            for (Size j=0; j<size(i); j++)
                newValues[j] = (pd_*values[j] + pu_*values[j+1])*discount_;

        }


        JarrowRudd::JarrowRudd(double volatility, Rate riskFreeRate,
            Rate dividendYield, 
            double underlying, Time end, Size steps)
        : BinomialTree(steps + 1), x0_(underlying), sigma_(volatility), 
          mu_(riskFreeRate - dividendYield - 0.5*sigma_*sigma_), dt_(end/steps) {}

        double JarrowRudd::underlying(Size i, Size index) const {
            int j = (2*index - i);
            return x0_*QL_EXP(i*mu_*dt_ + j*sigma_*QL_SQRT(dt_));
        }

        double JarrowRudd::probability(Size, Size, Size) const {
            return 0.5;
        }


        CoxRossRubinstein::CoxRossRubinstein(double volatility,
            Rate riskFreeRate, Rate dividendYield, double underlying,
            Time end, Size steps)
        : BinomialTree(steps + 1), x0_(underlying), sigma_(volatility), 
          mu_(riskFreeRate - dividendYield - 0.5*sigma_*sigma_), dt_(end/steps) {}

        double CoxRossRubinstein::underlying(Size i, Size index) const {
            int j = (2*index - i);
            return x0_*QL_EXP(j*sigma_*QL_SQRT(dt_));
        }

        double CoxRossRubinstein::probability(Size, Size, Size branch) const {
            if (branch == 1)
                return 0.5 + 0.5*(mu_/sigma_)*QL_SQRT(dt_);
            else
                return 0.5 - 0.5*(mu_/sigma_)*QL_SQRT(dt_);
        }

    }

}
