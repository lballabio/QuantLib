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
#include <ql/indexes/indexmanager.hpp>

namespace QuantLib {

    class TermStructure;

    //! base class for commodity indexes
    class CommodityIndex : public Observable,
                           public Observer {
      public:
        CommodityIndex(
                const std::string& name,
                const CommodityType& commodityType,
                const Currency& currency,
                const UnitOfMeasure& unitOfMeasure,
                const Calendar& calendar,
                Real lotQuantity,
                const boost::shared_ptr<CommodityCurve>& forwardCurve,
                const boost::shared_ptr<ExchangeContracts>& exchangeContracts,
                int nearbyOffset);
        //! \name Index interface
        //@{
        std::string name() const;
        //@}
        //! \name Observer interface
        //@{
        void update();
        //@}
        //! \name Inspectors
        //@{
        const CommodityType& commodityType() const;
        const Currency& currency() const;
        const UnitOfMeasure& unitOfMeasure() const;
        const Calendar& calendar() const;
        const boost::shared_ptr<CommodityCurve>& forwardCurve() const;
        Real lotQuantity() const;

        Real price(const Date& date);
        Real forwardPrice(const Date& date) const;
        Date lastQuoteDate() const;
        //@}
        void addQuote(const Date& quoteDate, Real quote);

        void addQuotes(const std::map<Date, Real>& quotes) {
            std::string tag = name();
            quotes_ = IndexManager::instance().getHistory(tag);
            for (std::map<Date, Real>::const_iterator ii = quotes.begin();
                 ii != quotes.end (); ii++) {
                quotes_[ii->first] = ii->second;
            }
            IndexManager::instance().setHistory(tag, quotes_);
        }

        void clearQuotes();
        //! returns TRUE if the quote date is valid
        bool isValidQuoteDate(const Date& quoteDate) const;
        bool empty() const;
        bool forwardCurveEmpty() const;
        const TimeSeries<Real>& quotes() const;

        friend std::ostream& operator<<(std::ostream&, const CommodityIndex&);
      protected:
        std::string name_;
        CommodityType commodityType_;
        UnitOfMeasure unitOfMeasure_;
        Currency currency_;
        Calendar calendar_;
        Real lotQuantity_;
        TimeSeries<Real> quotes_;
        boost::shared_ptr<CommodityCurve> forwardCurve_;
        Real forwardCurveUomConversionFactor_;
        boost::shared_ptr<ExchangeContracts> exchangeContracts_;
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

    inline const CommodityType& CommodityIndex::commodityType() const {
        return commodityType_;
    }

    inline const UnitOfMeasure& CommodityIndex::unitOfMeasure() const {
        return unitOfMeasure_;
    }

    inline const Currency& CommodityIndex::currency() const {
        return currency_;
    }

    inline const Calendar& CommodityIndex::calendar() const {
        return calendar_;
    }

    inline Real CommodityIndex::lotQuantity() const {
        return lotQuantity_;
    }

    inline const boost::shared_ptr<CommodityCurve>&
    CommodityIndex::forwardCurve() const {
        return forwardCurve_;
    }

    inline const TimeSeries<Real>& CommodityIndex::quotes() const {
        return quotes_;
    }

    inline Real CommodityIndex::price(const Date& date) {
        std::map<Date, Real>::const_iterator hq = quotes_.find(date);
        if (hq->second == Null<Real>()) {
            hq++;
            if (hq == quotes_.end())
                //if (hq->second == Null<Real>())
                return Null<Real>();
        }
        return hq->second;
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
        if (quotes_.empty())
            return Date::minDate();
        return quotes_.lastDate();
    }

    inline bool CommodityIndex::empty() const {
        return quotes_.empty();
    }

    inline bool CommodityIndex::forwardCurveEmpty() const {
        if (forwardCurve_ != 0)
            return forwardCurve_->empty();
        return false;
    }

    inline void CommodityIndex::addQuote(const Date& quoteDate, Real quote) {
        //QL_REQUIRE(isValidQuoteDate(quoteDate),
        //           "Quote date " << quoteDate.weekday() << ", " <<
        //           quoteDate << " is not valid");
        std::string tag = name();
        quotes_ = IndexManager::instance().getHistory(tag);
        quotes_[quoteDate] = quote;
        IndexManager::instance().setHistory(tag, quotes_);
    }

    inline void CommodityIndex::clearQuotes() {
        IndexManager::instance().clearHistory(name());
    }

    inline bool CommodityIndex::isValidQuoteDate(const Date& quoteDate) const {
        return calendar().isBusinessDay(quoteDate);
    }

}

#endif
