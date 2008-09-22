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


#ifndef quantlib_vega_bump_cluster_hpp
#define quantlib_vega_bump_cluster_hpp

#include <ql/types.hpp>
#include <ql/models/marketmodels/marketmodel.hpp>

/*! When bumping vols, bumping every pseudo-root element individually seems 
excessive so we need to couple some together. 

*/

namespace QuantLib
{
    class VegaBumpCluster
    {
    public:

        VegaBumpCluster(Size factorBegin,
                        Size factorEnd,
                        Size rateBegin,
                        Size rateEnd,
                        Size stepBegin,
                        Size stepEnd);

        bool doesIntersect(const VegaBumpCluster& comparee) const;

        bool isCompatible(const boost::shared_ptr<MarketModel>& volStructure) const;
        Size factorBegin() const
        {
            return factorBegin_;
        }

        Size factorEnd() const
        {
            return factorEnd_;
        }
        
            
        Size rateBegin() const
        {
            return rateBegin_;
        }
        Size rateEnd() const
        {
            return rateEnd_;
        }

        Size stepBegin() const
        {
            return stepBegin_;
        }

        Size stepEnd() const
        {
            return stepEnd_;
        }


    private:
        
        Size factorBegin_;
        Size factorEnd_;       
        Size rateBegin_;
        Size rateEnd_;       
        Size stepBegin_;
        Size stepEnd_;

    };

    /*! 
    There are too many pseudo-root elements to allow bumping them all independently so we cluster them together and 
    then divide all elements into a collection of such clusters. 
   


    */

    class VegaBumpCollection
    {
    public:

        VegaBumpCollection(const boost::shared_ptr<MarketModel>& volStructure, 
                           bool allowFactorwiseBumping = true); 
         
        VegaBumpCollection(const std::vector<VegaBumpCluster>& allBumps, const boost::shared_ptr<MarketModel>& volStructure);
    
        Size numberBumps() const;

        const boost::shared_ptr<MarketModel>& associatedModel() const
        {
            return associatedVolStructure_;
        }


        const std::vector<VegaBumpCluster>& allBumps() const;

        bool isFull() const; // i.e. is every alive pseudo-root element bumped at least once
        bool isNonOverlapping() const; // i.e. is every alive pseudo-root element bumped at most once
        bool isSensible() const; // i.e. is every alive pseudo-root element bumped precisely once

    
    private:

        std::vector<VegaBumpCluster> allBumps_;
        boost::shared_ptr<MarketModel> associatedVolStructure_;
        mutable bool checked_;
        mutable bool nonOverlapped_;
        mutable bool full_;


    };

}

#endif
