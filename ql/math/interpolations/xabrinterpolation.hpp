/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2007 Marco Bianchetti
 Copyright (C) 2007 François du Vignaud
 Copyright (C) 2007 Giorgio Facchinetti
 Copyright (C) 2006 Mario Pucci
 Copyright (C) 2006 StatPro Italia srl
 Copyright (C) 2014 Peter Caspers

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

/*! \file xabrinterpolation.hpp
    \brief generic interpolation class for sabr style underlying models
           like the Hagan 2002 expansion, Doust's no arbitrage sabr,
           Andreasen's zabr expansion for the masses and similar
*/

#ifndef ql_xabr_interpolation_hpp
#define ql_xabr_interpolation_hpp

#include <ql/math/interpolation.hpp>
#include <ql/math/optimization/constraint.hpp>
#include <ql/math/optimization/levenbergmarquardt.hpp>
#include <ql/math/optimization/method.hpp>
#include <ql/math/optimization/projectedcostfunction.hpp>
#include <ql/math/optimization/simplex.hpp>
#include <ql/math/randomnumbers/haltonrsg.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/termstructures/volatility/volatilitytype.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/utilities/null.hpp>
#include <utility>

namespace QuantLib {

namespace detail {

template <typename Model> class XABRCoeffHolder {
  public:
    XABRCoeffHolder(const Time t,
                    const Real& forward,
                    const std::vector<Real>& params,
                    const std::vector<bool>& paramIsFixed,
                    std::vector<Real> addParams)
    : t_(t), forward_(forward), params_(params), paramIsFixed_(paramIsFixed.size(), false),
      weights_(std::vector<Real>()), error_(Null<Real>()), maxError_(Null<Real>()),
      XABREndCriteria_(EndCriteria::None), addParams_(std::move(addParams)) {
        QL_REQUIRE(t > 0.0, "expiry time must be positive: " << t
                                                             << " not allowed");
        QL_REQUIRE(params.size() == Model().dimension(),
                   "wrong number of parameters (" << params.size()
                                                  << "), should be "
                                                  << Model().dimension());
        QL_REQUIRE(paramIsFixed.size() == Model().dimension(),
                   "wrong number of fixed parameters flags ("
                       << paramIsFixed.size() << "), should be "
                       << Model().dimension());

        for (Size i = 0; i < params.size(); ++i) {
            if (params[i] != Null<Real>())
                paramIsFixed_[i] = paramIsFixed[i];
        }
        Model().defaultValues(params_, paramIsFixed_, forward_, t_, addParams_);
        updateModelInstance();
    }
    virtual ~XABRCoeffHolder() = default;

    void updateModelInstance() {
        modelInstance_ = Model().instance(t_, forward_, params_, addParams_);
    }

    /*! Expiry, Forward */
    Real t_;
    const Real &forward_;
    /*! Parameters */
    std::vector<Real> params_;
    std::vector<bool> paramIsFixed_;
    std::vector<Real> weights_;
    /*! Interpolation results */
    Real error_, maxError_;
    EndCriteria::Type XABREndCriteria_;
    /*! Model instance (if required) */
    ext::shared_ptr<typename Model::type> modelInstance_;
    /*! additional parameters */
    std::vector<Real> addParams_;
};

template <class I1, class I2, typename Model>
class XABRInterpolationImpl : public Interpolation::templateImpl<I1, I2>,
                              public XABRCoeffHolder<Model> {
  public:
    XABRInterpolationImpl(const I1& xBegin,
                          const I1& xEnd,
                          const I2& yBegin,
                          Time t,
                          const Real& forward,
                          const std::vector<Real>& params,
                          const std::vector<bool>& paramIsFixed,
                          bool vegaWeighted,
                          ext::shared_ptr<EndCriteria> endCriteria,
                          ext::shared_ptr<OptimizationMethod> optMethod,
                          const Real errorAccept,
                          const bool useMaxError,
                          const Size maxGuesses,
                          const std::vector<Real>& addParams = std::vector<Real>(),
                          VolatilityType volatilityType = VolatilityType::ShiftedLognormal)
    : Interpolation::templateImpl<I1, I2>(xBegin, xEnd, yBegin, 1),
      XABRCoeffHolder<Model>(t, forward, params, paramIsFixed, addParams),
      endCriteria_(std::move(endCriteria)), optMethod_(std::move(optMethod)),
      errorAccept_(errorAccept), useMaxError_(useMaxError), maxGuesses_(maxGuesses),
      vegaWeighted_(vegaWeighted), volatilityType_(volatilityType) {
        // if no optimization method or endCriteria is provided, we provide one
        if (!optMethod_)
            optMethod_ = ext::shared_ptr<OptimizationMethod>(
                new LevenbergMarquardt(1e-8, 1e-8, 1e-8));
        // optMethod_ = ext::shared_ptr<OptimizationMethod>(new
        //    Simplex(0.01));
        if (!endCriteria_) {
            endCriteria_ = ext::make_shared<EndCriteria>(
                60000, 100, 1e-8, 1e-8, 1e-8);
        }
        this->weights_ =
            std::vector<Real>(xEnd - xBegin, 1.0 / (xEnd - xBegin));
    }

    void update() override {

        this->updateModelInstance();

        // we should also check that y contains positive values only

        // we must update weights if it is vegaWeighted
        if (vegaWeighted_) {
            I1 x = this->xBegin_;
            I2 y = this->yBegin_;
            // std::vector<Real>::iterator w = weights_.begin();
            this->weights_.clear();
            Real weightsSum = 0.0;
            for (; x != this->xEnd_; ++x, ++y) {
                Real stdDev = std::sqrt((*y) * (*y) * this->t_);
                this->weights_.push_back(Model().weight(*x, this->forward_, stdDev,
                                                        this->addParams_));
                weightsSum += this->weights_.back();
            }
            // weight normalization
            auto w = this->weights_.begin();
            for (; w != this->weights_.end(); ++w)
                *w /= weightsSum;
        }

        // there is nothing to optimize
        if (std::accumulate(this->paramIsFixed_.begin(),
                            this->paramIsFixed_.end(), true,
                            std::logical_and<bool>())) {
            this->error_ = interpolationError();
            this->maxError_ = interpolationMaxError();
            this->XABREndCriteria_ = EndCriteria::None;
            return;
        } else {
            XABRError costFunction(this);

            Array guess(Model().dimension());
            for (Size i = 0; i < guess.size(); ++i)
                guess[i] = this->params_[i];

            Size iterations = 0;
            Size freeParameters = 0;
            Real bestError = QL_MAX_REAL;
            Array bestParameters;
            for (Size i = 0; i < Model().dimension(); ++i)
                if (!this->paramIsFixed_[i])
                    ++freeParameters;
            HaltonRsg halton(freeParameters, 42);
            EndCriteria::Type tmpEndCriteria;
            Real tmpInterpolationError;

            do {

                if (iterations > 0) {
                    HaltonRsg::sample_type s = halton.nextSequence();
                    Model().guess(guess, this->paramIsFixed_, this->forward_,
                                  this->t_, s.value, this->addParams_);
                    for (Size i = 0; i < this->paramIsFixed_.size(); ++i)
                        if (this->paramIsFixed_[i])
                            guess[i] = this->params_[i];
                }

                Array inversedTransformatedGuess(Model().inverse(
                    guess, this->paramIsFixed_, this->params_, this->forward_));

                ProjectedCostFunction constrainedXABRError(
                    costFunction, inversedTransformatedGuess,
                    this->paramIsFixed_);

                Array projectedGuess(
                    constrainedXABRError.project(inversedTransformatedGuess));

                NoConstraint constraint;
                Problem problem(constrainedXABRError, constraint,
                                projectedGuess);
                tmpEndCriteria = optMethod_->minimize(problem, *endCriteria_);
                Array projectedResult(problem.currentValue());
                Array transfResult(
                    constrainedXABRError.include(projectedResult));

                Array result = Model().direct(transfResult, this->paramIsFixed_,
                                              this->params_, this->forward_);
                tmpInterpolationError = useMaxError_ ? interpolationMaxError()
                                                     : interpolationError();

                if (tmpInterpolationError < bestError) {
                    bestError = tmpInterpolationError;
                    bestParameters = result;
                    this->XABREndCriteria_ = tmpEndCriteria;
                }

            } while (++iterations < maxGuesses_ &&
                     tmpInterpolationError > errorAccept_);

            for (Size i = 0; i < bestParameters.size(); ++i)
                this->params_[i] = bestParameters[i];

            this->error_ = interpolationError();
            this->maxError_ = interpolationMaxError();
        }
    }

    Real value(Real x) const override { return this->modelInstance_->volatility(x, volatilityType_); }

    Real primitive(Real) const override { QL_FAIL("XABR primitive not implemented"); }
    Real derivative(Real) const override { QL_FAIL("XABR derivative not implemented"); }
    Real secondDerivative(Real) const override { QL_FAIL("XABR secondDerivative not implemented"); }

    // calculate total squared weighted difference (L2 norm)
    Real interpolationSquaredError() const {
        Real error, totalError = 0.0;
        I1 x = this->xBegin_;
        I2 y = this->yBegin_;
        auto w = this->weights_.begin();
        for (; x != this->xEnd_; ++x, ++y, ++w) {
            error = (value(*x) - *y);
            totalError += error * error * (*w);
        }
        return totalError;
    }

    // calculate weighted differences
    Disposable<Array> interpolationErrors() const {
        Array results(this->xEnd_ - this->xBegin_);
        I1 x = this->xBegin_;
        Array::iterator r = results.begin();
        I2 y = this->yBegin_;
        auto w = this->weights_.begin();
        for (; x != this->xEnd_; ++x, ++r, ++w, ++y) {
            *r = (value(*x) - *y) * std::sqrt(*w);
        }
        return results;
    }

    Real interpolationError() const {
        Size n = this->xEnd_ - this->xBegin_;
        Real squaredError = interpolationSquaredError();
        return std::sqrt(n * squaredError / (n==1 ? 1 : (n - 1)));
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
    class XABRError : public CostFunction {
      public:
        explicit XABRError(XABRInterpolationImpl *xabr) : xabr_(xabr) {}

        Real value(const Array& x) const override {
            const Array y = Model().direct(x, xabr_->paramIsFixed_,
                                           xabr_->params_, xabr_->forward_);
            for (Size i = 0; i < xabr_->params_.size(); ++i)
                xabr_->params_[i] = y[i];
            xabr_->updateModelInstance();
            return xabr_->interpolationSquaredError();
        }

        Disposable<Array> values(const Array& x) const override {
            const Array y = Model().direct(x, xabr_->paramIsFixed_,
                                           xabr_->params_, xabr_->forward_);
            for (Size i = 0; i < xabr_->params_.size(); ++i)
                xabr_->params_[i] = y[i];
            xabr_->updateModelInstance();
            return xabr_->interpolationErrors();
        }

      private:
        XABRInterpolationImpl *xabr_;
    };
    ext::shared_ptr<EndCriteria> endCriteria_;
    ext::shared_ptr<OptimizationMethod> optMethod_;
    const Real errorAccept_;
    const bool useMaxError_;
    const Size maxGuesses_;
    bool vegaWeighted_;
    NoConstraint constraint_;
    VolatilityType volatilityType_;
};

} // namespace detail
} // namespace QuantLib

#endif


#ifndef id_e842617c46115ebbf0cd18f968b22562
#define id_e842617c46115ebbf0cd18f968b22562
inline bool test_e842617c46115ebbf0cd18f968b22562(int* i) { return i != 0; }
#endif
