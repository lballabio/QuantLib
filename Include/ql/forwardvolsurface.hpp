
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file forwardvolsurface.hpp

    \fullpath
    Include/ql/%forwardvolsurface.hpp
    \brief Forward volatility surface

*/

// $Id$
// $Log$
// Revision 1.12  2001/08/31 15:23:44  sigmud
// refining fullpath entries for doxygen documentation
//
// Revision 1.11  2001/08/09 14:59:45  sigmud
// header modification
//
// Revision 1.10  2001/08/08 11:07:48  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.9  2001/08/07 11:25:53  sigmud
// copyright header maintenance
//
// Revision 1.8  2001/07/25 15:47:27  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.7  2001/07/24 08:49:32  sigmud
// pruned redundant header inclusions
//
// Revision 1.6  2001/07/05 15:57:22  lballabio
// Collected typedefs in a single file
//
// Revision 1.5  2001/06/22 16:38:15  lballabio
// Improved documentation
//
// Revision 1.4  2001/05/24 15:38:07  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef quantlib_forward_volatility_surface_h
#define quantlib_forward_volatility_surface_h

#include "ql/date.hpp"
#include "ql/handle.hpp"

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
