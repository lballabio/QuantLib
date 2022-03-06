/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

/*! \file noarbsabrinterpolatedsmilesection.hpp
    \brief noarb sabr interpolating smile section
*/

#ifndef quantlib_noarbsabr_interpolated_smile_section_hpp
#define quantlib_noarbsabr_interpolated_smile_section_hpp

#include <ql/handle.hpp>
#include <ql/patterns/lazyobject.hpp>
#include <ql/termstructures/volatility/smilesection.hpp>
#include <ql/experimental/volatility/noarbsabrinterpolation.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>

namespace QuantLib {

    class Quote;
    class NoArbSabrInterpolatedSmileSection : public SmileSection,
                                         public LazyObject {
      public:
        //! \name Constructors
        //@{
        //! all market data are quotes
        NoArbSabrInterpolatedSmileSection(
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
            const DayCounter& dc = Actual365Fixed());
        //! no quotes
        NoArbSabrInterpolatedSmileSection(
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
            const DayCounter& dc = Actual365Fixed());
        //@}
        //! \name LazyObject interface
        //@{
        void performCalculations() const override;
        void update() override;
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
        mutable ext::shared_ptr<NoArbSabrInterpolation> noArbSabrInterpolation_;

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
    };

    inline void NoArbSabrInterpolatedSmileSection::update() {
        LazyObject::update();
        SmileSection::update();
    }

    inline Real NoArbSabrInterpolatedSmileSection::volatilityImpl(Rate strike) const {
        calculate();
        return (*noArbSabrInterpolation_)(strike, true);
    }

    inline Real NoArbSabrInterpolatedSmileSection::alpha() const {
        calculate();
        return noArbSabrInterpolation_->alpha();
    }

    inline Real NoArbSabrInterpolatedSmileSection::beta() const {
        calculate();
        return noArbSabrInterpolation_->beta();
    }

    inline Real NoArbSabrInterpolatedSmileSection::nu() const {
        calculate();
        return noArbSabrInterpolation_->nu();
    }

    inline Real NoArbSabrInterpolatedSmileSection::rho() const {
        calculate();
        return noArbSabrInterpolation_->rho();
    }

    inline Real NoArbSabrInterpolatedSmileSection::rmsError() const {
        calculate();
        return noArbSabrInterpolation_->rmsError();
    }

    inline Real NoArbSabrInterpolatedSmileSection::maxError() const {
        calculate();
        return noArbSabrInterpolation_->maxError();
    }

    inline EndCriteria::Type NoArbSabrInterpolatedSmileSection::endCriteria() const {
        calculate();
        return noArbSabrInterpolation_->endCriteria();
    }

    inline Real NoArbSabrInterpolatedSmileSection::minStrike() const {
        calculate();
        return actualStrikes_.front();

    }

    inline Real NoArbSabrInterpolatedSmileSection::maxStrike() const {
        calculate();
        return actualStrikes_.back();
    }

    inline Real NoArbSabrInterpolatedSmileSection::atmLevel() const {
        calculate();
        return forwardValue_;
    }


}

#endif


#ifndef id_f5270ba61d54d7c15f28f2b3e4c57384
#define id_f5270ba61d54d7c15f28f2b3e4c57384
inline bool test_f5270ba61d54d7c15f28f2b3e4c57384(int* i) { return i != 0; }
#endif
