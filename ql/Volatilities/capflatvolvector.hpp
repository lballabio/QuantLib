
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

    //! Cap/floor at-the-money term-volatility vector
    /*! This class provides the at-the-money volatility for a given
        cap by interpolating a volatility vector whose elements are
        the market volatilities of a set of caps/floors with given
        length.

        \todo either add correct copy behavior or inhibit copy. Right
              now, a copied instance would end up with its own copy of
              the length vector but an interpolation pointing to the
              original ones.
    */
    class CapVolatilityVector : public CapVolatilityStructure {
      public:
        #ifndef QL_DISABLE_DEPRECATED
        /*! \deprecated use one of the other constructors */
        CapVolatilityVector(const Date& todaysDate,
                            const Calendar& calendar,
                            Integer settlementDays,
                            const std::vector<Period>& lengths,
                            const std::vector<Volatility>& volatilities,
                            const DayCounter& dayCounter);
        #endif
        CapVolatilityVector(const Date& settlementDate,
                            const std::vector<Period>& lengths,
                            const std::vector<Volatility>& volatilities,
                            const DayCounter& dayCounter);
        CapVolatilityVector(Integer settlementDays,
                            const Calendar& calendar,
                            const std::vector<Period>& lengths,
                            const std::vector<Volatility>& volatilities,
                            const DayCounter& dayCounter);
        // inspectors
        DayCounter dayCounter() const { return dayCounter_; }
        // observability
        void update();
      private:
        DayCounter dayCounter_;
        std::vector<Period> lengths_;
        std::vector<Time> timeLengths_;
        std::vector<Volatility> volatilities_;
        Interpolation interpolation_;
        void interpolate();
        Volatility volatilityImpl(Time length, Rate strike) const;
    };

    #ifndef QL_DISABLE_DEPRECATED
    /*! \deprecated renamed to CapVolatilityVector */
    typedef CapVolatilityVector CapFlatVolatilityVector;
    #endif


    // inline definitions

    #ifndef QL_DISABLE_DEPRECATED
    inline CapVolatilityVector::CapVolatilityVector(
                            const Date& today,
                            const Calendar& calendar, Integer settlementDays,
                            const std::vector<Period>& lengths,
                            const std::vector<Volatility>& vols,
                            const DayCounter& dayCounter)
    : CapVolatilityStructure(today,
                             calendar.advance(today,settlementDays,Days)),
      dayCounter_(dayCounter), lengths_(lengths),
      timeLengths_(lengths.size()+1), volatilities_(vols.size()+1) {
        QL_REQUIRE(lengths.size() == vols.size(),
                   "mismatch between number of cap lengths "
                   "and cap volatilities");
        volatilities_[0] = vols[0];
        std::copy(vols.begin(),vols.end(),volatilities_.begin()+1);
        interpolate();
    }
    #endif

    inline CapVolatilityVector::CapVolatilityVector(
                                          const Date& settlementDate,
                                          const std::vector<Period>& lengths,
                                          const std::vector<Volatility>& vols,
                                          const DayCounter& dayCounter)
    : CapVolatilityStructure(settlementDate),
      dayCounter_(dayCounter), lengths_(lengths),
      timeLengths_(lengths.size()+1), volatilities_(vols.size()+1) {
        QL_REQUIRE(lengths.size() == vols.size(),
                   "mismatch between number of cap lengths "
                   "and cap volatilities");
        volatilities_[0] = vols[0];
        std::copy(vols.begin(),vols.end(),volatilities_.begin()+1);
        interpolate();
    }

    inline CapVolatilityVector::CapVolatilityVector(
                                          Integer settlementDays,
                                          const Calendar& calendar,
                                          const std::vector<Period>& lengths,
                                          const std::vector<Volatility>& vols,
                                          const DayCounter& dayCounter)
    : CapVolatilityStructure(settlementDays,calendar),
      dayCounter_(dayCounter), lengths_(lengths),
      timeLengths_(lengths.size()+1), volatilities_(vols.size()+1) {
        QL_REQUIRE(lengths.size() == vols.size(),
                   "mismatch between number of cap lengths "
                   "and cap volatilities");
        volatilities_[0] = vols[0];
        std::copy(vols.begin(),vols.end(),volatilities_.begin()+1);
        interpolate();
    }

    inline void CapVolatilityVector::update() {
        CapVolatilityStructure::update();
        interpolate();
    }

    inline void CapVolatilityVector::interpolate() {
        timeLengths_[0] = 0.0;
        for (Size i=0; i<lengths_.size(); i++) {
            Date endDate = referenceDate() + lengths_[i];
            timeLengths_[i+1] = timeFromReference(endDate);
        }
        interpolation_ =
            Linear::make_interpolation(timeLengths_.begin(),
                                       timeLengths_.end(),
                                       volatilities_.begin());
    }

    inline Volatility CapVolatilityVector::volatilityImpl(
                                                    Time length, Rate) const {
        return interpolation_(length, false);
    }

}


#endif

