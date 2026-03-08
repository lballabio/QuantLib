/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Yassine Idyiahia

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

/*! \file smilesectionblackvolsurface.hpp
    \brief Black vol surface adapter for SmileSection objects
*/

#ifndef quantlib_smile_section_black_vol_surface_hpp
#define quantlib_smile_section_black_vol_surface_hpp

#include <ql/termstructures/volatility/equityfx/blackvoltermstructure.hpp>
#include <ql/termstructures/volatility/smilesection.hpp>
#include <ql/patterns/visitor.hpp>
#include <vector>

namespace QuantLib {

    //! Black vol surface built from SmileSection objects
    /*! This class adapts one or more SmileSection instances into a
        BlackVolTermStructure, bridging the two volatility hierarchies.

        For a single smile, the surface returns the smile volatility
        at all maturities (flat forward variance assumption).

        For multiple smiles at different expiries, it interpolates
        linearly in total variance space, which preserves the
        calendar arbitrage-free condition when the input smiles are
        arbitrage-free.

        \ingroup termstructures
    */
    class SmileSectionBlackVolSurface : public BlackVolatilityTermStructure {
      public:
        /*! Single smile constructor.
            Uses flat forward variance assumption: vol is independent
            of maturity.
        */
        SmileSectionBlackVolSurface(
            const Date& referenceDate,
            ext::shared_ptr<SmileSection> smile,
            const DayCounter& dc = DayCounter(),
            const Calendar& cal = Calendar(),
            BusinessDayConvention bdc = Following);

        /*! Multiple smiles constructor.
            Smiles are sorted by expiry time internally.
            Interpolates linearly in total variance space between
            adjacent expiries.
        */
        SmileSectionBlackVolSurface(
            const Date& referenceDate,
            std::vector<ext::shared_ptr<SmileSection>> smiles,
            const DayCounter& dc = DayCounter(),
            const Calendar& cal = Calendar(),
            BusinessDayConvention bdc = Following);

        //! \name TermStructure interface
        //@{
        Date maxDate() const override;
        //@}
        //! \name VolatilityTermStructure interface
        //@{
        Real minStrike() const override;
        Real maxStrike() const override;
        //@}
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&) override;
        //@}
      protected:
        Volatility blackVolImpl(Time t, Real strike) const override;
      private:
        std::vector<ext::shared_ptr<SmileSection>> smiles_;
        std::vector<Time> expiryTimes_;
    };

}

#endif
