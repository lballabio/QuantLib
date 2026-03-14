/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 J. Erik Radmall

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/experimental/commodities/quantity.hpp>
#include <ql/experimental/commodities/unitofmeasureconversionmanager.hpp>
#include <ql/math/comparison.hpp>
#include <ql/errors.hpp>

namespace QuantLib {

    Quantity::ConversionType Quantity::conversionType = Quantity::NoConversion;

    UnitOfMeasure Quantity::baseUnitOfMeasure = UnitOfMeasure();

    namespace {

        void convertTo(Quantity& m, const UnitOfMeasure& target) {
            if (m.unitOfMeasure() != target) {
                UnitOfMeasureConversion rate =
                    UnitOfMeasureConversionManager::instance().lookup(
                                m.commodityType(), m.unitOfMeasure(), target);
                m = rate.convert(m).rounded();
            }
        }

        void convertToBase(Quantity& m) {
            QL_REQUIRE(!Quantity::baseUnitOfMeasure.empty(),
                       "no base unitOfMeasure set");
            convertTo(m, Quantity::baseUnitOfMeasure);
        }

    }

    Quantity& Quantity::operator+=(const Quantity& m) {
        if (unitOfMeasure_ == m.unitOfMeasure_) {
            amount_ += m.amount_;
        } else if (conversionType == BaseUnitOfMeasureConversion) {
            convertToBase(*this);
            Quantity tmp = m;
            convertToBase(tmp);
            *this += tmp;
        } else if (conversionType == AutomatedConversion) {
            Quantity tmp = m;
            convertTo(tmp, unitOfMeasure_);
            *this += tmp;
        } else {
            QL_FAIL("unitOfMeasure mismatch and no conversion specified");
        }
        return *this;
    }

    Quantity& Quantity::operator-=(const Quantity& m) {
        if (unitOfMeasure_ == m.unitOfMeasure_) {
            amount_ -= m.amount_;
        } else if (conversionType == BaseUnitOfMeasureConversion) {
            convertToBase(*this);
            Quantity tmp = m;
            convertToBase(tmp);
            *this -= tmp;
        } else if (conversionType == AutomatedConversion) {
            Quantity tmp = m;
            convertTo(tmp, unitOfMeasure_);
            *this -= tmp;
        } else {
            QL_FAIL("unitOfMeasure mismatch and no conversion specified");
        }
        return *this;
    }

    Real operator/(const Quantity& m1, const Quantity& m2) {
        if (m1.unitOfMeasure() == m2.unitOfMeasure()) {
            return m1.amount()/m2.amount();
        } else if (Quantity::conversionType
                   == Quantity::BaseUnitOfMeasureConversion) {
            Quantity tmp1 = m1;
            convertToBase(tmp1);
            Quantity tmp2 = m2;
            convertToBase(tmp2);
            return tmp1/tmp2;
        } else if (Quantity::conversionType == Quantity::AutomatedConversion) {
            Quantity tmp = m2;
            convertTo(tmp, m1.unitOfMeasure());
            return m1/tmp;
        } else {
            QL_FAIL("unitOfMeasure mismatch and no conversion specified");
        }
    }

    bool operator==(const Quantity& m1, const Quantity& m2) {
        if (m1.unitOfMeasure() == m2.unitOfMeasure()) {
            return m1.amount() == m2.amount();
        } else if (Quantity::conversionType
                   == Quantity::BaseUnitOfMeasureConversion) {
            Quantity tmp1 = m1;
            convertToBase(tmp1);
            Quantity tmp2 = m2;
            convertToBase(tmp2);
            return tmp1 == tmp2;
        } else if (Quantity::conversionType
                   == Quantity::AutomatedConversion) {
            Quantity tmp = m2;
            convertTo(tmp, m1.unitOfMeasure());
            return m1 == tmp;
        } else {
            QL_FAIL("unitOfMeasure mismatch and no conversion specified");
        }
    }

    bool operator<(const Quantity& m1, const Quantity& m2) {
        if (m1.unitOfMeasure() == m2.unitOfMeasure()) {
            return m1.amount() < m2.amount();
        } else if (Quantity::conversionType
                   == Quantity::BaseUnitOfMeasureConversion) {
            Quantity tmp1 = m1;
            convertToBase(tmp1);
            Quantity tmp2 = m2;
            convertToBase(tmp2);
            return tmp1 < tmp2;
        } else if (Quantity::conversionType == Quantity::AutomatedConversion) {
            Quantity tmp = m2;
            convertTo(tmp, m1.unitOfMeasure());
            return m1 < tmp;
        } else {
            QL_FAIL("unitOfMeasure mismatch and no conversion specified");
        }
    }

    bool operator<=(const Quantity& m1, const Quantity& m2) {
        if (m1.unitOfMeasure() == m2.unitOfMeasure()) {
            return m1.amount() <= m2.amount();
        } else if (Quantity::conversionType
                   == Quantity::BaseUnitOfMeasureConversion) {
            Quantity tmp1 = m1;
            convertToBase(tmp1);
            Quantity tmp2 = m2;
            convertToBase(tmp2);
            return tmp1 <= tmp2;
        } else if (Quantity::conversionType == Quantity::AutomatedConversion) {
            Quantity tmp = m2;
            convertTo(tmp, m1.unitOfMeasure());
            return m1 <= tmp;
        } else {
            QL_FAIL("unitOfMeasure mismatch and no conversion specified");
        }
    }

    bool close(const Quantity& m1, const Quantity& m2, Size n) {
        if (m1.unitOfMeasure() == m2.unitOfMeasure()) {
            return close(m1.amount(),m2.amount(),n);
        } else if (Quantity::conversionType
                   == Quantity::BaseUnitOfMeasureConversion) {
            Quantity tmp1 = m1;
            convertToBase(tmp1);
            Quantity tmp2 = m2;
            convertToBase(tmp2);
            return close(tmp1,tmp2,n);
        } else if (Quantity::conversionType == Quantity::AutomatedConversion) {
            Quantity tmp = m2;
            convertTo(tmp, m1.unitOfMeasure());
            return close(m1,tmp,n);
        } else {
            QL_FAIL("unitOfMeasure mismatch and no conversion specified");
        }
    }

    bool close_enough(const Quantity& m1, const Quantity& m2, Size n) {
        if (m1.unitOfMeasure() == m2.unitOfMeasure()) {
            return close_enough(m1.amount(),m2.amount(),n);
        } else if (Quantity::conversionType
                   == Quantity::BaseUnitOfMeasureConversion) {
            Quantity tmp1 = m1;
            convertToBase(tmp1);
            Quantity tmp2 = m2;
            convertToBase(tmp2);
            return close_enough(tmp1,tmp2,n);
        } else if (Quantity::conversionType == Quantity::AutomatedConversion) {
            Quantity tmp = m2;
            convertTo(tmp, m1.unitOfMeasure());
            return close_enough(m1,tmp,n);
        } else {
            QL_FAIL("unitOfMeasure mismatch and no conversion specified");
        }
    }


    std::ostream& operator<<(std::ostream& out, const Quantity& quantity) {
        return out << quantity.commodityType_.code() << " "
                   << quantity.amount_ << " " << quantity.unitOfMeasure_.code();
    }

}

