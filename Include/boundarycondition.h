
/*
 * Copyright (C) 2000
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
 * 
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated 
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, contact ferdinando@ametrano.net
 *
 * QuantLib license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
*/

#ifndef quantlib_boundary_condition
#define quantlib_boundary_condition

#include "qldefines.h"
#include "null.h"
#include "qlerrors.h"

namespace QuantLib {

	namespace PDE {
	
		// WARNING: for Neumann conditions. the value passed must not be the value of the derivative.
		// Instead, it must be comprehensive of the grid step between the first two points--i.e., it must 
		// be the difference between f[0] and f[1].
		
		class BoundaryCondition {
		  public:
			// enumeration
			enum Type { None, Neumann, Dirichlet };
			// constructors
			BoundaryCondition(Type type = None, double value = Null<double>())
			: theType(type), theValue(value) {
				if (theType != None)
					Require(!IsNull(value), "A value must be supplied for this type of boundary condition");
			}
			// access methods
			Type type() const { return theType; }
			double value() const { return theValue; }
		  private:
			Type theType;
			double theValue;
		};
	
	}

}


#endif
