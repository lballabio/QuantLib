
/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

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

/* The implementation of the algorithm was highly inspired by
 * "Numerical Recipes in C", 2nd edition, Press, Teukolsky, Vetterling, 
 * Flannery, chapter 10
 */

#include <ql/Optimization/simplex.hpp>

namespace QuantLib {

    Real Simplex::extrapolate(const Problem& P,
                              Size iHighest,
                              Real &factor) const {

        Array pTry;
        do {
            Size dimensions = values_.size() - 1;
            Real factor1 = (1.0 - factor)/dimensions;
            Real factor2 = factor1 - factor;
            pTry = sum_*factor1 - vertices_[iHighest]*factor2;
            factor *= 0.5;
        } while (!P.constraint().test(pTry));
        factor *= 2.0;
        Real vTry = P.value(pTry);
        if (vTry < values_[iHighest]) {
            values_[iHighest] = vTry;
            sum_ += pTry - vertices_[iHighest];
            vertices_[iHighest] = pTry;
        }
        return vTry;

    }

    void Simplex::minimize(const Problem& P) const {
        bool end = false;

        Array& X = x();
        Size n = X.size(), i;

        vertices_ = std::vector<Array>(n+1, X);
        for (i=0; i<n; i++) {
            Array direction(n, 0.0);
            direction[i] = 1.0;
            P.constraint().update(vertices_[i+1], direction, lambda_);
        }
        values_ = Array(n+1, 0.0);
        for (i=0; i<=n; i++)
            values_[i] = P.value(vertices_[i]);


        do {
            sum_ = Array(n, 0.0);
            Size i;
            for (i=0; i<=n; i++)
                sum_ += vertices_[i];

            //Determine best, worst and 2nd worst vertices
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
            Real low = values_[iLowest], high = values_[iHighest];

            Real rtol = 2.0*QL_FABS(high - low)/
                (QL_FABS(high) + QL_FABS(low) + QL_EPSILON);
            if (rtol < tol_) {
                X = vertices_[iLowest];
                return;
            }

            Real factor = -1.0;
            Real vTry = extrapolate(P, iHighest, factor);
            if ((vTry <= values_[iLowest]) && (factor == -1.0)) {
                factor = 2.0;
                extrapolate(P, iHighest, factor);
            } else {
                if (vTry >= values_[iNextHighest]) {
                    Real vSave = values_[iHighest];
                    factor = 0.5;
                    vTry = extrapolate(P, iHighest, factor);
                    if (vTry >= vSave) {
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

        } while (end == false);

    }

}
