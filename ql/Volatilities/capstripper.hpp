/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Francois du Vignaud

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file capletvariancecurve.hpp
    \brief caplet variance curve
*/

#ifndef quantlib_interpolated_caplet_variance_curve_hpp
#define quantlib_interpolated_caplet_variance_curve_hpp

#include <ql/Patterns/lazyobject.hpp>
#include <ql/capvolstructures.hpp>
#include <ql/types.hpp>
#include <ql/calendar.hpp>
#include <ql/daycounter.hpp>
#include <ql/quote.hpp>
#include <ql/Math/bilinearinterpolation.hpp>
#include <ql/Math/matrix.hpp>
#include <ql/CashFlows/cashflowvectors.hpp>
#include <ql/PricingEngines/CapFloor/blackcapfloorengine.hpp>
#include <ql/Indexes/xibor.hpp>
#include <ql/schedule.hpp>
#include <iostream>

namespace QuantLib {
    typedef std::vector<boost::shared_ptr<CashFlow> > FloatingLeg;

    class LegHelper{
    public:
        LegHelper(  const Date & startDate,
                    const Calendar & calendar,
                    int fixingDays,
                    BusinessDayConvention convention,
                    const boost::shared_ptr<Xibor>& index)
                            :referenceDate_(startDate),
                            calendar_(calendar),
                            fixingDays_(fixingDays),
                            convention_(convention),
                            index_(index){};

    FloatingLeg makeLeg(const Period & startPeriod,
                        const Period & endPeriod)
    {
        Date startDate = referenceDate_ + startPeriod;
        Date endDate = referenceDate_ + endPeriod;
        Schedule schedule(startDate, endDate, index_->tenor(), calendar_,
                          convention_, convention_, true, false);
        return FloatingRateCouponVector(schedule, convention_, std::vector<Real>(1,100),
                                        fixingDays_, index_,
                                        std::vector<Real>(),
                                        std::vector<Spread>(),
                                        index_->dayCounter());
    };

    private:
        Date referenceDate_;
        const Calendar& calendar_;
        int fixingDays_;
        BusinessDayConvention convention_;
        const boost::shared_ptr<Xibor> index_;
    };
    
  
    class CapsStripper : public CapletVolatilityStructure, 
                                            public LazyObject{
      public:
        CapsStripper(const Calendar & calendar,
                                        BusinessDayConvention convention,
                                        int fixingDays,
                                        const std::vector<Period>& tenors,
                                        const std::vector<Rate>& strikes,
                                        const std::vector<std::vector<Handle<Quote> > >& volatilities,
                                        const DayCounter& dayCounter,
                                        const boost::shared_ptr<Xibor>& index,
                                        const Handle< YieldTermStructure > yieldTermStructure)
        :CapletVolatilityStructure(0, calendar),
        dayCounter_(dayCounter),
        strikes_(strikes),
        tenorTimes_(tenors.size()),
        volatilities_(tenors.size(), strikes.size(),.1),
        marketDataPrices_(tenors.size(), strikes.size()),
        evaluationDate_(Settings::instance().evaluationDate())
        {
            // we convert tenors periods into times
            for (Size i = 0 ; i < tenors.size(); i++){
                Date tenorDate = 
                    calendar.advance(evaluationDate_, tenors[i], convention);
                tenorTimes_[i] = dayCounter_.yearFraction(evaluationDate_,tenorDate);
            }
            bilinearInterpolation_ = boost::shared_ptr<BilinearInterpolation>(
                   new BilinearInterpolation(tenorTimes_.begin(),tenorTimes_.end(),
                               strikes_.begin(), strikes_.end(), volatilities_));

           // we create the caps we will need later on
            LegHelper legHelper(evaluationDate_, calendar, fixingDays, convention, index);
            // market data
            marketDataCap.resize(tenors.size());
            for (Size i = 0 ; i < tenorTimes_.size(); i++){
               FloatingLeg floatingLeg = legHelper.makeLeg(index->tenor(), tenors[i]);
               marketDataCap[i].resize(strikes_.size());
               for (Size j = 0 ; j < strikes_.size(); j++){
                   boost::shared_ptr<PricingEngine> blackCapFloorEngine
                                             (new BlackCapFloorEngine(volatilities[i][j]));
                   marketDataCap[i][j] = boost::shared_ptr<CapFloor>(new Cap(floatingLeg,
                                             std::vector<Real>(1,strikes_[j]),
                                             yieldTermStructure, blackCapFloorEngine));
                   registerWith(marketDataCap[i][j]);
               }
            }
            // stripped Caps
            strippedCap.resize(tenors.size()-1);
            for (Size i = 0 ; i < strippedCap.size(); i++){
               FloatingLeg floatingLeg = legHelper.makeLeg(tenors[i],tenors[i+1]);
               strippedCap[i].resize(strikes_.size());
                for (Size j = 0 ; j < strikes_.size(); j++){
                   boost::shared_ptr<PricingEngine> blackCapFloorEngine
                                             (new BlackCapFloorEngine(volatilities[i][j]));
                   strippedCap[i][j] = boost::shared_ptr<CapFloor>(new Cap(floatingLeg,
                                             std::vector<Real>(1,strikes_[j]),
                                             yieldTermStructure, blackCapFloorEngine));
                }
            }

        };
      //@}
      //! \name LazyObject interface
      //@{
      void performCalculations () const {
          // we refresh all market data prices since we don't know which has changed
          for (Size i = 0 ; i < tenorTimes_.size(); i++)
            for (Size j = 0 ; j < strikes_.size(); j++)
                marketDataPrices_[i][j] = marketDataCap[i][j]->NPV();
          // then we stripped volatilities
          for (Size j = 0 ; j < strikes_.size(); j++)
               volatilities_[0][j] = 
                    marketDataCap[0][j]->impliedVolatility(marketDataPrices_[0][j]);
          for (Size i = 1 ; i < tenorTimes_.size()-1; i++)
            for (Size j = 0 ; j < strikes_.size(); j++)
                volatilities_[i][j] = 
                    strippedCap[i][j]->impliedVolatility
                            (marketDataPrices_[i+1][j]- marketDataPrices_[i][j]);
      };

      void update(){/* to be completed ! */};

      //@}
      //! \name TermStructure interface
      //@{
      virtual Date maxDate() const;
      virtual DayCounter dayCounter() const;

      //@}
      //! \name CapletVolatilityStructure interface
      //@{
      virtual Real minStrike() const;
      virtual Real maxStrike() const;

      protected:
          Volatility volatilityImpl(Time t, Rate r) const{
            calculate();
            return bilinearInterpolation_->operator()(t, r, true);
          };
   //   private:
        public:
        std::vector<std::vector<boost::shared_ptr<CapFloor> > > marketDataCap;
        std::vector<std::vector<boost::shared_ptr<CapFloor> > > strippedCap;
        boost::shared_ptr<BilinearInterpolation> bilinearInterpolation_;
        DayCounter dayCounter_;
        Date evaluationDate_;
        std::vector<Time> tenorTimes_;
        std::vector<Rate> strikes_;
        mutable Matrix volatilities_, marketDataPrices_;
    };

    inline DayCounter CapsStripper::dayCounter() const {
        return dayCounter_;
    }

    inline Date CapsStripper::maxDate() const {
        return Date(50000);
    }

    inline Rate CapsStripper::minStrike() const {
        return QL_MIN_REAL;
    }

    inline Rate CapsStripper::maxStrike() const {
        return QL_MAX_REAL;
    }

}


#endif
