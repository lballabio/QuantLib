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

/*! \file gsr2.hpp
    \brief Two-factor GSR model for dual-curve (rate + credit spread) pricing.

    The model describes:
      dr(t) = [θ_r(t) - a_r·x(t)] dt + σ_r(t) dW_r
      ds(t) = [θ_s(t) - a_s·y(t)] dt + σ_s(t) dW_s
      dW_r·dW_s = ρ dt

    where r(t) = φ_r(t) + x(t) is the risk-free short rate fitted to
    the rate term structure, and s(t) = φ_s(t) + y(t) is the credit
    spread fitted to the spread term structure.

    The combined discount rate is r(t) + s(t). For forecasting IBOR
    rates, only the rate factor r(t) is used.

    Each factor is a standard GSR (Gaussian Short Rate) model with
    piecewise-constant volatility and mean reversion. The cross-factor
    interaction enters through:
      1. Correlation ρ in the Brownian motions (handled by the engine
         via Cholesky decomposition in the integration step)
      2. A cross-variance correction in the combined zerobond formula

    The combined zerobond factorizes as:
      P^{r+s}(t,T|x,y) = P^r(t,T|x) · P^s(t,T|y) · exp(C(t,T))
    where C(t,T) is the cross-covariance correction (see crossVariance()).
*/

#ifndef quantlib_gsr2_hpp
#define quantlib_gsr2_hpp

#include <ql/models/shortrate/twofactormodels/gaussian2dmodel.hpp>
#include <ql/processes/gsrprocess.hpp>

namespace QuantLib {

//! Two-factor GSR model with separate rate and spread dynamics
class Gsr2 : public Gaussian2dModel, public CalibratedModel {
  public:
    //! Constant correlation
    Gsr2(const Handle<YieldTermStructure>& rateTermStructure,
         const Handle<YieldTermStructure>& spreadTermStructure,
         std::vector<Date> rateVolstepdates,
         const std::vector<Real>& rateVolatilities,
         Real rateReversion,
         std::vector<Date> spreadVolstepdates,
         const std::vector<Real>& spreadVolatilities,
         Real spreadReversion,
         Real correlation,
         Real T = 60.0);

    //! Piecewise-constant correlation term structure
    Gsr2(const Handle<YieldTermStructure>& rateTermStructure,
         const Handle<YieldTermStructure>& spreadTermStructure,
         std::vector<Date> rateVolstepdates,
         const std::vector<Real>& rateVolatilities,
         Real rateReversion,
         std::vector<Date> spreadVolstepdates,
         const std::vector<Real>& spreadVolatilities,
         Real spreadReversion,
         std::vector<Date> correlationStepdates,
         const std::vector<Real>& correlations,
         Real T = 60.0);

    //! Accessors
    Real rateReversion() const { return rateReversion_; }
    Real spreadReversion() const { return spreadReversion_; }
    const Array& rateVolatility() const { return rateSigma_.params(); }
    const Array& spreadVolatility() const { return spreadSigma_.params(); }

    //! Time-dependent correlation (piecewise constant)
    Real correlation(Time t) const override;

    //! Cross-variance correction for the combined zerobond
    /*! Returns Cov(∫_t^T x(u)du, ∫_t^T y(u)du), the cross-covariance
        that corrects the factored zerobond:
          P^{r+s}(t,T|x,y) = P^r(t,T|x) · P^s(t,T|y) · exp(C(t,T))

        Computed by integrating over piecewise-constant intervals:
          C(t,T) = ∫_t^T ρ(s)·σ_r(s)·σ_s(s)·B(a_r,T-s)·B(a_s,T-s) ds

        where B(a,τ) = (1-e^{-aτ})/a. Each interval where all
        parameters are constant has a closed-form contribution.
    */
    Real crossVariance(Time t, Time T) const;

  protected:
    Real discountZerobondImpl(Time T, Time t, Real yRate, Real ySpread,
                              const Handle<YieldTermStructure>& yts) const override;

    Real forecastZerobondImpl(Time T, Time t, Real yRate,
                              const Handle<YieldTermStructure>& yts) const override;

    Real numeraireImpl(Time t, Real yRate, Real ySpread,
                       const Handle<YieldTermStructure>& yts) const override;

    void generateArguments() override;
    void update() override;
    void performCalculations() const override;

  private:
    void initialize(Real T);
    void updateTimes() const;

    //! Single-factor GSR zerobond: P(t,T|y) using one process and one curve
    Real singleFactorZerobond(Time T, Time t, Real y,
                              const ext::shared_ptr<GsrProcess>& process,
                              const Handle<YieldTermStructure>& curve) const;

    // Rate factor parameters
    Parameter& rateSigma_;
    Real rateReversion_;
    std::vector<Date> rateVolstepdates_;
    mutable std::vector<Time> rateVolsteptimes_;
    mutable Array rateVolsteptimesArray_;

    // Spread factor parameters
    Parameter& spreadSigma_;
    Real spreadReversion_;
    std::vector<Date> spreadVolstepdates_;
    mutable std::vector<Time> spreadVolsteptimes_;
    mutable Array spreadVolsteptimesArray_;

    // Correlation term structure (piecewise constant)
    std::vector<Date> rhoStepdates_;
    std::vector<Real> rhos_;  // n+1 values for n step dates
    mutable std::vector<Time> rhoSteptimes_;

    //! Evaluate piecewise-constant correlation at time t
    Real rhoAt(Time t) const;

    //! Closed-form cross-variance contribution for an interval [s0, s1]
    //! with constant ρ, σ_r, σ_s, a_r, a_s and maturity T.
    //!   ∫_{s0}^{s1} B(a_r, T-s) · B(a_s, T-s) ds
    static Real crossVarianceInterval(Real a_r, Real a_s, Time T,
                                      Time s0, Time s1);

    // Forward measure time
    Real T_;
};

}

#endif
