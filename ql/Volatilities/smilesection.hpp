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

#include <ql/Patterns/observable.hpp>
#include <ql/date.hpp>
#include <ql/DayCounters/actual365fixed.hpp>
#include <vector>

namespace QuantLib {

    //! interest rate volatility smile section
    /*! This abstract class provides volatility smile section interface */
    class SmileSection: public Observable {
      public:
        SmileSection(const Date& d,
                     const DayCounter& dc = Actual365Fixed(),
                     const Date& referenceDate = Date());
        SmileSection(Time exerciseTime,
                     const DayCounter& dc = Actual365Fixed());
        virtual ~SmileSection() {};

        virtual Real variance(Rate strike) const = 0;
        virtual Volatility volatility(Rate strike) const = 0;

        //virtual Rate atmLevel() const = 0;
        const Date& exerciseDate() const { return exerciseDate_; }
        Time exerciseTime() const { return exerciseTime_; };
        const DayCounter& dayCounter() const { return dc_; }
      protected:
        Date exerciseDate_;
        DayCounter dc_;
        Time exerciseTime_;
    };

    class FlatSmileSection : public SmileSection {
      public:
        FlatSmileSection(const Date& d,
                         Volatility vol,
                         const DayCounter& dc,
                         const Date& referenceDate = Date())
        : SmileSection(d, dc, referenceDate), vol_(vol) {};

        FlatSmileSection(Time exerciseTime,
                         Volatility vol,
                         const DayCounter& dc = Actual365Fixed())
        : SmileSection(exerciseTime, dc), vol_(vol) {};

        Real variance(Rate) const { return vol_*vol_*exerciseTime_; }
        Volatility volatility(Rate) const { return vol_; }
      private:
        Volatility vol_;
    };

    class SabrSmileSection : public SmileSection {
      public:
        SabrSmileSection(Time timeToExpiry,
                         const Rate forward,
                         const std::vector<Real>& sabrParameters);
        SabrSmileSection(const Date& d,
                         const Rate forward,
                         const std::vector<Real>& sabrParameters,
                         const DayCounter& dc = Actual365Fixed());
        Real variance(Rate strike) const;
        Volatility volatility(Rate strike) const;
    private:
        Real alpha_, beta_, nu_, rho_, forward_;
    };

}

#endif
