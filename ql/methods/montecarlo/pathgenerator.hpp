/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006 StatPro Italia srl
 Copyright (C) 2015 Peter Caspers

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

/*! \file pathgenerator.hpp
    \brief Generates random paths using a sequence generator
*/

#ifndef quantlib_montecarlo_path_generator_hpp
#define quantlib_montecarlo_path_generator_hpp

#include <ql/methods/montecarlo/brownianbridge.hpp>
#include <ql/stochasticprocess.hpp>

namespace QuantLib {
    class StochasticProcess;
    class StochasticProcess1D;
    //! Generates random paths using a sequence generator
    /*! Generates random paths with drift(S,t) and variance(S,t)
        using a gaussian sequence generator

        \ingroup mcarlo

        \test the generated paths are checked against cached results
    */
    template <class GSG>
    class PathGenerator {
      public:
        typedef Sample<Path> sample_type;
        // constructors
        PathGenerator(const boost::shared_ptr<StochasticProcess>&,
                      Time length,
                      Size timeSteps,
                      const GSG& generator,
                      bool brownianBridge);
        PathGenerator(const boost::shared_ptr<StochasticProcess>&,
                      const TimeGrid& timeGrid,
                      const GSG& generator,
                      bool brownianBridge);
        //! \name inspectors
        //@{
        const sample_type& next(unsigned int threadId = 0) const;
        const sample_type& antithetic(unsigned int threadId = 0) const;
        Size size() const { return dimension_; }
        const TimeGrid& timeGrid() const { return timeGrid_; }
        //@}
      private:
        const sample_type& next(bool antithetic, unsigned int threadId = 0) const;
        bool brownianBridge_;
        GSG generator_;
        Size dimension_;
        TimeGrid timeGrid_;
        boost::shared_ptr<StochasticProcess1D> process_;
        mutable std::vector<sample_type> next_;
        mutable std::vector<std::vector<Real> > temp_;
        BrownianBridge bb_;
    };


    // template definitions

    template <class GSG>
    PathGenerator<GSG>::PathGenerator(
                          const boost::shared_ptr<StochasticProcess>& process,
                          Time length,
                          Size timeSteps,
                          const GSG& generator,
                          bool brownianBridge)
    : brownianBridge_(brownianBridge), generator_(generator),
      dimension_(generator_.dimension()), timeGrid_(length, timeSteps),
      process_(boost::dynamic_pointer_cast<StochasticProcess1D>(process)),
      next_(std::vector<sample_type>(GSG::maxNumberOfThreads,
                                     sample_type(Path(timeGrid_),1.0))),
      temp_(GSG::maxNumberOfThreads,std::vector<Real>(dimension_)), bb_(timeGrid_) {
        QL_REQUIRE(dimension_==timeSteps,
                   "sequence generator dimensionality (" << dimension_
                   << ") != timeSteps (" << timeSteps << ")");
    }

    template <class GSG>
    PathGenerator<GSG>::PathGenerator(
                          const boost::shared_ptr<StochasticProcess>& process,
                          const TimeGrid& timeGrid,
                          const GSG& generator,
                          bool brownianBridge)
    : brownianBridge_(brownianBridge), generator_(generator),
      dimension_(generator_.dimension()), timeGrid_(timeGrid),
      process_(boost::dynamic_pointer_cast<StochasticProcess1D>(process)),
      next_(std::vector<sample_type>(GSG::maxNumberOfThreads,
                                     sample_type(Path(timeGrid_),1.0))),
      temp_(GSG::maxNumberOfThreads,std::vector<Real>(dimension_)), bb_(timeGrid_) {
        QL_REQUIRE(dimension_==timeGrid_.size()-1,
                   "sequence generator dimensionality (" << dimension_
                   << ") != timeSteps (" << timeGrid_.size()-1 << ")");
    }

    template <class GSG>
    const typename PathGenerator<GSG>::sample_type&
    PathGenerator<GSG>::next(unsigned int threadId) const {
        return next(false, threadId);
    }

    template <class GSG>
    const typename PathGenerator<GSG>::sample_type&
    PathGenerator<GSG>::antithetic(unsigned int threadId) const {
        return next(true, threadId);
    }

    template <class GSG>
    const typename PathGenerator<GSG>::sample_type&
    PathGenerator<GSG>::next(bool antithetic, unsigned int threadId) const {
        QL_REQUIRE(threadId < GSG::maxNumberOfThreads,
                   "thread id (" << threadId << ") out of bounds [0..."
                   << GSG::maxNumberOfThreads - 1 << "]");

        typedef typename GSG::sample_type sequence_type;
        const sequence_type& sequence_ =
            antithetic ? generator_.lastSequence(threadId)
                       : generator_.nextSequence(threadId);

        if (brownianBridge_) {
            bb_.transform(sequence_.value.begin(),
                          sequence_.value.end(),
                          temp_[threadId].begin());
        } else {
            std::copy(sequence_.value.begin(),
                      sequence_.value.end(),
                      temp_[threadId].begin());
        }

        next_[threadId].weight = sequence_.weight;

        Path& path = next_[threadId].value;
        path.front() = process_->x0();

        for (Size i=1; i<path.length(); i++) {
            Time t = timeGrid_[i-1];
            Time dt = timeGrid_.dt(i-1);
            path[i] = process_->evolve(t, path[i-1], dt,
                                       antithetic ? -temp_[threadId][i-1] :
                                                     temp_[threadId][i-1]);
        }
        return next_[threadId];
    }

}


#endif
