
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

/*! \file secant.h
	\brief secant 1-D solver
	
	$Source$
	$Name$
	$Log$
	Revision 1.3  2000/12/14 12:32:30  lballabio
	Added CVS tags in Doxygen file documentation blocks

*/

#ifndef quantlib_solver1d_secant_h
#define quantlib_solver1d_secant_h

#include "qldefines.h"
#include "solver1d.h"

namespace QuantLib {

	namespace Solvers1D {
	
		class Secant : public Solver1D {
		  public:
			Secant() : Solver1D() {}
		  private:
			double _solve(const ObjectiveFunction& f, double xAccuracy) const;
		};
	
	}

}

#endif
