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
    \brief Swaption volatility cube
*/

#ifndef quantlib_swaption_volatility_cube_bylinear_h
#define quantlib_swaption_volatility_cube_bylinear_h

#include <ql/Volatilities/swaptionvolcube.hpp>
#include <ql/Math/matrix.hpp>
#include <ql/Math/bilinearinterpolation.hpp>
#include <ql/quote.hpp>

namespace QuantLib {

    class SwaptionVolatilityCubeByLinear : public SwaptionVolatilityCube {
      public:
        SwaptionVolatilityCubeByLinear(
            const Handle<SwaptionVolatilityStructure>& atmVolStructure,
            const std::vector<Period>& expiries,
            const std::vector<Period>& lengths,
            const std::vector<Spread>& strikeSpreads,
            const std::vector<std::vector<Handle<Quote> > >& volatilitySpreads,
            const Calendar& calendar,
            const boost::shared_ptr<SwapIndex>& swapIndexBase);
        //! \name Other inspectors
        //@{
        const Matrix& volSpreads(Size i) const { return volSpreads_[i]; }
        boost::shared_ptr<SmileSectionInterface> smileSection(
                                                 const Date& exerciseDate,
                                                 const Period& length) const;
        boost::shared_ptr<SmileSectionInterface> smileSection(Time start,
                                                     Time length) const;
        //@}
      protected:
        Volatility volatilityImpl(Time start,
                                  Time length,
                                  Rate strike) const;
        Volatility volatilityImpl(const Date& exerciseDate,
                                  const Period& length,
                                  Rate strike) const;
      private:
        std::vector<Matrix> volSpreads_;
        std::vector<Interpolation2D> volSpreadsInterpolator_;
    };

}

#endif
