
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file capflatvolvector.hpp
    \brief Cap/floor at-the-money flat volatility vector
*/

#ifndef quantlib_cap_volatility_vector_h
#define quantlib_cap_volatility_vector_h

#include <ql/capvolstructures.hpp>
#include <ql/Math/interpolationtraits.hpp>
#include <ql/DayCounters/thirty360.hpp>
#include <vector>

namespace QuantLib {

    //! Cap/floor at-the-money flat volatility vector
    /*! This class provides the at-the-money volatility for a given
        cap by interpolating a volatility vector whose elements are
        the market volatilities of a set of caps/floors with given
        length.
            
        \todo either add correct copy behavior or inhibit copy. Right
              now, a copied instance would end up with its own copy of
              the length vector but an interpolation pointing to the
              original ones.
    */
    class CapFlatVolatilityVector : public CapFlatVolatilityStructure {
      public:
        CapFlatVolatilityVector(const Date& todaysDate,
                                const Calendar& calendar,
                                Integer settlementDays,
                                const std::vector<Period>& lengths,
                                const std::vector<Volatility>& volatilities,
                                const DayCounter& dayCounter = Thirty360());
        // inspectors
        Date todaysDate() const;
        Date settlementDate() const;
        DayCounter dayCounter() const;
      private:
        Date todaysDate_;
        Date settlementDate_;
        Calendar calendar_;
        Integer settlementDays_;
        DayCounter dayCounter_;
        std::vector<Period> lengths_;
        std::vector<Time> timeLengths_;
        std::vector<Volatility> volatilities_;
        // interpolation
        Interpolation interpolation_;
        Volatility volatilityImpl(Time length, Rate strike) const;
    };


    // inline definitions

    inline CapFlatVolatilityVector::CapFlatVolatilityVector(
                            const Date& today, 
                            const Calendar& calendar, Integer settlementDays, 
                            const std::vector<Period>& lengths, 
                            const std::vector<Volatility>& vols,
                            const DayCounter& dayCounter)
    : todaysDate_(today), calendar_(calendar), 
      settlementDays_(settlementDays), dayCounter_(dayCounter), 
      lengths_(lengths), timeLengths_(lengths.size()+1), 
      volatilities_(vols.size()+1) {
        QL_REQUIRE(lengths.size() == vols.size(),
                   "mismatch between number of cap lengths "
                   "and cap volatilities");
        settlementDate_ = calendar_.advance(today,settlementDays,Days);
        timeLengths_[0] = 0.0;
        volatilities_[0] = vols[0];
        for (Size i=0; i<lengths_.size(); i++) {
            Date endDate = settlementDate_.plus(lengths_[i]);
            timeLengths_[i+1] = dayCounter_.yearFraction(
                             settlementDate_,endDate,settlementDate_,endDate);
            volatilities_[i+1] = vols[i];
        }
        interpolation_ = 
            Linear::make_interpolation(timeLengths_.begin(),
                                       timeLengths_.end(),
                                       volatilities_.begin());
    }

    inline Date CapFlatVolatilityVector::todaysDate() const {
        return todaysDate_;
    }

    inline Date CapFlatVolatilityVector::settlementDate() const {
        return settlementDate_;
    }

    inline DayCounter CapFlatVolatilityVector::dayCounter() const {
        return dayCounter_;
    }

    inline Volatility CapFlatVolatilityVector::volatilityImpl(
                                                    Time length, Rate) const {
        return interpolation_(length, false);
    }

}


#endif

