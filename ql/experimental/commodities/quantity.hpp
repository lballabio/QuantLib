/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 J. Erik Radmall

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

/*! \file quantity.hpp
    \brief Amount of a commodity
*/

#ifndef quantlib_quantity_hpp
#define quantlib_quantity_hpp

#include <ql/experimental/commodities/commoditytype.hpp>
#include <ql/experimental/commodities/unitofmeasure.hpp>
#include <utility>

namespace QuantLib {

    //! Amount of a commodity
    class Quantity {
      public:
        //! \name Constructors
        //@{
        Quantity() = default;
        Quantity(CommodityType commodityType, UnitOfMeasure unitOfMeasure, Real amount);
        //@}
        //! \name Inspectors
        //@{
        const CommodityType& commodityType() const;
        const UnitOfMeasure& unitOfMeasure() const;
        Real amount() const;
        Quantity rounded() const;
        //@}
        /*! \name Quantity arithmetics

            See below for non-member functions and for settings which
            determine the behavior of the operators.
        */
        //@{
        Quantity operator+() const;
        Quantity operator-() const;
        Quantity& operator+=(const Quantity&);
        Quantity& operator-=(const Quantity&);
        Quantity& operator*=(Real);
        Quantity& operator/=(Real);
        //@}
        /*! \name Conversion settings

            These parameters are used for combining quantity amounts
            in different currencies
        */
        //@{
        enum ConversionType {
            NoConversion,                /*!< do not perform conversions */
            BaseUnitOfMeasureConversion, /*!< convert both operands to
                                              the base unitOfMeasure before
                                              converting */
            AutomatedConversion          /*!< return the result in the
                                              unitOfMeasure of the first
                                              operand */
        };
        static ConversionType conversionType;
        static UnitOfMeasure baseUnitOfMeasure;
        //@}

        friend std::ostream& operator<<(std::ostream&, const Quantity&);
      private:
        CommodityType commodityType_;
        UnitOfMeasure unitOfMeasure_;
        Real amount_ = 0.0;
    };


    // More arithmetics and comparisons

    /*! \relates Quantity */
    Quantity operator+(const Quantity&, const Quantity&);
    /*! \relates Quantity */
    Quantity operator-(const Quantity&, const Quantity&);
    /*! \relates Quantity */
    Quantity operator*(const Quantity&, Real);
    /*! \relates Quantity */
    Quantity operator*(Real, const Quantity&);
    /*! \relates Quantity */
    Quantity operator/(const Quantity&, Real);
    /*! \relates Quantity */
    Real operator/(const Quantity&, const Quantity&);

    /*! \relates Quantity */
    bool operator==(const Quantity&, const Quantity&);
    /*! \relates Quantity */
    bool operator!=(const Quantity&, const Quantity&);
    /*! \relates Quantity */
    bool operator<(const Quantity&, const Quantity&);
    /*! \relates Quantity */
    bool operator<=(const Quantity&, const Quantity&);
    /*! \relates Quantity */
    bool operator>(const Quantity&, const Quantity&);
    /*! \relates Quantity */
    bool operator>=(const Quantity&, const Quantity&);

    /*! \relates Quantity */
    bool close(const Quantity&, const Quantity&, Size n = 42);
    /*! \relates Quantity */
    bool close_enough(const Quantity&, const Quantity&, Size n = 42);


    // inline definitions

    inline Quantity::Quantity(CommodityType commodityType, UnitOfMeasure unitOfMeasure, Real amount)
    : commodityType_(std::move(commodityType)), unitOfMeasure_(std::move(unitOfMeasure)),
      amount_(amount) {}

    inline const CommodityType& Quantity::commodityType() const {
        return commodityType_;
    }

    inline const UnitOfMeasure& Quantity::unitOfMeasure() const {
        return unitOfMeasure_;
    }

    inline Real Quantity::amount() const {
        return amount_;
    }

    inline Quantity Quantity::rounded() const {
        return Quantity(commodityType_,
                        unitOfMeasure_,
                        unitOfMeasure_.rounding()(amount_));
    }

    inline Quantity Quantity::operator+() const {
        return *this;
    }

    inline Quantity Quantity::operator-() const {
        return Quantity(commodityType_, unitOfMeasure_, -amount_);
    }

    inline Quantity& Quantity::operator*=(Real x) {
        amount_ *= x;
        return *this;
    }

    inline Quantity& Quantity::operator/=(Real x) {
        amount_ /= x;
        return *this;
    }


    inline Quantity operator+(const Quantity& m1, const Quantity& m2) {
        Quantity tmp = m1;
        tmp += m2;
        return tmp;
    }

    inline Quantity operator-(const Quantity& m1, const Quantity& m2) {
        Quantity tmp = m1;
        tmp -= m2;
        return tmp;
    }

    inline Quantity operator*(const Quantity& m, Real x) {
        Quantity tmp = m;
        tmp *= x;
        return tmp;
    }

    inline Quantity operator*(Real x, const Quantity& m) {
        return m*x;
    }

    inline Quantity operator/(const Quantity& m, Real x) {
        Quantity tmp = m;
        tmp /= x;
        return tmp;
    }

    inline bool operator!=(const Quantity& m1, const Quantity& m2) {
        return !(m1 == m2);
    }

    inline bool operator>(const Quantity& m1, const Quantity& m2) {
        return m2 < m1;
    }

    inline bool operator>=(const Quantity& m1, const Quantity& m2) {
        return m2 <= m1;
    }

}


#endif
