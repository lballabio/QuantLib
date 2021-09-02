/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

/*! \file moneysettings.hpp
    \brief money settings for the session
*/

#include <ql/moneysettings.hpp>

namespace QuantLib {

    MoneySettings::MoneySettings() :
        conversionType_(Money::NoConversion),
        baseCurrency_(Currency()) {
    }

    MoneySettings::MoneySettings(const MoneySettings & other) :
        conversionType_(other.conversionType_),
        baseCurrency_(other.baseCurrency_) {
    }

    MoneySettings & MoneySettings::operator=(const MoneySettings & other) {
        conversionType_ = other.conversionType_;
        baseCurrency_ = other.baseCurrency_;
        return *this;
    }

    const Money::ConversionType & MoneySettings::conversionType() const {
        return conversionType_;
    }

    Money::ConversionType & MoneySettings::conversionType() {
        return conversionType_;
    }

    const Currency & MoneySettings::baseCurrency() const {
        return baseCurrency_;
    }

    Currency & MoneySettings::baseCurrency() {
        return baseCurrency_;
    }

}
