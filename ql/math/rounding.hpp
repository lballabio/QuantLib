/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 Decillion Pty(Ltd)

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

/*! \file rounding.hpp
    \brief Rounding implementation
*/

#ifndef quantlib_rounding_hpp
#define quantlib_rounding_hpp

#include <ql/types.hpp>

namespace QuantLib {

    //! basic rounding class
    /*! \test the correctness of the returned values is tested by
              checking them against known good results.
    */
    class Rounding {
      public:
        //! rounding methods
        /*! The rounding methods follow the OMG specification available
            at <http://www.omg.org/cgi-bin/doc?formal/00-06-29.pdf>.

            \warning the names of the Floor and Ceiling methods might
                     be misleading. Check the provided reference.
        */
        enum Type {
            None,    /*!< do not round: return the number unmodified */
            Up,      /*!< the first decimal place past the precision will be
                          rounded up. This differs from the OMG rule which
                          rounds up only if the decimal to be rounded is
                          greater than or equal to the rounding digit */
            Down,    /*!< all decimal places past the precision will be
                          truncated */
            Closest, /*!< the first decimal place past the precision
                          will be rounded up if greater than or equal
                          to the rounding digit; this corresponds to
                          the OMG round-up rule.  When the rounding
                          digit is 5, the result will be the one
                          closest to the original number, hence the
                          name. */
            Floor,   /*!< positive numbers will be rounded up and negative
                          numbers will be rounded down using the OMG round up
                          and round down rules */
            Ceiling  /*!< positive numbers will be rounded down and negative
                          numbers will be rounded up using the OMG round up
                          and round down rules */
        };
        //! default constructor
        /*! Instances built through this constructor don't perform
            any rounding.
        */
        Rounding() = default;
        explicit Rounding(Integer precision,
                          Type type = Closest,
                          Integer digit = 5)
        : precision_(precision), type_(type), digit_(digit) {}
        //! perform rounding
        Decimal operator()(Decimal value) const;
        //! \name Inspectors
        //@{
        Integer precision() const { return precision_; }
        Type type() const { return type_; }
        Integer roundingDigit() const { return digit_; }
      private:
        Integer precision_;
        Type type_ = None;
        Integer digit_;
    };


    //! Up-rounding.
    class UpRounding : public Rounding {
      public:
        explicit UpRounding(Integer precision,
                            Integer digit = 5)
        : Rounding(precision,Up,digit) {}
    };

    //! Down-rounding.
    class DownRounding : public Rounding {
      public:
        explicit DownRounding(Integer precision,
                              Integer digit = 5)
        : Rounding(precision,Down,digit) {}
    };

    //! Closest rounding.
    class ClosestRounding : public Rounding {
      public:
        explicit ClosestRounding(Integer precision,
                                 Integer digit = 5)
        : Rounding(precision,Closest,digit) {}
    };

    //! Ceiling truncation.
    class CeilingTruncation : public Rounding {
      public:
        explicit CeilingTruncation(Integer precision,
                                   Integer digit = 5)
        : Rounding(precision,Ceiling,digit) {}
    };

    //! %Floor truncation.
    class FloorTruncation : public Rounding {
      public:
        explicit FloorTruncation(Integer precision,
                                 Integer digit = 5)
        : Rounding(precision,Floor,digit) {}
    };

}


#endif


#ifndef id_7211db6f2f3e0e7ef16091381b52de69
#define id_7211db6f2f3e0e7ef16091381b52de69
inline bool test_7211db6f2f3e0e7ef16091381b52de69(int* i) { return i != 0; }
#endif
