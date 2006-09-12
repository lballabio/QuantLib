/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006  Mario Pucci

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

/*! \file smilesection.hpp
    \brief Swaption volatility structure
*/

#ifndef quantlib_smile_section_hpp
#define quantlib_smile_section_hpp

#include <ql/termstructure.hpp>
#include <ql/Math/linearinterpolation.hpp>

namespace QuantLib {

     //! swaption volatility smile section
    /*! This class provides the volatility smile section
    */
    class SmileSection {
      public:
        SmileSection(Time expiryTime,
              const std::vector<Rate>& strikes,
              const std::vector<Rate>& volatilities);

        SmileSection(
          const std::vector<Real>& sabrParameters,
          const Time timeToExpiry);

        Real variance(const Rate& strike) const;

        Volatility volatility(const Rate& strike) const;

    private:
          
        SmileSection& operator=(const SmileSection& o);
        SmileSection(const SmileSection& o);

        Time timeToExpiry_;
        std::vector<Rate> strikes_;
        std::vector<Volatility> volatilities_;
        boost::shared_ptr<Interpolation> interpolation_;
    };

}

#endif
