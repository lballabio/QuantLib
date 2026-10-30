/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Raghav Subbarao

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

/*! \file slvleveragecalibrator.hpp
    \brief Abstract interface and Heston implementation for SLV leverage function calibration
*/

#ifndef quantlib_slv_leverage_calibrator_hpp
#define quantlib_slv_leverage_calibrator_hpp

#include <ql/handle.hpp>
#include <ql/models/equity/hestonmodel.hpp>
#include <ql/models/equity/hestonslvfdmmodel.hpp>
#include <ql/models/model.hpp>
#include <ql/shared_ptr.hpp>
#include <ql/termstructures/volatility/equityfx/localvoltermstructure.hpp>
#include <ql/time/date.hpp>
#include <ql/types.hpp>
#include <vector>

namespace QuantLib {

    //! Abstract base class for SLV leverage function calibrators.
    /*!
        In a Stochastic Local Volatility (SLV) model the asset price satisfies

        \f[
            dS_t = (r_d - r_f) S_t\,dt
                   + L(t,S_t)\,\sqrt{v_t}\,S_t\,dW_t^S
        \f]

        where \f$v_t\f$ is the instantaneous variance driven by a separate
        stochastic process (e.g. CIR in the Heston model).  The leverage
        function \f$L(t,S)\f$ is chosen so that the marginal distribution of
        \f$S_t\f$ matches the market-implied distribution, i.e.

        \f[
            L(t,S)^2 = \frac{\sigma_{\mathrm{loc}}(t,S)^2}
                            {\mathbb{E}[v_t \mid S_t = S]}
        \f]

        Concrete subclasses implement this calibration for a specific
        stochastic volatility model (Heston, Bates, rough vol, etc.).

        \ingroup experimental
    */
    class SLVLeverageCalibrator {
      public:
        virtual ~SLVLeverageCalibrator() = default;

        //! Inject the calibrated stochastic volatility model.
        /*!
            Called by \c FxSLVPricingContext after the stochastic vol calibration
            step, before \c calibrate() is invoked.  Concrete subclasses that need
            a specific model type (e.g. Heston) should override this and
            dynamic-cast the argument.  The default implementation is a no-op for
            subclasses that receive their model at construction time.
        */
        virtual void setStochVolModel(const ext::shared_ptr<CalibratedModel>&) {}

        //! Calibrate the leverage function.
        /*!
            \param localVol    Dupire local volatility surface consistent with
                               the market implied vol surface
            \param endDate     latest date for which the leverage function is
                               required (typically the last option expiry)
            \param mandatoryDates additional time points to include in the
                               PDE/MC time grid (e.g. major event dates)
        */
        virtual void calibrate(
            const Handle<LocalVolTermStructure>& localVol,
            const Date& endDate,
            const std::vector<Date>& mandatoryDates = {}) = 0;

        //! Returns the calibrated leverage function \f$L(t,S)\f$.
        /*! Throws if calibrate() has not yet been called. */
        virtual ext::shared_ptr<LocalVolTermStructure> leverageFunction() const = 0;
    };


    //! SLV leverage function calibration using the Heston model and a Fokker-Planck PDE.
    /*!
        Solves the 2-D Fokker-Planck forward equation for the joint density
        \f$p(t,S,v)\f$ of the Heston SLV process, then extracts the leverage
        function slice-by-slice in time via

        \f[
            L(t,S) = \frac{\sigma_{\mathrm{loc}}(t,S)}
                          {\bigl(\int_0^\infty v\,p(t,S,v)\,dv \big/
                                 \int_0^\infty p(t,S,v)\,dv\bigr)^{1/2}}
        \f]

        The implementation delegates to \c HestonSLVFDMModel which is
        already part of QuantLib.

        Numerical parameters are exposed through \c HestonSLVFokkerPlanckFdmParams.
        Sensible defaults are provided; the key parameters to tune for
        production are \c xGrid, \c vGrid, and \c leverageFctPropEps.

        \sa HestonSLVFDMModel, HestonSLVFokkerPlanckFdmParams

        \ingroup experimental
    */
    class HestonSLVLeverageCalibrator : public SLVLeverageCalibrator {
      public:
        //! \name Construction
        //@{
        /*!
            \param params       finite-difference solver parameters
            \param mixingFactor blending parameter in (0,1] scaling the
                                Heston vol-of-vol: 1 = full SLV, towards 0
                                the local-vol limit, which itself (a
                                deterministic variance) is not supported
            \param logging      store per-timestep density snapshots for diagnostics

            The Heston model is supplied either via \c setStochVolModel() (when used
            through \c FxSLVPricingContext) or, for standalone use, via the
            explicit \c setHestonModel() method before calling \c calibrate().
        */
        explicit HestonSLVLeverageCalibrator(
            HestonSLVFokkerPlanckFdmParams params = defaultParams(),
            Real mixingFactor = 1.0,
            bool logging = false);
        //@}

        //! Default Fokker-Planck PDE parameters suitable for FX options.
        static HestonSLVFokkerPlanckFdmParams defaultParams();

        //! \name SLVLeverageCalibrator interface
        //@{
        //! Accepts a \c HestonModel; throws if the dynamic cast fails.
        void setStochVolModel(const ext::shared_ptr<CalibratedModel>& model) override;

        void calibrate(const Handle<LocalVolTermStructure>& localVol,
                       const Date& endDate,
                       const std::vector<Date>& mandatoryDates = {}) override;

        ext::shared_ptr<LocalVolTermStructure> leverageFunction() const override;
        //@}

        //! Convenience setter for standalone use (bypasses the CalibratedModel cast).
        void setHestonModel(const ext::shared_ptr<HestonModel>& model);

        //! Access the underlying FDM model (e.g. to inspect log entries).
        ext::shared_ptr<HestonSLVFDMModel> fdmModel() const;

      private:
        const HestonSLVFokkerPlanckFdmParams params_;
        const Real mixingFactor_;
        const bool logging_;

        ext::shared_ptr<HestonModel> hestonModel_;
        ext::shared_ptr<HestonSLVFDMModel> fdmModel_;
    };

}

#endif
