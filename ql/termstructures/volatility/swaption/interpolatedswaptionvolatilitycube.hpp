/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2023 Ignacio Anguita

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

/*! \file interpolatedswaptionvolatilitycube.hpp
    \brief Swaption volatility cube, fit-later-interpolate-early approach
*/

#ifndef quantlib_interpolated_swaption_volatility_cube_hpp
#define quantlib_interpolated_swaption_volatility_cube_hpp

#include <ql/termstructures/volatility/swaption/swaptionvolcube.hpp>
#include <ql/math/interpolations/interpolation2d.hpp>

namespace QuantLib {    

    //! Interpolated Swaption Volatility Cube
    /*! This class implements the Interpolated Swaption Volatility Cube,
        which is able to interpolate between the volatility spreads provided.

    */
    class InterpolatedSwaptionVolatilityCube : public SwaptionVolatilityCube{
      public:
          /*! The swaption vol cube is made up of ordered swaption vol surface
              layers, each layer referring to a swap index of a given length
              (in years), all indexes belonging to the same family. In order
              to identify the family (and its market conventions) an index of
              whatever length from that family must be passed in as
              swapIndexBase.

              Often for short swap length the swap index family is different,
              e.g. the EUR case: swap vs 6M Euribor is used for length>1Y,
              while swap vs 3M Euribor is used for the 1Y length. The
              shortSwapIndexBase is used to identify this second family.
        */
        InterpolatedSwaptionVolatilityCube(
            const Handle<SwaptionVolatilityStructure>& atmVolStructure,
            const std::vector<Period>& optionTenors,
            const std::vector<Period>& swapTenors,
            const std::vector<Spread>& strikeSpreads,
            const std::vector<std::vector<Handle<Quote> > >& volSpreads,
            const ext::shared_ptr<SwapIndex>& swapIndexBase,
            const ext::shared_ptr<SwapIndex>& shortSwapIndexBase,
            bool vegaWeightedSmileFit);
        //! \name LazyObject interface
        //@{
        void performCalculations() const override;
        //@}
        //! \name SwaptionVolatilityCube inspectors
        //@{
        const Matrix& volSpreads(Size i) const { return volSpreadsMatrix_[i]; }
        ext::shared_ptr<SmileSection> smileSectionImpl(const Date& optionDate,
                                                       const Period& swapTenor) const override;
        ext::shared_ptr<SmileSection> smileSectionImpl(Time optionTime,
                                                       Time swapLength) const override;
        //@}
      private:
        mutable std::vector<Interpolation2D> volSpreadsInterpolator_;
        mutable std::vector<Matrix> volSpreadsMatrix_;
    };

}

#endif
