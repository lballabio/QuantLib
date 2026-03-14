/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*

Copyright (C) 2007 Mark Joshi

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

#include <ql/models/marketmodels/forwardforwardmappings.hpp>
#include <ql/models/marketmodels/curvestate.hpp>
#include <ql/models/marketmodels/curvestates/lmmcurvestate.hpp>
#include <vector>

namespace QuantLib {

    Matrix ForwardForwardMappings::ForwardForwardJacobian(const CurveState& cs,
                                                          Size multiplier,
                                                          Size offset) {

        Size n = cs.numberOfRates();

        QL_REQUIRE(offset < multiplier, "offset  must be less than period in"
            "  forward forward mappings");
        Size k = (n-offset)/multiplier;

        const std::vector<Time>& tau = cs.rateTaus();

        Matrix jacobian = Matrix(k, n, 0.0);

        Size m=offset;
        for (Size l=0; l < k; ++l)
            {
            Real df = cs.discountRatio(m,m+multiplier);
            Real bigTau = cs.rateTimes()[m+multiplier]
            -  cs.rateTimes()[m];

            for (Size r=0; r < multiplier; ++r, ++m)
                {
                Real value = df * tau[m]*cs.discountRatio(m+1,m)-1;
                value /= bigTau;
                jacobian[l][m]=-value;

                }
            }

        return jacobian;
    }

    Matrix ForwardForwardMappings::YMatrix(const CurveState& cs,
                                           const std::vector<Spread>& shortDisplacements,
                                           const std::vector<Spread>& longDisplacements,
                                           Size multiplier,
                                           Size offset) {
        Size n = cs.numberOfRates();

        QL_REQUIRE(offset < multiplier, "offset  must be less than period in"
            "  forward forward mappings");
        Size k = (n-offset)/multiplier;


        QL_REQUIRE(shortDisplacements.size() == n , "shortDisplacements must be of size"
            " equal to number of rates");

        QL_REQUIRE(longDisplacements.size() == k , "longDisplacements must be of size"
            " equal to (number of rates minus offset) divided by multiplier");

        Matrix jacobian(ForwardForwardJacobian(cs,multiplier,offset));

        for (Size i=0; i < k ; ++i)
            {
            Real tau = cs.rateTimes()[(i+1)*multiplier+offset]
            -  cs.rateTimes()[i*multiplier+offset];

            Real longForward = (cs.discountRatio((i+1)*multiplier+offset,i*multiplier+offset)-1.0)
                /tau;
            Real longForwardDisplaced = longForward+ longDisplacements[i];
            for (Size j=0; j < n; ++j)
                {
                Real shortForward = cs.forwardRate(j);
                Real shortForwardDisplaced = shortForward+shortDisplacements[j];
                jacobian[i][j] *= shortForwardDisplaced/longForwardDisplaced;
                }

            }

        return jacobian;
    }

    LMMCurveState ForwardForwardMappings::RestrictCurveState(const CurveState& cs,
                                                             Size multiplier,
                                                             Size offset) {
           Size n = cs.numberOfRates();

           QL_REQUIRE(offset < multiplier, "offset  must be less than period in"
           "  forward forward mappings");
           Size k = (n-offset)/multiplier;

           std::vector<Time> times(k+1);
           std::vector<DiscountFactor> discRatios(k+1);


           for (Size i=0; i < k+1; ++i)
           {
               times[i] = cs.rateTimes()[i*multiplier+offset];
               discRatios[i] = cs.discountRatio(i*multiplier+offset,0);
           }

           LMMCurveState newState(times);
           newState.setOnDiscountRatios(discRatios);
           return newState;

    }
}
