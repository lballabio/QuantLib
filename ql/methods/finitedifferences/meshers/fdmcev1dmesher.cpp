/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2018 Klaus Spanderen

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file fdmcev1dmesher.cpp */

#include <ql/shared_ptr.hpp>
#include <ql/methods/finitedifferences/meshers/fdmcev1dmesher.hpp>
#include <ql/methods/finitedifferences/meshers/uniform1dmesher.hpp>
#include <ql/methods/finitedifferences/meshers/concentrating1dmesher.hpp>

#include <ql/methods/finitedifferences/utilities/cevrndcalculator.hpp>


namespace QuantLib {

    FdmCEV1dMesher::FdmCEV1dMesher(
            Size size,
            Real f0, Real alpha, Real beta,
            Time maturity, Real eps,Real scaleFactor,
            const std::pair<Real, Real>& cPoint)

    : Fdm1dMesher(size) {

        const CEVRNDCalculator rndCalculator(f0, alpha, beta);

        const Real upperBound =
            scaleFactor*rndCalculator.invcdf(1-eps, maturity);

        const Real massAtZero = rndCalculator.massAtZero(maturity);

        const Real lowerBound = (massAtZero > eps)
            ? ((beta < 0)? QL_EPSILON : 0.0)
            : Real(rndCalculator.invcdf(eps, maturity)/scaleFactor);


        ext::shared_ptr<Fdm1dMesher> helper;
        if (   cPoint.first != Null<Real>()
            && cPoint.first >= lowerBound && cPoint.first <= upperBound) {

            helper = ext::make_shared<Concentrating1dMesher>(
                lowerBound, upperBound,size, cPoint);
        }
        else {
            helper = ext::make_shared<Uniform1dMesher>(
                lowerBound, upperBound, size );
        }

        std::copy(helper->locations().begin(),
                  helper->locations().end(),
                  locations_.begin());

        for (Size i=0; i < locations_.size(); ++i) {
            dplus_[i]  = helper->dplus(i);
            dminus_[i] = helper->dminus(i);
        }
    }
}

