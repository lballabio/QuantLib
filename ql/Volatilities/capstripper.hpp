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


namespace QuantLib {
    
    typedef std::vector<boost::shared_ptr<CashFlow> > FloatingLeg;

    //debugging routine
    void printFloatingLeg(const FloatingLeg& floatingLeg);
    
    //! this class simplifies Cap instanciations
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
                        const Period & endPeriod);
    private:
        Date referenceDate_;
        const Calendar& calendar_;
        int fixingDays_;
        BusinessDayConvention convention_;
        const boost::shared_ptr<Xibor> index_;
    };
    
  
    class CapsStripper :    public CapletVolatilityStructure, 
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
                    const Handle< YieldTermStructure > yieldTermStructure);
      //@}
      //! \name LazyObject interface
      //@{
      void performCalculations () const;

      void update(){
          LazyObject::update();
      };

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
      private:
        std::vector<std::vector<boost::shared_ptr<CapFloor> > > marketDataCap;
        std::vector<std::vector<boost::shared_ptr<CapFloor> > > strippedCap;
        boost::shared_ptr<BilinearInterpolation> bilinearInterpolation_;
        DayCounter dayCounter_;
        Date evaluationDate_, maxDate_;
        std::vector<Time> tenorTimes_;
        std::vector<Rate> strikes_;
        mutable Matrix volatilities_, marketDataPrices_;
    };

    inline DayCounter CapsStripper::dayCounter() const {
        return dayCounter_;
    }

    inline Date CapsStripper::maxDate() const {
        return maxDate_;
    }

    inline Rate CapsStripper::minStrike() const {
        return QL_MIN_REAL;
    }

    inline Rate CapsStripper::maxStrike() const {
        return QL_MAX_REAL;
    }
}

#endif
