/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Ralph Schreyer

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

/*! \file concentrating1dmesher.hpp
    \brief One-dimensional grid mesher concentrating around critical points
*/

#ifndef quantlib_concentrating_1d_mesher_hpp
#define quantlib_concentrating_1d_mesher_hpp

#include <ql/experimental/finitedifferences/fdm1dmesher.hpp>
#include <ql/errors.hpp>
#include <ql/utilities/null.hpp>

#include <iostream>

namespace QuantLib {

    class Concentrating1dMesher : public Fdm1dMesher {
      public:
    	  Concentrating1dMesher(Real start, Real end, Size size,
    	      const std::vector<std::pair<Real,Real> >& cPoints)
        : Fdm1dMesher(size) {
            QL_REQUIRE(end > start, "end must be larger than start");
            QL_REQUIRE(cPoints.size() <= 1, "max. 1 cPoint supported");
            
            const Real cPoint  = cPoints.size() == 0 ? 
            					 Null<Real>() : cPoints[0].first;
            const Real density = cPoints.size() == 0 ? 
            					 Null<Real>() : cPoints[0].second*(end-start);
            
            QL_REQUIRE(    cPoint == Null<Real>() 
            		   || (cPoint >= start && cPoint <= end),
            		   "cPoint must be between start and end");
            QL_REQUIRE(density == Null<Real>() || density > 0.0, 
            		   "density > 0 required" );
            
            const Real dx = 1.0/(size-1);
            for (Size i=1; i < size-1; ++i) {
            	if(cPoint != Null<Real>()) {
                    const Real c1 = asinh((start-cPoint)/density);
                    const Real c2 = asinh((end-cPoint)/density);
            		locations_[i] = cPoint + density*sinh(c1*(1.0-i*dx)+c2*i*dx);
            	}
            	else {
            		locations_[i] = start + i*dx*(end-start);
            	}
            }
            locations_.front() = start;
            locations_.back() = end;

            for (Size i=0; i < size-1; ++i) {
                dplus_[i] = dminus_[i+1] = locations_[i+1] - locations_[i];
            }
            dplus_.back() = dminus_.front() = Null<Real>();
        }
    };
}

#endif
