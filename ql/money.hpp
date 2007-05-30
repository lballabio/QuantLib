/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 Decillion Pty(Ltd)
 Copyright (C) 2004, 2005, 2006 StatPro Italia srl

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

/*! \file money.hpp
    \brief cash amount in a given currency
*/

#ifndef quantlib_money_hpp
#define quantlib_money_hpp

#include <ql/currency.hpp>

namespace QuantLib {

    //! amount of cash
    /*! \test money arithmetic is tested with and without currency
              conversions.
    */
    class Money {
      public:
        //! \name Constructors
        //@{
        Money();
        Money(const Currency& currency, Decimal value);
        Money(Decimal value, const Currency& currency);
        //@}
        //! \name Inspectors
        //@{
        const Currency& currency() const;
        Decimal value() const;
        Money rounded() const;
        //@}
        /*! \name Money arithmetics

            See below for non-member functions and for settings which
            determine the behavior of the operators.
        */
        //@{
        Money operator+() const;
        Money operator-() const;
        Money& operator+=(const Money&);
        Money& operator-=(const Money&);
        Money& operator*=(Decimal);
        Money& operator/=(Decimal);
        //@}
        /*! \name Conversion settings

            These parameters are used for combining money amounts
            in different currencies
        */
        //@{
        enum ConversionType {
            NoConversion,           /*!< do not perform conversions */
            BaseCurrencyConversion, /*!< convert both operands to
                                         the base currency before
                                         converting */
            AutomatedConversion     /*!< return the result in the
                                         currency of the first
                                         operand */
        };
        static ConversionType conversionType;
        static Currency baseCurrency;
        //@}
      private:
        Decimal value_;
        Currency currency_;
    };


    // More arithmetics and comparisons

    /*! \relates Money */
    Money operator+(const Money&, const Money&);
    /*! \relates Money */
    Money operator-(const Money&, const Money&);
    /*! \relates Money */
    Money operator*(const Money&, Decimal);
    /*! \relates Money */
    Money operator*(Decimal, const Money&);
    /*! \relates Money */
    Money operator/(const Money&, Decimal);
    /*! \relates Money */
    Decimal operator/(const Money&, const Money&);

    /*! \relates Money */
    bool operator==(const Money&, const Money&);
    /*! \relates Money */
    bool operator!=(const Money&, const Money&);
    /*! \relates Money */
    bool operator<(const Money&, const Money&);
    /*! \relates Money */
    bool operator<=(const Money&, const Money&);
    /*! \relates Money */
    bool operator>(const Money&, const Money&);
    /*! \relates Money */
    bool operator>=(const Money&, const Money&);

    /*! \relates Money */
    bool close(const Money&, const Money&, Size n = 42);
    /*! \relates Money */
    bool close_enough(const Money&, const Money&, Size n = 42);

    // syntactic sugar

    /*! \relates Money */
    Money operator*(Decimal, const Currency&);
    /*! \relates Money */
    Money operator*(const Currency&, Decimal);

    // formatting

    /*! \relates Money */
    std::ostream& operator<<(std::ostream&, const Money&);


    // inline definitions

    inline Money::Money()
    : value_(0.0) {}

    inline Money::Money(const Currency& currency, Decimal value)
    : value_(value), currency_(currency) {}

    inline Money::Money(Decimal value, const Currency& currency)
    : value_(value), currency_(currency) {}

    inline const Currency& Money::currency() const {
        return currency_;
    }

    inline Decimal Money::value() const {
        return value_;
    }

    inline Money Money::rounded() const {
        return Money(currency_.rounding()(value_), currency_);
    }

    inline Money Money::operator+() const {
        return *this;
    }

    inline Money Money::operator-() const {
        return Money(-value_, currency_);
    }

    inline Money& Money::operator*=(Decimal x) {
        value_ *= x;
        return *this;
    }

    inline Money& Money::operator/=(Decimal x) {
        value_ /= x;
        return *this;
    }


    inline Money operator+(const Money& m1, const Money& m2) {
        Money tmp = m1;
        tmp += m2;
        return tmp;
    }

    inline Money operator-(const Money& m1, const Money& m2) {
        Money tmp = m1;
        tmp -= m2;
        return tmp;
    }

    inline Money operator*(const Money& m, Decimal x) {
        Money tmp = m;
        tmp *= x;
        return tmp;
    }

    inline Money operator*(Decimal x, const Money& m) {
        return m*x;
    }

    inline Money operator/(const Money& m, Decimal x) {
        Money tmp = m;
        tmp /= x;
        return tmp;
    }

    inline bool operator!=(const Money& m1, const Money& m2) {
        return !(m1 == m2);
    }

    inline bool operator>(const Money& m1, const Money& m2) {
        return m2 < m1;
    }

    inline bool operator>=(const Money& m1, const Money& m2) {
        return m2 <= m1;
    }

    inline Money operator*(Decimal value, const Currency& c) {
        return Money(value,c);
    }

    inline Money operator*(const Currency& c, Decimal value) {
        return Money(value,c);
    }

}


#endif
