
/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano
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

/*! \file pathgenerator.hpp
    \brief Generates random paths using a sequence generator
*/

#ifndef quantlib_montecarlo_path_generator_h
#define quantlib_montecarlo_path_generator_h

#include <ql/diffusionprocess.hpp>
#include <ql/MonteCarlo/path.hpp>
#include <ql/MonteCarlo/brownianbridge.hpp>
#include <ql/RandomNumbers/randomarraygenerator.hpp>

namespace QuantLib {

    //! Generates random paths using a sequence generator
    /*! Generates random paths with drift(S,t) and variance(S,t)
        using a gaussian sequence generator
    */
    template <class GSG>
    class PathGenerator {
      public:
        typedef Sample<Path> sample_type;
        // constructors
        PathGenerator(const Handle<DiffusionProcess>& diffProcess,
                      Time length,
                      Size timeSteps,
                      const GSG& generator,
                      bool brownianBridge);
        PathGenerator(const Handle<DiffusionProcess>& diffProcess,
                      const TimeGrid& timeGrid,
                      const GSG& generator,
                      bool brownianBridge);
        //! \name inspectors
        //@{
        const sample_type& next() const;
        const sample_type& antithetic() const;
        Size size() const { return dimension_; }
        const TimeGrid& timeGrid() const { return timeGrid_; }
        //@}
      private:
        bool brownianBridge_;
        GSG generator_;
        Size dimension_;
        TimeGrid timeGrid_;
        Handle<DiffusionProcess> diffProcess_;
        mutable sample_type next_;
        BrownianBridge<GSG> bb_;
        mutable double asset_;
    };

    template <class GSG>
    PathGenerator<GSG>::PathGenerator(
        const Handle<DiffusionProcess>& diffProcess,
        Time length,
        Size timeSteps,
        const GSG& generator,
        bool brownianBridge)
    : brownianBridge_(brownianBridge), generator_(generator),
      dimension_(generator_.dimension()),
      timeGrid_(length, timeSteps), diffProcess_(diffProcess),
      next_(Path(timeGrid_),1.0),
      bb_(diffProcess_, timeGrid_, generator_) {
        QL_REQUIRE(dimension_==timeSteps,
                   "PathGenerator::PathGenerator :"
                   "sequence generator dimensionality ("
                   + IntegerFormatter::toString(dimension_) +
                   ") != timeSteps ("
                   + IntegerFormatter::toString(timeSteps) +
                   ")");
    }

    template <class GSG>
    PathGenerator<GSG>::PathGenerator(
        const Handle<DiffusionProcess>& diffProcess,
        const TimeGrid& timeGrid,
        const GSG& generator,
        bool brownianBridge)
    : brownianBridge_(brownianBridge), generator_(generator),
      dimension_(generator_.dimension()),
      timeGrid_(timeGrid), diffProcess_(diffProcess),
      next_(Path(timeGrid_),1.0),
      bb_(diffProcess_, timeGrid_, generator_) {
        QL_REQUIRE(dimension_==timeGrid_.size()-1,
                   "PathGenerator::PathGenerator :"
                   "sequence generator dimensionality ("
                   + IntegerFormatter::toString(dimension_) +
                   ") != timeSteps ("
                   + IntegerFormatter::toString(timeGrid_.size()-1) +
                   ")");
    }

    template <class GSG>
    inline const typename PathGenerator<GSG>::sample_type&
    PathGenerator<GSG>::next() const {


        if (brownianBridge_) {
            typedef typename BrownianBridge<GSG>::sample_type sequence_type;
            const sequence_type& stdDev_ = bb_.next();

            next_.weight = stdDev_.weight;

            Time t = timeGrid_[1];
            double dt= timeGrid_.dt(0);
            next_.value.drift()[0] = dt * 
                diffProcess_->drift(t, asset_);
            next_.value.diffusion()[0] = stdDev_.value[0];
            for (Size i=1; i<next_.value.size(); i++) {
                t = timeGrid_[i+1];
                dt = timeGrid_.dt(i);
                next_.value.drift()[i] = dt * 
                    diffProcess_->drift(t, asset_);
                next_.value.diffusion()[i] =
                    stdDev_.value[i] - stdDev_.value[i-1];
            }
            return next_;
        } else {
            typedef typename GSG::sample_type sequence_type;
            const sequence_type& sequence_ = generator_.nextSequence();

            next_.weight = sequence_.weight;

            // starting point for asset value
            asset_ = diffProcess_->x0();
            double dt;
            Time t;
            for (Size i=0; i<next_.value.size(); i++) {
                t = timeGrid_[i+1];
                dt = timeGrid_.dt(i);
                next_.value.drift()[i] = dt * 
                    diffProcess_->drift(t, asset_);
                next_.value.diffusion()[i] = sequence_.value[i] *
                    QL_SQRT(diffProcess_->variance(t, asset_, dt));
                asset_ *= QL_EXP(next_.value.drift()[i] + 
                                 next_.value.diffusion()[i]);
            }

            return next_;
        }
    }

    template <class GSG>
    inline const typename PathGenerator<GSG>::sample_type&
    PathGenerator<GSG>::antithetic() const {

        if (brownianBridge_) {
            typedef typename BrownianBridge<GSG>::sample_type sequence_type;
            const sequence_type& stdDev_ = bb_.last();

            next_.weight = stdDev_.weight;

            Time t = timeGrid_[1];
            double dt= timeGrid_.dt(0);
            next_.value.drift()[0] = dt * 
                diffProcess_->drift(t, asset_);
            next_.value.diffusion()[0] = - stdDev_.value[0];
            for (Size i=1; i<next_.value.size(); i++) {
                t = timeGrid_[i+1];
                dt = timeGrid_.dt(i);
                next_.value.drift()[i] = dt * 
                    diffProcess_->drift(t, asset_);
                next_.value.diffusion()[i] =
                    - stdDev_.value[i] + stdDev_.value[i-1];
            }
            return next_;
        } else {
            typedef typename GSG::sample_type sequence_type;
            const sequence_type& sequence_ = generator_.lastSequence();

            next_.weight = sequence_.weight;

            // starting point for asset value
            asset_ = diffProcess_->x0();
            double dt;
            Time t;
            for (Size i=0; i<next_.value.size(); i++) {
                t = timeGrid_[i+1];
                dt = timeGrid_.dt(i);
                next_.value.drift()[i] = dt * 
                    diffProcess_->drift(t, asset_);
                next_.value.diffusion()[i] = - sequence_.value[i] *
                    QL_SQRT(diffProcess_->variance(t, asset_, dt));
                asset_ *= QL_EXP(next_.value.drift()[i] + 
                                 next_.value.diffusion()[i]);
            }

            return next_;
        }
    }



    //! Generates random paths from a random number generator
    /*! \deprecated use PathGenerator instead */
    template <class RNG>
    class PathGenerator_old {
      public:
        typedef Sample<Path> sample_type;
        // constructors
        PathGenerator_old(double drift,
                          double variance,
                          Time length,
                          Size timeSteps,
                          long seed = 0);
        /*! \warning the initial time is assumed to be zero
          and must <b>not</b> be included in the passed vector */
        PathGenerator_old(double drift,
                          double variance,
                          const TimeGrid& times,
                          long seed = 0);
        PathGenerator_old(const std::vector<double>& drift,
                          const std::vector<double>& variance,
                          const TimeGrid& times,
                          long seed = 0);
        //! \name inspectors
        //@{
        const sample_type& next() const;
        const sample_type& antithetic() const {
            QL_FAIL("old framework doesn't support antithetic here");}
        Size size() const { return next_.value.size(); }
        //@}
      private:
        mutable Sample<Path> next_;
        Handle<RandomArrayGenerator<RNG> > generator_;
    };

    template <class RNG>
    PathGenerator_old<RNG>::PathGenerator_old(double drift, 
                                              double variance, Time length, 
                                              Size timeSteps, long seed)
    : next_(Path(TimeGrid(length, timeSteps)),1.0) {
        QL_REQUIRE(timeSteps > 0, "PathGenerator_old: Time steps(" +
                   IntegerFormatter::toString(timeSteps) +
                   ") must be greater than zero");
        QL_REQUIRE(length > 0, "PathGenerator_old: length must be > 0");
        Time dt = length/timeSteps;

        next_.value.drift() = Array(timeSteps, drift*dt);

        QL_REQUIRE(variance >= 0.0, 
                   "PathGenerator_old: negative variance");
        generator_ = Handle<RandomArrayGenerator<RNG> >(
                         new RandomArrayGenerator<RNG>(
                                        Array(timeSteps, variance*dt), seed));
    }

    template <class RNG>
    PathGenerator_old<RNG>::PathGenerator_old(double drift, 
                                              double variance, 
                                              const TimeGrid& times, long seed)
    : next_(Path(times),1.0) {
        QL_REQUIRE(variance >= 0.0, 
                   "PathGenerator_old: negative variance");
        QL_REQUIRE(times.size() > 1, "PathGenerator_old: no times given");
        Array variancePerTime(times.size()-1);
        for(Size i = 0; i < times.size()-1; i++) {
            next_.value.drift()[i] = drift*times.dt(i);
            variancePerTime[i] = variance*times.dt(i);
        }

        generator_ = Handle<RandomArrayGenerator<RNG> >(
                         new RandomArrayGenerator<RNG>(variancePerTime, seed));
    }

    template <class RNG>
    PathGenerator_old<RNG>::PathGenerator_old(
                                          const std::vector<double>& drift,
                                          const std::vector<double>& variance,
                                          const TimeGrid& times, long seed)
    : next_(Path(times),1.0) {
        QL_REQUIRE(times.size() > 1, "PathGenerator_old: no times given");
        QL_REQUIRE(variance.size()==times.size()-1,
                   "PathGenerator_old: "
                   "mismatch between variance and time arrays");
        QL_REQUIRE(drift.size()==times.size()-1,
                   "PathGenerator_old: mismatch between "
                   "drift and time arrays");

        Array variancePerTime(times.size()-1);
        for(Size i = 0; i < times.size()-1; i++) {
            next_.value.drift()[i] = drift[i]*times.dt(i);
            QL_REQUIRE(variance[i] >= 0.0, 
                       "PathGenerator_old: negative variance");
            variancePerTime[i] = variance[i]*times.dt(i);
        }

        generator_ = Handle<RandomArrayGenerator<RNG> >(
                         new RandomArrayGenerator<RNG>(variancePerTime, seed));
    }

    template <class RNG>
    inline const typename PathGenerator_old<RNG>::sample_type&
    PathGenerator_old<RNG>::next() const {
        const Sample<Array>& sample = generator_->next();
        next_.weight = sample.weight;
        next_.value.diffusion() = sample.value;
        return next_;
    }

}


#endif
