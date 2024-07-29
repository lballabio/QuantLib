/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2007 Marco Bianchetti
 Copyright (C) 2007 François du Vignaud

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/* The implementation of the algorithm was highly inspired by
 * "Numerical Recipes in C", 2nd edition, Press, Teukolsky, Vetterling,
 * Flannery, chapter 10.
 * Modified may 2007: end criteria set on x instead on fx,
 * inspired by bad behaviour found with test function fx=x*x+x+1,
 * xStart = -100, lambda = 1.0, ftol = 1.e-16
 * (it reports x=0 as the minimum!)
 * and by GSL implementation, v. 1.9 (http://www.gnu.org/software/gsl/)
 */

#include <ql/math/optimization/simplex.hpp>
#include <ql/math/optimization/constraint.hpp>

namespace QuantLib {

    namespace {
    // Computes the size of the simplex
        Real computeSimplexSize (const std::vector<Array>& vertices) {
            Array center(vertices.front().size(),0);
            for (const auto& vertice : vertices)
                center += vertice;
            center *=1/Real(vertices.size());
            Real result = 0;
            for (const auto& vertice : vertices) {
                Array temp = vertice - center;
                result += Norm2(temp);
            }
            return result/Real(vertices.size());
        }
    }

    Real Simplex::extrapolate(Problem& P,
                              Size iHighest,
                              Real &factor) const {

        Array pTry;
        do {
            Size dimensions = values_.size() - 1;
            Real factor1 = (1.0 - factor)/dimensions;
            Real factor2 = factor1 - factor;
            pTry = sum_*factor1 - vertices_[iHighest]*factor2;
            factor *= 0.5;
        } while (!P.constraint().test(pTry) && std::fabs(factor) > QL_EPSILON);
        if (std::fabs(factor) <= QL_EPSILON) {
            return values_[iHighest];
        }
        factor *= 2.0;
        Real vTry = P.value(pTry);
        if (vTry < values_[iHighest]) {
            values_[iHighest] = vTry;
            sum_ += pTry - vertices_[iHighest];
            vertices_[iHighest] = pTry;
        }
        return vTry;

    }


    EndCriteria::Type Simplex::minimize(Problem& P,
                                        const EndCriteria& endCriteria) {
        // set up of the problem
        //Real ftol = endCriteria.functionEpsilon();    // end criteria on f(x) (see Numerical Recipes in C++, p.410)
        Real xtol = endCriteria.rootEpsilon();          // end criteria on x (see GSL v. 1.9, http://www.gnu.org/software/gsl/)
        Size maxStationaryStateIterations_
            = endCriteria.maxStationaryStateIterations();
        EndCriteria::Type ecType = EndCriteria::None;
        P.reset();

        Array x_ = P.currentValue();
        if (!P.constraint().test(x_))
            QL_FAIL("Initial guess " << x_ << " is not in the feasible region.");

        Integer iterationNumber_=0;

        // Initialize vertices of the simplex
        Size n = x_.size();
        vertices_ = std::vector<Array>(n+1, x_);
        for (Size i=0; i<n; ++i) {
            Array direction(n, 0.0);
            direction[i] = 1.0;
            P.constraint().update(vertices_[i+1], direction, lambda_);
        }
        // Initialize function values at the vertices of the simplex
        values_ = Array(n+1, 0.0);
        for (Size i=0; i<=n; ++i)
            values_[i] = P.value(vertices_[i]);
        // Loop looking for minimum
        do {
            sum_ = Array(n, 0.0);
            Size i;
            for (i=0; i<=n; i++)
                sum_ += vertices_[i];
            // Determine the best (iLowest), worst (iHighest)
            // and 2nd worst (iNextHighest) vertices
            Size iLowest = 0;
            Size iHighest, iNextHighest;
            if (values_[0]<values_[1]) {
                iHighest = 1;
                iNextHighest = 0;
            } else {
                iHighest = 0;
                iNextHighest = 1;
            }
            for (i=1;i<=n; i++) {
                if (values_[i]>values_[iHighest]) {
                    iNextHighest = iHighest;
                    iHighest = i;
                } else {
                    if ((values_[i]>values_[iNextHighest]) && i!=iHighest)
                        iNextHighest = i;
                }
                if (values_[i]<values_[iLowest])
                    iLowest = i;
            }
            // Now compute accuracy, update iteration number and check end criteria
            //// Numerical Recipes exit strategy on fx (see NR in C++, p.410)
            //Real low = values_[iLowest];
            //Real high = values_[iHighest];
            //Real rtol = 2.0*std::fabs(high - low)/
            //    (std::fabs(high) + std::fabs(low) + QL_EPSILON);
            //++iterationNumber_;
            //if (rtol < ftol ||
            //    endCriteria.checkMaxIterations(iterationNumber_, ecType)) {
            // GSL exit strategy on x (see GSL v. 1.9, http://www.gnu.org/software/gsl
            Real simplexSize = computeSimplexSize(vertices_);
            ++iterationNumber_;
            if (simplexSize < xtol ||
                endCriteria.checkMaxIterations(iterationNumber_, ecType)) {
                endCriteria.checkStationaryPoint(0.0, 0.0,
                    maxStationaryStateIterations_, ecType);
                endCriteria.checkMaxIterations(iterationNumber_, ecType);
                x_ = vertices_[iLowest];
                Real low = values_[iLowest];
                P.setFunctionValue(low);
                P.setCurrentValue(x_);
                return ecType;
            }
            // If end criteria is not met, continue
            Real factor = -1.0;
            Real vTry = extrapolate(P, iHighest, factor);
            if ((vTry <= values_[iLowest]) && (factor == -1.0)) {
                factor = 2.0;
                extrapolate(P, iHighest, factor);
            } else if (std::fabs(factor) > QL_EPSILON) {
                if (vTry >= values_[iNextHighest]) {
                    Real vSave = values_[iHighest];
                    factor = 0.5;
                    vTry = extrapolate(P, iHighest, factor);
                    if (vTry >= vSave && std::fabs(factor) > QL_EPSILON) {
                        for (Size i=0; i<=n; i++) {
                            if (i!=iLowest) {
                                vertices_[i] =
                                    0.5*(vertices_[i] + vertices_[iLowest]);
                                values_[i] = P.value(vertices_[i]);
                            }
                        }
                    }
                }
            }
            // If can't extrapolate given the constraints, exit
            if (std::fabs(factor) <= QL_EPSILON) {
                x_ = vertices_[iLowest];
                Real low = values_[iLowest];
                P.setFunctionValue(low);
                P.setCurrentValue(x_);
                return EndCriteria::StationaryFunctionValue;
            }
        } while (true);
        QL_FAIL("optimization failed: unexpected behaviour");
    }
}
