/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Cristina Duminuco
 Copyright (C) 2006 François du Vignaud
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

/*! \file sabrinterpolatedsmilesection.hpp
    \brief Interpolated smile section class
*/

#ifndef quantlib_sabr_interpolated_smile_section_hpp
#define quantlib_sabr_interpolated_smile_section_hpp

#include <ql/handle.hpp>
#include <ql/patterns/lazyobject.hpp>
#include <ql/termstructures/volatility/smilesection.hpp>
#include <ql/math/interpolations/sabrinterpolation.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>

namespace QuantLib {

    class Quote;
    class SabrInterpolatedSmileSection : public SmileSection,
                                         public LazyObject {
      public:
        //! \name Constructors
        //@{
        //! all market data are quotes
        SabrInterpolatedSmileSection(
            const Date& optionDate,
            Handle<Quote> forward,
            const std::vector<Rate>& strikes,
            bool hasFloatingStrikes,
            Handle<Quote> atmVolatility,
            const std::vector<Handle<Quote> >& volHandles,
            Real alpha,
            Real beta,
            Real nu,
            Real rho,
            bool isAlphaFixed = false,
            bool isBetaFixed = false,
            bool isNuFixed = false,
            bool isRhoFixed = false,
            bool vegaWeighted = true,
            ext::shared_ptr<EndCriteria> endCriteria = ext::shared_ptr<EndCriteria>(),
            ext::shared_ptr<OptimizationMethod> method = ext::shared_ptr<OptimizationMethod>(),
            const DayCounter& dc = Actual365Fixed(),
            Real shift = 0.0);
        //! no quotes
        SabrInterpolatedSmileSection(
            const Date& optionDate,
            const Rate& forward,
            const std::vector<Rate>& strikes,
            bool hasFloatingStrikes,
            const Volatility& atmVolatility,
            const std::vector<Volatility>& vols,
            Real alpha,
            Real beta,
            Real nu,
            Real rho,
            bool isAlphaFixed = false,
            bool isBetaFixed = false,
            bool isNuFixed = false,
            bool isRhoFixed = false,
            bool vegaWeighted = true,
            ext::shared_ptr<EndCriteria> endCriteria = ext::shared_ptr<EndCriteria>(),
            ext::shared_ptr<OptimizationMethod> method = ext::shared_ptr<OptimizationMethod>(),
            const DayCounter& dc = Actual365Fixed(),
            Real shift = 0.0);
        //@}
        //! \name LazyObject interface
        //@{
        void performCalculations() const override;
        void update() override;
        //@}
        //! \name Observer interface
        //@{
        std::pair<Observer::iterator, bool>
        registerWith(const ext::shared_ptr<Observable>& o) override;
        //@}
        //! \name SmileSection interface
        //@{
        Real minStrike() const override;
        Real maxStrike() const override;
        Real atmLevel() const override;
        //@}
        Real varianceImpl(Rate strike) const override;
        Volatility volatilityImpl(Rate strike) const override;
        //! \name Inspectors
        //@{
        Real alpha() const;
        Real beta() const;
        Real nu() const;
        Real rho() const;
        Real rmsError() const;
        Real maxError() const;
        EndCriteria::Type endCriteria() const;
        //@}

      protected:

        //! Creates the mutable SABRInterpolation
        void createInterpolation() const;
        mutable ext::shared_ptr<SABRInterpolation> sabrInterpolation_;

        //! Market data
        const Handle<Quote> forward_;
        const Handle<Quote> atmVolatility_;
        std::vector<Handle<Quote> > volHandles_;
        mutable std::vector<Rate> strikes_;
        //! Only strikes corresponding to valid market data
        mutable std::vector<Rate> actualStrikes_;
        bool hasFloatingStrikes_;

        mutable Real forwardValue_;
        mutable std::vector<Volatility> vols_;
        //! Sabr parameters
        Real alpha_, beta_, nu_, rho_;
        //! Sabr interpolation settings
        bool isAlphaFixed_, isBetaFixed_, isNuFixed_, isRhoFixed_;
        bool vegaWeighted_;
        const ext::shared_ptr<EndCriteria> endCriteria_;
        const ext::shared_ptr<OptimizationMethod> method_;

        mutable Date evaluationDate_;
    };

    inline void SabrInterpolatedSmileSection::update() {
        LazyObject::update();
        SmileSection::update();
    }

    inline Real SabrInterpolatedSmileSection::volatilityImpl(Rate strike) const {
        calculate();
        return (*sabrInterpolation_)(strike, true);
    }

    inline Real SabrInterpolatedSmileSection::alpha() const {
        calculate();
        return sabrInterpolation_->alpha();
    }

    inline Real SabrInterpolatedSmileSection::beta() const {
        calculate();
        return sabrInterpolation_->beta();
    }

    inline Real SabrInterpolatedSmileSection::nu() const {
        calculate();
        return sabrInterpolation_->nu();
    }

    inline Real SabrInterpolatedSmileSection::rho() const {
        calculate();
        return sabrInterpolation_->rho();
    }

    inline Real SabrInterpolatedSmileSection::rmsError() const {
        calculate();
        return sabrInterpolation_->rmsError();
    }

    inline Real SabrInterpolatedSmileSection::maxError() const {
        calculate();
        return sabrInterpolation_->maxError();
    }

    inline EndCriteria::Type SabrInterpolatedSmileSection::endCriteria() const {
        calculate();
        return sabrInterpolation_->endCriteria();
    }

    inline Real SabrInterpolatedSmileSection::minStrike() const {
        calculate();
        return actualStrikes_.front();

    }

    inline Real SabrInterpolatedSmileSection::maxStrike() const {
        calculate();
        return actualStrikes_.back();
    }

    inline Real SabrInterpolatedSmileSection::atmLevel() const {
        calculate();
        return forwardValue_;
    }

    inline std::pair<Observer::iterator, bool>
    SabrInterpolatedSmileSection::registerWith(const ext::shared_ptr<Observable>& o) {
        return LazyObject::registerWith(o);
    }

}

#endif
