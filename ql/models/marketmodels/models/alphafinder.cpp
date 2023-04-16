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

#include <ql/math/quadratic.hpp>
#include <ql/models/marketmodels/models/alphafinder.hpp>
#include <utility>

namespace QuantLib {

namespace
{

    template<class T>
    Real Bisection(Real target,
                   Real low,
                   Real high,
                   Real tolerance,
                   T& theObject,
                   Real (T::*Value)(Real)) {

        Real x=0.5*(low+high);
        Real y=(theObject.*Value)(x);

        do {
            if (y < target) low = x;
            else if (y > target) high = x;

            x = 0.5*(low+high);
            y = (theObject.*Value)(x);
        } while ((std::fabs(high-low) > tolerance));

        return x;
    }

    template<class T>
    Real FindHighestOK(Real low,
                       Real high,
                       Real tolerance,
                       T& theObject,
                       bool (T::*Value)(Real)) {

        Real x=0.5*(low+high);
        bool ok=(theObject.*Value)(x);

        do {
            if (ok) low = x;
            else    high = x;

            x = 0.5*(low+high);
            ok = (theObject.*Value)(x);
        } while ((std::fabs(high-low) > tolerance));

        return x;
    }

    template<class T>
    Real FindLowestOK(Real low,
                      Real high,
                      Real tolerance,
                      T& theObject,
                      bool (T::*Value)(Real)) {

        Real x=0.5*(low+high);
        bool ok=(theObject.*Value)(x);

        do {
            if (ok) high = x;
            else    low = x;

            x = 0.5*(low+high);
            ok = (theObject.*Value)(x);
        } while ( (std::fabs(high-low) > tolerance) );

        return x;
    }


    template<class T>
    Real Minimize(Real low,
                  Real high,
                  Real tolerance,
                  T& theObject,
                  Real (T::*Value)(Real),
                  bool (T::*Condition)(Real),
                  bool& failed) {

        Real leftValue = (theObject.*Value)(low);
        Real rightValue = (theObject.*Value)(high);
        Real W = 0.5*(3.0-std::sqrt(5.0));
        Real x=W*low+(1-W)*high;
        Real midValue =  (theObject.*Value)(x);

        failed = true;

        while(high - low > tolerance) {

            if (x - low > high -x) // left interval is bigger
            {
                Real tentativeNewMid = W*low+(1-W)*x;
                Real tentativeNewMidValue =  (theObject.*Value)(tentativeNewMid);
                bool conditioner = (theObject.*Condition)(tentativeNewMidValue);
                if (!conditioner) {
                    if  ((theObject.*Condition)(x))
                        return x;
                    else
                        if (leftValue < rightValue)
                            return low;
                        else
                            return high;
                }

                if (tentativeNewMidValue < midValue) // go left
                {
                    high =x;
                    rightValue = midValue;
                    x = tentativeNewMid;
                    midValue = tentativeNewMidValue;
                }
                else // go right
                {
                    low = tentativeNewMid;
                    leftValue = tentativeNewMidValue;
                }
            }
            else
            {
                Real tentativeNewMid = W*x+(1-W)*high;
                Real tentativeNewMidValue =  (theObject.*Value)(tentativeNewMid);
                bool conditioner = (theObject.*Condition)(tentativeNewMidValue);
                if (!conditioner) {
                    if  ((theObject.*Condition)(x))
                        return x;
                    else
                        if (leftValue < rightValue)
                            return low;
                        else
                            return high;
                }

                if (tentativeNewMidValue < midValue) // go right
                {
                    low =x;
                    leftValue = midValue;
                    x = tentativeNewMid;
                    midValue = tentativeNewMidValue;
                }
                else // go left
                {
                    high = tentativeNewMid;
                    rightValue = tentativeNewMidValue;
                }
            }




            }
        failed = false;
        return x;
    }
}

AlphaFinder::AlphaFinder(std::shared_ptr<AlphaForm> parametricform)
: parametricform_(std::move(parametricform)) {}


Real AlphaFinder::computeLinearPart(Real alpha) {
    Real cov = 0.0;
    parametricform_->setAlpha(alpha);

    for (Integer i = 0; i < stepindex_ + 1; ++i) {
        Real vol1 = ratetwohomogeneousvols_[i] * (*parametricform_)(i);
        cov += vol1 * rateonevols_[i] * correlations_[i];
    }

    cov *= 2 * w0_ * w1_;
    return cov;
    }


    Real AlphaFinder::computeQuadraticPart(Real alpha) {
        Real var =0.0;
        parametricform_->setAlpha(alpha);

        for (Integer i=0; i < stepindex_+1; ++i) {
            Real vol = ratetwohomogeneousvols_[i]*(*parametricform_)(i);
            var+= vol*vol;
        }

        var *= w1_*w1_;
        return var;
    }

    Real AlphaFinder::homogeneityfailure(Real alpha) {
        Real dum1, dum2, dum3;
        finalPart(alpha,
                  stepindex_,
                  ratetwohomogeneousvols_,
                  computeQuadraticPart(alpha),
                  computeLinearPart(alpha),
                  constantPart_,
                  dum1,
                  dum2,
                  dum3,
                  putativevols_);

    Real result=0.0;
    for (Size i=0; i<=static_cast<Size>(stepindex_)+1; ++i) {
        Real val =  putativevols_[i]-ratetwohomogeneousvols_[i];
        result +=val*val;
    }

    return result;
}

    bool AlphaFinder::finalPart(Real alphaFound,
                                Integer stepindex,
                                const std::vector<Volatility>& ratetwohomogeneousvols,
                                Real quadraticPart,
                                Real linearPart,
                                Real constantPart,
                                Real& alpha,
                                Real& a,
                                Real& b,
                                std::vector<Volatility>& ratetwovols) {
            alpha = alphaFound;
            quadratic q2(quadraticPart, linearPart, constantPart-targetVariance_ );
            parametricform_->setAlpha(alpha);
            Real y; // dummy
            q2.roots(a,y);

            Real varSoFar=0.0;
            for (Integer i =0; i < stepindex+1; ++i) {
                ratetwovols[i] = ratetwohomogeneousvols[i] *
                                            (*parametricform_)(i) * a;
                varSoFar += ratetwovols[i]* ratetwovols[i];
            }

            Real VarToFind = totalVar_-varSoFar;
            if (VarToFind < 0)
                return false;
            Real requiredSd = std::sqrt(VarToFind);
            b = requiredSd / (ratetwohomogeneousvols[stepindex+1] *
                                            (*parametricform_)(stepindex));
            ratetwovols[stepindex+1] = requiredSd;
            return true;
    }

    Real AlphaFinder::valueAtTurningPoint(Real alpha) {

        linearPart_ = computeLinearPart(alpha);
        quadraticPart_ = computeQuadraticPart(alpha);

        quadratic q(quadraticPart_, linearPart_, constantPart_);
        Real valueAtTP =q.valueAtTurningPoint();
        return valueAtTP;
    }

    Real AlphaFinder::minusValueAtTurningPoint(Real alpha) {
        return -valueAtTurningPoint(alpha);
    }

    bool AlphaFinder::testIfSolutionExists(Real alpha) {
        bool aExists =  valueAtTurningPoint(alpha)<targetVariance_;
        if (!aExists)
            return false;

        Real dum1, dum2, dum3;
        return finalPart(alpha,
                         stepindex_,
                         ratetwohomogeneousvols_,
                         computeQuadraticPart(alpha),
                         computeLinearPart(alpha),
                         constantPart_,
                         dum1,
                         dum2,
                         dum3,
                         putativevols_);
    }

    bool AlphaFinder::solve(Real alpha0,
                            Integer stepindex, // caplet index
                            const std::vector<Volatility>& rateonevols,
                            const std::vector<Volatility>& ratetwohomogeneousvols,
                            const std::vector<Real>& correlations,
                            Real w0,
                            Real w1,
                            Real targetVariance,
                            Real tolerance,
                            Real alphaMax,
                            Real alphaMin,
                            Integer steps,
                            Real& alpha,
                            Real& a,
                            Real& b,
                            std::vector<Volatility>& ratetwovols) {
        stepindex_=stepindex;
        rateonevols_=rateonevols;
        ratetwohomogeneousvols_=ratetwohomogeneousvols;
        correlations_=correlations;
        w0_=w0;
        w1_=w1;
        totalVar_=0;
        for (Size i=0; i <=static_cast<Size>(stepindex)+1; ++i)
            totalVar_+=ratetwohomogeneousvols[i]*ratetwohomogeneousvols[i];
        targetVariance_ = targetVariance;

        // constant part will not depend on alpha

        constantPart_ =0.0;
        for (Integer i=0; i < stepindex+1; ++i)
                constantPart_+=rateonevols[i]*rateonevols[i];

        constantPart_ *= w0*w0;

        // compute linear part with initial alpha
        Real valueAtTP = valueAtTurningPoint(alpha0);

        if (valueAtTP <= targetVariance) {
            finalPart(alpha0,
                      stepindex,
                      ratetwohomogeneousvols,
                      quadraticPart_,
                      linearPart_,
                      constantPart_,
                      alpha,
                      a,
                      b,
                      ratetwovols);
            return true;
        }

        // we now have to solve
        Real bottomValue = valueAtTurningPoint(alphaMin);
        Real bottomAlpha = alphaMin;
        Real topValue = valueAtTurningPoint(alphaMax);
        Real topAlpha = alphaMax;
        Real bilimit = alpha0;

        if (bottomValue > targetVariance && topValue > targetVariance) {
            // see if if ok at some intermediate point by stepping through
            Integer i=1;
            while ( i < steps && topValue> targetVariance) {
                topAlpha = alpha0 + (alphaMax-alpha0)*(i+0.0)/(steps+0.0);
                topValue=valueAtTurningPoint(topAlpha);
                ++i;
            }

            if (topValue <= targetVariance)
                bilimit = alpha0 + (topAlpha-alpha0)*(i-2.0)/(steps+0.0);
        }

        if (bottomValue > targetVariance && topValue > targetVariance) {
            // see if if ok at some intermediate point by stepping through
            Integer i=1;
            while ( i < steps && topValue> targetVariance) {
                bottomAlpha = alpha0 + (alphaMin-alpha0)*(i+0.0)/(steps+0.0);
                bottomValue=valueAtTurningPoint(bottomAlpha);
                ++i;
            }

            if (bottomValue <= targetVariance)
                bilimit = alpha0 +(bottomAlpha-alpha0)*(i-2.0)/(steps+0.0);
        }

        if (bottomValue > targetVariance && topValue > targetVariance)
            return false;

        if (bottomValue <= targetVariance) {
            // then find root of increasing function
            // (or as if increasing function)
            alpha = Bisection<AlphaFinder>(
                                           targetVariance,
                                           bottomAlpha,
                                           bilimit,
                                           tolerance,
                                           *this,
                                           &AlphaFinder::valueAtTurningPoint);
        } else {
            // find root of decreasing function (or as if decreasing function)
            alpha=Bisection<AlphaFinder>(
                                         -targetVariance,
                                         bilimit,
                                         topAlpha,
                                         tolerance,
                                         *this,
                                         &AlphaFinder::minusValueAtTurningPoint);
        }
        finalPart(alpha,
                  stepindex,
                  ratetwohomogeneousvols,
                  quadraticPart_,
                  linearPart_,
                  constantPart_,
                  alpha,
                  a,
                  b,
                  ratetwovols);
        return true;
    }

    bool AlphaFinder::solveWithMaxHomogeneity(
                        Real alpha0,
                        Integer stepindex, // caplet index
                        const std::vector<Volatility>& rateonevols,
                        const std::vector<Volatility>& ratetwohomogeneousvols,
                        const std::vector<Real>& correlations,
                        Real w0,
                        Real w1,
                        Real targetVariance,
                        Real tolerance,
                        Real alphaMax,
                        Real alphaMin,
                        Integer steps,
                        Real& alpha,
                        Real& a,
                        Real& b,
                        std::vector<Volatility>& ratetwovols) {

        stepindex_=stepindex;
        rateonevols_=rateonevols;
        ratetwohomogeneousvols_=ratetwohomogeneousvols;
        putativevols_.resize(ratetwohomogeneousvols_.size());
        correlations_=correlations;
        w0_=w0;
        w1_=w1;
        totalVar_=0;
        for (Size i=0; i <=static_cast<Size>(stepindex)+1; ++i)
            totalVar_+=ratetwohomogeneousvols[i]*ratetwohomogeneousvols[i];
        targetVariance_=targetVariance;

        // constant part will not depend on alpha

        constantPart_ =0.0;
        for (Integer i=0; i < stepindex+1; ++i)
            constantPart_+=rateonevols[i]*rateonevols[i];

        constantPart_ *= w0*w0;

        Real alpha1 = alphaMin;
        Real alpha2 = alphaMax;

        // compute linear part with initial alpha
        bool alpha0OK = testIfSolutionExists(alpha0);
        bool alphaMaxOK = testIfSolutionExists(alphaMax);
        bool alphaMinOK = testIfSolutionExists(alphaMin);

        bool foundOKPoint = alpha0OK || alphaMaxOK || alphaMinOK;

        if (foundOKPoint) {
            if (!alphaMinOK) {
                // lower alpha is bad
                if (alpha0OK) {
                    // must die somewhere in between
                    alpha1 = FindLowestOK<AlphaFinder>(
                         alphaMin,
                         alpha0,
                         tolerance,
                        *this,
                         &AlphaFinder::testIfSolutionExists);
                } else {
                    // alphaMaxOK must be true to get here
                    alpha1 = FindLowestOK<AlphaFinder>(
                         alpha0,
                         alphaMax,
                         tolerance,
                        *this,
                         &AlphaFinder::testIfSolutionExists);
                }
            }


            if (!alphaMaxOK) {
                // higher alpha is bad
                alpha2 = FindHighestOK<AlphaFinder>(
                     alpha1,
                     alphaMax,
                     tolerance,
                     *this,
                     &AlphaFinder::testIfSolutionExists);
            } else
                alpha2= alphaMax;
            }
        else {
            // ok let's see if we can find a value of alpha that works
            bool foundUpOK = false;
            bool foundDownOK = false;
            Real alphaUp = alpha0, alphaDown = alpha0;
            Real stepSize = (alphaMax-alpha0)/steps;

            for (Size j=0;
                 j<static_cast<Size>(steps) && !foundUpOK && !foundDownOK;
                 ++j) {
                alphaUp = alpha0+j*stepSize;
                foundUpOK=testIfSolutionExists(alphaUp);
                alphaDown = alpha0-j*stepSize;
                foundDownOK=testIfSolutionExists(alphaDown);
            }
            foundOKPoint = foundUpOK || foundDownOK;
            if (!foundOKPoint)
                return false;

            if (foundUpOK) {
                alpha1 = alphaUp;
                alpha2 = FindHighestOK<AlphaFinder>(
                     alpha1,
                     alphaMax,
                     tolerance,
                     *this,
                     &AlphaFinder::testIfSolutionExists);
            } else {
                alpha2 = alphaDown;
                alpha1 = FindLowestOK<AlphaFinder>(
                     alphaMin,
                     alpha2,
                     tolerance,
                    *this,
                     &AlphaFinder::testIfSolutionExists);
            }
        }

        // we have now found alpha1, alpha2 such that solution exists
        // at endpoints. we now want to minimize within that interval
        bool failed;
        alpha =  Minimize<AlphaFinder>(
                                        alpha1,
                                        alpha2,
                                        tolerance,
                                        *this,
                                        &AlphaFinder::homogeneityfailure,
                                        &AlphaFinder::testIfSolutionExists,
                                        failed) ;

        finalPart(alpha,
                  stepindex,
                  ratetwohomogeneousvols,
                  computeQuadraticPart(alpha),
                  computeLinearPart(alpha),
                  constantPart_,
                  alpha,
                  a,
                  b,
                  ratetwovols);

        return true;;
    }
}
