
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software developed by the QuantLib Group; you can
 redistribute it and/or modify it under the terms of the QuantLib License;
 either version 1.0, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 QuantLib License for more details.

 You should have received a copy of the QuantLib License along with this
 program; if not, please email ferdinando@ametrano.net

 The QuantLib License is also available at http://quantlib.org/license.html
 The members of the QuantLib Group are listed in the QuantLib License
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
