
/*
 Copyright (C) 2003 StatPro Italia srl

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

/*! \file comparison.hpp
    \brief floating-point comparisons
*/

#ifndef quantlib_comparison_hpp
#define quantlib_comparison_hpp

#include <ql/qldefines.hpp>

namespace QuantLib {

    /*! Follows somewhat the advice of Knuth on checking for floating-point
        equality. The closeness relationship is:
        \f[
        \mathrm{close}(x,y,n) \equiv |x-y| \leq \varepsilon |x|
                              \wedge |x-y| \leq \varepsilon |y|
        \f]
        where \f$ \varepsilon \f$ is \f$ n \f$ times the machine accuracy.
    */
    bool close(Real x, Real y, Size n = 42);

    /*! Follows somewhat the advice of Knuth on checking for floating-point
        equality. The closeness relationship is:
        \f[
        \mathrm{close}(x,y,n) \equiv |x-y| \leq \varepsilon |x|
                                \vee |x-y| \leq \varepsilon |y|
        \f]
        where \f$ \varepsilon \f$ is \f$ n \f$ times the machine accuracy.
    */
    bool close_enough(Real x, Real y, Size n = 42);


    // inline definitions

    inline bool close(Real x, Real y, Size n) {
        Real diff = std::fabs(x-y), tolerance = n*QL_EPSILON;
        return diff <= tolerance*std::fabs(x) &&
               diff <= tolerance*std::fabs(y);
    }

    inline bool close_enough(Real x, Real y, Size n) {
        Real diff = std::fabs(x-y), tolerance = n*QL_EPSILON;
        return diff <= tolerance*std::fabs(x) ||
               diff <= tolerance*std::fabs(y);
    }

}


#endif
