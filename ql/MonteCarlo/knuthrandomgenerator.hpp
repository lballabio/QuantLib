
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
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
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file knuthrandomgenerator.hpp

    \fullpath
    Include/ql/MonteCarlo/%knuthrandomgenerator.hpp
    \brief Knuth uniform random number generator

*/

// $Id$
// $Log$
// Revision 1.1  2001/09/03 13:56:11  nando
// source (*.hpp and *.cpp) moved under topdir/ql
//
// Revision 1.8  2001/08/31 15:23:46  sigmud
// refining fullpath entries for doxygen documentation
//
// Revision 1.7  2001/08/09 14:59:46  sigmud
// header modification
//
// Revision 1.6  2001/08/08 11:07:49  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.5  2001/08/07 11:25:54  sigmud
// copyright header maintenance
//
// Revision 1.4  2001/07/25 15:47:28  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.3  2001/07/19 16:40:10  lballabio
// Improved docs a bit
//
// Revision 1.2  2001/07/04 08:05:15  uid37043
// Worked around Visual C++ being off-standard again
//
// Revision 1.1  2001/07/03 13:19:38  lballabio
// Added Knuth random generator after doubts were casted on the NR one
//

#ifndef quantlib_knuth_random_generator_h
#define quantlib_knuth_random_generator_h

#include "ql/qldefines.hpp"
#include <vector>

namespace QuantLib {

    namespace MonteCarlo {

        //! Uniform random number generator
        /*! Random number generator by Knuth.
            For more details see Knuth, Seminumerical Algorithms, 3rd edition, 
            Section 3.6.
            \note This is <b>not</b> Knuth's original implementation which 
            is available at 
            http://www-cs-faculty.stanford.edu/~knuth/programs.html, 
            but rather a slightly modified version wrapped in a C++ class.
            Such modifications did not affect the code but only the data 
            structures used, which were converted in their C++/STL 
            equivalents.
        */
        class KnuthRandomGenerator {
          public:
            /*! if the given seed is 0, a random seed will be chosen
                based on clock() */
            explicit KnuthRandomGenerator(long seed = 0);
            typedef double SampleType;
            //! returns a random number uniformly chosen from (0.0,1.0)
            double next() const;
            //! uniformly returns 1.0
            double weight() const;
          private:
            /* Knuth's names and routines were preserved as much as possible
               while changing the data structures to more modern ones. */
            static const int KK, LL, TT, QUALITY;
            mutable std::vector<double> ranf_arr_buf;
            mutable std::vector<double>::const_iterator ranf_arr_ptr, 
                                                        ranf_arr_sentinel;
            mutable std::vector<double> ran_u;
            double mod_sum(double x, double y) const;
            bool is_odd(int s) const;
            void ranf_start(long seed);
            void ranf_array(std::vector<double>& aa, int n) const;
            double ranf_arr_cycle() const;
        };


        // inline definitions
        
        inline double KnuthRandomGenerator::next() const {
            return (ranf_arr_ptr != ranf_arr_sentinel ? 
                    *ranf_arr_ptr++ : 
                    ranf_arr_cycle());
        }

        inline double KnuthRandomGenerator::weight() const {
            return 1.0;
        }

        inline double KnuthRandomGenerator::mod_sum(double x, double y) const {
            return (x+y)-int(x+y);
        }
        
        inline bool KnuthRandomGenerator::is_odd(int s) const {
            return (s&1) != 0;
        }
        
    }

}


#endif
