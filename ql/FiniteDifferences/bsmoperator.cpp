
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

/*! \file bsmoperator.cpp
    \brief differential operator for Black-Scholes-Merton equation
*/

// $Id$

#include <ql/FiniteDifferences/bsmoperator.hpp>

namespace QuantLib {

    namespace FiniteDifferences {

        BSMOperator::BSMOperator(Size size, double dx, double r,
            double q, double sigma)
        : TridiagonalOperator(size) {
            double sigma2 = sigma*sigma;
            double nu = r-q-sigma2/2;
            double pd = -(sigma2/dx-nu)/(2*dx);
            double pu = -(sigma2/dx+nu)/(2*dx);
            double pm = sigma2/(dx*dx)+r;
            setMidRows(pd,pm,pu);
        }

    }

}
