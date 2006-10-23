/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2006 StatPro Italia srl

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

/*! \file brownianbridge.hpp
    \brief Browian bridge
*/

// ===========================================================================
// NOTE: The following copyright notice applies to the original code,
//
// Copyright (C) 2002 Peter Jäckel "Monte Carlo Methods in Finance".
// All rights reserved.
//
// Permission to use, copy, modify, and distribute this software is freely
// granted, provided that this notice is preserved.
// ===========================================================================

#ifndef  quantlib_brownian_bridge_hpp
#define  quantlib_brownian_bridge_hpp

#include <ql/MonteCarlo/path.hpp>
#include <ql/MonteCarlo/sample.hpp>
#include <ql/stochasticprocess.hpp>
#include <ql/voltermstructure.hpp>

namespace QuantLib {

    namespace Old {

        //! Builds Wiener process paths using Gaussian variates
        /*! For more details: "Monte Carlo Methods in Finance" by
            P. Jäckel, section 10.8.3

            \ingroup mcarlo

            \note this class does not work if the diffusion term of the
                  underlying stochastic process is asset-dependent.

            \deprecated use New::BrownianBridge instead
        */
        template <class GSG> // Gaussian Sequence Generator
        class BrownianBridge {
          public:
            typedef Sample<std::vector<Real> > sample_type;
            //! normalised (unit time, unit variance) Wiener process paths
            BrownianBridge(const GSG& generator);
            //! unit variance Wiener process paths
            BrownianBridge(Time length,
                           Size timeSteps,
                           const GSG& generator);
            //! unit variance Wiener process paths
            BrownianBridge(const TimeGrid& timeGrid,
                           const GSG& generator);
            //! general Wiener process paths
            BrownianBridge(const std::vector<Real>& sigma,
                           const TimeGrid& timeGrid,
                           const GSG& generator);
            BrownianBridge(const boost::shared_ptr<BlackVolTermStructure>&,
                           const TimeGrid& timeGrid,
                           const GSG& generator);
            BrownianBridge(const boost::shared_ptr<StochasticProcess1D>&,
                           const TimeGrid& timeGrid,
                           const GSG& generator);
            //! \name inspectors
            //@{
            const sample_type& next() const;
            const sample_type& last() const { return next_; }
            Size size() const { return dimension_; }
            const TimeGrid& timeGrid() const { return timeGrid_; }
            //@}
          private:
            void initialize(const std::vector<Real>& v);
            GSG generator_;
            Size dimension_;
            TimeGrid timeGrid_;
            mutable Sample<std::vector<Real> > next_;
            std::vector<Size> bridgeIndex_, leftIndex_, rightIndex_;
            std::vector<Real> leftWeight_, rightWeight_, stdDev_;
        };


        template <class GSG>
        BrownianBridge<GSG>::BrownianBridge(const GSG& generator)
        : generator_(generator), dimension_(generator_.dimension()),
          timeGrid_(Time(dimension_), dimension_),
          next_(std::vector<Real>(dimension_), 1.0),
          bridgeIndex_(dimension_),
          leftIndex_(dimension_),  rightIndex_(dimension_),
          leftWeight_(dimension_), rightWeight_(dimension_),
          stdDev_(dimension_) {
            initialize(std::vector<Real>(dimension_, 1.0));
        }

        template <class GSG>
        BrownianBridge<GSG>::BrownianBridge(Time length,
                                            Size timeSteps,
                                            const GSG& generator)
        : generator_(generator), dimension_(generator_.dimension()),
          timeGrid_(length, timeSteps),
          next_(std::vector<Real>(dimension_), 1.0),
          bridgeIndex_(dimension_),
          leftIndex_(dimension_),  rightIndex_(dimension_),
          leftWeight_(dimension_), rightWeight_(dimension_),
          stdDev_(dimension_) {

            QL_REQUIRE(dimension_>0,
                       "there must be at least one step");

            QL_REQUIRE(dimension_==timeGrid_.size()-1,
                       "GSG/timeGrid dimension mismatch"
                       "(" << dimension_ << "/" + timeGrid_.size()-1 << ")");

            initialize(std::vector<Real>(dimension_, 1.0));
        }

        template <class GSG>
        BrownianBridge<GSG>::BrownianBridge(const TimeGrid& timeGrid,
                                            const GSG& generator)
        : generator_(generator), dimension_(generator_.dimension()),
          timeGrid_(timeGrid),
          next_(std::vector<Real>(dimension_), 1.0),
          bridgeIndex_(dimension_),
          leftIndex_(dimension_),  rightIndex_(dimension_),
          leftWeight_(dimension_), rightWeight_(dimension_),
          stdDev_(dimension_) {

            QL_REQUIRE(dimension_>0,
                       "there must be at least one step");

            QL_REQUIRE(dimension_==timeGrid_.size()-1,
                       "GSG/timeGrid dimension mismatch"
                       "(" << dimension_ << "/" << timeGrid_.size()-1 << ")");

            initialize(std::vector<Real>(dimension_, 1.0));
        }

        template <class GSG>
        BrownianBridge<GSG>::BrownianBridge(const std::vector<Real>& variances,
                                            const TimeGrid& timeGrid,
                                            const GSG& generator)
        : generator_(generator), dimension_(generator_.dimension()),
          timeGrid_(timeGrid),
          next_(std::vector<Real>(dimension_), 1.0),
          bridgeIndex_(dimension_),
          leftIndex_(dimension_),  rightIndex_(dimension_),
          leftWeight_(dimension_), rightWeight_(dimension_),
          stdDev_(dimension_) {

            QL_REQUIRE(dimension_>0,
                       "there must be at least one step");

            QL_REQUIRE(dimension_==timeGrid_.size()-1,
                       "GSG/timeGrid dimension mismatch"
                       "(" << dimension_ << "/" << timeGrid_.size()-1 << ")");

            QL_REQUIRE(dimension_==variances.size(),
                       "GSG/variances dimension mismatch"
                       "(" << dimension_ << "/" << variances.size() << ")");

            initialize(variances);
        }

        template <class GSG>
        BrownianBridge<GSG>::BrownianBridge(
                     const boost::shared_ptr<BlackVolTermStructure>& blackVol,
                     const TimeGrid& timeGrid,
                     const GSG& generator)
        : generator_(generator), dimension_(generator_.dimension()),
          timeGrid_(timeGrid),
          next_(std::vector<Real>(dimension_), 1.0),
          bridgeIndex_(dimension_),
          leftIndex_(dimension_),  rightIndex_(dimension_),
          leftWeight_(dimension_), rightWeight_(dimension_),
          stdDev_(dimension_) {

            QL_REQUIRE(dimension_>0,
                       "there must be at least one step");

            QL_REQUIRE(dimension_==timeGrid_.size()-1,
                       "GSG/timeGrid dimension mismatch"
                       "(" << dimension_ << "/" << timeGrid_.size()-1 << ")");

            std::vector<Real> variances(dimension_);
            for (Size i=0; i<dimension_; i++) {
                // problems here if the blackVol is asset dependant
                variances[i]=blackVol->blackVariance(timeGrid_[i+1], 1.0);
            }
            initialize(variances);
        }


        template <class GSG>
        BrownianBridge<GSG>::BrownianBridge(
                    const boost::shared_ptr<StochasticProcess1D>& diffProcess,
                    const TimeGrid& timeGrid,
                    const GSG& generator)
        : generator_(generator), dimension_(generator_.dimension()),
          timeGrid_(timeGrid),
          next_(std::vector<Real>(dimension_), 1.0),
          bridgeIndex_(dimension_),
          leftIndex_(dimension_),  rightIndex_(dimension_),
          leftWeight_(dimension_), rightWeight_(dimension_),
          stdDev_(dimension_) {

            QL_REQUIRE(dimension_>0,
                       "there must be at least one step");

            QL_REQUIRE(dimension_==timeGrid_.size()-1,
                       "GSG/timeGrid dimension mismatch ("
                       << dimension_ << "/" << timeGrid_.size()-1 << ")");

            std::vector<Real> variances(dimension_);
            for (Size i=0; i<dimension_; i++) {
                // problems here if the blackVol is asset dependant
                // dummy asset level is used
                variances[i] =
                    diffProcess->variance(0.0, 1.0, timeGrid_[i+1]);
            }
            initialize(variances);
        }

        template <class GSG>
        void BrownianBridge<GSG>::initialize(const std::vector<Real>& v) {

            QL_REQUIRE(v.size()==dimension_,
                       "GSG/variance vector dimension mismatch ("
                       << dimension_ << "/" << v.size() << ")");

            std::vector<Size> map(dimension_, 0);
            // map is used to indicate which points are already constructed.
            // If map[i] is zero, path point i is yet unconstructed.
            // map[i]-1 is the index of the variate that constructs
            // the path point # i.

            Size i,j,k,l;
            //  The first point in the construction is the global step.
            map[dimension_-1] = 1;
            //  The global step is constructed from the first variate.
            bridgeIndex_[0] = dimension_-1;
            //  The variance of the global step
            stdDev_[0] = std::sqrt(v[dimension_-1]);
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
                if (j) {
                    leftWeight_[i]  = (v[k]-v[l])  /(v[k]-v[j-1]);
                    rightWeight_[i] = (v[l]-v[j-1])/(v[k]-v[j-1]);
                    stdDev_[i] =
                        std::sqrt(((v[l]-v[j-1])*(v[k]-v[l]))/(v[k]-v[j-1]));
                } else {
                    leftWeight_[i]  = (v[k]-v[l])  /v[k];
                    rightWeight_[i] =  v[l]        /v[k];
                    stdDev_[i] = std::sqrt(v[l]*(v[k]-v[l])/v[k]);
                }
                j=k+1;
                if (j>=dimension_) j=0;	//	Wrap around.
            }
        }

        template <class GSG>
        inline const typename BrownianBridge<GSG>::sample_type&
        BrownianBridge<GSG>::next() const {

            typedef typename GSG::sample_type sequence_type;
            const sequence_type& sequence_ = generator_.nextSequence();

            next_.weight = sequence_.weight;

            Size i, j, k, l;
            // The global step.
            next_.value[dimension_-1] = stdDev_[0] * sequence_.value[0];
            for (i=1; i<dimension_; i++) {
                j = leftIndex_[i];
                k = rightIndex_[i];
                l = bridgeIndex_[i];
                // using only the drift component here for the time being ....
                if (j) next_.value[l] =
                    leftWeight_[i] * next_.value[j-1] +
                    rightWeight_[i] * next_.value[k]   +
                    stdDev_[i] * sequence_.value[i];
                else   next_.value[l] =
                    rightWeight_[i] * next_.value[k]   +
                    stdDev_[i] * sequence_.value[i];
            }

            return next_;
        }

    }

    #ifndef QL_DISABLE_DEPRECATED
    using Old::BrownianBridge;
    #endif



    namespace New {

        //! Builds Wiener process paths using Gaussian variates
        /*! This class generates normalized (i.e., unit-variance)
            paths as sequences of variations. In order to obtain the
            actual path of the underlying, the returned variations
            must be multiplied by the integrated variance (including
            time) over the corresponding time step.

            \ingroup mcarlo
        */
        class BrownianBridge {
          public:
            //! unit-time path
            BrownianBridge(Size steps);
            //! generic times
            /*! \note the starting time of the path is assumed to be 0
                      and must not be included
            */
            BrownianBridge(const std::vector<Time>& times);
            //! generic times
            BrownianBridge(const TimeGrid& timeGrid);
            //! \name inspectors
            //@{
            Size size() const;
            const std::vector<Time>& times() const;
            //@}
            //! \name Brownian-bridge constructor
            //@{
            template <class RandomAccessIterator1,
                      class RandomAccessIterator2>
            void transform(RandomAccessIterator1 begin,
                           RandomAccessIterator1 end,
                           RandomAccessIterator2 output) const {
                // We use output to store the path...
                output[size_-1] = stdDev_[0] * begin[0];
                for (Size i=1; i<size_; ++i) {
                    Size j = leftIndex_[i];
                    Size k = rightIndex_[i];
                    Size l = bridgeIndex_[i];
                    if (j != 0) {
                        output[l] =
                            leftWeight_[i] * output[j-1] +
                            rightWeight_[i] * output[k]   +
                            stdDev_[i] * begin[i];
                    } else {
                        output[l] =
                            rightWeight_[i] * output[k]   +
                            stdDev_[i] * begin[i];
                    }
                }
                // ...after which, we calculate the variations and
                // normalize to unit times
                for (Size i=size_-1; i>=1; --i) {
                    output[i] -= output[i-1];
                    output[i] /= sqrtdt_[i];
                }
                output[0] /= sqrtdt_[0];
            }
          private:
            void initialize();
            Size size_;
            std::vector<Time> t_;
            std::vector<Real> sqrtdt_;
            std::vector<Size> bridgeIndex_, leftIndex_, rightIndex_;
            std::vector<Real> leftWeight_, rightWeight_, stdDev_;
        };

        inline Size BrownianBridge::size() const {
            return size_;
        }

        inline const std::vector<Time>& BrownianBridge::times() const {
            return t_;
        }

    }

}


#endif
