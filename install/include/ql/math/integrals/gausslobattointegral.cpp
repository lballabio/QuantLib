/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Klaus Spanderen

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

/*! \file gausslabottointegral.cpp
    \brief integral of a one-dimensional function using the adaptive 
    Gauss-Lobatto integral
*/

#include <ql/math/integrals/gausslobattointegral.hpp>
#include <algorithm>

namespace QuantLib {

    const Real GaussLobattoIntegral::alpha_ = std::sqrt(2.0/3.0); 
    const Real GaussLobattoIntegral::beta_  = 1.0/std::sqrt(5.0);
    const Real GaussLobattoIntegral::x1_    = 0.94288241569547971906; 
    const Real GaussLobattoIntegral::x2_    = 0.64185334234578130578;
    const Real GaussLobattoIntegral::x3_    = 0.23638319966214988028;

    GaussLobattoIntegral::GaussLobattoIntegral(Size maxIterations,
                                               Real absAccuracy,
                                               Real relAccuracy,
                                               bool useConvergenceEstimate)
    : Integrator(absAccuracy, maxIterations),
      relAccuracy_(relAccuracy),
      useConvergenceEstimate_(useConvergenceEstimate) {
    }

    Real GaussLobattoIntegral::integrate(
                                     const std::function<Real (Real)>& f, 
                                     Real a, Real b) const {

        setNumberOfEvaluations(0);
        const Real calcAbsTolerance = calculateAbsTolerance(f, a, b);

        increaseNumberOfEvaluations(2);
        return adaptivGaussLobattoStep(f, a, b, f(a), f(b), calcAbsTolerance);
    }

    Real GaussLobattoIntegral::calculateAbsTolerance(
                                     const std::function<Real (Real)>& f, 
                                     Real a, Real b) const {
        

        Real relTol = std::max(relAccuracy_, QL_EPSILON);
        
        const Real m = (a+b)/2; 
        const Real h = (b-a)/2;
        const Real y1 = f(a);
        const Real y3 = f(m-alpha_*h);
        const Real y5 = f(m-beta_*h);
        const Real y7 = f(m);
        const Real y9 = f(m+beta_*h);
        const Real y11= f(m+alpha_*h);
        const Real y13= f(b);

        const Real f1 = f(m-x1_*h);
        const Real f2 = f(m+x1_*h);
        const Real f3 = f(m-x2_*h);
        const Real f4 = f(m+x2_*h);
        const Real f5 = f(m-x3_*h);
        const Real f6 = f(m+x3_*h);

        Real acc=h*(0.0158271919734801831*(y1+y13)
                  +0.0942738402188500455*(f1+f2)
                  +0.1550719873365853963*(y3+y11)
                  +0.1888215739601824544*(f3+f4)
                  +0.1997734052268585268*(y5+y9) 
                  +0.2249264653333395270*(f5+f6)
                  +0.2426110719014077338*y7);  
        
        increaseNumberOfEvaluations(13);
        if (acc == 0.0 && (   f1 != 0.0 || f2 != 0.0 || f3 != 0.0
                           || f4 != 0.0 || f5 != 0.0 || f6 != 0.0)) {
            QL_FAIL("can not calculate absolute accuracy "
                    "from relative accuracy");
        }

        Real r = 1.0;
        if (useConvergenceEstimate_) {
            const Real integral2 = (h/6)*(y1+y13+5*(y5+y9));
            const Real integral1 = (h/1470)*(77*(y1+y13)+432*(y3+y11)+
                                             625*(y5+y9)+672*y7);
        
            if (std::fabs(integral2-acc) != 0.0) 
                r = std::fabs(integral1-acc)/std::fabs(integral2-acc);
            if (r == 0.0 || r > 1.0)
                r = 1.0;
        }

        if (relAccuracy_ != Null<Real>())
            return std::min(absoluteAccuracy(), acc*relTol)/(r*QL_EPSILON);
        else {
            return absoluteAccuracy()/(r*QL_EPSILON);
        }
    }
    
    Real GaussLobattoIntegral::adaptivGaussLobattoStep(
                                     const std::function<Real (Real)>& f,
                                     Real a, Real b, Real fa, Real fb,
                                     Real acc) const {
        QL_REQUIRE(numberOfEvaluations() < maxEvaluations(),
                   "max number of iterations reached");
        
        const Real h=(b-a)/2; 
        const Real m=(a+b)/2;
        
        const Real mll=m-alpha_*h; 
        const Real ml =m-beta_*h; 
        const Real mr =m+beta_*h; 
        const Real mrr=m+alpha_*h;
        
        const Real fmll= f(mll);
        const Real fml = f(ml);
        const Real fm  = f(m);
        const Real fmr = f(mr);
        const Real fmrr= f(mrr);
        increaseNumberOfEvaluations(5);
        
        const Real integral2=(h/6)*(fa+fb+5*(fml+fmr));
        const Real integral1=(h/1470)*(77*(fa+fb)
                                       +432*(fmll+fmrr)+625*(fml+fmr)+672*fm);
        
        // avoid 80 bit logic on x86 cpu
        const Real dist = acc + (integral1-integral2);
        if(dist==acc || mll<=a || b<=mrr) {
            QL_REQUIRE(m>a && b>m,"Interval contains no more machine number");
            return integral1;
        }
        else {
            return  adaptivGaussLobattoStep(f,a,mll,fa,fmll,acc)  
                  + adaptivGaussLobattoStep(f,mll,ml,fmll,fml,acc)
                  + adaptivGaussLobattoStep(f,ml,m,fml,fm,acc)
                  + adaptivGaussLobattoStep(f,m,mr,fm,fmr,acc)
                  + adaptivGaussLobattoStep(f,mr,mrr,fmr,fmrr,acc)
                  + adaptivGaussLobattoStep(f,mrr,b,fmrr,fb,acc);
        }
    }
}
