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
                           Real mid,
                           Real high,
                           Real tolerance,
                           Size maxIt,
                           const T& theObject) {

            Real leftValue = (theObject.*Value)(low);
            Real rightValue = (theObject.*Value)(high);
            Real W = 0.5*(3.0-sqrt(5.0));
            Real x = W*low+(1-W)*high;
            if (mid > low && mid < high)
                x = mid; 

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
        
        if (fabs(alpha-s) > r )
            nonEmpty_=false;
        else
            nonEmpty_=true;

        Real cylinderInside = r*r - (s + alpha)*(s+alpha);

        if (cylinderInside >0.0)
        {
            topValue_ = alpha+s;
            bottomValue_ = alpha-s;                
        }
        else
        {
          
            bottomValue_ = alpha-s;
            Real tmp = r*r - (s *s+alpha*alpha);

            if (  tmp <=0)
            { // max to left of maximimum
                Real topValue2 = sqrt(s*s - tmp*tmp/(4*alpha*alpha));
                topValue_ = alpha -sqrt(s*s - topValue2*topValue2);
                
            }
            else    
            { 
                topValue_ = alpha+ tmp/(2.0*alpha);
                
            }
           
            
        }
            
    }

    bool SphereCylinderOptimizer::isIntersectionNonEmpty() const {
        return nonEmpty_;
    }

    void SphereCylinderOptimizer::findClosest(Size maxIterations,
                                              Real tolerance,
                                              Real& y1,
                                              Real& y2,
                                              Real& y3) const 
    {
         Real x1,x2,x3;
         findByProjection(x1,x2,x3);

         y1 = BrentMinimize<SphereCylinderOptimizer,
                            &SphereCylinderOptimizer::objectiveFunction>(
                                bottomValue_, x1, topValue_,tolerance, maxIterations,*this);
         y2 =sqrt(s_*s_ - (y1-alpha_)*(y1-alpha_));
         y3= sqrt(r_*r_ - y1*y1-y2*y2);
    }

    Real SphereCylinderOptimizer::objectiveFunction(Real x1) const 
    {
   //     Real x1 = alpha_ - sqrt(s_*s_-x2*x2);
      
        Real x2 = sqrt(s_*s_ - (x1-alpha_)*(x1-alpha_));
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
       y1 = topValue_;
       y2 = sqrt(r_*r_ -y1*y1);

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
