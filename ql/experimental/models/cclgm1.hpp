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

/*! \file cclgm1.hpp
    \brief multicurrency lgm model with piecewise parameters
*/

#ifndef quantlib_multicurrency_lgm1_hpp
#define quantlib_multicurrency_lgm1_hpp

#include <ql/experimental/models/cclgm.hpp>
#include <ql/experimental/models/cclgmpiecewise.hpp>
#include <ql/experimental/models/lgmpiecewisealphaconstantkappa.hpp>
#include <ql/experimental/models/lgmfxpiecewisesigma.hpp>

namespace QuantLib {

class CcLgm1 : public CcLgm<detail::CcLgmPiecewise, detail::LgmFxPiecewiseSigma,
                            detail::LgmPiecewiseAlphaConstantKappa>,
               public CalibratedModel {
  public:
    typedef detail::CcLgmPiecewise cclgm_model_type;
    typedef detail::LgmPiecewiseAlphaConstantKappa lgm_model_type;
    typedef detail::LgmFxPiecewiseSigma lgmfx_model_type;
    typedef CcLgm<detail::CcLgmPiecewise, detail::LgmFxPiecewiseSigma,
                  detail::LgmPiecewiseAlphaConstantKappa> model_type;
    typedef CcLgmProcess<detail::CcLgmPiecewise, detail::LgmFxPiecewiseSigma,
                         detail::LgmPiecewiseAlphaConstantKappa> process_type;

    /*! fx spots are interpreted as spots as of today (or discounted spots)
        note that they are assumed to be given as log spot values throughout
        this model; we could allow for different fx vol step dates for each
        currency without any difficulty, only the constructor here would have
        to be extended. */

    CcLgm1(const std::vector<boost::shared_ptr<
               Lgm<detail::LgmPiecewiseAlphaConstantKappa> > > &models,
           const std::vector<Handle<Quote> > &fxSpots,
           const std::vector<Date> &fxVolStepDates,
           const std::vector<std::vector<Real> > &fxVolatilities,
           const Matrix &correlation,
           const std::vector<Handle<YieldTermStructure> > &curves);

    const Array &fxVolatility(Size i) {
        QL_REQUIRE(i < n(), "index (" << i << ") out of bounds (0..." << n() - 1
                                      << ")");
        return arguments_[i].params();
    }

    // calibration constraints
    Disposable<std::vector<bool> > MoveFxVolatility(Size i, Size step) {
        QL_REQUIRE(i < n(), "fx index (" << i << ") out of range (0..."
                                         << n() - 1 << ")");
        QL_REQUIRE(step <= fxVolStepTimes_.size(),
                   "fx volatility step (" << step << ") out of range (0..."
                                          << fxVolStepTimes_.size() << ")");
        std::vector<bool> res(n() * (fxVolStepTimes_.size() + 1), true);
        res[(fxVolStepTimes_.size() + 1) * i + step] = false;
        return res;
    }

    // calibrate the stepwise fx volatilities dom - currency(i)
    void calibrateFxVolatilitiesIterative(
        Size i,
        const std::vector<boost::shared_ptr<CalibrationHelper> > &helpers,
        OptimizationMethod &method, const EndCriteria &endCriteria,
        const Constraint &constraint = Constraint(),
        const std::vector<Real> &weights = std::vector<Real>()) {
        for (Size j = 0; j < helpers.size(); ++j) {
            std::vector<boost::shared_ptr<CalibrationHelper> > h(1, helpers[j]);
            calibrate(h, method, endCriteria, constraint, weights,
                      MoveFxVolatility(i, j));
        }
    }

  protected:
    void generateArguments() {
        CcLgm<detail::CcLgmPiecewise, detail::LgmFxPiecewiseSigma,
              detail::LgmPiecewiseAlphaConstantKappa>::update();
    }

  private:
    void updateTimes() const;
    void initialize();

    void update() {
        updateTimes();
        CcLgm<detail::CcLgmPiecewise, detail::LgmFxPiecewiseSigma,
              detail::LgmPiecewiseAlphaConstantKappa>::update();
    }

    const std::vector<Handle<Quote> > fxSpots_;
    const std::vector<Date> fxVolStepDates_;
    const std::vector<std::vector<Real> > fxVolatilities_;
    const Matrix correlation_;
    std::vector<Handle<YieldTermStructure> > curves_;

    mutable std::vector<Time> fxVolStepTimes_;
    mutable Array fxVolStepTimesArray_;
};

} // namespace QuantLib

#endif
