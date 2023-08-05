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

#include <ql/experimental/commodities/commodity.hpp>
#include <iomanip>
#include <utility>

namespace QuantLib {

    Commodity::Commodity(ext::shared_ptr<SecondaryCosts> secondaryCosts)
    : secondaryCosts_(std::move(secondaryCosts)) {}

    const SecondaryCostAmounts& Commodity::secondaryCostAmounts() const {
        return secondaryCostAmounts_;
    }

    const PricingErrors& Commodity::pricingErrors() const {
        return pricingErrors_;
    }

    void Commodity::addPricingError(PricingError::Level errorLevel,
                                    const std::string& error,
                                    const std::string& detail) const {
        pricingErrors_.emplace_back(errorLevel, error, detail);
    }


    std::ostream& operator<<(std::ostream& out,
                             const SecondaryCostAmounts& secondaryCostAmounts) {
        std::string currencyCode;
        Real totalAmount = 0;

        out << "secondary costs" << std::endl;
        for (const auto& secondaryCostAmount : secondaryCostAmounts) {
            Real amount = secondaryCostAmount.second.value();
            if (currencyCode.empty())
                currencyCode = secondaryCostAmount.second.currency().code();
            totalAmount += amount;
            out << std::setw(28) << std::left << secondaryCostAmount.first << std::setw(12)
                << std::right << std::fixed << std::setprecision(2) << amount << " " << currencyCode
                << std::endl;
        }
        out << std::setw(28) << std::left << "total"
            << std::setw(12) << std::right << std::fixed
            << std::setprecision(2) << totalAmount << " " << currencyCode
            << std::endl;
        return out;
    }


    std::ostream& operator<<(std::ostream& out, const PricingError& error) {
        switch (error.errorLevel) {
          case PricingError::Info:
            out << "info: ";
            break;
          case PricingError::Warning:
            out << "warning: ";
            break;
          case PricingError::Error:
            out << "*** error: ";
            break;
          case PricingError::Fatal:
            out << "*** fatal: ";
            break;
        }
        out << error.error;
        if (!error.detail.empty())
            out << ": " << error.detail;
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const PricingErrors& errors) {
        if (!errors.empty()) {
            out << "*** pricing errors" << std::endl;
            for (const auto& error : errors)
                out << error << std::endl;
        }
        return out;
    }

}

