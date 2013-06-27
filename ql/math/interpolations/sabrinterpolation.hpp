/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2007 Marco Bianchetti
 Copyright (C) 2007 François du Vignaud
 Copyright (C) 2007 Giorgio Facchinetti
 Copyright (C) 2006 Mario Pucci
 Copyright (C) 2006 StatPro Italia srl

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

/*! \file sabrinterpolation.hpp
    \brief SABR interpolation interpolation between discrete points
*/

#ifndef quantlib_sabr_interpolation_hpp
#define quantlib_sabr_interpolation_hpp

#include <ql/math/interpolation.hpp>
#include <ql/math/optimization/method.hpp>
#include <ql/math/optimization/simplex.hpp>
#include <ql/math/optimization/levenbergmarquardt.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/utilities/null.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/termstructures/volatility/sabr.hpp>
#include <ql/math/optimization/projectedcostfunction.hpp>
#include <ql/math/optimization/constraint.hpp>
#include <ql/math/randomnumbers/haltonrsg.hpp>

namespace QuantLib {

    namespace detail {

        class SABRCoeffHolder {
          public:
            SABRCoeffHolder(Time t,
                            const Real& forward,
                            Real alpha,
                            Real beta,
                            Real nu,
                            Real rho,
                            bool alphaIsFixed,
                            bool betaIsFixed,
                            bool nuIsFixed,
                            bool rhoIsFixed)
            : t_(t), forward_(forward),
              alpha_(alpha), beta_(beta), nu_(nu), rho_(rho),
              alphaIsFixed_(false),
              betaIsFixed_(false),
              nuIsFixed_(false),
              rhoIsFixed_(false),
              weights_(std::vector<Real>()),
              error_(Null<Real>()),
              maxError_(Null<Real>()),
              SABREndCriteria_(EndCriteria::None)
            {
                QL_REQUIRE(t>0.0, "expiry time must be positive: "
                                  << t << " not allowed");
                if (alpha_ != Null<Real>())
                    alphaIsFixed_ = alphaIsFixed;
                else alpha_ = std::sqrt(0.2);
                if (beta_ != Null<Real>())
                    betaIsFixed_ = betaIsFixed;
                else beta_ = 0.5;
                if (nu_ != Null<Real>())
                    nuIsFixed_ = nuIsFixed;
                else nu_ = std::sqrt(0.4);
                if (rho_ != Null<Real>())
                    rhoIsFixed_ = rhoIsFixed;
                else rho_ = 0.0;
                validateSabrParameters(alpha_, beta_, nu_, rho_);
            }
            virtual ~SABRCoeffHolder() {}

            /*! Option expiry */
            Real t_;
            /*! */
            const Real& forward_;
            /*! Sabr parameters */
            Real alpha_, beta_, nu_, rho_;
            bool alphaIsFixed_, betaIsFixed_, nuIsFixed_, rhoIsFixed_;
            std::vector<Real> weights_;
            /*! Sabr interpolation results */
            Real error_, maxError_;
            EndCriteria::Type SABREndCriteria_;
        };

        template <class I1, class I2>
        class SABRInterpolationImpl : public Interpolation::templateImpl<I1,I2>,
                                      public SABRCoeffHolder {
          public:
            SABRInterpolationImpl(
                const I1& xBegin, const I1& xEnd,
                const I2& yBegin,
                Time t,
                const Real& forward,
                Real alpha, Real beta, Real nu, Real rho,
                bool alphaIsFixed,
                bool betaIsFixed,
                bool nuIsFixed,
                bool rhoIsFixed,
                bool vegaWeighted,
                const boost::shared_ptr<EndCriteria>& endCriteria,
                const boost::shared_ptr<OptimizationMethod>& optMethod,
                const Real errorAccept,
                const bool useMaxError,
                const Size maxGuesses)
            : Interpolation::templateImpl<I1,I2>(xBegin, xEnd, yBegin),
              SABRCoeffHolder(t, forward, alpha, beta, nu, rho,
                              alphaIsFixed,betaIsFixed,nuIsFixed,rhoIsFixed),
              endCriteria_(endCriteria), optMethod_(optMethod),
              errorAccept_(errorAccept), useMaxError_(useMaxError),
              maxGuesses_(maxGuesses), forward_(forward),
              vegaWeighted_(vegaWeighted)
            {
                // if no optimization method or endCriteria is provided, we provide one
                if (!optMethod_)
                    optMethod_ = boost::shared_ptr<OptimizationMethod>(new
                       LevenbergMarquardt(1e-8, 1e-8, 1e-8));
                    //optMethod_ = boost::shared_ptr<OptimizationMethod>(new
                    //    Simplex(0.01));
                if (!endCriteria_) {
                    endCriteria_ = boost::shared_ptr<EndCriteria>(new
                        EndCriteria(60000, 100, 1e-8, 1e-8, 1e-8));
                }
                weights_ = std::vector<Real>(xEnd-xBegin, 1.0/(xEnd-xBegin));
            }

            void update() {
                // forward_ might have changed
                QL_REQUIRE(forward_>0.0, "at the money forward rate must be "
                           "positive: " << io::rate(forward_) << " not allowed");

                // we should also check that y contains positive values only

                // we must update weights if it is vegaWeighted
                if (vegaWeighted_) {
                    std::vector<Real>::const_iterator x = this->xBegin_;
                    std::vector<Real>::const_iterator y = this->yBegin_;
                    //std::vector<Real>::iterator w = weights_.begin();
                    weights_.clear();
                    Real weightsSum = 0.0;
                    for ( ; x!=this->xEnd_; ++x, ++y) {
                        Real stdDev = std::sqrt((*y)*(*y)*t_);
                        weights_.push_back(
                            blackFormulaStdDevDerivative(*x, forward_, stdDev));
                        weightsSum += weights_.back();
                    }
                    // weight normalization
                    std::vector<Real>::iterator w = weights_.begin();
                    for ( ; w!=weights_.end(); ++w)
                        *w /= weightsSum;
                }

                // there is nothing to optimize
                if (alphaIsFixed_ && betaIsFixed_ && nuIsFixed_ && rhoIsFixed_) {
                    error_ = interpolationError();
                    maxError_ = interpolationMaxError();
                    SABREndCriteria_ = EndCriteria::None;
                    return;

                } else {

                    SABRError costFunction(this);
                    transformation_ = boost::shared_ptr<ParametersTransformation>(new
                        SabrParametersTransformation);

                    Array guess(4);
                    guess[0] = alpha_;
                    guess[1] = beta_;
                    guess[2] = nu_;
                    guess[3] = rho_;

                    std::vector<bool> parameterAreFixed(4);
                    parameterAreFixed[0] = alphaIsFixed_;
                    parameterAreFixed[1] = betaIsFixed_;
                    parameterAreFixed[2] = nuIsFixed_;
                    parameterAreFixed[3] = rhoIsFixed_;

                    Size iterations = 0;
                    Size freeParameters = 0;
                    Real bestError = QL_MAX_REAL;
                    Array bestParameters;
                    for(Size i=0;i<4;i++) if(!parameterAreFixed[i]) freeParameters++;
                    HaltonRsg halton(freeParameters,42);
                    EndCriteria::Type tmpEndCriteria;
                    Real tmpInterpolationError;

                    do {

                        if(iterations > 0) {
                            HaltonRsg::sample_type s = halton.nextSequence();
                            Size j = 0;
                            //for(int i=0;i<4;i++) {
                            //  if(!parameterAreFixed[i]) guess[i] = tan(-M_PI/2.0 + 1E-6 + (1-1E-6) * M_PI * s.value[j++]);
                            //}
                            if(!parameterAreFixed[0]) guess[0] = (1.0-2E-6)*s.value[j++]+1E-6;
                            if(!parameterAreFixed[1]) guess[1] = (1.0-2E-6)*s.value[j++]+1E-6;
                            if(!parameterAreFixed[2]) guess[2] = 5.0*s.value[j++]+1E-6;
                            if(!parameterAreFixed[3]) guess[3] = (2.0*s.value[j++]-1.0)*(1.0-1E-6);
                            guess = transformation_->direct(guess);
                            if(alphaIsFixed_) guess[0] = alpha_;
                            if(betaIsFixed_) guess[1] = beta_;
                            if(nuIsFixed_) guess[2] = nu_;
                            if(rhoIsFixed_) guess[3] = rho_;
                        }

                        Array inversedTransformatedGuess(transformation_->inverse(guess));

                        ProjectedCostFunction constrainedSABRError(costFunction,
                                        inversedTransformatedGuess, parameterAreFixed);

                        Array projectedGuess
                            (constrainedSABRError.project(inversedTransformatedGuess));

                        NoConstraint constraint;
                        Problem problem(constrainedSABRError, constraint, projectedGuess);
                        tmpEndCriteria = optMethod_->minimize(problem, *endCriteria_);
                        Array projectedResult(problem.currentValue());
                        Array transfResult(constrainedSABRError.include(projectedResult));

                        Array result = transformation_->direct(transfResult);

                        tmpInterpolationError = useMaxError_ ? interpolationMaxError() : interpolationError();

                        if(tmpInterpolationError < bestError) {
                            bestError = tmpInterpolationError;
                            bestParameters = result;
                            SABREndCriteria_ = tmpEndCriteria;
                        }

                    } while( ++iterations < maxGuesses_ && tmpInterpolationError > errorAccept_ );

                    alpha_ = bestParameters[0];
                    beta_ = bestParameters[1];
                    nu_ = bestParameters[2];
                    rho_ = bestParameters[3];
                    error_ = interpolationError();
                    maxError_ = interpolationMaxError();

                }

            }

            Real value(Real x) const {
                QL_REQUIRE(x>0.0, "strike must be positive: " <<
                                  io::rate(x) << " not allowed");
                return sabrVolatility(x, forward_, t_,
                                      alpha_, beta_, nu_, rho_);
            }
            Real primitive(Real) const {
                QL_FAIL("SABR primitive not implemented");
            }
            Real derivative(Real) const {
                QL_FAIL("SABR derivative not implemented");
            }
            Real secondDerivative(Real) const {
                QL_FAIL("SABR secondDerivative not implemented");
            }
            // calculate total squared weighted difference (L2 norm)
            Real interpolationSquaredError() const {
                Real error, totalError = 0.0;
                std::vector<Real>::const_iterator x = this->xBegin_;
                std::vector<Real>::const_iterator y = this->yBegin_;
                std::vector<Real>::const_iterator w = weights_.begin();
                for (; x != this->xEnd_; ++x, ++y, ++w) {
                    error = (value(*x) - *y);
                    totalError += error*error * (*w);
                }
                return totalError;
            }
            // calculate weighted differences
            Disposable<Array> interpolationErrors(const Array&) const {
                Array results(this->xEnd_ - this->xBegin_);
                std::vector<Real>::const_iterator x = this->xBegin_;
                Array::iterator r = results.begin();
                std::vector<Real>::const_iterator y = this->yBegin_;
                std::vector<Real>::const_iterator w = weights_.begin();
                for (; x != this->xEnd_; ++x, ++r, ++w, ++y) {
                    *r = (value(*x) - *y)* std::sqrt(*w);
                }
                return results;
            }

            Real interpolationError() const {
                Size n = this->xEnd_-this->xBegin_;
                Real squaredError = interpolationSquaredError();
                return std::sqrt(n*squaredError/(n-1));
            }

            Real interpolationMaxError() const {
                Real error, maxError = QL_MIN_REAL;
                I1 i = this->xBegin_;
                I2 j = this->yBegin_;
                for (; i != this->xEnd_; ++i, ++j) {
                    error = std::fabs(value(*i) - *j);
                    maxError = std::max(maxError, error);
                }
                return maxError;
            }
          private:
            class SabrParametersTransformation :
                  public ParametersTransformation {
                     mutable Array y_;
                     const Real eps1_, eps2_, dilationFactor_ ;
             public:
                SabrParametersTransformation() : y_(Array(4)),
                    eps1_(.0000001),
                    eps2_(.9999),
                    dilationFactor_(0.001){
                }

                Array direct(const Array& x) const {
                    y_[0] = std::fabs(x[0])<5.0 ? x[0]*x[0] + eps1_ : 25.0;
                    //y_[1] = std::atan(dilationFactor_*x[1])/M_PI + 0.5;
                    y_[1] = std::fabs(x[1])<1000.0 ? std::exp(-(x[1]*x[1])) : eps1_;
                    y_[2] = std::fabs(x[2])<5.0 ? x[2]*x[2] + eps1_ : 25.0;
                    y_[3] = std::fabs(x[3])<10.0 ? eps2_ * std::sin(x[3]) : eps1_;
                    return y_;
                }

                Array inverse(const Array& x) const {
                    y_[0] = std::sqrt(x[0] - eps1_);
                    //y_[1] = std::tan(M_PI*(x[1] - 0.5))/dilationFactor_;
                    y_[1] = std::sqrt(-std::log(x[1]));
                    y_[2] = std::sqrt(x[2] - eps1_);
                    y_[3] = std::asin(x[3]/eps2_);

                    return y_;
                }
            };

            class SABRError : public CostFunction {
              public:
                SABRError(SABRInterpolationImpl* sabr)
                : sabr_(sabr) {}

                Real value(const Array& x) const {
                    const Array y = sabr_->transformation_->direct(x);
                    sabr_->alpha_ = y[0];
                    sabr_->beta_  = y[1];
                    sabr_->nu_    = y[2];
                    sabr_->rho_   = y[3];
                    return sabr_->interpolationSquaredError();
                }

                Disposable<Array> values(const Array& x) const{
                    const Array y = sabr_->transformation_->direct(x);
                    sabr_->alpha_ = y[0];
                    sabr_->beta_  = y[1];
                    sabr_->nu_    = y[2];
                    sabr_->rho_   = y[3];
                    return sabr_->interpolationErrors(x);
                }

              private:
                SABRInterpolationImpl* sabr_;
            };
            boost::shared_ptr<EndCriteria> endCriteria_;
            boost::shared_ptr<OptimizationMethod> optMethod_;
            const Real errorAccept_;
            const bool useMaxError_;
            const Size maxGuesses_;
            const Real& forward_;
            bool vegaWeighted_;
            boost::shared_ptr<ParametersTransformation> transformation_;
            NoConstraint constraint_;

        };

    }

    //! %SABR smile interpolation between discrete volatility points.
    class SABRInterpolation : public Interpolation {
      public:
        template <class I1, class I2>
        SABRInterpolation(const I1& xBegin,  // x = strikes
                          const I1& xEnd,
                          const I2& yBegin,  // y = volatilities
                          Time t,            // option expiry
                          const Real& forward,
                          Real alpha,
                          Real beta,
                          Real nu,
                          Real rho,
                          bool alphaIsFixed,
                          bool betaIsFixed,
                          bool nuIsFixed,
                          bool rhoIsFixed,
                          bool vegaWeighted = true,
                          const boost::shared_ptr<EndCriteria>& endCriteria
                                  = boost::shared_ptr<EndCriteria>(),
                          const boost::shared_ptr<OptimizationMethod>& optMethod
                                  = boost::shared_ptr<OptimizationMethod>(),
                          const Real errorAccept=0.0020,
                          const bool useMaxError=false,
                          const Size maxGuesses=50) {

            impl_ = boost::shared_ptr<Interpolation::Impl>(new
                detail::SABRInterpolationImpl<I1,I2>(xBegin, xEnd, yBegin,
                                                     t, forward,
                                                     alpha, beta, nu, rho,
                                                     alphaIsFixed, betaIsFixed,
                                                     nuIsFixed, rhoIsFixed,
                                                     vegaWeighted,
                                                     endCriteria,
                                                     optMethod,
                                                     errorAccept, useMaxError, maxGuesses));
            coeffs_ =
                boost::dynamic_pointer_cast<detail::SABRCoeffHolder>(
                                                                       impl_);
        }
        Real expiry()  const { return coeffs_->t_; }
        Real forward() const { return coeffs_->forward_; }
        Real alpha()   const { return coeffs_->alpha_; }
        Real beta()    const { return coeffs_->beta_; }
        Real nu()      const { return coeffs_->nu_; }
        Real rho()     const { return coeffs_->rho_; }
        Real rmsError() const { return coeffs_->error_; }
        Real maxError() const { return coeffs_->maxError_; }
        const std::vector<Real>& interpolationWeights() const {
            return coeffs_->weights_; }
        EndCriteria::Type endCriteria(){ return coeffs_->SABREndCriteria_; }

      private:
        boost::shared_ptr<detail::SABRCoeffHolder> coeffs_;
    };

    //! %SABR interpolation factory and traits
    class SABR {
      public:
        SABR(Time t, Real forward,
             Real alpha, Real beta, Real nu, Real rho,
             bool alphaIsFixed, bool betaIsFixed,
             bool nuIsFixed, bool rhoIsFixed,
             bool vegaWeighted = false,
             const boost::shared_ptr<EndCriteria> endCriteria
                 = boost::shared_ptr<EndCriteria>(),
             const boost::shared_ptr<OptimizationMethod> optMethod
                 = boost::shared_ptr<OptimizationMethod>(),
             const Real errorAccept=0.0020, const bool useMaxError=false,
             const Size maxGuesses=50)
        : t_(t), forward_(forward),
          alpha_(alpha), beta_(beta), nu_(nu), rho_(rho),
          alphaIsFixed_(alphaIsFixed), betaIsFixed_(betaIsFixed),
          nuIsFixed_(nuIsFixed), rhoIsFixed_(rhoIsFixed),
          vegaWeighted_(vegaWeighted),
          endCriteria_(endCriteria),
          optMethod_(optMethod), errorAccept_(errorAccept), useMaxError_(useMaxError), maxGuesses_(maxGuesses) {}
        template <class I1, class I2>
        Interpolation interpolate(const I1& xBegin, const I1& xEnd,
                                  const I2& yBegin) const {
            return SABRInterpolation(xBegin, xEnd, yBegin,
                                     t_,  forward_,
                                     alpha_, beta_, nu_, rho_,
                                     alphaIsFixed_, betaIsFixed_,
                                     nuIsFixed_, rhoIsFixed_,
                                     vegaWeighted_,
                                     endCriteria_, optMethod_,
                                     errorAccept_, useMaxError_, maxGuesses_);
        }
        static const bool global = true;
      private:
        Time t_;
        Real forward_;
        Real alpha_, beta_, nu_, rho_;
        bool alphaIsFixed_, betaIsFixed_, nuIsFixed_, rhoIsFixed_;
        bool vegaWeighted_;
        const boost::shared_ptr<EndCriteria> endCriteria_;
        const boost::shared_ptr<OptimizationMethod> optMethod_;
        const Real errorAccept_;
        const bool useMaxError_;
        const Size maxGuesses_;
    };

}

#endif
