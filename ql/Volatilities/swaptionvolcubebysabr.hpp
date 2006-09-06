/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Giorgio Facchinetti

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

/*! \file swaptionvolcubebysabr.hpp
    \brief Swaption volatility cube by Sabr
*/

#ifndef quantlib_swaption_volatility_cube_by_sabr_h
#define quantlib_swaption_volatility_cube_by_sabr_h

#include <ql/Volatilities/swaptionvolmatrix.hpp>
#include <ql/Instruments/vanillaswap.hpp>
#include <ql/Math/linearinterpolation.hpp>
#include <ql/Calendars/target.hpp>
#include <ql/swaptionvolstructure.hpp>

namespace QuantLib {

     class SwaptionVolatilityCubeBySabr : public SwaptionVolatilityStructure {

     public:

         class Cube {
             std::vector<Real> expiries_, lengths_;
             Size nLayers_;
             std::vector<Matrix> points_;

             bool extrapolation_;
             mutable std::vector< boost::shared_ptr<BilinearInterpolation> > interpolators_;

         public:

             Cube() {};
             Cube(const std::vector<Real>& expiries, const std::vector<Real>& lengths, 
                 Size nLayers, bool extrapolation = true);
             Cube& operator=(const Cube& o);
             Cube(const Cube&);
             virtual ~Cube(){};

             virtual void setElement(Size IndexOfLayer, Size IndexOfRow,
                                                  Size IndexOfColumn, Real x);
             virtual void setPoints(const std::vector<Matrix>& x);
             virtual void setPoint(const Real& expiry, const Real& lengths,
                                                const std::vector<Real> point);
             void setLayer(Size i, const Matrix& x);
             void expandLayers(Size i, bool expandExpiries, Size j, bool expandLengths);

	         const std::vector<Real>& expiries() const;		
	         const std::vector<Real>& lengths() const;		
	         const std::vector<Matrix>& points() const;

	         virtual std::vector<Real> operator()(const Real& expiry, const Real& lengths) const;
             void updateInterpolators()const;
             Matrix browse() const;
         };

        SwaptionVolatilityCubeBySabr::SwaptionVolatilityCubeBySabr(
            const Handle<SwaptionVolatilityStructure>& atmVolStructure,
            const std::vector<Period>& expiries,
            const std::vector<Period>& lengths,
            const std::vector<Spread>& strikeSpreads,
            const Matrix& volSpreads,
            const Calendar& calendar,
		    Integer swapSettlementDays,
            Frequency fixedLegFrequency,
            BusinessDayConvention fixedLegConvention,
            const DayCounter& fixedLegDayCounter,
            const boost::shared_ptr<Xibor>& iborIndex,
            Time shortTenor,
            const boost::shared_ptr<Xibor>& iborIndexShortTenor,            
            const Matrix& parametersGuess, 
            std::vector<bool> isParameterFixed,
            Real maxTolerance);
        //! \name TermStructure interface
        //@{

        const Date& referenceDate() const {
            return atmVolStructure_->referenceDate();
        }
        DayCounter dayCounter() const {
            return atmVolStructure_->dayCounter();
        }
        //@}
        //! \name SwaptionVolatilityStructure interface
        //@{
        Date maxStartDate() const { return atmVolStructure_->maxStartDate(); }
        Time maxStartTime() const { return atmVolStructure_->maxStartTime(); }
        Period maxLength() const { return atmVolStructure_->maxLength(); }
        Time maxTimeLength() const { return atmVolStructure_->maxTimeLength(); }
        Rate minStrike() const { return 0.0; }
        Rate maxStrike() const { return 1.0; }
        //@}
        const Matrix& marketVolCube(Size i) const { return marketVolCube_.points()[i]; }
        Rate atmStrike(const Date& start,
                       const Period& length) const {
            std::pair<Time,Time> times = convertDates(start, length);
            return atmStrike(times.first, times.second);
        }
        Matrix sparseSabrParameters() const;
        Matrix denseSabrParameters() const;
        Matrix marketVolCube() const;
        Matrix volCubeAtmCalibrated() const;

     protected: 

       boost::shared_ptr<Interpolation> smile(Time start, Time length) const;

       virtual boost::shared_ptr<VarianceSmileSection> smileSection(Time start, Time length, 
                                                 const Cube& sabrParametersCube) const;
       virtual boost::shared_ptr<VarianceSmileSection> smileSection(Time start, Time length) const;
       
       Rate atmStrike(Time start, Time length) const;
       Volatility volatilityImpl(Time start,
                                  Time length,
                                  Rate strike) const;
       Cube sabrCalibration(const Cube& marketVolCube) const;
       void fillVolatilityCube();
       void createSparseSmiles();
       std::vector<Real> spreadVolInterpolation(double atmExerciseTime, 
                                                double atmTimeLength);

      private:

        Handle<SwaptionVolatilityStructure> atmVolStructure_;
        std::vector<Date> exerciseDates_;
        std::vector<Time> exerciseTimes_;
        std::vector<Real> exerciseDatesAsReal_;
        LinearInterpolation exerciseInterpolator_;
        std::vector<Period> lengths_;
        std::vector<Time> timeLengths_;
        Size nStrikes_;
        std::vector<Spread> strikeSpreads_;
        Matrix volSpreads_;
        Cube marketVolCube_;
        Cube volCubeAtmCalibrated_;
        mutable std::vector<Rate> localStrikes_;
        mutable std::vector<Volatility> localSmile_;
        Calendar calendar_ ;
		Integer swapSettlementDays_;
        Frequency fixedLegFrequency_;
        BusinessDayConvention fixedLegConvention_;
        DayCounter fixedLegDayCounter_;
        boost::shared_ptr<Xibor> iborIndex_;
        Time shortTenor_;
        boost::shared_ptr<Xibor> iborIndexShortTenor_;

        Cube sparseParameters_;
        Cube denseParameters_;
        std::vector< std::vector<boost::shared_ptr<VarianceSmileSection> > > sparseSmiles_;

        std::vector<Real> fictitiousStrikes_;

        Cube parametersGuess_; 
        std::vector<bool> isParameterFixed_;
        Real maxTolerance_;
    };

}

#endif
