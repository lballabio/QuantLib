
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

// $Source$

// $Log$
// Revision 1.2  2000/12/13 18:09:53  nando
// CVS keyword added
//

#ifndef quantlib_solver1d_ridder_h
#define quantlib_solver1d_ridder_h

#include "qldefines.h"
#include "solver1d.h"

namespace QuantLib {

	namespace Solvers1D {
	
		class Ridder : public Solver1D {
		  public:
			Ridder() : Solver1D() {}
		  private:
			double _solve(const ObjectiveFunction& f, double xAccuracy) const;
		};
	
	}

}

#endif
