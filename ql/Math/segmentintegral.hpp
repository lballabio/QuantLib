

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
/*! \file segmentintegral.hpp
    \brief Integral of a one-dimensional function

    \fullpath
    ql/Math/%integral.hpp
*/

// $Id$

#include <ql/solver1d.hpp>
#include <functional>

#ifndef quantlib_segment_integral_h
#define quantlib_segment_integral_h

namespace QuantLib {

    namespace Math {

        /*! \brief Integral of a one-dimensional function

            \warning the use of this class is not recommended since
            it will be redesigned in one of the next minor releases.

            \todo Redesign as a template function.
        */
        class SegmentIntegral{
          public:
            SegmentIntegral(Size intervals);
            double operator()(const ObjectiveFunction &f,
                             double a, double b) const;
          private:
            Size intervals_;
       };

    }

}


#endif
