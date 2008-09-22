/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*

 Copyright (C) 2008 Mark Joshi

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


#ifndef quantlib_swaption_pseudo_jacobian_hpp
#define quantlib_swaption_pseudo_jacobian_hpp


#include <ql/models/marketmodels/marketmodel.hpp>

/*! In order to compute market vegas, we need a class that gives the
derivative of a swaption implied vol against changes in pseudo-root elements.
This is that class.

This is tested in the pathwise vegas routine in MarketModels.cpp

*/

namespace QuantLib
{
    class SwaptionPseudoDerivative
    {

        public:
            SwaptionPseudoDerivative(boost::shared_ptr<MarketModel> inputModel,
                                   Size startIndex,
                                   Size endIndex);

            const Matrix& varianceDerivative(Size i) const;
            const Matrix& volatilityDerivative(Size i) const;

            Real impliedVolatility() const;
            Real variance() const;
            Real expiry() const;



        private:
            boost::shared_ptr<MarketModel> inputModel_;
            std::vector<Matrix> varianceDerivatives_;
            std::vector<Matrix> volatilityDerivatives_;

            Real impliedVolatility_;
            Real expiry_;
            Real variance_;


    };

/*! In order to compute market vegas, we need a class that gives the
derivative of a cap implied vol against changes in pseudo-root elements.
This is that class.

The operation is non-trivial because the cap implied vol has a complicated
relationship with the caplet implied vols. 

This is  tested in the pathwise vegas routine in MarketModels.cpp

*/

   class CapPseudoDerivative
    {

        public:
            CapPseudoDerivative(boost::shared_ptr<MarketModel> inputModel,
                                   Real strike,
                                   Size startIndex,
                                   Size endIndex, 
                                   Real firstDF);

            const Matrix& volatilityDerivative(Size i) const;
            const Matrix& priceDerivative(Size i) const;

            Real impliedVolatility() const;



        private:
            boost::shared_ptr<MarketModel> inputModel_;
      
            std::vector<Matrix> volatilityDerivatives_;
            
            std::vector<Matrix> priceDerivatives_;

            Real impliedVolatility_;
            Real vega_;
            Real firstDF_;



    };

}

#endif
