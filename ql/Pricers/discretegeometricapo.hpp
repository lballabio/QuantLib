
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

/*! \file discretegeometricapo.hpp
    \brief Discrete Geometric Average Price Option
*/

#ifndef quantlib_discrete_geometric_average_price_option_h
#define quantlib_discrete_geometric_average_price_option_h

#include <ql/Pricers/singleassetoption.hpp>
#include <ql/Math/normaldistribution.hpp>


namespace QuantLib {

    //! Discrete geometric average-price Asian option (European style)
    /*! This class implements a discrete geometric average price asian
        option, with european exercise.  The formula is from "Asian
        Option", E. Levy (1997) in "Exotic Options: The State of the
        Art", edited by L. Clewlow, C. Strickland, pag65-97

        \todo add analytical greeks
    */
    class DiscreteGeometricAPO : public SingleAssetOption    {
      public:
        DiscreteGeometricAPO(Option::Type type,
                             double underlying,
                             double strike,
                             Spread dividendYield,
                             Rate riskFreeRate,
                             const std::vector<Time>& times,
                             double volatility);
        double value() const;
        double delta() const {return 0.0;}
        double gamma() const {return 0.0;}
        double theta() const {return 0.0;}
        boost::shared_ptr<SingleAssetOption> clone() const;
      private:
        static const CumulativeNormalDistribution f_;
        std::vector<Time> times_;
    };


    // inline definitions
    inline boost::shared_ptr<SingleAssetOption> 
    DiscreteGeometricAPO::clone() const {
        return boost::shared_ptr<SingleAssetOption>(
                                             new DiscreteGeometricAPO(*this));
    }

}


#endif
