

/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

// $Id$

#ifndef quantlib_montecarlo_path_generator_h
#define quantlib_montecarlo_path_generator_h

#include <ql/MonteCarlo/path.hpp>
#include <ql/RandomNumbers/randomarraygenerator.hpp>
#include <ql/diffusionprocess.hpp>
#include <ql/grid.hpp>

namespace QuantLib {

    namespace MonteCarlo {

        //! Generates random paths from a random number generator
        /*! \todo add more general path generator with drift(S,t) and
                  variance(S,t)
        */
        template <class RNG>
        class PathGenerator {
          public:
            typedef Sample<Path> sample_type;
            // constructors
            PathGenerator(double drift,
                          double variance,
                          Time length,
                          Size timeSteps,
                          long seed = 0);
            /*! \warning the initial time is assumed to be zero
                and must <b>not</b> be included in the passed vector */
            PathGenerator(double drift,
                          double variance,
                          const std::vector<Time>& times,
                          long seed = 0);
            PathGenerator(const std::vector<double>& drift,
                          const std::vector<double>& variance,
                          const std::vector<Time>& times,
                          long seed = 0);
            //! \name inspectors
            //@{
            const sample_type& next() const;
            Size size() const { return next_.size(); }
            //@}
          private:
            mutable Sample<Path> next_;
            Handle<RandomNumbers::RandomArrayGenerator<RNG> > generator_;
        };

        template <class RNG>
        PathGenerator<RNG>::PathGenerator(double drift, double variance,
            Time length, Size timeSteps, long seed)
        : next_(Path(timeSteps),1.0) {
            QL_REQUIRE(timeSteps > 0, "PathGenerator: Time steps(" +
                IntegerFormatter::toString(timeSteps) +
                ") must be greater than zero");
            QL_REQUIRE(length > 0, "PathGenerator: length must be > 0");
            Time dt = length/timeSteps;
            for (Size i=0; i<timeSteps; i++) {
                next_.value.times()[i] = (i+1)*dt;
            }

            next_.value.drift() = Array(timeSteps, drift*dt);

            QL_REQUIRE(variance >= 0.0, "PathGenerator: negative variance");
            generator_ = Handle<RandomNumbers::RandomArrayGenerator<RNG> >(
                new RandomNumbers::RandomArrayGenerator<RNG>(
                    Array(timeSteps, variance*dt), seed));
        }

        template <class RNG>
        PathGenerator<RNG>::PathGenerator(double drift, double variance,
            const std::vector<Time>& times, long seed)
        : next_(Path(times.size()),1.0) {
            QL_REQUIRE(variance >= 0.0, "PathGenerator: negative variance");
            QL_REQUIRE(times.size() > 0, "PathGenerator: no times given");
            QL_REQUIRE(times[0] >= 0.0, "PathGenerator: first time(" +
                 DoubleFormatter::toString(times[0]) + ") must be non negative");
            Array variancePerTime(times.size());
            Time dt = times[0];
            next_.value.drift()[0] = drift*dt;
            variancePerTime[0] = variance*dt;
            for(Size i = 1; i < times.size(); i++) {
                QL_REQUIRE(times[i] >= times[i-1],
                    "MultiPathGenerator: time(" +
                    IntegerFormatter::toString(i-1)+")=" +
                    DoubleFormatter::toString(times[i-1]) +
                    " is later than time(" +
                    IntegerFormatter::toString(i) + ")=" +
                    DoubleFormatter::toString(times[i]));
                dt = times[i] - times[i-1];
                next_.value.drift()[i] = drift*dt;
                variancePerTime[i] = variance*dt;
            }
            next_.value.times() = times;

            generator_ = Handle<RandomNumbers::RandomArrayGenerator<RNG> >(
                new RandomNumbers::RandomArrayGenerator<RNG>(variancePerTime,
                seed));
        }

        template <class RNG>
        PathGenerator<RNG>::PathGenerator(
            const std::vector<double>& drift,
            const std::vector<double>& variance,
            const std::vector<Time>& times, long seed)
        : next_(Path(times.size()),1.0) {
            QL_REQUIRE(times.size() > 0, "PathGenerator: no times given");
            QL_REQUIRE(times[0] >= 0.0, "PathGenerator: first time(" +
                 DoubleFormatter::toString(times[0]) + ") must be non negative");

            QL_REQUIRE(variance.size()==times.size(),
                "PathGenerator: mismatch between variance and time arrays");
            QL_REQUIRE(drift.size()==times.size(),
                "PathGenerator: mismatch between drift and time arrays");
            
            
            Array variancePerTime(times.size());
            double dt = times[0];
            next_.value.drift()[0] = drift[0]*dt;
            QL_REQUIRE(variance[0] >= 0.0, "PathGenerator: negative variance");
            variancePerTime[0] = variance[0]*dt;
            for(Size i = 1; i < times.size(); i++) {
                QL_REQUIRE(times[i] >= times[i-1],
                    "MultiPathGenerator: time(" +
                    IntegerFormatter::toString(i-1)+")=" +
                    DoubleFormatter::toString(times[i-1]) +
                    " is later than time(" +
                    IntegerFormatter::toString(i) + ")=" +
                    DoubleFormatter::toString(times[i]));
                dt = times[i] - times[i-1];
                next_.value.drift()[i] = drift[i]*dt;
                QL_REQUIRE(variance[i] >= 0.0, "PathGenerator: negative variance");
                variancePerTime[i] = variance[i]*dt;
            }
            next_.value.times() = times;

            generator_ = Handle<RandomNumbers::RandomArrayGenerator<RNG> >(
                new RandomNumbers::RandomArrayGenerator<RNG>(variancePerTime,
                seed));
        }
    
        template <class RNG>
        inline const typename PathGenerator<RNG>::sample_type&
        PathGenerator<RNG>::next() const {
            const Sample<Array>& sample = generator_->next();
            next_.weight = sample.weight;
            next_.value.diffusion() = sample.value;
            return next_;
        }






        //! Generates random paths using a sequence generator
        /*! Generates random paths with drift(S,t) and variance(S,t)
            using a sequence generator
        */
        template <class SG>
        class PathGenerator2 {
          public:
            typedef Sample<Path> sample_type;
            // constructors
            PathGenerator2(const Handle<DiffusionProcess>& diffProcess,
                           Time length,
                           Size timeSteps,
                           SG generator);
            PathGenerator2(const Handle<DiffusionProcess>& diffProcess,
                           const Handle<TimeGrid>& times,
                           SG generator);
            //! \name inspectors
            //@{
            const sample_type& next() const;
            const sample_type& antithetic() const;
            Size size() const { return next_.size(); }
            //@}
          private:
            mutable Sample<Path> next_;
            mutable typename SG::sample_type sequence_;
            mutable double asset_;
            Handle<DiffusionProcess> diffProcess_;
            SG generator_;
            Size dimension_;
            Handle<TimeGrid> times_;
        };

        template <class SG>
        PathGenerator2<SG>::PathGenerator2(
            const Handle<DiffusionProcess>& diffProcess,
            Time length, Size timeSteps, SG generator)
        : next_(Path(timeSteps),1.0), sequence_(Array(timeSteps), 0.0),
          diffProcess_(diffProcess), generator_(generator),
          dimension_(generator_.dimension()) {
            QL_REQUIRE(dimension_==timeSteps,
                "PathGenerator2::PathGenerator2 :"
                "sequence generator dimensionality != timeSteps");
            times_ = Handle<TimeGrid>(new TimeGrid(length, timeSteps));
        }

        template <class SG>
        PathGenerator2<SG>::PathGenerator2(
            const Handle<DiffusionProcess>& diffProcess,
            const Handle<TimeGrid>& times, SG generator)
        : next_(Path(times.size())-1,1.0), sequence_(Array(times.size()-1), 0.0),
          diffProcess_(diffProcess), generator_(generator),
          dimension_(generator_.dimension()), times_(times) {
            QL_REQUIRE(dimension_==times_.size()-1,
                "PathGenerator2::PathGenerator2 :"
                "sequence generator dimensionality != timeSteps");
        }
    
        template <class SG>
        inline const typename PathGenerator2<SG>::sample_type&
        PathGenerator2<SG>::next() const {

            sequence_ = generator_.next();

            next_.weight = sequence_.weight;

            // starting point for asset value
            asset_ = diffProcess_->x0();
            double dt;
            Time t;
            for (Size i=0; i<next_.value.size(); i++) {
                t = (*times_)[i+1];
                dt = times_->dt(i);
                next_.value.drift()[i] = dt;
                next_.value.drift()[i] *= 
                    diffProcess_->drift(t, asset_);
                next_.value.diffusion()[i] = sequence_.value[i];
                next_.value.diffusion()[i] *=
                    QL_SQRT(diffProcess_->variance(t, asset_, dt));
                asset_ *= QL_EXP(next_.value.drift()[i] + next_.value.diffusion()[i]);
            }

            return next_;
        }

        template <class RNG>
        inline const typename PathGenerator2<RNG>::sample_type&
        PathGenerator2<RNG>::antithetic() const {

            // starting point for asset value
            asset_ = diffProcess_->x0();
            double dt;
            Time t;
            for (Size i=0; i<next_.value.size(); i++) {
                t = (*times_)[i+1];
                dt = times_->dt(i);
                next_.value.drift()[i] = dt;
                next_.value.drift()[i] *= 
                    diffProcess_->drift(t, asset_);
                next_.value.diffusion()[i] = - sequence_.value[i] *
                    QL_SQRT(diffProcess_->variance(t, asset_, dt));
                asset_ *= QL_EXP(next_.value.drift()[i] + next_.value.diffusion()[i]);
            }

            return next_;
        }






    }

}


#endif
