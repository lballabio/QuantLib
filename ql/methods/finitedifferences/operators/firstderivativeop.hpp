/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Andreas Gaida
 Copyright (C) 2008 Ralph Schreyer
 Copyright (C) 2008 Klaus Spanderen

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

/*! \file firstderivativeop.hpp
    \brief first derivative linear operator
*/

#ifndef quantlib_first_derivative_op_hpp
#define quantlib_first_derivative_op_hpp

#include <ql/methods/finitedifferences/operators/triplebandlinearop.hpp>

namespace QuantLib {

    class FirstDerivativeOp : public TripleBandLinearOp {
      public:
        FirstDerivativeOp(Size direction,
                          const ext::shared_ptr<FdmMesher>& mesher);
    };

}

#endif


#ifndef id_2023136e72cd290eddfe593a196191ba
#define id_2023136e72cd290eddfe593a196191ba
inline bool test_2023136e72cd290eddfe593a196191ba(int* i) { return i != 0; }
#endif
