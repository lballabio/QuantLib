/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2004, 2005 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file numericalmethod.hpp
    \brief Numerical method class
*/

#ifndef quantlib_lattice_hpp
#define quantlib_lattice_hpp

#include <ql/math/array.hpp>
#include <ql/timegrid.hpp>
#include <utility>

namespace QuantLib {

    class DiscretizedAsset;

    //! %Lattice (tree, finite-differences) base class
    class Lattice {
      public:
        explicit Lattice(TimeGrid timeGrid) : t_(std::move(timeGrid)) {}
        virtual ~Lattice() = default;

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

            \warning In version 0.3.7 and earlier, this method was
                     called rollAlmostBack method and performed
                     pre-adjustment. This is no longer true; when
                     migrating your code, you'll have to replace calls
                     such as:
                     \code
                     method->rollAlmostBack(asset,t);
                     \endcode
                     with the two statements:
                     \code
                     method->partialRollback(asset,t);
                     asset->preAdjustValues();
                     \endcode
        */
        virtual void partialRollback(DiscretizedAsset&,
                                     Time to) const = 0;

        //! computes the present value of an asset.
        virtual Real presentValue(DiscretizedAsset&) const = 0;

        //@}

        // this is a smell, but we need it. We'll rethink it later.
        virtual Array grid(Time) const = 0;
      protected:
        TimeGrid t_;
    };

}


#endif
