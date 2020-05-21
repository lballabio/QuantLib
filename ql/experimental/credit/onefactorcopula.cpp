/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Roland Lichters

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

#include <ql/experimental/credit/onefactorcopula.hpp>

using namespace std;

namespace QuantLib {

    //-------------------------------------------------------------------------
    Real OneFactorCopula::conditionalProbability(Real p, Real m) const {
    //-------------------------------------------------------------------------
        calculate ();
        // FIXME
        if (p < 1e-10) return 0;

        Real c = correlation_->value();

        Real res = cumulativeZ ((inverseCumulativeY (p) - sqrt(c) * m)
                                / sqrt (1. - c));

        QL_REQUIRE (res >= 0 && res <= 1,
                    "conditional probability " << res << "out of range");

        return res;
    }

    //-------------------------------------------------------------------------
    vector<Real> OneFactorCopula::conditionalProbability(
                                                     const vector<Real>& prob,
                                                     Real m) const {
    //-------------------------------------------------------------------------
        calculate ();
        vector<Real> p (prob.size(), 0);
        for (Size i = 0; i < p.size(); i++)
            p[i] = conditionalProbability (prob[i], m);
        return p;
    }

    //-------------------------------------------------------------------------
    Real OneFactorCopula::cumulativeY (Real y) const {
    //-------------------------------------------------------------------------
        calculate ();

        QL_REQUIRE(!y_.empty(), "cumulative Y not tabulated yet");

        // linear interpolation on the tabulated cumulative distribution of Y
        if (y < y_.front())
            return cumulativeY_.front();

        for (Size i = 0; i < y_.size(); i++) {
            if (y_[i] > y)
                return (   (y_[i] - y)   * cumulativeY_[i-1]
                           + (y - y_[i-1]) * cumulativeY_[i]   )
                    / (y_[i] - y_[i-1]);
        }

        return cumulativeY_.back();
    }

    //-------------------------------------------------------------------------
    Real OneFactorCopula::inverseCumulativeY (Real x) const {
    //-------------------------------------------------------------------------
        calculate ();

        QL_REQUIRE(!y_.empty(), "cumulative Y not tabulated yet");

        // linear interpolation on the tabulated cumulative distribution of Y
        if (x < cumulativeY_.front())
            return y_.front();

        for (Size i = 0; i < cumulativeY_.size(); i++) {
            if (cumulativeY_[i] > x)
                return (   (cumulativeY_[i] - x)   * y_[i-1]
                           + (x - cumulativeY_[i-1]) * y_[i]   )
                    / (cumulativeY_[i] - cumulativeY_[i-1]);
        }

        return y_.back();
    }

    //-------------------------------------------------------------------------
    int OneFactorCopula::checkMoments (Real tolerance) const {
    //-------------------------------------------------------------------------
        calculate ();

        Real norm = 0, mean = 0, var = 0;
        for (Size i = 0; i < steps(); i++) {
            norm += densitydm (i);
            mean += m(i) * densitydm (i);
            var += pow (m(i), 2) * densitydm (i);
        }

        QL_REQUIRE (fabs (norm - 1.0) < tolerance, "norm out of tolerance range");
        QL_REQUIRE (fabs (mean) < tolerance, "mean out of tolerance range");
        QL_REQUIRE (fabs (var - 1.0) < tolerance, "variance out of tolerance range");

        // FIXME: define range for Y via cutoff quantil?
        Real zMin = -10;
        Real zMax = +10;
        Size zSteps = 200;
        norm = 0;
        mean = 0;
        var = 0;
        for (Size i = 1; i < zSteps; i++) {
            Real z1 = zMin + (zMax - zMin) / zSteps * (i - 1);
            Real z2 = zMin + (zMax - zMin) / zSteps * i;
            Real z  = (z1 + z2) / 2;
            Real densitydz = cumulativeZ (z2) - cumulativeZ (z1);
            norm += densitydz;
            mean += z * densitydz;
            var += pow (z, 2) * densitydz;
        }

        QL_REQUIRE (fabs (norm - 1.0) < tolerance, "norm out of tolerance range");
        QL_REQUIRE (fabs (mean) < tolerance, "mean out of tolerance range");
        QL_REQUIRE (fabs (var - 1.0) < tolerance, "variance out of tolerance range");

        // FIXME: define range for Y via cutoff quantil?
        Real yMin = -10;
        Real yMax = +10;
        Size ySteps = 200;
        norm = 0;
        mean = 0;
        var = 0;
        for (Size i = 1; i < ySteps; i++) {
            Real y1 = yMin + (yMax - yMin) / ySteps * (i - 1);
            Real y2 = yMin + (yMax - yMin) / ySteps * i;
            Real y  = (y1 + y2) / 2;
            Real densitydy = cumulativeY (y2) - cumulativeY (y1);
            norm += densitydy;
            mean += y * densitydy;
            var += y * y * densitydy;
        }

        QL_REQUIRE (fabs (norm - 1.0) < tolerance, "norm out of tolerance range");
        QL_REQUIRE (fabs (mean) < tolerance, "mean out of tolerance range");
        QL_REQUIRE (fabs (var - 1.0) < tolerance, "variance out of tolerance range");

        return 0;
    }

}

