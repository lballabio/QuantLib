/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006  François du Vignaud

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
#include <ql/volatilities/smilesection.hpp>
#include <ql/PricingEngines/CapFloor/blackcapfloorengine.hpp>
#include <ql/Math/bilinearinterpolation.hpp>
#include <ql/Math/Matrix.hpp>

namespace QuantLib {

    typedef std::vector<boost::shared_ptr<InterpolatedSmileSection> > \
        SmileSectionInterfaceVector;
    typedef std::vector<std::vector<boost::shared_ptr<CapFloor> > > CapMatrix;

   class SmileSectionsVolStructure: public CapletVolatilityStructure{
    public:
        SmileSectionsVolStructure(const Date& referenceDate,
                           const DayCounter& dayCounter,
                           const SmileSectionInterfaceVector& smileSections);
        
        Volatility volatilityImpl(Time length,
            Rate strike) const;

        void setClosestTenors(Time time, Time& nextLowerTenor, 
            Time& nextHigherTenor) const;

        Time maxTime() const{ return tenorTimes_.back();}

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
        DayCounter dayCounter_;
        std::vector<Time> tenorTimes_;
        SmileSectionInterfaceVector smileSections_;
    };
   
    class ParametrizedCapletVolStructure:
       public CapletVolatilityStructure{
    public:
        ParametrizedCapletVolStructure(Date referenceDate):
          CapletVolatilityStructure(referenceDate){};

       virtual Matrix& volatilityParameters() const = 0;
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
    private:
        DayCounter dayCounter_;
        LinearInterpolation firstRowInterpolator_;
        std::vector<Time> tenorTimes_;
        std::vector<Rate> strikes_;
        mutable Matrix volatilities_;
        boost::shared_ptr<BilinearInterpolation> bilinearInterpolation_;
    };


    class HybridCapletVolatilityStructure: public CapletVolatilityStructure{
        public:
        HybridCapletVolatilityStructure(const Date& referenceDate, const DayCounter& dayCounter,
            boost::shared_ptr<SmileSectionsVolStructure> volatilitiesFromFutureOptions,
            boost::shared_ptr<BilinInterpCapletVolStructure> volatilitiesFromCaps);

        Volatility volatilityImpl(Time length,
                                  Rate strike) const;

        boost::shared_ptr<BilinInterpCapletVolStructure> capVolStructure(){
            return volatilitiesFromCaps_;
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
    private:
        DayCounter dayCounter_;
        Time overlapStart, overlapEnd;
        boost::shared_ptr<BilinInterpCapletVolStructure> 
            volatilitiesFromCaps_;
        boost::shared_ptr<SmileSectionsVolStructure> 
            volatilitiesFromFutureOptions_;
    };

}

#endif
