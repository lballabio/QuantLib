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

/*! \file commodityindex.hpp
    \brief Commodity index
*/

#ifndef quantlib_commodity_index_hpp
#define quantlib_commodity_index_hpp

#include <ql/experimental/commodities/commoditycurve.hpp>
#include <ql/index.hpp>

namespace QuantLib {

    class TermStructure;

    //! base class for commodity indexes
    class CommodityIndex : public Index {
      public:
        CommodityIndex(std::string  name,
                       CommodityType commodityType,
                       Currency currency,
                       UnitOfMeasure unitOfMeasure,
                       Calendar calendar,
                       Real lotQuantity,
                       ext::shared_ptr<CommodityCurve> forwardCurve,
                       ext::shared_ptr<ExchangeContracts> exchangeContracts,
                       int nearbyOffset);
        //! \name Index interface
        //@{
        std::string name() const override;
        Calendar fixingCalendar() const override;
        bool isValidFixingDate(const Date& fixingDate) const override;
        Real fixing(const Date& fixingDate,
                    bool forecastTodaysFixing = false) const override;
        //@}
        //! \name Observer interface
        //@{
        void update() override;
        //@}
        //! \name Inspectors
        //@{
        const CommodityType& commodityType() const;
        const Currency& currency() const;
        const UnitOfMeasure& unitOfMeasure() const;
        const ext::shared_ptr<CommodityCurve>& forwardCurve() const;
        Real lotQuantity() const;
        Real forwardPrice(const Date& date) const;
        Date lastQuoteDate() const;
        bool empty() const;
        bool forwardCurveEmpty() const;
        //@}

        /*! \deprecated Use fixingCalendar instead.
                        Deprecated in version 1.37.
        */
        [[deprecated("Use fixingCalendar instead")]]
        const Calendar& calendar() const {
            return calendar_;
        }

        /*! \deprecated Use fixing instead.
                        Deprecated in version 1.37.
        */
        [[deprecated("Use fixing instead")]]
        Real price(const Date& date) {
            return fixing(date);
        }

        /*! \deprecated Use addFixing instead.
                        Deprecated in version 1.37.
        */
        [[deprecated("Use fixing instead")]]
        void addQuote(const Date& quoteDate, Real quote) {
            addFixing(quoteDate, quote);
        }

        /*! \deprecated Use addFixings instead.
                        Deprecated in version 1.37.
        */
        [[deprecated("Use addFixings instead")]]
        void addQuotes(const std::map<Date, Real>& quotes) {
            for (auto quote : quotes) {
                addFixing(quote.first, quote.second);
            }
        }

        /*! \deprecated Use clearFixings instead.
                        Deprecated in version 1.37.
        */
        [[deprecated("Use clearFixings instead")]]
        void clearQuotes() {
            clearFixings();
        }

        /*! \deprecated Use isValidFixingDate instead.
                        Deprecated in version 1.37.
        */
        [[deprecated("Use isValidFixingDate instead")]]
        bool isValidQuoteDate(const Date& quoteDate) const {
            return isValidFixingDate(quoteDate);
        }

        /*! \deprecated Use timeSeries instead.
                        Deprecated in version 1.37.
        */
        [[deprecated("Use timeSeries instead")]]
        const TimeSeries<Real>& quotes() const {
            return timeSeries();
        }

        friend std::ostream& operator<<(std::ostream&, const CommodityIndex&);
      protected:
        std::string name_;
        CommodityType commodityType_;
        UnitOfMeasure unitOfMeasure_;
        Currency currency_;
        Calendar calendar_;
        Real lotQuantity_;
        ext::shared_ptr<CommodityCurve> forwardCurve_;
        Real forwardCurveUomConversionFactor_ = 1;
        ext::shared_ptr<ExchangeContracts> exchangeContracts_;
        Integer nearbyOffset_;
    };


    // inline definitions

    inline bool operator==(const CommodityIndex& i1, const CommodityIndex& i2) {
        return i1.name() == i2.name();
    }

    inline void CommodityIndex::update() {
        notifyObservers();
    }

    inline std::string CommodityIndex::name() const {
        return name_;
    }

    inline Calendar CommodityIndex::fixingCalendar() const {
        return calendar_;
    }

    inline bool CommodityIndex::isValidFixingDate(const Date& fixingDate) const {
        return fixingCalendar().isBusinessDay(fixingDate);
    }

    inline Real CommodityIndex::fixing(const Date& date, bool) const {
        return pastFixing(date);
    }

    inline const CommodityType& CommodityIndex::commodityType() const {
        return commodityType_;
    }

    inline const UnitOfMeasure& CommodityIndex::unitOfMeasure() const {
        return unitOfMeasure_;
    }

    inline const Currency& CommodityIndex::currency() const {
        return currency_;
    }

    inline Real CommodityIndex::lotQuantity() const {
        return lotQuantity_;
    }

    inline const ext::shared_ptr<CommodityCurve>&
    CommodityIndex::forwardCurve() const {
        return forwardCurve_;
    }

    inline Real CommodityIndex::forwardPrice(const Date& date) const {
        try {
            Real forwardPrice =
                forwardCurve_->price(date, exchangeContracts_, nearbyOffset_);
            return forwardPrice * forwardCurveUomConversionFactor_;
        } catch (const std::exception& e) {
            QL_FAIL("error fetching forward price for index " << name_
                    << ": " << e.what());
        }
    }

    inline Date CommodityIndex::lastQuoteDate() const {
        return timeSeries().lastDate();
    }

    inline bool CommodityIndex::empty() const {
        return timeSeries().empty();
    }

    inline bool CommodityIndex::forwardCurveEmpty() const {
        if (forwardCurve_ != nullptr)
            return forwardCurve_->empty();
        return false;
    }

}

#endif
