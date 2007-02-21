/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 François du Vignaud

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

/*! \file CapletVolatilitiesStructures.hpp
    \brief Caplet Volatilities Structures used during bootstrapping procedure
*/

#ifndef caplet_volatilities_structures_hpp
#define caplet_volatilities_structures_hpp

#include <ql/capvolstructures.hpp>
#include <ql/Volatilities/smilesection.hpp>
#include <ql/Math/bilinearinterpolation.hpp>
#include <ql/Math/linearinterpolation.hpp>

namespace QuantLib {

    class CapFloor;


    typedef std::vector<boost::shared_ptr<SmileSection> > \
        SmileSectionInterfaceVector;
    typedef std::vector<std::vector<boost::shared_ptr<CapFloor> > > CapMatrix;

    typedef std::vector<Handle<SmileSection> > \
        SmileSectionInterfaceHandlesVector;

   class SmileSectionsVolStructure: public CapletVolatilityStructure{
    public:
        SmileSectionsVolStructure(
           const Date& referenceDate,
           const DayCounter& dayCounter,
           const SmileSectionInterfaceVector& smileSections);

        Volatility volatilityImpl(Time length,
            Rate strike) const;

        void setClosestTenors(Time time, Time& nextLowerTenor,
            Time& nextHigherTenor) const;

        //! \name TermStructure interface
        //@{
        Date maxDate() const;
        DayCounter dayCounter() const;
        //@}

        //! \name CapletVolatilityStructure interface
        //@{
        Real minStrike() const;
        Real maxStrike() const;
        //@}
    private:
        Real minStrike_, maxStrike_;
        Date maxDate_;
        DayCounter dayCounter_;
        std::vector<Time> tenorTimes_;
        SmileSectionInterfaceVector smileSections_;
    };

    class ParametrizedCapletVolStructure:
       public CapletVolatilityStructure{
    public:
        ParametrizedCapletVolStructure(Date referenceDate):
          CapletVolatilityStructure(referenceDate){};
       virtual void update() = 0;
       virtual Matrix& volatilityParameters() const = 0;
       virtual Time minTime() const = 0;
       virtual Time maxTime() const = 0;
       virtual void setClosestTenors(Time time,
            Time& nextLowerTenor, Time& nextHigherTenor) = 0;
    };

    class BilinInterpCapletVolStructure:
        public ParametrizedCapletVolStructure{
    public:
        BilinInterpCapletVolStructure(
            const Date& referenceDate,
            const DayCounter dayCounter,
            const CapMatrix& referenceCaps,
            const std::vector<Rate>& strikes);

        Volatility volatilityImpl(Time length, Rate strike) const;

        void setClosestTenors(Time time,
            Time& nextLowerTenor, Time& nextHigherTenor);

        Time minTime() const{ return tenorTimes_.front();}

        Time maxTime() const{ return tenorTimes_.back();}

        Real& volatilityParameter(Size i, Size j) const {
            return volatilities_[i][j];
        }

        Matrix& volatilityParameters() const {
            return volatilities_;
        }

        //! \name TermStructure interface
        //@{
        Date maxDate() const;
        DayCounter dayCounter() const;
        //@}

        //! \name CapletVolatilityStructure interface
        //@{
        Real minStrike() const;
        Real maxStrike() const;
        //@}
        void update(){};
    private:
        DayCounter dayCounter_;
        LinearInterpolation firstRowInterpolator_;
        std::vector<Time> tenorTimes_;
        std::vector<Rate> strikes_;
        mutable Matrix volatilities_;
        boost::shared_ptr<BilinearInterpolation> bilinearInterpolation_;
        Date maxDate_;
        Rate maxStrike_, minStrike_;
    };

	//! this class is interpolating caplets volatilities linealy in two steps (instead of 
	// one for the BilinInterpCapletVolStructure
	
    class DecInterpCapletVolStructure:
        public ParametrizedCapletVolStructure{
    public:
        DecInterpCapletVolStructure(
            const Date& referenceDate,
            const DayCounter dayCounter,
            const CapMatrix& referenceCaps,
            const std::vector<Rate>& strikes);

        Volatility volatilityImpl(Time length, Rate strike) const;
        
        void setClosestTenors(Time time,
            Time& nextLowerTenor, Time& nextHigherTenor);

        Time minTime() const{ return tenorTimes_.front();}
        
        Time maxTime() const{ return tenorTimes_.back();}

        Real& volatilityParameter(Size i, Size j) const {
            return volatilities_[i][j];
        }

        Matrix& volatilityParameters() const {
            return volatilities_;
        }

        //! \name TermStructure interface
        //@{
        Date maxDate() const;
        DayCounter dayCounter() const;
        //@}

        //! \name CapletVolatilityStructure interface
        //@{
        Real minStrike() const;
        Real maxStrike() const;
        //@}
         
        void update();

    private:
        DayCounter dayCounter_;
        std::vector<Time> tenorTimes_;
        std::vector<Rate> strikes_;
        mutable Matrix volatilities_;
        //mutable std::vector< std::vector<Volatility> > volatilities_;
        std::vector< boost::shared_ptr<Interpolation> >
            strikeInterpolations_;
        Date maxDate_;
        Rate maxStrike_, minStrike_;
    };

    template <class T>
    class HybridCapletVolatilityStructure:
        public ParametrizedCapletVolStructure{
    public:
        HybridCapletVolatilityStructure(
            const Date& referenceDate,
            const DayCounter dayCounter,
            const CapMatrix& referenceCaps,
            const std::vector<Rate>& strikes,
            const boost::shared_ptr<SmileSectionsVolStructure>
                shortTermCapletVolatilityStructure);

        Volatility volatilityImpl(Time length,
                                  Rate strike) const;

        Matrix& volatilityParameters() const {
            return volatilitiesFromCaps_->volatilityParameters();
        }

        //! \name TermStructure interface
        //@{
        Date maxDate() const;
        DayCounter dayCounter() const;
        //@}

        //! \name CapletVolatilityStructure interface
        //@{
        Real minStrike() const;
        Real maxStrike() const;
        //@}

        Time minTime() const{ return volatilitiesFromCaps_->minTime();}

        Time maxTime() const{ return volatilitiesFromCaps_->maxTime();}

        void setClosestTenors(Time time,
            Time& nextLowerTenor, Time& nextHigherTenor){
                // to be implemnted ...
         };
        void update();
    private:
        DayCounter dayCounter_;
        Time overlapStart, overlapEnd;
        boost::shared_ptr<ParametrizedCapletVolStructure>
            volatilitiesFromCaps_;
        boost::shared_ptr<SmileSectionsVolStructure>
            shortTermCapletVolatilityStructure_;
        Date maxDate_;
        Rate minStrike_, maxStrike_;
    };

    inline Real linearInterpolation(Real x, Real x1, Real x2,
                                            Real y1, Real y2){
        if (x == x1)
            return y1;
        return y1 + (x-x1)*(y2-y1)/(x2-x1);
    }

    template <class T>
    inline HybridCapletVolatilityStructure<T>::HybridCapletVolatilityStructure(
            const Date& referenceDate,
            const DayCounter dayCounter,
            const CapMatrix& referenceCaps,
            const std::vector<Rate>& strikes,
            const boost::shared_ptr<SmileSectionsVolStructure>
                shortTermCapletVolatilityStructure):
        ParametrizedCapletVolStructure(referenceDate), dayCounter_(dayCounter),
            shortTermCapletVolatilityStructure_(
                shortTermCapletVolatilityStructure){

        volatilitiesFromCaps_ =
            boost::shared_ptr<ParametrizedCapletVolStructure>(
                new T(referenceDate, dayCounter,
                    referenceCaps, strikes));

        registerWith(shortTermCapletVolatilityStructure);

        Time maxShortTermMaturity = shortTermCapletVolatilityStructure->
                                        maxTime();
            Time minCapMaturity = volatilitiesFromCaps_->minTime();
            overlapStart = std::min(maxShortTermMaturity, minCapMaturity);
            overlapEnd = std::max(maxShortTermMaturity, minCapMaturity);
        }

    template <class T>
    inline Volatility HybridCapletVolatilityStructure<T>::volatilityImpl(
                              Time length,
                              Rate strike) const {
            if (length < overlapStart)
                return shortTermCapletVolatilityStructure_->volatility(length,
                strike, true);
            if (length > overlapEnd)
                return volatilitiesFromCaps_->volatility(length, strike,
                                                        true);

            Time nextLowerFutureTenor, nextHigherFutureTenor,
                nextLowerCapTenor, nextHigherCapTenor,
                nextLowerTenor, nextHigherTenor;
            Volatility volAtNextLowerTenor, volAtNextHigherTenor;

            volatilitiesFromCaps_->setClosestTenors(length,
                nextLowerCapTenor, nextHigherCapTenor);

            shortTermCapletVolatilityStructure_->setClosestTenors(length,
                nextLowerFutureTenor, nextHigherFutureTenor);

            /* we determine which volatility surface should be used for the
               lower value*/
            if (nextLowerCapTenor < nextLowerFutureTenor) {
                nextLowerTenor = nextLowerFutureTenor;
                volAtNextLowerTenor = shortTermCapletVolatilityStructure_->
                    volatility(nextLowerTenor, strike, true);
            } else {
                nextLowerTenor = nextLowerCapTenor;
                volAtNextLowerTenor = volatilitiesFromCaps_->volatility(
                    nextLowerTenor, strike, true);
            }

            /* we determine which volatility surface should be used for
               the higher value*/
            if (nextHigherCapTenor < nextHigherFutureTenor){
                nextHigherTenor = nextHigherCapTenor;
                volAtNextHigherTenor = volatilitiesFromCaps_->volatility(
                    nextHigherTenor, strike, true);
            }else{
                nextHigherTenor = nextHigherFutureTenor;
                volAtNextHigherTenor = shortTermCapletVolatilityStructure_->
                    volatility(nextHigherTenor, strike, true);
            }

            return linearInterpolation(length, nextLowerTenor,
                nextHigherTenor, volAtNextLowerTenor, volAtNextHigherTenor);
    }

    template <class T>
    inline Date HybridCapletVolatilityStructure<T>::maxDate() const{
        return volatilitiesFromCaps_->maxDate();}

    template <class T>
    inline DayCounter HybridCapletVolatilityStructure<T>::dayCounter() const{
        return dayCounter_;}

    template <class T>
    inline Real HybridCapletVolatilityStructure<T>::minStrike() const {return 0;}

    template <class T>
    inline Real HybridCapletVolatilityStructure<T>::maxStrike() const {return 10;}

    template <class T>
    inline void HybridCapletVolatilityStructure<T>::update(){
        volatilitiesFromCaps_->update();
    }

}

#endif
