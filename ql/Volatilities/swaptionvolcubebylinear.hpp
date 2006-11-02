/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file swaptionvolcube.hpp
    \brief Swaption volatility cube, fit-later-interpolate-early approach
*/

#ifndef quantlib_swaption_volcube_fit_later_interpolate_early_h
#define quantlib_swaption_volcube_fit_later_interpolate_early_h

#include <ql/Volatilities/swaptionvolcube.hpp>
#include <ql/Math/interpolation2D.hpp>

namespace QuantLib {

    class SwaptionVolatilityCubeByLinear : public SwaptionVolatilityCube {
      public:
        SwaptionVolatilityCubeByLinear(
            const Handle<SwaptionVolatilityStructure>& atmVolStructure,
            const std::vector<Period>& expiries,
            const std::vector<Period>& lengths,
            const std::vector<Spread>& strikeSpreads,
            const std::vector<std::vector<Handle<Quote> > >& volSpreads,
            const boost::shared_ptr<SwapIndex>& swapIndexBase,
            bool vegaWeightedSmileFit);
        //! \name SwaptionVolatilityCube inspectors
        //@{
        boost::shared_ptr<SmileSectionInterface> smileSection(
                                              const Date& optionDate,
                                              const Period& swapTenor) const;
        boost::shared_ptr<SmileSectionInterface> smileSection(
                                                      Time optionTime,
                                                      Time swapLength) const;
        //@}
      private:
        std::vector<Interpolation2D> volSpreadsInterpolator_;
    };

}

#endif
