
/*
 Copyright (C) 2003 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file brownianbridge.hpp
    \brief Browian bridge

    \fullpath
    ql/MonteCarlo/%brownianbridge.hpp

*/

// $Id$

// ===========================================================================
// NOTE: The following copyright notice applies to the original code of
// the brownian bridge construction, which was taken from
// "Monte Carlo Methods in Finance", by Peter Jäckel.
//
// Copyright (C) 2002 "Monte Carlo Methods in Finance". All rights reserved.
//
// Permission to use, copy, modify, and distribute this software is freely
// granted, provided that this notice is preserved.
// ===========================================================================

#ifndef   quantlib_brownian_bridge_h
#define   quantlib_brownian_bridge_h

#include <ql/MonteCarlo/path.hpp>
#include <ql/MonteCarlo/sample.hpp>

namespace QuantLib {

    namespace MonteCarlo {

        //! Builds Wiener process paths of equal time steps of delta t = 1
        /*! Builds Wiener process paths of equal time steps of delta t = 1

            For more details: "Monte Carlo Methods in Finance" by P. Jäckel,
            section 10.8
        */
        template <class SG>
        class BrownianBridge {
        public:
            typedef Sample<Path> sample_type;
            BrownianBridge(SG generator);
            //! \name inspectors
            //@{
            const sample_type& next() const;
            const sample_type& antithetic() const;
            Size size() const { return dimension_; }
            //@}
        private:
            SG generator_;
            Size dimension_;
            std::vector<unsigned long> leftIndex_, rightIndex_, bridgeIndex_;
            std::vector<double> leftWeight_, rightWeight_, sigma_;
            mutable Sample<Path> next_;
        };



        template <class SG>
        BrownianBridge<SG>::BrownianBridge(SG generator)
        : generator_(generator), dimension_(generator_.dimension()),
          next_(Path(dimension_), 1.0), bridgeIndex_(dimension_),
          leftIndex_(dimension_),  rightIndex_(dimension_),
          leftWeight_(dimension_), rightWeight_(dimension_),
          sigma_(dimension_) {

            QL_REQUIRE(dimension_>0,
              "BrownianBridge::BrownianBridge : "
              "there must be at least one step");

            std::vector<unsigned long> map(dimension_);
            // map is used to indicate which points are already constructed.
            // If map[i] is zero, path point i is yet unconstructed.
            // map[i]-1 is the index of the variate that constructs the path point # i.

            Size i,j,k,l;
            //  The first point in the construction is the global step.
            map[dimension_-1] = 1;
            //  The global step is constructed from the first variate.
            bridgeIndex_[0] = dimension_-1;
            //  The variance of the global step is numberOfSteps*1.0.
            sigma_[0] = sqrt(dimension_);
            //  The global step to the last point in time is special.
            leftWeight_[0] = rightWeight_[0] = 0.;
            for (j=0, i=1; i<dimension_; ++i) {
                // Find the next unpopulated entry in the map.
                while (map[j]) ++j;
                k=j;
                // Find the next populated entry in the map from there.
                while ((!map[k])) ++k;
                // l-1 is now the index of the point to be constructed next.
                l=j+((k-1-j)>>1);
                map[l]=i;
                // The i-th Gaussian variate will be used to set point l-1.
                bridgeIndex_[i] = l;
                leftIndex_[i]   = j;
                rightIndex_[i]  = k;
                leftWeight_[i]  = (k-l)/(k+1.-j);
                rightWeight_[i] = (l+1.-j)/(k+1.-j);
                sigma_[i]       = sqrt(((l+1.-j)*(k-l))/(k+1.-j));
                j=k+1;
                if (j>=dimension_) j=0;	//	Wrap around.
            }
        }

        template <class SG>
        inline const typename BrownianBridge<SG>::sample_type&
        BrownianBridge<SG>::next() const {

            typedef typename SG::sample_type sequence_type;
            const sequence_type& sequence_ = generator_.nextSequence();

            next_.weight = sequence_.weight;

            Size i, j, k, l;
            // The global step.
            next_[dimension_-1] = sigma_[0]*sequence_.value[0];
            for (i=1; i<dimension_; i++) {
                j = leftIndex_[i];
                k = rightIndex_[i];
                l = bridgeIndex_[i];
                // using only the drift component here for the time being ....
                if (j) next_.value.drift()[l] =
                     leftWeight_[i] * next_.value.drift()[j-1] +
                    rightWeight_[i] * next_.value.drift()[k]   +
                          sigma_[i] * sequence_.value[i];
                else   next_.value.drift()[l] =
                    rightWeight_[i] * next_.value.drift()[k]   +
                          sigma_[i] * sequence_.value[i];
            }
        }

        template <class SG>
        inline const typename BrownianBridge<SG>::sample_type&
        BrownianBridge<SG>::antithetic() const {

            Size i, j, k, l;
            // The global step.
            next_[dimension_-1] = sigma_[0]*sequence_.value[0];
            for (i=1; i<dimension_; i++) {
                j = leftIndex_[i];
                k = rightIndex_[i];
                l = bridgeIndex_[i];
                if (j) next_.value.drift()[l] =
                     leftWeight_[i] * next_.value.drift()[j-1] +
                    rightWeight_[i] * next_.value.drift()[k]   +
                          sigma_[i] * sequence_.value[i];
                else   next_.value.drift()[l] =
                    rightWeight_[i] * next_.value.drift()[k]   +
                          sigma_[i] * sequence_.value[i];
            }

        }

    }

}

#endif
