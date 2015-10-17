/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Peter Caspers

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

/*! \file lgm1.hpp
    \brief LGM model with piecewise alpha and constant kappa
*/

#ifndef quantlib_lgm1_hpp
#define quantlib_lgm1_hpp

#include <ql/experimental/models/lgm.hpp>
#include <ql/experimental/models/lgmpiecewisealphaconstantkappa.hpp>
#include <ql/experimental/models/lgmstateprocess.hpp>
#include <ql/models/model.hpp>

namespace QuantLib {

class Lgm1 : public Lgm<detail::LgmPiecewiseAlphaConstantKappa>,
             public CalibratedModel {
  public:
    typedef Lgm<detail::LgmPiecewiseAlphaConstantKappa> model_type;
    typedef LgmStateProcess<detail::LgmPiecewiseAlphaConstantKappa>
        process_type;
    Lgm1(const Handle<YieldTermStructure> &yts,
         const std::vector<Date> &volstepdates, const std::vector<Real> &alphas,
         const Real &kappa);

    // floating model data
    Lgm1(const Handle<YieldTermStructure> &yts,
         const std::vector<Date> &volstepdates,
         const std::vector<Handle<Quote> > &alphas, const Handle<Quote> &kappa);

    const Array &alpha() const { return alpha_.params(); };
    const Real kappa() const { return kappa_.params()[0]; };

    // calibration constraints

    // fixed reversion
    Disposable<std::vector<bool> > FixedReversion() {
        std::vector<bool> res(alpha_.size() + 1, false);
        res.back() = true;
        return res;
    }

    // move volatility #i
    Disposable<std::vector<bool> > MoveAlpha(Size i) {
        QL_REQUIRE(i < alpha_.size(), "alpha with index "
                                          << i << " does not exist (0..."
                                          << alpha_.size() - 1 << ")");
        std::vector<bool> res(alpha_.size() + 1, true);
        res[i] = false;
        return res;
    }

    // similar to gsr model, see the documentation there
    void calibrateAlphasIterative(
        const std::vector<boost::shared_ptr<CalibrationHelper> > &helpers,
        OptimizationMethod &method, const EndCriteria &endCriteria,
        const Constraint &constraint = Constraint(),
        const std::vector<Real> &weights = std::vector<Real>()) {

        for (Size i = 0; i < helpers.size(); i++) {
            std::vector<boost::shared_ptr<CalibrationHelper> > h(1, helpers[i]);
            calibrate(h, method, endCriteria, constraint, weights,
                      MoveAlpha(i));
        }
    }

  protected:
    void generateArguments() {
        Lgm<detail::LgmPiecewiseAlphaConstantKappa>::generateArguments();
    }
    void update() { LazyObject::update(); }
    void performCalculations() const {
        Lgm::performCalculations();
        updateTimes();
        parametrization()->update();
    }

  private:
    void updateTimes() const;
    void updateAlpha();
    void updateKappa();
    void initialize();

    const std::vector<Date> volstepdates_;
    mutable std::vector<Real>
        volsteptimes_;                // used for parameters, parametrization_
    mutable Array volsteptimesArray_; // used for state process

    std::vector<Handle<Quote> > alphaQuotes_; // floating model data
    Handle<Quote> kappaQuote_;

    Parameter &alpha_, &kappa_; // calibrated model parameters

    struct AlphaObserver : public Observer {
        AlphaObserver(Lgm1 *p) : p_(p) {}
        void update() { p_->updateAlpha(); }
        Lgm1 *p_;
    };

    struct KappaObserver : public Observer {
        KappaObserver(Lgm1 *p) : p_(p) {}
        void update() { p_->updateKappa(); }
        Lgm1 *p_;
    };

    boost::shared_ptr<AlphaObserver> alphaObserver_;
    boost::shared_ptr<KappaObserver> kappaObserver_;
};

} // namespace QuantLib

#endif
