
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file expressiontemplates.hpp
    \brief expression template implementation
*/

// $Id$

#ifndef quantlib_expression_templates_h
#define quantlib_expression_templates_h

#include <ql/qldefines.hpp>

/* The concepts implemented here are from
   T. L. Veldhuizen, "Expression templates", C++ Report, 7(5):26-31, June 1995
   http://extreme.indiana.edu/~tveldhui/papers/
*/

namespace QuantLib {

    // iterator on generic vectorial expression

    template <class Iter>
    class VectorialExpression {
      public:
        VectorialExpression(const Iter& i, int size) : i(i), n(size) {}
        double operator*() const { return *i; }
        const VectorialExpression& operator++() const { ++i; return *this; }
        // pre-increment operator not defined - inefficient
        int size() const { return n; }
      private:
        mutable Iter i;
        int n;
    };

    // dummy iterator on scalar

    class Scalar {
      public:
        Scalar(double x) : x(x) {}
        double operator*() const { return x; }
        const Scalar& operator++() const { return *this; }
        // pre-increment operator not defined - inefficient
      private:
        double x;
    };

    // iterator on unary vectorial expression

    template <class Iter1, class Operation>
    class UnaryVectorialExpression {
      public:
        UnaryVectorialExpression(const Iter1& i, int size) : i(i), n(size) {}
        double operator*() const { return Operation::apply(*i); }
        const UnaryVectorialExpression& operator++() const { ++i; return *this; }
        // pre-increment operator not defined - inefficient
        int size() const { return n; }
      private:
        mutable Iter1 i;
        int n;
    };

    // iterator on binary vectorial expression

    template <class Iter1, class Iter2, class Operation>
    class BinaryVectorialExpression {
      public:
        BinaryVectorialExpression(const Iter1& i, const Iter2& j, int size)
        : i(i), j(j), n(size) {}
        double operator*() const { return Operation::apply(*i,*j); }
        const BinaryVectorialExpression& operator++() const {
            ++i; ++j; return *this; }
        // pre-increment operator not defined - inefficient
        int size() const { return n; }
      private:
        mutable Iter1 i;
        mutable Iter2 j;
        int n;
    };


    // unary operations

    class Plus {
      public:
        static inline double apply(double a) { return +a; }
    };

    class Minus {
      public:
        static inline double apply(double a) { return -a; }
    };


    // binary operations

    class Add {
      public:
        static inline double apply(double a, double b) { return a+b; }
    };

    class Subtract {
      public:
        static inline double apply(double a, double b) { return a-b; }
    };

    class Multiply {
      public:
        static inline double apply(double a, double b) { return a*b; }
    };

    class Divide {
      public:
        static inline double apply(double a, double b) { return a/b; }
    };


    // functions

    class AbsoluteValue {
      public:
        static inline double apply(double a) { return QL_FABS(a); }
    };

    class SquareRoot {
      public:
        static inline double apply(double a) { return QL_SQRT(a); }
    };

    class Sine {
      public:
        static inline double apply(double a) { return QL_SIN(a); }
    };

    class Cosine {
      public:
        static inline double apply(double a) { return QL_COS(a); }
    };

    class Logarithm {
      public:
        static inline double apply(double a) { return QL_LOG(a); }
    };

    class Exponential {
      public:
        static inline double apply(double a) { return QL_EXP(a); }
    };

}


#endif
