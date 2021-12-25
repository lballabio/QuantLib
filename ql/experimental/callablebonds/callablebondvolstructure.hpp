/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Allen Kuo

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

/*! \file callablebondvolstructure.hpp
    \brief Callable-bond volatility structure
*/

#ifndef quantlib_callable_bond_volatility_structure_hpp
#define quantlib_callable_bond_volatility_structure_hpp

#include <ql/termstructure.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/termstructures/volatility/smilesection.hpp>

namespace QuantLib {

    //! Callable-bond volatility structure
    /*! This class is purely abstract and defines the interface of
        concrete callable-bond volatility structures which will be
        derived from this one.
    */
    class CallableBondVolatilityStructure : public TermStructure {
      public:
        /*! \name Constructors
            See the TermStructure documentation for issues regarding
            constructors.
        */
        //@{
        //! default constructor
        /*! \warning term structures initialized by means of this
                     constructor must manage their own reference date
                     by overriding the referenceDate() method.
        */
        CallableBondVolatilityStructure(const DayCounter& dc = DayCounter(),
                                        BusinessDayConvention bdc = Following);
        //! initialize with a fixed reference date
        CallableBondVolatilityStructure(const Date& referenceDate,
                                        const Calendar& calendar = Calendar(),
                                        const DayCounter& dc = DayCounter(),
                                        BusinessDayConvention bdc = Following);
        //! calculate the reference date based on the global evaluation date
        CallableBondVolatilityStructure(Natural settlementDays,
                                        const Calendar&,
                                        const DayCounter& dc = DayCounter(),
                                        BusinessDayConvention bdc = Following);
        //@}
        ~CallableBondVolatilityStructure() override = default;
        //! \name Volatility, variance and smile
        //@{
        //! returns the volatility for a given option time and bondLength
        Volatility volatility(Time optionTime,
                              Time bondLength,
                              Rate strike,
                              bool extrapolate = false) const;
        //! returns the Black variance for a given option time and bondLength
        Real blackVariance(Time optionTime,
                           Time bondLength,
                           Rate strike,
                           bool extrapolate = false) const;

        //! returns the volatility for a given option date and bond tenor
        Volatility volatility(const Date& optionDate,
                              const Period& bondTenor,
                              Rate strike,
                              bool extrapolate = false) const;
        //! returns the Black variance for a given option date and bond tenor
        Real blackVariance(const Date& optionDate,
                           const Period& bondTenor,
                           Rate strike,
                           bool extrapolate = false) const;
        virtual ext::shared_ptr<SmileSection> smileSection(
                                              const Date& optionDate,
                                              const Period& bondTenor) const {
            const std::pair<Time, Time> p = convertDates(optionDate, bondTenor);
            return smileSectionImpl(p.first, p.second);
        }

        //! returns the volatility for a given option tenor and bond tenor
        Volatility volatility(const Period& optionTenor,
                              const Period& bondTenor,
                              Rate strike,
                              bool extrapolate = false) const;
        //! returns the Black variance for a given option tenor and bond tenor
        Real blackVariance(const Period& optionTenor,
                           const Period& bondTenor,
                           Rate strike,
                           bool extrapolate = false) const;
        ext::shared_ptr<SmileSection> smileSection(
                                               const Period& optionTenor,
                                               const Period& bondTenor) const;
        //@}
        //! \name Limits
        //@{
        //! the largest length for which the term structure can return vols
        virtual const Period& maxBondTenor() const = 0;
        //! the largest bondLength for which the term structure can return vols
        virtual Time maxBondLength() const;
        //! the minimum strike for which the term structure can return vols
        virtual Rate minStrike() const = 0;
        //! the maximum strike for which the term structure can return vols
        virtual Rate maxStrike() const = 0;
        //@}
        //! implements the conversion between dates and times
        virtual std::pair<Time,Time> convertDates(
                                               const Date& optionDate,
                                               const Period& bondTenor) const;
        //! the business day convention used for option date calculation
        virtual BusinessDayConvention businessDayConvention() const;
        //! implements the conversion between optionTenors and optionDates
        Date optionDateFromTenor(const Period& optionTenor) const;
    protected:

        //! return smile section
        virtual ext::shared_ptr<SmileSection> smileSectionImpl(
                                                   Time optionTime,
                                                   Time bondLength) const = 0;

        //! implements the actual volatility calculation in derived classes
        virtual Volatility volatilityImpl(Time optionTime,
                                          Time bondLength,
                                          Rate strike) const = 0;
        virtual Volatility volatilityImpl(const Date& optionDate,
                                          const Period& bondTenor,
                                          Rate strike) const {
            const std::pair<Time, Time> p = convertDates(optionDate, bondTenor);
            return volatilityImpl(p.first, p.second, strike);
        }
        void checkRange(Time, Time, Rate strike, bool extrapolate) const;
        void checkRange(const Date& optionDate,
                        const Period& bondTenor,
                        Rate strike, bool extrapolate) const;
      private:
        BusinessDayConvention bdc_;
    };


    // inline definitions

    inline BusinessDayConvention
    CallableBondVolatilityStructure::businessDayConvention() const {
        return bdc_;
    }

    inline Date CallableBondVolatilityStructure::optionDateFromTenor(
                                            const Period& optionTenor) const {
        return calendar().advance(referenceDate(),
                                  optionTenor,
                                  businessDayConvention());
    }

    inline Volatility CallableBondVolatilityStructure::volatility(
                                                     Time optionTime,
                                                     Time bondLength,
                                                     Rate strike,
                                                     bool extrapolate) const {
        checkRange(optionTime, bondLength, strike, extrapolate);
        return volatilityImpl(optionTime, bondLength, strike);
    }


    inline Real CallableBondVolatilityStructure::blackVariance(
                                                     Time optionTime,
                                                     Time bondLength,
                                                     Rate strike,
                                                     bool extrapolate) const {
        checkRange(optionTime, bondLength, strike, extrapolate);
        Volatility vol = volatilityImpl(optionTime, bondLength, strike);
        return vol*vol*optionTime;
    }


    inline Volatility CallableBondVolatilityStructure::volatility(
                                                     const Date& optionDate,
                                                     const Period& bondTenor,
                                                     Rate strike,
                                                     bool extrapolate) const {
        checkRange(optionDate, bondTenor, strike, extrapolate);
        return volatilityImpl(optionDate, bondTenor, strike);
    }

    inline Real CallableBondVolatilityStructure::blackVariance(
                                                     const Date& optionDate,
                                                     const Period& bondTenor,
                                                     Rate strike,
                                                     bool extrapolate) const {
        Volatility vol =
            volatility(optionDate, bondTenor, strike, extrapolate);
        const std::pair<Time, Time> p = convertDates(optionDate, bondTenor);
        return vol*vol*p.first;
    }

    inline Volatility CallableBondVolatilityStructure::volatility(
                                                    const Period& optionTenor,
                                                    const Period& bondTenor,
                                                    Rate strike,
                                                    bool extrapolate) const {
        Date optionDate = optionDateFromTenor(optionTenor);
        return volatility(optionDate, bondTenor, strike, extrapolate);
    }

    inline Real CallableBondVolatilityStructure::blackVariance(
                                                    const Period& optionTenor,
                                                    const Period& bondTenor,
                                                    Rate strike,
                                                    bool extrapolate) const {
        Date optionDate = optionDateFromTenor(optionTenor);
        Volatility vol =
            volatility(optionDate, bondTenor, strike, extrapolate);
        const std::pair<Time, Time> p = convertDates(optionDate, bondTenor);
        return vol*vol*p.first;
    }


    inline ext::shared_ptr<SmileSection>
    CallableBondVolatilityStructure::smileSection(
                                              const Period& optionTenor,
                                              const Period& bondTenor) const {
        Date optionDate = optionDateFromTenor(optionTenor);
        return smileSection(optionDate, bondTenor);
    }


    inline void CallableBondVolatilityStructure::checkRange(
        Time optionTime, Time bondLength, Rate k, bool extrapolate) const {
        TermStructure::checkRange(optionTime, extrapolate);
        QL_REQUIRE(bondLength >= 0.0,
                   "negative bondLength (" << bondLength << ") given");
        QL_REQUIRE(extrapolate || allowsExtrapolation() ||
                   bondLength <= maxBondLength(),
                   "bondLength (" << bondLength << ") is past max curve bondLength ("
                   << maxBondLength() << ")");
        QL_REQUIRE(extrapolate || allowsExtrapolation() ||
                   (k >= minStrike() && k <= maxStrike()),
                   "strike (" << k << ") is outside the curve domain ["
                   << minStrike() << "," << maxStrike()<< "]");
    }

}

#endif
