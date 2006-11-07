/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Mario Pucci

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

    //! interest rate volatility smile section
    /*! This abstract class provides volatility smile section interface */
    class SmileSectionInterface {
      public:
        SmileSectionInterface(const Date& d,
                              const DayCounter& dc,
                              const Date& referenceDate = Date())
        : exerciseDate_(d), dc_(dc) {
            Date refDate = referenceDate!=Date() ? referenceDate :
                           Settings::instance().evaluationDate();
            QL_REQUIRE(d>=refDate,
                       "expiry date (" << d << 
                       ") must be greater than reference date (" <<
                       refDate << ")");
            exerciseTime_ = dc_.yearFraction(refDate, d);
        };
        SmileSectionInterface(Time exerciseTime,
                              const DayCounter& dc = DayCounter())
        : dc_(dc), exerciseTime_(exerciseTime) {
            QL_REQUIRE(exerciseTime_>=0.0,
                       "expiry time must be positive: " <<
                       exerciseTime_ << " not allowed");
        };
        virtual Real variance(Rate strike) const = 0;
        virtual Real volatility(Rate strike) const = 0;
        virtual ~SmileSectionInterface() {};
        virtual const Date& exerciseDate() const { return exerciseDate_; }
        virtual Time exerciseTime() const { return exerciseTime_; };
        virtual const DayCounter& dayCounter() const { return dc_; }
      protected:
        Date exerciseDate_;
        DayCounter dc_;
        Time exerciseTime_;
    };


    class InterpolatedSmileSection : public SmileSectionInterface {
      public:
        InterpolatedSmileSection(Time expiryTime,
                                 const std::vector<Rate>& strikes,
                                 const std::vector<Volatility>& volatilities);
        InterpolatedSmileSection(const Date&,
                                 const DayCounter&,
                                 const std::vector<Rate>& strikes,
                                 const std::vector<Volatility>& volatilities);
        Real variance(Rate strike) const;
        Real volatility(Rate strike) const;
    private:
        std::vector<Rate> strikes_;
        std::vector<Volatility> volatilities_;
        boost::shared_ptr<Interpolation> interpolation_;
    };


    class SabrSmileSection : public SmileSectionInterface {
      public:
        SabrSmileSection(Time timeToExpiry,
                         const std::vector<Real>& sabrParameters);
        SabrSmileSection(const Date&,
                         const DayCounter&,
                         const std::vector<Real>& sabrParameters);
        Real variance(Rate strike) const;
        Real volatility(Rate strike) const;
    private:
        Real alpha_, beta_, nu_, rho_, forward_;
    };

}

#endif
