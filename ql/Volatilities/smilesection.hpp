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

    class SmileSectionInterface {
      public:
        virtual Real variance(Rate strike) const = 0;
        virtual Real volatility(Rate strike) const = 0;
        //virtual Date exerciseDate() const = 0;
        //virtual Time exerciseTime() const = 0;
    };


    //! interest rate volatility smile section
    /*! This class provides the volatility smile section
    */
    class InterpolatedSmileSection : public SmileSectionInterface {
      public:
        InterpolatedSmileSection(Time expiryTime,
                     const std::vector<Rate>& strikes,
                     const std::vector<Volatility>& volatilities);
        Real variance(Rate strike) const;
        Real volatility(Rate strike) const;
    private:
        Time timeToExpiry_;
        std::vector<Rate> strikes_;
        std::vector<Volatility> volatilities_;
        boost::shared_ptr<Interpolation> interpolation_;
    };


    class SabrSmileSection : public SmileSectionInterface {
      public:
        SabrSmileSection(const std::vector<Real>& sabrParameters,
                         Time timeToExpiry);
        Real variance(Rate strike) const;
        Real volatility(Rate strike) const;
    private:
        Time timeToExpiry_;
        Real alpha_, beta_, nu_, rho_, forward_;
    };


}

#endif
