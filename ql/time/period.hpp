/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2005, 2006, 2007, 2008 Ferdinando Ametrano
 Copyright (C) 2006 Katiuscia Manzoni
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006, 2007 StatPro Italia srl

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

/*! \file period.hpp
    \brief period- and frequency-related classes and enumerations
*/

#ifndef quantlib_period_hpp
#define quantlib_period_hpp

#include <ql/time/frequency.hpp>
#include <ql/time/timeunit.hpp>
#include <ql/types.hpp>


namespace QuantLib {

    /*! This class provides a Period (length + TimeUnit) class
        and implements a limited algebra.

        \ingroup datetime

        \test self-consistency of algebra is checked.
    */
    class Period {
      public:
        Period() = default;
        Period(Integer n, TimeUnit units)
        : length_(n), units_(units) {}
        explicit Period(Frequency f);
        Integer length() const { return length_; }
        TimeUnit units() const { return units_; }
        Frequency frequency() const;
        Period& operator+=(const Period&);
        Period& operator-=(const Period&);
        Period& operator*=(Integer);
        Period& operator/=(Integer);
        void normalize();
        Period normalized() const;
      private:
        Integer length_ = 0;
        TimeUnit units_ = Days;
    };

    /*! \relates Period */
    Real years(const Period&);
    /*! \relates Period */
    Real months(const Period&);
    /*! \relates Period */
    Real weeks(const Period&);
    /*! \relates Period */
    Real days(const Period&);

    /*! \relates Period */
    template <typename T> Period operator*(T n, TimeUnit units);
    /*! \relates Period */
    template <typename T> Period operator*(TimeUnit units, T n);

    /*! \relates Period */
    Period operator-(const Period&);

    /*! \relates Period */
    Period operator*(Integer n, const Period&);
    /*! \relates Period */
    Period operator*(const Period&, Integer n);

    /*! \relates Period */
    Period operator/(const Period&, Integer n);

    /*! \relates Period */
    Period operator+(const Period&, const Period&);
    /*! \relates Period */
    Period operator-(const Period&, const Period&);

    /*! \relates Period */
    bool operator<(const Period&, const Period&);
    /*! \relates Period */
    bool operator==(const Period&, const Period&);
    /*! \relates Period */
    bool operator!=(const Period&, const Period&);
    /*! \relates Period */
    bool operator>(const Period&, const Period&);
    /*! \relates Period */
    bool operator<=(const Period&, const Period&);
    /*! \relates Period */
    bool operator>=(const Period&, const Period&);

    /*! \relates Period */
    std::ostream& operator<<(std::ostream&, const Period&);

    namespace detail {

        struct long_period_holder {
            explicit long_period_holder(const Period& p) : p(p) {}
            Period p;
        };
        std::ostream& operator<<(std::ostream&, const long_period_holder&);

        struct short_period_holder {
            explicit short_period_holder(Period p) : p(p) {}
            Period p;
        };
        std::ostream& operator<<(std::ostream&, const short_period_holder&);

    }

    namespace io {

        //! output periods in long format (e.g. "2 weeks")
        /*! \ingroup manips */
        detail::long_period_holder long_period(const Period&);

        //! output periods in short format (e.g. "2w")
        /*! \ingroup manips */
        detail::short_period_holder short_period(const Period&);

    }

    // inline definitions

    inline Period Period::normalized() const {
        Period p = *this;
        p.normalize();
        return p;
    }

    template <typename T>
    inline Period operator*(T n, TimeUnit units) {
        return {Integer(n), units};
    }

    template <typename T>
    inline Period operator*(TimeUnit units, T n) {
        return {Integer(n), units};
    }

    inline Period operator-(const Period& p) { return {-p.length(), p.units()}; }

    inline Period operator*(Integer n, const Period& p) { return {n * p.length(), p.units()}; }

    inline Period operator*(const Period& p, Integer n) { return {n * p.length(), p.units()}; }

    inline bool operator==(const Period& p1, const Period& p2) {
        return !(p1 < p2 || p2 < p1);
    }

    inline bool operator!=(const Period& p1, const Period& p2) {
        return !(p1 == p2);
    }

    inline bool operator>(const Period& p1, const Period& p2) {
        return p2 < p1;
    }

    inline bool operator<=(const Period& p1, const Period& p2) {
        return !(p1 > p2);
    }

    inline bool operator>=(const Period& p1, const Period& p2) {
        return !(p1 < p2);
    }

}

#endif
