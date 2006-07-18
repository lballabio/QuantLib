/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2001, 2002, 2003 Nicolas Di Césaré

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

#include <ql/Optimization/criteria.hpp>
#include <ql/errors.hpp>

namespace QuantLib {

    EndCriteria::EndCriteria()
    : maxIteration_(100), functionEpsilon_(1e-8),
      gradientEpsilon_(1e-8), maxIterStatPt_(10),
      statState_(0), endCriteria_(none),
      positiveOptimization_(true) {}

    EndCriteria::EndCriteria(Size maxIteration, Real epsilon)
    : maxIteration_(maxIteration), functionEpsilon_(epsilon),
      gradientEpsilon_(epsilon), maxIterStatPt_(maxIteration/10),
      statState_(0), endCriteria_(none),
      positiveOptimization_(true) {}

	std::ostream& operator<<(std::ostream& out,  EndCriteria::Type ec) {
		switch (ec) {
		case QuantLib::EndCriteria::none:
			return out << "None";
		case QuantLib::EndCriteria::maxIter:
			return out << "MaxIterations";
		case QuantLib::EndCriteria::statPt:
			return out << "StationaryPoint";
		case QuantLib::EndCriteria::statGd:
			return out << "StationaryGradient";
		default:
            QL_FAIL("unknown EndCriteria::Type (" << Integer(ec) << ")");
		}
	}

}
