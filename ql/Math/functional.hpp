
/*
 Copyright (C) 2003 RiskMap srl

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

/*! \file functional.hpp
    \brief functionals and combinators not included in the STL
*/

#ifndef quantlib_functional_hpp
#define quantlib_functional_hpp

#include <ql/types.hpp>
#include <functional>

namespace QuantLib {

    // functions

    template <class T, class U>
    class constant : public std::unary_function<T,U> {
      public:
        constant(const U& u) : u_(u) {}
        U operator()(const T&) const { return u_; }
      private:
        U u_;
    };

    template <class T>
    class identity : public std::unary_function<T,T> {
      public:
        T operator()(const T& t) const { return t; }
    };

    template <class T>
    class square : public std::unary_function<T,T> {
      public:
        T operator()(const T& t) const { return t*t; }
    };

    template <class T>
    class cube : public std::unary_function<T,T> {
      public:
        T operator()(const T& t) const { return t*t*t; }
    };

    template <class T>
    class fourth_power : public std::unary_function<T,T> {
      public:
        T operator()(const T& t) const { T t2 = t*t; return t2*t2; }
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
        composed_function(const F& f, const G& g) : f_(f), g_(g) {}
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

}


#endif
