/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 RiskMap srl

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

/*! \file ql/math/functional.hpp
    \brief functionals and combinators not included in the STL
*/

#ifndef quantlib_math_functional_hpp
#define quantlib_math_functional_hpp

#include <ql/types.hpp>
#include <ql/utilities/null.hpp>
#include <cmath>
#include <utility>

namespace QuantLib {

    // functions

    template <class T, class U>
    class constant {
      public:
        typedef T argument_type;
        typedef U result_type;
        explicit constant(const U& u) : u_(u) {}
        U operator()(const T&) const { return u_; }
      private:
        U u_;
    };

    template <class T>
    class identity {
      public:
        typedef T argument_type;
        typedef T result_type;
        T operator()(const T& t) const { return t; }
    };

    template <class T>
    class square {
      public:
        typedef T argument_type;
        typedef T result_type;
        T operator()(const T& t) const { return t*t; }
    };

    template <class T>
    class cube {
      public:
        typedef T argument_type;
        typedef T result_type;
        T operator()(const T& t) const { return t*t*t; }
    };

    template <class T>
    class fourth_power {
      public:
        typedef T argument_type;
        typedef T result_type;
        T operator()(const T& t) const { T t2 = t*t; return t2*t2; }
    };

    // a few shortcuts for common binders

    template <class T>
    class add {
        T y;
      public:
        typedef T argument_type;
        typedef Real result_type;

        explicit add(Real y) : y(y) {}
        Real operator()(T x) const { return x + y; }
    };

    template <class T>
    class subtract {
        T y;
      public:
        typedef T argument_type;
        typedef Real result_type;

        explicit subtract(Real y) : y(y) {}
        Real operator()(T x) const { return x - y; }
    };

    template <class T>
    class subtract_from {
        T y;
      public:
        typedef T argument_type;
        typedef Real result_type;

        explicit subtract_from(Real y) : y(y) {}
        Real operator()(T x) const { return y - x; }
    };

    template <class T>
    class multiply_by {
        T y;
      public:
        typedef T argument_type;
        typedef Real result_type;

        explicit multiply_by(Real y) : y(y) {}
        Real operator()(T x) const { return x * y; }
    };

    template <class T>
    class divide {
        T y;
      public:
        typedef T argument_type;
        typedef Real result_type;

        explicit divide(Real y) : y(y) {}
        Real operator()(T x) const { return y / x; }
    };

    template <class T>
    class divide_by {
        T y;
      public:
        typedef T argument_type;
        typedef Real result_type;

        explicit divide_by(Real y) : y(y) {}
        Real operator()(T x) const { return x / y; }
    };

    template <class T>
    class less_than {
        T y;
      public:
        typedef T argument_type;
        typedef bool result_type;

        explicit less_than(Real y) : y(y) {}
        bool operator()(T x) const { return x < y; }
    };

    template <class T>
    class greater_than {
        T y;
      public:
        typedef T argument_type;
        typedef bool result_type;

        explicit greater_than(Real y) : y(y) {}
        bool operator()(T x) const { return x > y; }
    };

    template <class T>
    class greater_or_equal_to {
        T y;
      public:
        typedef T argument_type;
        typedef bool result_type;

        explicit greater_or_equal_to(Real y) : y(y) {}
        bool operator()(T x) const { return x >= y; }
    };

    template <class T>
    class not_zero {
      public:
        typedef T argument_type;
        typedef bool result_type;
        bool operator()(T x) const { return x != T(); }
    };

    template <class T>
    class not_null {
        T null;
      public:
        typedef T argument_type;
        typedef bool result_type;

        not_null() : null(Null<T>()) {}
        bool operator()(T x) const { return x != null; }
    };
    
    // predicates

    class everywhere : public constant<Real,bool> {
      public:
        everywhere() : constant<Real,bool>(true) {}
    };

    class nowhere : public constant<Real,bool> {
      public:
        nowhere() : constant<Real,bool>(false) {}
    };

    template <class T>
    class equal_within {
      public:
        typedef T first_argument_type;
        typedef T second_argument_type;
        typedef bool result_type;

        explicit equal_within(const T& eps) : eps_(eps) {}
        bool operator()(const T& a, const T& b) const {
            return std::fabs(a-b) <= eps_;
        }
      private:
        const T eps_;
    };

    // combinators
    template <class F, class R>
    class clipped_function {
      public:
        typedef typename F::argument_type argument_type;
        typedef typename F::result_type result_type;
        clipped_function(const F& f, const R& r) : f_(f), r_(r) {}
        result_type operator()(const argument_type& x) const {
            return r_(x) ? f_(x) : result_type();
        }
      private:
        F f_;
        R r_;
    };

    template <class F, class R>
    clipped_function<F,R> clip(const F& f, const R& r) {
        return clipped_function<F,R>(f,r);
    }


    template <class F, class G>
    class composed_function {
      public:
        typedef typename G::argument_type argument_type;
        typedef typename F::result_type result_type;
        composed_function(const F& f, G g) : f_(f), g_(std::move(g)) {}
        result_type operator()(const argument_type& x) const {
            return f_(g_(x));
        }
      private:
        F f_;
        G g_;
    };

    template <class F, class G>
    composed_function<F,G> compose(const F& f, const G& g) {
        return composed_function<F,G>(f,g);
    }

    template <class F, class G, class H>
    class binary_compose3_function {
      public:
        typedef typename G::argument_type first_argument_type;
        typedef typename H::argument_type second_argument_type;
        typedef typename F::result_type result_type;

        binary_compose3_function(const F& f, const G& g, const H& h)
        : f_(f), g_(g), h_(h) {}

        result_type operator()(const first_argument_type&  x,
                               const second_argument_type& y) const {
            return f_(g_(x), h_(y));
        }

      private:
        F f_;
        G g_;
        H h_;
    };

    template <class F, class G, class H> binary_compose3_function<F, G, H>
    compose3(const F& f, const G& g, const H& h) {
        return binary_compose3_function<F, G, H>(f, g, h);
    }
}


#endif


#ifndef id_3f02b76b901d4003d30eaa073f06977f
#define id_3f02b76b901d4003d30eaa073f06977f
inline bool test_3f02b76b901d4003d30eaa073f06977f(int* i) { return i != 0; }
#endif
