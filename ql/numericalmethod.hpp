
/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2004 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file numericalmethod.hpp
    \brief Numerical method class
*/

#ifndef quantlib_numerical_method_h
#define quantlib_numerical_method_h

#include <ql/grid.hpp>
#include <boost/shared_ptr.hpp>

namespace QuantLib {

    class DiscretizedAsset;

    //! Numerical method (tree, finite-differences) base class
    class NumericalMethod {
      public:
        NumericalMethod(const TimeGrid& timeGrid) : t_(timeGrid) {}
        virtual ~NumericalMethod() {}

        //!\name Inspectors
        //{
        const TimeGrid& timeGrid() const { return t_; }
        //@}

        /*! \name Numerical method interface

            These methods are to be used by discretized assets and
            must be overridden by developers implementing numerical
            methods. Users are advised to use the corresponding
            methods of DiscretizedAsset instead.

            @{
        */

        //! initialize an asset at the given time.
        virtual void initialize(DiscretizedAsset&,
                                Time time) const = 0;

        /*! Roll back an asset until the given time, performing any
            needed adjustment.
        */
        virtual void rollback(DiscretizedAsset&,
                              Time to) const = 0;

        /*! Roll back an asset until the given time, but do not perform
            the final adjustment.
        */
        virtual void partialRollback(DiscretizedAsset&,
                                     Time to) const = 0;

        //! computes the present value of an asset.
        virtual Real presentValue(DiscretizedAsset&) = 0;

        //@}
      protected:
        TimeGrid t_;
    };

}


#endif
