
/*
 * Copyright (C) 2000-2001 QuantLib Group
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
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/

/*! \file knuthrandomgenerator.hpp
    \brief Knuth uniform random number generator

    $Id$
*/

// $Source$
// $Log$
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
