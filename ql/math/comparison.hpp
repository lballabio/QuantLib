/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2004, 2005, 2007 StatPro Italia srl

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

/*! \file comparison.hpp
    \brief floating-point comparisons
*/

#ifndef quantlib_comparison_hpp
#define quantlib_comparison_hpp

#include <ql/types.hpp>
#include <ql/shared_ptr.hpp>

namespace QuantLib {

    /*! Follows somewhat the advice of Knuth on checking for floating-point
        equality. The closeness relationship is:
        \f[
        \mathrm{close}(x,y,n) \equiv |x-y| \leq \varepsilon |x|
                              \wedge |x-y| \leq \varepsilon |y|
        \f]
        where \f$ \varepsilon \f$ is \f$ n \f$ times the machine accuracy;
        \f$ n \f$ equals 42 if not given.
    */
    bool close(Real x, Real y);
    bool close(Real x, Real y, Size n);

    /*! Follows somewhat the advice of Knuth on checking for floating-point
        equality. The closeness relationship is:
        \f[
        \mathrm{close}(x,y,n) \equiv |x-y| \leq \varepsilon |x|
                                \vee |x-y| \leq \varepsilon |y|
        \f]
        where \f$ \varepsilon \f$ is \f$ n \f$ times the machine accuracy;
        \f$ n \f$ equals 42 if not given.
    */
    bool close_enough(Real x, Real y);
    bool close_enough(Real x, Real y, Size n);


    // inline definitions

    inline bool close(Real x, Real y) {
        return close(x,y,42);
    }

    inline bool close(Real x, Real y, Size n) {
        // Deals with +infinity and -infinity representations etc.
        if (x == y)
            return true;

        Real diff = std::fabs(x-y), tolerance = n * QL_EPSILON;

        if (x * y == 0.0) // x or y = 0.0
            return diff < (tolerance * tolerance);

        return diff <= tolerance*std::fabs(x) &&
               diff <= tolerance*std::fabs(y);
    }

    inline bool close_enough(Real x, Real y) {
        return close_enough(x,y,42);
    }

    inline bool close_enough(Real x, Real y, Size n) {
        // Deals with +infinity and -infinity representations etc.
        if (x == y)
            return true;

        Real diff = std::fabs(x-y), tolerance = n * QL_EPSILON;

        if (x * y == 0.0) // x or y = 0.0
            return diff < (tolerance * tolerance);

        return diff <= tolerance*std::fabs(x) ||
               diff <= tolerance*std::fabs(y);
    }



    //! compare two objects by date
    /*! There is no generic implementation of this struct.
        Template specializations will have to be defined for
        each needed type (see CashFlow for an example.)
    */
    template <class T> struct earlier_than;

    /* partial specialization for shared pointers, forwarding to their
       pointees. */
    template <class T>
    struct earlier_than<ext::shared_ptr<T> > {
        typedef ext::shared_ptr<T> first_argument_type;
        typedef ext::shared_ptr<T> second_argument_type;
        typedef bool result_type;

        bool operator()(const ext::shared_ptr<T>& x,
                        const ext::shared_ptr<T>& y) const {
            return earlier_than<T>()(*x,*y);
        }
    };

}


#endif


#ifndef id_384bf1db04c1b5e59c203e0d97612307
#define id_384bf1db04c1b5e59c203e0d97612307
inline bool test_384bf1db04c1b5e59c203e0d97612307(int* i) { return i != 0; }
#endif
