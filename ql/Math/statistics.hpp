
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

/*! \file statistics.hpp
    \brief statistics tool with risk measures
*/

// $Id$

#ifndef quantlib_statistics_h
#define quantlib_statistics_h

#include <ql/Math/generalstatistics.hpp>
#include <ql/Math/incrementalstatistics.hpp>
#include <ql/Math/gaussianstatistics.hpp>
#include <ql/Math/riskstatistics.hpp>

namespace QuantLib {

    namespace Math {

        //! default statistics tool
        typedef GeneralStatistics Statistics;

    }

}


#endif
