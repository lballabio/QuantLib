
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
#include <ql/MonteCarlo/brownianbridge.hpp>

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
        PathGenerator(const boost::shared_ptr<DiffusionProcess>& diffProcess,
                      Time length,
                      Size timeSteps,
                      const GSG& generator,
                      bool brownianBridge);
        PathGenerator(const boost::shared_ptr<DiffusionProcess>& diffProcess,
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
        boost::shared_ptr<DiffusionProcess> diffProcess_;
        mutable sample_type next_;
        BrownianBridge<GSG> bb_;
        mutable double asset_;
    };

    template <class GSG>
    PathGenerator<GSG>::PathGenerator(
        const boost::shared_ptr<DiffusionProcess>& diffProcess,
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
        const boost::shared_ptr<DiffusionProcess>& diffProcess,
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
                   + SizeFormatter::toString(dimension_) +
                   ") != timeSteps ("
                   + SizeFormatter::toString(timeGrid_.size()-1) +
                   ")");
    }

    template <class GSG>
    const typename PathGenerator<GSG>::sample_type&
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
                asset_ *= QL_EXP(next_.value[i]);
            }

            return next_;
        }
    }

    template <class GSG>
    const typename PathGenerator<GSG>::sample_type&
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
                asset_ *= QL_EXP(next_.value[i]);
            }

            return next_;
        }
    }

}


#endif
