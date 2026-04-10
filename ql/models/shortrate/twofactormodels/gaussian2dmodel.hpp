/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file gaussian2dmodel.hpp
    \brief Two-factor Gaussian interest rate model interface for
           dual-curve (rate + credit spread) pricing.

    The model describes two correlated Gaussian factors:
      - Factor 1 ("rate"): drives the risk-free short rate
      - Factor 2 ("spread"): drives the credit/funding spread

    State variables y_r and y_s are standardized (zero mean, unit
    variance) versions of the model's original state variables.

    The key extension over Gaussian1dModel is the separation of
    discounting (which uses r+s) from forecasting (which uses r only).
*/

#ifndef quantlib_gaussian2dmodel_hpp
#define quantlib_gaussian2dmodel_hpp

#include <ql/indexes/iborindex.hpp>
#include <ql/models/model.hpp>
#include <ql/patterns/lazyobject.hpp>
#include <ql/stochasticprocess.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/utilities/null.hpp>

namespace QuantLib {

/*! Two-factor Gaussian interest rate model interface.

    Factor 1 = risk-free rate, Factor 2 = credit spread.
    Total discount rate = r(t) + s(t).

    Subclasses must implement:
      - discountZerobondImpl: zero-coupon bond using both factors (for discounting)
      - forecastZerobondImpl: zero-coupon bond using rate factor only (for IBOR forecasting)
      - numeraireImpl: T-forward numeraire using the combined r+s curve

    \warning The variance of each state process conditional on its
    current value must be independent of that value (Gaussian property).
*/
class Gaussian2dModel : public TermStructureConsistentModel, public LazyObject {
  public:
    //! Rate-process state process (factor 1)
    const ext::shared_ptr<StochasticProcess1D>& rateProcess() const;

    //! Spread-process state process (factor 2)
    const ext::shared_ptr<StochasticProcess1D>& spreadProcess() const;

    //! Correlation between rate and spread Brownian motions at time t
    virtual Real correlation(Time t) const { return rho_; }

    //! Convenience: correlation at t=0
    Real correlation() const { return correlation(0.0); }

    // --- Discount zerobond: uses BOTH factors (rate + spread) ---

    Real discountZerobond(Time T, Time t = 0.0, Real yRate = 0.0, Real ySpread = 0.0,
                          const Handle<YieldTermStructure>& yts =
                              Handle<YieldTermStructure>()) const;

    Real discountZerobond(const Date& maturity, const Date& referenceDate = Date(),
                          Real yRate = 0.0, Real ySpread = 0.0,
                          const Handle<YieldTermStructure>& yts =
                              Handle<YieldTermStructure>()) const;

    // --- Forecast zerobond: uses ONLY rate factor ---

    Real forecastZerobond(Time T, Time t = 0.0, Real yRate = 0.0,
                          const Handle<YieldTermStructure>& yts =
                              Handle<YieldTermStructure>()) const;

    Real forecastZerobond(const Date& maturity, const Date& referenceDate = Date(),
                          Real yRate = 0.0,
                          const Handle<YieldTermStructure>& yts =
                              Handle<YieldTermStructure>()) const;

    // --- Numeraire: T-forward bond under combined r+s measure ---

    Real numeraire(Time t, Real yRate = 0.0, Real ySpread = 0.0,
                   const Handle<YieldTermStructure>& yts =
                       Handle<YieldTermStructure>()) const;

    Real numeraire(const Date& referenceDate, Real yRate = 0.0, Real ySpread = 0.0,
                   const Handle<YieldTermStructure>& yts =
                       Handle<YieldTermStructure>()) const;

    // --- Forward rate: uses ONLY rate factor ---

    Real forwardRate(const Date& fixing, const Date& referenceDate = Date(),
                     Real yRate = 0.0,
                     const ext::shared_ptr<IborIndex>& iborIdx =
                         ext::shared_ptr<IborIndex>()) const;

    // --- Grid generation (same as Gaussian1dModel) ---

    /*! Generates a standardized state grid at time T conditional on y(t)=y,
        covering yStdDevs standard deviations with 2*gridPoints+1 points.
        Used for both rate and spread dimensions independently. */
    Array yGrid(const ext::shared_ptr<StochasticProcess1D>& process,
                Real yStdDevs, int gridPoints,
                Real T = 1.0, Real t = 0.0, Real y = 0.0) const;

    /*! Convenience: standardized grid (T=1, t=0, y=0) */
    Array yGrid(Real yStdDevs, int gridPoints) const;

    // --- Gaussian polynomial integration (reuse from Gaussian1dModel) ---

    static Real gaussianPolynomialIntegral(Real a, Real b, Real c, Real d, Real e,
                                           Real x0, Real x1);

    static Real gaussianShiftedPolynomialIntegral(Real a, Real b, Real c, Real d, Real e,
                                                  Real h, Real x0, Real x1);

  protected:
    Gaussian2dModel(const Handle<YieldTermStructure>& rateTermStructure,
                    const Handle<YieldTermStructure>& spreadTermStructure,
                    Real correlation)
        : TermStructureConsistentModel(rateTermStructure),
          spreadTermStructure_(spreadTermStructure), rho_(correlation) {
        registerWith(Settings::instance().evaluationDate());
        if (!spreadTermStructure_.empty())
            registerWith(spreadTermStructure_);
    }

    // --- Pure virtual interface for subclasses ---

    virtual Real discountZerobondImpl(Time T, Time t, Real yRate, Real ySpread,
                                      const Handle<YieldTermStructure>& yts) const = 0;

    virtual Real forecastZerobondImpl(Time T, Time t, Real yRate,
                                      const Handle<YieldTermStructure>& yts) const = 0;

    virtual Real numeraireImpl(Time t, Real yRate, Real ySpread,
                               const Handle<YieldTermStructure>& yts) const = 0;

    void performCalculations() const override {
        evaluationDate_ = Settings::instance().evaluationDate();
        enforcesTodaysHistoricFixings_ =
            Settings::instance().enforcesTodaysHistoricFixings();
    }

    void generateArguments() {
        calculate();
        notifyObservers();
    }

    ext::shared_ptr<StochasticProcess1D> rateStateProcess_;
    ext::shared_ptr<StochasticProcess1D> spreadStateProcess_;
    Handle<YieldTermStructure> spreadTermStructure_;
    Real rho_;
    mutable Date evaluationDate_;
    mutable bool enforcesTodaysHistoricFixings_;
};


// --- Inline implementations ---

inline const ext::shared_ptr<StochasticProcess1D>& Gaussian2dModel::rateProcess() const {
    QL_REQUIRE(rateStateProcess_ != nullptr, "rate state process not set");
    return rateStateProcess_;
}

inline const ext::shared_ptr<StochasticProcess1D>& Gaussian2dModel::spreadProcess() const {
    QL_REQUIRE(spreadStateProcess_ != nullptr, "spread state process not set");
    return spreadStateProcess_;
}

inline Real Gaussian2dModel::discountZerobond(Time T, Time t, Real yRate, Real ySpread,
                                               const Handle<YieldTermStructure>& yts) const {
    return discountZerobondImpl(T, t, yRate, ySpread, yts);
}

inline Real Gaussian2dModel::discountZerobond(const Date& maturity, const Date& referenceDate,
                                               Real yRate, Real ySpread,
                                               const Handle<YieldTermStructure>& yts) const {
    return discountZerobond(
        termStructure()->timeFromReference(maturity),
        referenceDate != Date() ? termStructure()->timeFromReference(referenceDate) : 0.0,
        yRate, ySpread, yts);
}

inline Real Gaussian2dModel::forecastZerobond(Time T, Time t, Real yRate,
                                               const Handle<YieldTermStructure>& yts) const {
    return forecastZerobondImpl(T, t, yRate, yts);
}

inline Real Gaussian2dModel::forecastZerobond(const Date& maturity, const Date& referenceDate,
                                               Real yRate,
                                               const Handle<YieldTermStructure>& yts) const {
    return forecastZerobond(
        termStructure()->timeFromReference(maturity),
        referenceDate != Date() ? termStructure()->timeFromReference(referenceDate) : 0.0,
        yRate, yts);
}

inline Real Gaussian2dModel::numeraire(Time t, Real yRate, Real ySpread,
                                        const Handle<YieldTermStructure>& yts) const {
    return numeraireImpl(t, yRate, ySpread, yts);
}

inline Real Gaussian2dModel::numeraire(const Date& referenceDate, Real yRate, Real ySpread,
                                        const Handle<YieldTermStructure>& yts) const {
    return numeraire(termStructure()->timeFromReference(referenceDate), yRate, ySpread, yts);
}

}

#endif
