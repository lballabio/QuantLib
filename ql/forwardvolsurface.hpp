

/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file forwardvolsurface.hpp
    \brief Forward volatility surface

    \fullpath
    ql/%forwardvolsurface.hpp
*/

// $Id$

#ifndef quantlib_forward_volatility_surface_h
#define quantlib_forward_volatility_surface_h

#include <ql/date.hpp>
#include <ql/handle.hpp>

namespace QuantLib {

    //! Forward volatility surface
    /*! This class is purely abstract and defines the interface of concrete
        forward volatility structures which will be derived from this one.

        \todo A way should be implemented of constructing a
        ForwardVolatilitySurface from a generic term volatility structure.
    */
    class ForwardVolatilitySurface {
      public:
        virtual ~ForwardVolatilitySurface() {}
        //! returns the volatility for a given date and strike
        virtual Rate vol(const Date& d, Rate strike) const = 0;
    };

    //! Forward volatility curve (not smiled)
    class ForwardVolatilityCurve : public ForwardVolatilitySurface {
      public:
        //! implementation of the ForwardVolatilitySurface interface.
        /*! This method calls vol(date) to return the volatility for a given
            date regardless of strike
        */
        virtual Rate vol(const Date& d, Rate strike) const;
        //! returns the volatility for a given date
        /*! This method must be implemented by derived classes instead of
            vol(date,strike) which calls this one after discarding the strike
        */
        virtual Rate vol(const Date& d) const = 0;
    };

    //! Constant forward volatility surface
    class ConstantForwardVolatilitySurface
    : public ForwardVolatilitySurface {
      public:
        ConstantForwardVolatilitySurface(Rate volatility);
        //! returns the given volatility regardless of date and strike
        Rate vol(const Date& d, Rate strike) const;
      private:
        Rate volatility_;
    };

    //! Forward volatility surface with an added spread
    /*! This surface will remain linked to the original surface, i.e., any
        changes in the latter will be reflected in this surface as well.
    */
    class SpreadedForwardVolatilitySurface
    : public ForwardVolatilitySurface {
      public:
        SpreadedForwardVolatilitySurface(
            const Handle<ForwardVolatilitySurface>&, Spread spread);
        //! returns the volatility of the original surface plus the given spread
        Rate vol(const Date& d, Rate strike) const;
      private:
        Handle<ForwardVolatilitySurface> theOriginalCurve;
        Spread theSpread;
    };


    // inline definitions

    // curve without smile

    inline Rate ForwardVolatilityCurve::vol(const Date& d, Rate strike) const {
        return vol(d);
    }

    // constant surface

    inline ConstantForwardVolatilitySurface::ConstantForwardVolatilitySurface(
        Rate volatility)
    : volatility_(volatility) {}

    inline Rate ConstantForwardVolatilitySurface::vol(const Date& d,
        Rate strike) const {
        return volatility_;
    }

    // spreaded surface

    inline SpreadedForwardVolatilitySurface::SpreadedForwardVolatilitySurface(
        const Handle<ForwardVolatilitySurface>& h, Spread spread)
    : theOriginalCurve(h), theSpread(spread) {}

    inline Rate SpreadedForwardVolatilitySurface::vol(const Date& d,
        Rate strike) const {
        return theOriginalCurve->vol(d,strike)+theSpread;
    }

}


#endif
