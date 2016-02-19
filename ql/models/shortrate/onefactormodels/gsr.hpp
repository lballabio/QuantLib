/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013, 2015 Peter Caspers

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

/*! \file gsr.hpp
    \brief GSR 1 factor model

    \bug when the reference date changes, the state processes have
    to be reconstructed, since they take the reference date as a 
    parameter to provide the time method (otherwise it wouldn't be
    necessary since the time array is referenced from the process)

    \todo check if the class works properly if the term structure's
    date (which is the reference date the model uses) moves after
    the first step date.
*/

#ifndef quantlib_gsr_hpp
#define quantlib_gsr_hpp

#include <ql/models/shortrate/onefactormodels/gaussian1dmodel.hpp>
#include <ql/processes/gsrprocess.hpp>

namespace QuantLib {

//! One factor gsr model, formulation is in forward measure

class Gsr : public Gaussian1dModel, public CalibratedModel {

  public:
    // constant mean reversion
    Gsr(const Handle<YieldTermStructure> &termStructure,
        const std::vector<Date> &volstepdates,
        const std::vector<Real> &volatilities, const Real reversion,
        const Real T = 60.0,
        const std::vector<Real> &adjusters = std::vector<Real>());
    // piecewise mean reversion (with same step dates as volatilities)
    Gsr(const Handle<YieldTermStructure> &termStructure,
        const std::vector<Date> &volstepdates,
        const std::vector<Real> &volatilities,
        const std::vector<Real> &reversions, const Real T = 60.0,
        const std::vector<Real> &adjusters = std::vector<Real>());
    // constant mean reversion with floating model data
    Gsr(const Handle<YieldTermStructure> &termStructure,
        const std::vector<Date> &volstepdates,
        const std::vector<Handle<Quote> > &volatilities,
        const Handle<Quote> reversion, const Real T = 60.0,
        const std::vector<Handle<Quote> > &adjusters =
            std::vector<Handle<Quote> >());
    // piecewise mean reversion with floating model data
    Gsr(const Handle<YieldTermStructure> &termStructure,
        const std::vector<Date> &volstepdates,
        const std::vector<Handle<Quote> > &volatilities,
        const std::vector<Handle<Quote> > &reversions, const Real T = 60.0,
        const std::vector<Handle<Quote> > &adjusters =
            std::vector<Handle<Quote> >());

    const Real numeraireTime() const;
    const void numeraireTime(const Real T);

    const Array &reversion() const { return reversion_.params(); }
    const Array &volatility() const { return sigma_.params(); }
    const Array &adjuster() const { return adjuster_.params(); }

    // calibration constraints

    // fixed reversions and adjusters, only volatilities are free
    Disposable<std::vector<bool> > FixedReversions() {
        std::vector<bool> res(reversions_.size(), true);
        std::vector<bool> vol(volatilities_.size(), false);
        std::vector<bool> adj(adjusters_.size(), true);
        res.insert(res.end(), vol.begin(), vol.end());
        res.insert(res.end(), adj.begin(), adj.end());
        return res;
    }

    // fixed volatilities and adjusters, only reversions are free
    Disposable<std::vector<bool> > FixedVolatilities() {
        std::vector<bool> res(reversions_.size(), false);
        std::vector<bool> vol(volatilities_.size(), true);
        std::vector<bool> adj(adjusters_.size(), true);
        res.insert(res.end(), vol.begin(), vol.end());
        res.insert(res.end(), adj.begin(), adj.end());
        return res;
    }

    // fixed adjusters, volatilities and reversions are free
    Disposable<std::vector<bool> > FixedAdjusters() {
        std::vector<bool> res(reversions_.size(), false);
        std::vector<bool> vol(volatilities_.size(), false);
        std::vector<bool> adj(adjusters_.size(), true);
        res.insert(res.end(), vol.begin(), vol.end());
        res.insert(res.end(), adj.begin(), adj.end());
        return res;
    }

    Disposable<std::vector<bool> > MoveVolatility(Size i) {
        QL_REQUIRE(i < volatilities_.size(),
                   "volatility with index " << i << " does not exist (0..."
                                            << volatilities_.size() - 1 << ")");
        std::vector<bool> res(reversions_.size() + volatilities_.size() +
                                  adjusters_.size(),
                              true);
        res[reversions_.size() + i] = false;
        return res;
    }

    Disposable<std::vector<bool> > MoveReversion(Size i) {
        QL_REQUIRE(i < reversions_.size(),
                   "reversion with index " << i << " does not exist (0..."
                                           << reversions_.size() - 1 << ")");
        std::vector<bool> res(reversions_.size() + volatilities_.size() +
                                  adjusters_.size(),
                              true);
        res[i] = false;
        return res;
    }

    Disposable<std::vector<bool> > MoveAdjuster(Size i) {
        QL_REQUIRE(i < volatilities_.size(),
                   "adjuster with index " << i << " does not exist (0..."
                                          << adjusters_.size() - 1 << ")");
        std::vector<bool> res(reversions_.size() + volatilities_.size() +
                                  adjusters_.size(),
                              true);
        res[reversions_.size() + volatilities_.size() + i] = false;
        return res;
    }

    // With fixed reversion calibrate the volatilities one by one
    // to the given helpers. It is assumed that that volatility step
    // dates are suitable for this, i.e. they should be identical to
    // the fixing dates of the helpers (except for the last one where
    // we do not need a step). Also note that the endcritera reflect
    // only the status of the last calibration when using this method.
    void calibrateVolatilitiesIterative(
        const std::vector<boost::shared_ptr<CalibrationHelper> > &helpers,
        OptimizationMethod &method, const EndCriteria &endCriteria,
        const Constraint &constraint = Constraint(),
        const std::vector<Real> &weights = std::vector<Real>()) {

        for (Size i = 0; i < helpers.size(); i++) {
            std::vector<boost::shared_ptr<CalibrationHelper> > h(1, helpers[i]);
            calibrate(h, method, endCriteria, constraint, weights,
                      MoveVolatility(i));
        }
    }

    // With fixed volatility calibrate the reversions one by one
    // to the given helpers. In this case the step dates must be chosen
    // according to the maturities of the calibration instruments.
    void calibrateReversionsIterative(
        const std::vector<boost::shared_ptr<CalibrationHelper> > &helpers,
        OptimizationMethod &method, const EndCriteria &endCriteria,
        const Constraint &constraint = Constraint(),
        const std::vector<Real> &weights = std::vector<Real>()) {

        for (Size i = 0; i < helpers.size(); i++) {
            std::vector<boost::shared_ptr<CalibrationHelper> > h(1, helpers[i]);
            calibrate(h, method, endCriteria, constraint, weights,
                      MoveReversion(i));
        }
    }

    // Calibrate the adjsuters one by one to the given helpers.
    // As for the iterative volatility calibration the step
    // dates should be chosen according to the fixing dates
    // of the coupon to adjust the model to.
    void calibrateAdjustersIterative(
        const std::vector<boost::shared_ptr<CalibrationHelperBase> > &helpers,
        OptimizationMethod &method, const EndCriteria &endCriteria,
        const Constraint &constraint = Constraint(),
        const std::vector<Real> &weights = std::vector<Real>()) {

        for (Size i = 0; i < helpers.size(); i++) {
            std::vector<boost::shared_ptr<CalibrationHelperBase> > h(
                1, helpers[i]);
            calibrate(h, method, endCriteria, constraint, weights,
                      MoveAdjuster(i));
        }
    }

  protected:
    const Real numeraireImpl(const Time t, const Real y,
                             const Handle<YieldTermStructure> &yts) const;

    const Real zerobondImpl(const Time T, const Time t, const Real y,
                            const Handle<YieldTermStructure> &yts,
                            const bool adjusted) const;

    void generateArguments() {
        boost::static_pointer_cast<GsrProcess>(stateProcess_)->flushCache();
        boost::static_pointer_cast<GsrProcess>(adjustedStateProcess_)
            ->flushCache();
        notifyObservers();
    }

    void update() { LazyObject::update(); }

    void performCalculations() const {
        Gaussian1dModel::performCalculations();
        updateTimes();
    }

  private:
    void updateTimes() const;
    void updateVolatility();
    void updateReversion();
    void updateAdjuster();

    void initialize(Real);

    Parameter &reversion_, &sigma_, &adjuster_;
    Parameter unitAdjuster_;

    std::vector<Handle<Quote> > volatilities_;
    std::vector<Handle<Quote> > reversions_;
    std::vector<Handle<Quote> > adjusters_;
    std::vector<Date> volstepdates_; // this is shared between vols,
                                     // adjusters and reverisons in
                                     // case of piecewise reversions
    mutable std::vector<Time> volsteptimes_;
    mutable Array volsteptimesArray_; // FIXME this is redundant (just a copy of
                                      // volsteptimes_)
    boost::shared_ptr<StochasticProcess1D> adjustedStateProcess_;

    struct VolatilityObserver : public Observer {
        VolatilityObserver(Gsr *p) : p_(p) {}
        void update() { p_->updateVolatility(); }
        Gsr *p_;
    };
    struct ReversionObserver : public Observer {
        ReversionObserver(Gsr *p) : p_(p) {}
        void update() { p_->updateReversion(); }
        Gsr *p_;
    };
    struct AdjusterObserver : public Observer {
        AdjusterObserver(Gsr *p) : p_(p) {}
        void update() { p_->updateAdjuster(); }
        Gsr *p_;
    };

    boost::shared_ptr<VolatilityObserver> volatilityObserver_;
    boost::shared_ptr<ReversionObserver> reversionObserver_;
    boost::shared_ptr<AdjusterObserver> adjusterObserver_;
};

inline const Real Gsr::numeraireTime() const {
    return boost::dynamic_pointer_cast<GsrProcess>(stateProcess_)
        ->getForwardMeasureTime();
}

inline const void Gsr::numeraireTime(const Real T) {
    boost::dynamic_pointer_cast<GsrProcess>(stateProcess_)
        ->setForwardMeasureTime(T);
}
}

#endif
