
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
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
