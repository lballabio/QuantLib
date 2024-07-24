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
    \brief chebyshev interpolation between discrete Chebyshev nodes
*/

#ifndef quantlib_chebyshev_interpolation_hpp
#define quantlib_chebyshev_interpolation_hpp

#include <ql/functional.hpp>
#include <ql/math/array.hpp>
#include <ql/math/interpolation.hpp>


namespace QuantLib {
    class LagrangeInterpolation;

    /*! References:
        S.A. Sarra: Chebyshev Interpolation: An Interactive Tour,
        https://www.maa.org/sites/default/files/images/upload_library/4/vol6/Sarra/Chebyshev.html
     */

    class ChebyshevInterpolation: public Interpolation {
      public:
        enum PointsType {FirstKind, SecondKind};

        explicit ChebyshevInterpolation(
            const Array& y, PointsType pointsType = SecondKind);
        ChebyshevInterpolation(
            Size n, const std::function<Real(Real)>& f,
            PointsType pointsType = SecondKind);

        void updateY(const Array& y);

        Array nodes() const;
        static Array nodes(Size n, PointsType pointsType);

      private:
        const Array x_;
        Array y_;
        ext::shared_ptr<LagrangeInterpolation> lagrangeInterp_;
    };
}

#endif
