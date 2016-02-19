/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

/*! \file longstaffschwartzproxypathpricer.hpp
    \brief ls pricer providing an extended regression model
           also for the otm states, for one dimension
*/

#ifndef quantlib_longstaff_schwartz_proxy_path_pricer_hpp
#define quantlib_longstaff_schwartz_proxy_path_pricer_hpp

#include <ql/methods/montecarlo/longstaffschwartzpathpricer.hpp>
#include <ql/methods/montecarlo/mctraits.hpp>

namespace QuantLib {

class LongstaffSchwartzProxyPathPricer
    : public LongstaffSchwartzPathPricer<SingleVariate<>::path_type> {
  public:
    typedef EarlyExerciseTraits<SingleVariate<>::path_type>::StateType StateType;
    typedef SingleVariate<>::path_type PathType;

    LongstaffSchwartzProxyPathPricer(
        const TimeGrid &times,
        const boost::shared_ptr<EarlyExercisePathPricer<PathType> > &pricer,
        const boost::shared_ptr<YieldTermStructure> &termStructure);

    const std::vector<boost::function1<Real, StateType> > basisSystem() const {
        return v_;
    }
    const std::vector<Array> coefficientsItm() const { return coeffItm_; }
    const std::vector<Array> coefficientsOtm() const { return coeffOtm_; }
    const StateType cutoff() const { return cutoff_; }

  protected:
    void post_processing(const Size i, const std::vector<StateType> &state,
                         const std::vector<Real> &price,
                         const std::vector<Real> &exercise);

    std::vector<Array> coeffItm_, coeffOtm_;
    StateType cutoff_;
};

} // namespace

#endif
