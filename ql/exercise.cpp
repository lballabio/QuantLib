
/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
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

/*! \file exercise.cpp
    \brief Option exercise classes and exercise payoff function
*/

// $Id$

#include <ql/exercise.hpp>

namespace QuantLib {

    AmericanExercise::AmericanExercise(Date earliest, Date latest)
    : Exercise() {
        type_ = American;
        dates_ = std::vector<Date>(2);
        dates_[0] = earliest;
        dates_[1] = latest;
    }

    BermudanExercise::BermudanExercise(const std::vector<Date>& dates)
    : Exercise() {

        QL_REQUIRE(dates.size()>0,
            "BermudanExercise::BermudanExercise : "
            "no dates given");
        dates_ = dates;

        // if the following approach is not viable
        // we should require that dates.size()>1 above
        if (dates.size()==1) type_=European;
        else type_ = Bermudan;
    }

    EuropeanExercise::EuropeanExercise(Date date)
    : Exercise() {
        type_ = European;
        dates_ = std::vector<Date>(1,date);
    }



}
