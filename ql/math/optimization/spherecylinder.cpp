/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Mark Joshi

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

#include <ql/math/optimization/spherecylinder.hpp>
#include <ql/errors.hpp>

namespace QuantLib {

    namespace {

        template<class T, Real (T::*Value)(Real) const >
        Real BrentMinimize(Real low,
                           Real high,
                           Real tolerance,
                           Size maxIt,
                           const T& theObject) {

            Real leftValue = (theObject.*Value)(low);
            Real rightValue = (theObject.*Value)(high);
            Real W = 0.5*(3.0-sqrt(5.0));
            Real x = W*low+(1-W)*high;
            Real midValue = (theObject.*Value)(x);

            Size iterations = 0;
            while (high-low > tolerance && iterations < maxIt) {
                if (x - low > high -x) { // left interval is bigger
                    Real tentativeNewMid = W*low+(1-W)*x;
                    Real tentativeNewMidValue = 
                        (theObject.*Value)(tentativeNewMid);

                    if (tentativeNewMidValue < midValue) { // go left
                        high =x;
                        rightValue = midValue;
                        x = tentativeNewMid;
                        midValue = tentativeNewMidValue;
                    } else { // go right
                        low = tentativeNewMid;
                        leftValue = tentativeNewMidValue;
                    }
                } else {
                    Real tentativeNewMid = W*x+(1-W)*high;
                    Real tentativeNewMidValue =
                        (theObject.*Value)(tentativeNewMid);

                    if (tentativeNewMidValue < midValue) { // go right
                        low =x;
                        leftValue = midValue;
                        x = tentativeNewMid;
                        midValue = tentativeNewMidValue;
                    } else { // go left
                        high = tentativeNewMid;
                        rightValue = tentativeNewMidValue;
                    }
                }
                ++iterations;
            }
            return x;
        }
    }

    SphereCylinderOptimizer::SphereCylinderOptimizer(Real r,
                                                     Real s,
                                                     Real alpha,
                                                     Real z1,
                                                     Real z2,
                                                     Real z3)
    : r_(r), s_(s), alpha_(alpha), z1_(z1), z2_(z2), z3_(z3) {

        QL_REQUIRE(r>0,
                   "sphere must have positive radius");
        QL_REQUIRE(s>0,
                   "cylinder must have positive radius");
        QL_REQUIRE(alpha>0,
                   "cylinder centre must have positive coordinate");
        Real cylinderInside = r*r - (s*s + alpha*alpha);

        if (cylinderInside >0.0)
            topValue_ = s;
        else
            topValue_ = sqrt(s*s - cylinderInside*cylinderInside/(4*alpha*alpha));
    }

    bool SphereCylinderOptimizer::isIntersectionNonEmpty() const {
        return topValue_ >=0.0;
    }

    void SphereCylinderOptimizer::findClosest(Size maxIterations,
                                              Real tolerance,
                                              Real& y1,
                                              Real& y2,
                                              Real& y3) const {
         y2 = BrentMinimize<SphereCylinderOptimizer,
                            &SphereCylinderOptimizer::objectiveFunction>(
                                0.0, topValue_,tolerance, maxIterations,*this);
         y1 = alpha_ - sqrt(s_*s_-y2*y2);
         y3= sqrt(r_*r_ - y1*y1-y2*y2);
    }

    Real SphereCylinderOptimizer::objectiveFunction(Real x2) const {
        Real x1 = alpha_ - sqrt(s_*s_-x2*x2);
        Real x3= sqrt(r_*r_ - x1*x1-x2*x2);

        Real err=0.0;
        err+= (x1-z1_)*(x1-z1_);
        err+= (x2-z2_)*(x2-z2_);
        err+= (x3-z3_)*(x3-z3_);

        return err;
    }

    bool SphereCylinderOptimizer::findByProjection(Real& y1,
                                                   Real& y2,
                                                   Real& y3) const {
        Real z1moved = z1_-alpha_;
        Real distance = sqrt( z1moved*z1moved + z2_*z2_);
        Real scale = s_/distance;
        Real y1moved = z1moved*scale;
        y1 = alpha_+ y1moved;
        y2 = scale*z2_;
        Real residual = r_*r_ - y1*y1 -y2*y2;
        if (residual >=0.0) {
            y3 = sqrt(residual);
            return true;
        }
        // we are outside the sphere
        if (!isIntersectionNonEmpty()) {
            y3=0.0;
            return false;
        }

       // intersection is non-empty but projection point is outside sphere
       // so take rightmost point
       y3 = 0.0;
       y2 = topValue_;
       y1 = sqrt(r_*r_ -y2*y2);

       return true;
    }

    std::vector<Real> sphereCylinderOptimizerClosest(Real r,
                                                     Real s,
                                                     Real alpha,
                                                     Real z1,
                                                     Real z2,
                                                     Real z3,
                                                     Natural maxIterations,
                                                     Real tolerance) {

        SphereCylinderOptimizer optimizer(r, s, alpha, z1, z2, z3);
        std::vector<Real> y(3);

        QL_REQUIRE(optimizer.isIntersectionNonEmpty(),
                   "intersection empty so no solution");

        if (maxIterations ==0)
            optimizer.findByProjection(y[0], y[1], y[2]);
        else
            optimizer.findClosest(maxIterations, tolerance, y[0], y[1], y[2]);
   
        return y;
     }

}
