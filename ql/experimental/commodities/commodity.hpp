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

/*! \file commodity.hpp
    \brief Commodity base class
*/

#ifndef quantlib_commodity_hpp
#define quantlib_commodity_hpp

#include <ql/instrument.hpp>
#include <ql/money.hpp>
#include <iosfwd>
#include <utility>
#include <vector>

namespace QuantLib {

    typedef std::map<std::string, boost::any> SecondaryCosts;
    typedef std::map<std::string, Money> SecondaryCostAmounts;

    std::ostream& operator<<(std::ostream& out,
                             const SecondaryCostAmounts& secondaryCostAmounts);


    struct PricingError {
        enum Level { Info, Warning, Error, Fatal };

        Level errorLevel;
        std::string tradeId;
        std::string error;
        std::string detail;

        PricingError(Level errorLevel, std::string error, std::string detail)
        : errorLevel(errorLevel), error(std::move(error)), detail(std::move(detail)) {}
    };

    typedef std::vector<PricingError> PricingErrors;

    std::ostream& operator<<(std::ostream& out, const PricingError& error);
    std::ostream& operator<<(std::ostream& out, const PricingErrors& errors);


    //! Commodity base class
    /*! \ingroup instruments */
    class Commodity : public Instrument {
      public:
        explicit Commodity(ext::shared_ptr<SecondaryCosts> secondaryCosts);
        const ext::shared_ptr<SecondaryCosts>& secondaryCosts() const;
        const SecondaryCostAmounts& secondaryCostAmounts() const;
        const PricingErrors& pricingErrors() const;
        void addPricingError(PricingError::Level errorLevel,
                             const std::string& error,
                             const std::string& detail = "") const;
      protected:
        ext::shared_ptr<SecondaryCosts> secondaryCosts_;
        mutable PricingErrors pricingErrors_;
        mutable SecondaryCostAmounts secondaryCostAmounts_;
    };

}

#endif


#ifndef id_7eed2446ef0fc6743598d2d783c01f87
#define id_7eed2446ef0fc6743598d2d783c01f87
inline bool test_7eed2446ef0fc6743598d2d783c01f87(const int* i) {
    return i != nullptr;
}
#endif
