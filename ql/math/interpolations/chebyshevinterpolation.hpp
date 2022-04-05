/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2021 Klaus Spanderen

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

/*! \file chebyshevinterpolation.hpp
    \brief chebyshev interpolation between discrete points z_i=-cos(i*pi/n)
*/

#ifndef quantlib_chebyshev_interpolation_hpp
#define quantlib_chebyshev_interpolation_hpp

#include <ql/functional.hpp>
#include <ql/math/array.hpp>


namespace QuantLib {

    /*! References:
        S.A. Sarra: Chebyshev Interpolation: An Interactive Tour,
        https://www.maa.org/sites/default/files/images/upload_library/4/vol6/Sarra/Chebyshev.html

        https://dsp.stackexchange.com/questions/2807/fast-cosine-transform-via-fft
     */

    class ChebyshevInterpolation {
      public:
        typedef Real argument_type;
        typedef Real result_type;

        ChebyshevInterpolation(const Array& f);
        ChebyshevInterpolation(Size n, const ext::function<Real(Real)>& f);

        Real operator()(Real z) const;
        static Array nodes(Size n);

      private:
        static Array apply(const Array& x, const ext::function<Real(Real)>& f);

        const Size n_;
        const Array nodes_;
        Array a_;
    };
}

#endif
