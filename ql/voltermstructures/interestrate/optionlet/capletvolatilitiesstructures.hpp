/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 François du Vignaud

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

/*! \file CapletVolatilitiesStructures.hpp
    \brief Caplet Volatilities Structures used during bootstrapping procedure
*/

#ifndef caplet_volatilities_structures_hpp
#define caplet_volatilities_structures_hpp

#include <ql/voltermstructures/interestrate/optionlet/optionletvolatilitystructure.hpp>
#include <ql/math/matrix.hpp>
#include <ql/math/interpolation.hpp>


namespace QuantLib {

    class CapFloor;
    class BilinearInterpolation;
    class SmileSection;

    typedef std::vector<boost::shared_ptr<SmileSection> > \
        SmileSectionInterfaceVector;
    typedef std::vector<std::vector<boost::shared_ptr<CapFloor> > > CapMatrix;

    typedef std::vector<Handle<SmileSection> > \
        SmileSectionInterfaceHandlesVector;

   class SmileSectionsVolStructure: public OptionletVolatilityStructure{
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

        //! \name OptionletVolatilityStructure interface
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
       public OptionletVolatilityStructure{
    public:
        ParametrizedCapletVolStructure(Date referenceDate):
          OptionletVolatilityStructure(referenceDate){};
       virtual Matrix& volatilityParameters() const = 0;
       virtual Time minTime() const = 0;
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

        //! \name OptionletVolatilityStructure interface
        //@{
        Real minStrike() const;
        Real maxStrike() const;
        //@}
        void update(){};
    private:
        DayCounter dayCounter_;
        Interpolation firstRowInterpolator_;
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

        //! \name OptionletVolatilityStructure interface
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
        std::vector< boost::shared_ptr<Interpolation> >
            strikeInterpolations_;
        Date maxDate_;
        Rate maxStrike_, minStrike_;
    };


    class HybridCapletVolatilityStructure:
        public ParametrizedCapletVolStructure{
    public:
        HybridCapletVolatilityStructure(
            const Date& referenceDate,
            const DayCounter dayCounter,
            const boost::shared_ptr<ParametrizedCapletVolStructure>&
            volatilitiesFromCaps,
            const boost::shared_ptr<SmileSectionsVolStructure>&
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

        //! \name OptionletVolatilityStructure interface
        //@{
        Real minStrike() const;
        Real maxStrike() const;
        //@}

        Time minTime() const{ return volatilitiesFromCaps_->minTime();}

        Time maxTime() const{ return volatilitiesFromCaps_->maxTime();}

        void setClosestTenors(Time, //time,
                              Time&, //nextLowerTenor,
                              Time&) { //nextHigherTenor) {
                // to be implemented ...
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

    inline Date HybridCapletVolatilityStructure::maxDate() const{
        return volatilitiesFromCaps_->maxDate();}

    inline DayCounter HybridCapletVolatilityStructure::dayCounter() const{
        return dayCounter_;}

    inline Real HybridCapletVolatilityStructure::minStrike() const {return 0;}

    inline Real HybridCapletVolatilityStructure::maxStrike() const {return 10;}

    inline void HybridCapletVolatilityStructure::update(){
        volatilitiesFromCaps_->update();
    }

}

#endif
