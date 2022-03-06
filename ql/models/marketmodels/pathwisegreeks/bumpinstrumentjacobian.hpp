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


#ifndef quantlib_bump_instrument_jacobian_hpp
#define quantlib_bump_instrument_jacobian_hpp


#include <ql/models/marketmodels/marketmodel.hpp>
#include <ql/models/marketmodels/pathwisegreeks/vegabumpcluster.hpp>
#include <valarray>

namespace QuantLib
{   
    class VolatilityBumpInstrumentJacobian
    {
    public:

        struct Swaption
        {
            Size startIndex_;
            Size endIndex_;
        };

        struct Cap
        {
            Size startIndex_;
            Size endIndex_;
            Real strike_;

        };

    
        VolatilityBumpInstrumentJacobian(const VegaBumpCollection& bumps,
            const std::vector<Swaption>& swaptions,
            const std::vector<Cap>& caps);


        const VegaBumpCollection& getInputBumps() const
        {
            return bumps_;
        }

        std::vector<Real> derivativesVolatility(Size j) const;

        std::vector<Real> onePercentBump(
            Size j) const; // vector of smallest size that changes instrument implied vol by 1
                           // percent i.e. 0.01 v / <v,v> with v result of derivativesVolatility

        const Matrix& getAllOnePercentBumps() const;

    private:
         VegaBumpCollection bumps_;
         std::vector<Swaption> swaptions_;
         std::vector<Cap> caps_;
         mutable std::valarray<bool> computed_;
         mutable bool allComputed_;
         mutable std::vector<std::vector<Real> > derivatives_;
         
         mutable std::vector<std::vector<Real> > onePercentBumps_;
         mutable Matrix bumpMatrix_;
    };

    /*!
    Pass in a market model, a list of instruments, and possible bumps.

    Get out pseudo-root bumps that shift each implied vol by one percent, and leave the other instruments fixed. 

    If the contribution of an instrument is too correlated with other instruments used, discard it. 

    */

    class OrthogonalizedBumpFinder
    {
        public:
            OrthogonalizedBumpFinder(const VegaBumpCollection& bumps,
                                     const std::vector<VolatilityBumpInstrumentJacobian::Swaption>& swaptions,
                                     const std::vector<VolatilityBumpInstrumentJacobian::Cap>& caps,
                                     Real multiplierCutOff, // if vector length grows by more than this discard
                                     Real tolerance);      // if vector projection before scaling less than this discard

            void GetVegaBumps(std::vector<std::vector<Matrix> >& theBumps) const; // this is precisely the vector to pass into PathwiseVegasAccountingEngine
             

        private:

            VolatilityBumpInstrumentJacobian derivativesProducer_;
            Real multiplierCutOff_;
            Real tolerance_;


    };


  

}

#endif


#ifndef id_4b1bfaaa39284d374b9d60f28c96296c
#define id_4b1bfaaa39284d374b9d60f28c96296c
inline bool test_4b1bfaaa39284d374b9d60f28c96296c(int* i) { return i != 0; }
#endif
