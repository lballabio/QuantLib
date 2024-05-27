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

/*! \file commoditycurve.hpp
    \brief Commodity curve
*/

#ifndef quantlib_commodity_curve_hpp
#define quantlib_commodity_curve_hpp

#include <ql/termstructure.hpp>
#include <ql/experimental/commodities/commoditytype.hpp>
#include <ql/experimental/commodities/unitofmeasure.hpp>
#include <ql/experimental/commodities/exchangecontract.hpp>
#include <ql/currency.hpp>
#include <ql/math/interpolations/forwardflatinterpolation.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>

namespace QuantLib {

    //! Commodity term structure
    class CommodityCurve : public TermStructure {
        friend class CommodityIndex;
      public:
        // constructor
        CommodityCurve(std::string name,
                       CommodityType commodityType,
                       Currency currency,
                       UnitOfMeasure unitOfMeasure,
                       const Calendar& calendar,
                       const std::vector<Date>& dates,
                       std::vector<Real> prices,
                       const DayCounter& dayCounter = Actual365Fixed());

        CommodityCurve(std::string name,
                       CommodityType commodityType,
                       Currency currency,
                       UnitOfMeasure unitOfMeasure,
                       const Calendar& calendar,
                       const DayCounter& dayCounter = Actual365Fixed());

        //! \name Inspectors
        //@{
        const std::string& name() const;
        const CommodityType& commodityType() const;
        const UnitOfMeasure& unitOfMeasure() const;
        const Currency& currency() const;
        Date maxDate() const override;
        const std::vector<Time>& times() const;
        const std::vector<Date>& dates() const;
        const std::vector<Real>& prices() const;
        std::vector<std::pair<Date,Real> > nodes() const;
        bool empty() const;

        void setPrices(std::map<Date, Real>& prices);
        void setBasisOfCurve(
                       const ext::shared_ptr<CommodityCurve>& basisOfCurve);

        Real price(
               const Date& d,
               const ext::shared_ptr<ExchangeContracts>& exchangeContracts,
               Integer nearbyOffset) const;
        Real basisOfPrice(const Date& d) const;
        Date underlyingPriceDate(
                const Date& date,
                const ext::shared_ptr<ExchangeContracts>& exchangeContracts,
                Integer nearbyOffset) const;

        const ext::shared_ptr<CommodityCurve>& basisOfCurve() const;

        friend std::ostream& operator<<(std::ostream& out,
                                        const CommodityCurve& curve);
      protected:
        Real basisOfPriceImpl(Time t) const;

        std::string name_;
        CommodityType commodityType_;
        UnitOfMeasure unitOfMeasure_;
        Currency currency_;
        mutable std::vector<Date> dates_;
        mutable std::vector<Time> times_;
        mutable std::vector<Real> data_;
        mutable Interpolation interpolation_;
        ForwardFlat interpolator_;
        ext::shared_ptr<CommodityCurve> basisOfCurve_;
        Real basisOfCurveUomConversionFactor_;

        Real priceImpl(Time t) const;
    };


    // inline definitions

    inline bool operator==(const CommodityCurve& c1, const CommodityCurve& c2) {
        return c1.name() == c2.name();
    }

    inline const CommodityType& CommodityCurve::commodityType() const {
        return commodityType_;
    }

    inline const UnitOfMeasure& CommodityCurve::unitOfMeasure() const {
        return unitOfMeasure_;
    }

    inline const Currency& CommodityCurve::currency() const {
        return currency_;
    }

    inline const std::string& CommodityCurve::name() const {
        return name_;
    }

    inline Date CommodityCurve::maxDate() const {
        return dates_.back();
    }

    inline const std::vector<Time>& CommodityCurve::times() const {
        return times_;
    }

    inline const std::vector<Date>& CommodityCurve::dates() const {
        return dates_;
    }

    inline const std::vector<Real>& CommodityCurve::prices() const {
        return data_;
    }

    inline bool CommodityCurve::empty() const {
        return dates_.empty();
    }

    inline const ext::shared_ptr<CommodityCurve>&
    CommodityCurve::basisOfCurve() const {
        return basisOfCurve_;
    }

    inline std::vector<std::pair<Date,Real> > CommodityCurve::nodes() const {
        std::vector<std::pair<Date,Real> > results(dates_.size());
        for (Size i = 0; i < dates_.size(); ++i)
            results[i] = std::make_pair(dates_[i], data_[i]);
        return results;
    }

    inline Real CommodityCurve::basisOfPrice(const Date& d) const {
        Time t = timeFromReference(d);
        return basisOfPriceImpl(t);
    }

    // gets a price that can include an arbitrary number of basis curves
    inline Real CommodityCurve::price(
                const Date& d,
                const ext::shared_ptr<ExchangeContracts>& exchangeContracts,
                Integer nearbyOffset) const {
        Date date = nearbyOffset > 0 ?
            underlyingPriceDate(d, exchangeContracts, nearbyOffset) : d;
        Time t = timeFromReference(date);
        Real priceValue = 0;
        try {
            priceValue = priceImpl(t);
        } catch (const std::exception& e) {
            QL_FAIL("error retrieving price for curve [" << name() << "]: "
                    << e.what());
        }
        return priceValue + basisOfPriceImpl(t);
    }

    // get the date for the underlying price, in the case of nearby
    // curves, rolls on the underlying contract expiry
    inline Date CommodityCurve::underlyingPriceDate(
                const Date& date,
                const ext::shared_ptr<ExchangeContracts>& exchangeContracts,
                Integer nearbyOffset) const {
        QL_REQUIRE(nearbyOffset > 0, "nearby offset must be > 0");
        ExchangeContracts::const_iterator ic =
            exchangeContracts->lower_bound(date);
        if (ic != exchangeContracts->end()) {
            for (int i = 0; i < nearbyOffset-1 && ic!=exchangeContracts->end(); ++i)
                ++ic;
            QL_REQUIRE(ic != exchangeContracts->end(),
                       "not enough nearby contracts available for curve ["
                       << name() << "] for date [" << date << "].");
            return ic->second.underlyingStartDate();
        }
        return date;
    }

    inline Real CommodityCurve::basisOfPriceImpl(Time t) const {
        if (basisOfCurve_ != nullptr) {
            Real basisCurvePriceValue = 0;
            try {
                basisCurvePriceValue =
                    basisOfCurve_->priceImpl(t)
                    * basisOfCurveUomConversionFactor_;
            } catch (const std::exception& e) {
                QL_FAIL("error retrieving price for curve [" << name() <<
                        "]: " << e.what());
            }
            return basisCurvePriceValue + basisOfCurve_->basisOfPriceImpl(t);
        }
        return 0;
    }

    inline Real CommodityCurve::priceImpl(Time t) const {
        return interpolation_(t, true);
    }

}


#endif
