/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Andrea Odetti

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

#include <ql/experimental/mcbasket/adaptedpathpayoff.hpp>

namespace QuantLib {

  /*
    Initializing maximumTimeRead_ to -1 would make more sense,
    but it is unsigned and 0 has exactly the same behaviour.
   */
  AdaptedPathPayoff::ValuationData::ValuationData(
                                              const Matrix       & path,
                                              Array              & payments,
                                              Array              & exercises,
                                              std::vector<Array> & states) :
    path_(path),
    payments_(payments), exercises_(exercises), states_(states),
    maximumTimeRead_(0)
  { }

  Size AdaptedPathPayoff::ValuationData::numberOfTimes() const {
    return path_.columns();
  }

  Size AdaptedPathPayoff::ValuationData::numberOfAssets() const {
    return path_.rows();
  }

  Real AdaptedPathPayoff::ValuationData::getAssetValue(Size time, Size asset) {
    maximumTimeRead_ = std::max(maximumTimeRead_, time);

    return path_[asset][time];
  }

  void AdaptedPathPayoff::ValuationData::setPayoffValue(Size time, Real value) {
    /*
      This is to ensure the payoff is an adapted function.
      We prevent payments to depend on future fixings.
     */
    QL_REQUIRE(time >= maximumTimeRead_,
               "not adapted payoff: looking into the future");

    payments_[time] = value;
  }

  void AdaptedPathPayoff::ValuationData::setExerciseData(
                                     Size time, Real exercise, Array & state) {
    /*
      This is to ensure the payoff is an adapted function.
      We prevent payments to depend on future fixings.
     */
    QL_REQUIRE(time >= maximumTimeRead_,
               "not adapted payoff: looking into the future");

    if (!exercises_.empty())
      exercises_[time] = exercise;

    if (!states_.empty())
      std::swap(states_[time], state);
  }


  void AdaptedPathPayoff::value(const Matrix       & path,
                                Array              & payments,
                                Array              & exercises,
                                std::vector<Array> & states) const {
    ValuationData data(path, payments, exercises, states);

    operator()(data);
  }
}
