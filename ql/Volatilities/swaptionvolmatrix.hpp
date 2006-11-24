/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 François du Vignaud
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file swaptionvolmatrix.hpp
    \brief Swaption at-the-money volatility matrix
*/

#ifndef quantlib_swaption_volatility_matrix_h
#define quantlib_swaption_volatility_matrix_h

#include <ql/swaptionvolstructure.hpp>

#include <ql/DayCounters/actual365fixed.hpp>
#include <ql/Math/matrix.hpp>
#include <ql/Math/bilinearinterpolation.hpp>
#include <ql/quote.hpp>
#include <ql/Patterns/lazyobject.hpp> 
#include <boost/noncopyable.hpp>
#include <vector>

namespace QuantLib {

    //! At-the-money swaption-volatility matrix
    /*! This class provides the at-the-money volatility for a given
        swaption by interpolating a volatility matrix whose elements
        are the market volatilities of a set of swaption with given
        option date and swapLength.

        The volatility matrix <tt>M</tt> must be defined so that:
        - the number of rows equals the number of option dates;
        - the number of columns equals the number of swap tenors;
        - <tt>M[i][j]</tt> contains the volatility corresponding
          to the <tt>i</tt>-th option and <tt>j</tt>-th tenor.
    */
    class SwaptionVolatilityMatrix : public LazyObject, 
                                     public SwaptionVolatilityStructure,
                                     private boost::noncopyable {
      public:
        //! floating reference date, floating market data
        SwaptionVolatilityMatrix(
                    const Calendar& calendar,
                    const std::vector<Period>& optionTenors,
                    const std::vector<Period>& swapTenors,
                    const std::vector<std::vector<Handle<Quote> > >& vols,
                    const DayCounter& dayCounter = Actual365Fixed(),
                    BusinessDayConvention bdc = Following);
        //! fixed reference date, floating market data
        SwaptionVolatilityMatrix(
                    const Date& referenceDate,
                    const Calendar& calendar,
                    const std::vector<Period>& optionTenors,
                    const std::vector<Period>& swapTenors,
                    const std::vector<std::vector<Handle<Quote> > >& vols,
                    const DayCounter& dayCounter = Actual365Fixed(),
                    BusinessDayConvention bdc = Following);
        //! floating reference date, fixed market data
        SwaptionVolatilityMatrix(
                    const Calendar& calendar,
                    const std::vector<Period>& optionTenors,
                    const std::vector<Period>& swapTenors,
                    const Matrix& volatilities,
                    const DayCounter& dayCounter = Actual365Fixed(),
                    BusinessDayConvention bdc = Following);
        //! fixed reference date, fixed market data
        SwaptionVolatilityMatrix(
                    const Date& referenceDate,
                    const Calendar& calendar,
                    const std::vector<Period>& optionTenors,
                    const std::vector<Period>& swapTenors,
                    const Matrix& volatilities,
                    const DayCounter& dayCounter = Actual365Fixed(),
                    BusinessDayConvention bdc = Following);
        //! \deprecated alternative constructors instead
        // fixed reference date and fixed market data, option dates 
        SwaptionVolatilityMatrix(const Date& referenceDate,
                                 const std::vector<Date>& optionDates,
                                 const std::vector<Period>& swapTenors,
                                 const Matrix& volatilities,
                                 const DayCounter& dayCounter);

        #ifndef QL_DISABLE_DEPRECATED
        //! \deprecated alternative constructors instead
        SwaptionVolatilityMatrix(const std::vector<Period>& optionTenors,
                                 const Calendar& calendar,
                                 const BusinessDayConvention bdc,
                                 const std::vector<Period>& swapTenors,
                                 const std::vector<std::vector<Handle<Quote> > >& vols,
                                 const DayCounter& dayCounter);
        //! \deprecated alternative constructors instead
        SwaptionVolatilityMatrix(const std::vector<Period>& optionTenors,
                                 const Calendar& calendar,
                                 const BusinessDayConvention bdc,
                                 const std::vector<Period>& swapTenors,
                                 const Matrix& volatilities,
                                 const DayCounter& dayCounter);
        //! \deprecated alternative constructors instead
        SwaptionVolatilityMatrix(const std::vector<Date>& optionDates,
                                 const std::vector<Period>& swapTenors,
                                 const Matrix& volatilities,
                                 const DayCounter& dayCounter);
        #endif

        //! \name TermStructure interface
        //@{
        DayCounter dayCounter() const;
        Date maxDate() const;
        //@}
        //! \name LazyObject interface
        //@{
        void update();
        void performCalculations() const;
        //@}
        //! \name SwaptionVolatilityStructure interface
        //@{
        Period maxSwapTenor() const;
        Time maxSwapLength() const;
        Rate minStrike() const;
        Rate maxStrike() const;

		//! return trivial smile section
        boost::shared_ptr<SmileSectionInterface> smileSection(
                                                 const Period& optionTenor,
                                                 const Period& swapTenor) const;
        //! return trivial smile section
        boost::shared_ptr<SmileSectionInterface> smileSection(
                                                 const Date& optionDates,
                                                 const Period& swapTenor) const;
        //! return trivial smile section
        virtual boost::shared_ptr<SmileSectionInterface> smileSection(
                                                        Time optionTime,
                                                        Time swapLength) const;
        //! implements the conversion between dates and times
        std::pair<Time,Time> convertDates(const Date& optionDates,
                                          const Period& swapTenor) const;
        //@}
        //! \name Other inspectors
        //@{
        const std::vector<Period>& optionTenors() const;
        const std::vector<Date>& optionDates() const;
        const std::vector<Time>& optionTimes() const;
        const std::vector<Period>& swapTenors() const;
        const std::vector<Time>& swapLengths() const;
        //! returns the lower indexes of surrounding volatility matrix corners
        std::pair<Size,Size> locate(const Date& optionDates,
                                    const Period& swapTenor) const {
            std::pair<Time,Time> times = convertDates(optionDates,swapTenor);
            return locate(times.first, times.second);
        }
        //! returns the lower indexes of surrounding volatility matrix corners
        std::pair<Size,Size> locate(Time optionTime,
                                    Time swapLength) const {
            return std::make_pair(interpolation_.locateY(optionTime),
                                  interpolation_.locateX(swapLength));
        }
        //@}
    private:
        void checkInputs(Size optionsNb, Size SwapNb, 
                         Size volRows, Size volsColumns) const;
        void initializeTimes() const;
        void initializeOptionDatesAndTimes() const;
        void registerWithMarketData();
        Volatility volatilityImpl(Time optionTime,
                                  Time swapLength,
                                  Rate strike) const;
        Volatility volatilityImpl(const Date& optionDates,
                                  const Period& swapTenor,
                                  Rate strike) const;
		Volatility volatilityImpl(const Period& optionTenor,
                                  const Period& swapTenor, 
								  Rate strike) const;
        std::vector<Period> optionTenors_;
        mutable std::vector<Date> optionDates_;
        mutable std::vector<Time> optionTimes_;
        std::vector<Period> swapTenors_;
        mutable std::vector<Time> swapLengths_;
        std::vector<std::vector<Handle<Quote> > > volHandles_;
        mutable Matrix volatilities_;
        DayCounter dayCounter_;
        Interpolation2D interpolation_;
    };


    // inline definitions
    
    inline Date SwaptionVolatilityMatrix::maxDate() const {
        return optionDates_.back();
    }

    inline DayCounter SwaptionVolatilityMatrix::dayCounter() const { 
        return dayCounter_; 
    }

    inline const std::vector<Period>&
    SwaptionVolatilityMatrix::optionTenors() const {
         return optionTenors_;
    }

    inline const std::vector<Date>&
    SwaptionVolatilityMatrix::optionDates() const {
        return optionDates_;
    }

    inline const std::vector<Time>&
    SwaptionVolatilityMatrix::optionTimes() const {
        return optionTimes_;
    }

     inline const std::vector<Period>&
     SwaptionVolatilityMatrix::swapTenors() const {
         return swapTenors_;
     }

    inline const std::vector<Time>&
    SwaptionVolatilityMatrix::swapLengths() const {
        return swapLengths_;
    }

    inline Period SwaptionVolatilityMatrix::maxSwapTenor() const {
        return swapTenors_.back();
    }

    inline Time SwaptionVolatilityMatrix::maxSwapLength() const {
        return swapLengths_.back();
    }

    inline Rate SwaptionVolatilityMatrix::minStrike() const {
        return -5.0; //FIXME
    }

    inline Rate SwaptionVolatilityMatrix::maxStrike() const {
        return 5.0; //FIXME
    }

    inline Volatility SwaptionVolatilityMatrix::volatilityImpl(
                                Time optionTime, Time swapLength, Rate) const { 
        calculate();
        return interpolation_(swapLength, optionTime, true);
    }

    inline Volatility SwaptionVolatilityMatrix::volatilityImpl(
                                          const Date& optionDate,
                                          const Period& swapTenor, Rate) const {
        const std::pair<Time, Time> p = convertDates(optionDate, swapTenor);
        return volatilityImpl(p.first, p.second,true);
    }
	
	inline Volatility SwaptionVolatilityMatrix::volatilityImpl(
                                          const Period& optionTenor,
                                          const Period& swapTenor, Rate) const {
		Date optionDate = optionDateFromOptionTenor(optionTenor); 
        return volatilityImpl(optionDate, swapTenor,true);
    }

    inline void SwaptionVolatilityMatrix::update(){
        TermStructure::update();
        LazyObject::update();
    }
}


#endif
