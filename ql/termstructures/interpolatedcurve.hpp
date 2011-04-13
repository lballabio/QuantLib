/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file interpolatedcurve.hpp
    \brief Helper class to build interpolated term structures
*/

#ifndef quantlib_interpolated_curve_hpp
#define quantlib_interpolated_curve_hpp

#include <ql/math/interpolation.hpp>
#include <vector>

namespace QuantLib {

    //! Helper class to build interpolated term structures
    /*! Interpolated term structures can use proected or private
        inheritance from this class to obtain the relevant data
        members and implement correct copy behavior.
    */
    template <class Interpolator>
    class InterpolatedCurve {
      protected:
        //! \name Building
        //@{
        InterpolatedCurve(const std::vector<Time>& times,
                          const std::vector<Real>& data,
                          const Interpolator& i = Interpolator())
        : times_(times), data_(data), interpolator_(i) {}

        InterpolatedCurve(const std::vector<Time>& times,
                          const Interpolator& i = Interpolator())
        : times_(times), data_(times.size()), interpolator_(i) {}

        InterpolatedCurve(Size n,
                          const Interpolator& i = Interpolator())
        : times_(n), data_(n), interpolator_(i) {}

        InterpolatedCurve(const Interpolator& i = Interpolator())
        : interpolator_(i) {}
        //@}

        //! \name Copying
        //@{
        InterpolatedCurve(const InterpolatedCurve& c)
        : times_(c.times_), data_(c.data_), interpolator_(c.interpolator_) {
            setupInterpolation();
        }

        InterpolatedCurve& operator=(const InterpolatedCurve& c) {
            times_ = c.times_;
            data_ = c.data_;
            interpolator_ = c.interpolator_;
            setupInterpolation();
            return *this;
        }
        //@}

        void setupInterpolation() {
            interpolation_ = interpolator_.interpolate(times_.begin(),
                                                       times_.end(),
                                                       data_.begin());
        }

        mutable std::vector<Time> times_;
        mutable std::vector<Real> data_;
        mutable Interpolation interpolation_;
        Interpolator interpolator_;
    };

}

#endif

