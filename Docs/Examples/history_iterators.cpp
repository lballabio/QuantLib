
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

/*
    $Id$
    $Source$
    $Log$
    Revision 1.1  2001/06/22 14:54:48  nando
    doxygen example file moved under Docs/Examples

    Revision 1.5  2001/04/10 14:51:51  lballabio
    Added Microsoft Help format to Doxygen output

    Revision 1.4  2001/04/06 18:46:19  nando
    changed Authors, Contributors, Licence and copyright header

*/
    // print out the mean value and its standard deviation.

    Statistics s;
    s.addSequence(h.vdbegin(),h.vdend());
    cout << "Historical mean: " << s.mean() << endl;
    cout << "Std. deviation:  " << s.standardDeviation() << endl;

    // Another possibility: print out the maximum value.

    History::const_valid_iterator max = h.vbegin(), i=max, end = h.vend();
    for (i++; i!=end; i++)
        if (i->value() > max->value())
            max = i;
    cout << "Maximum value: " << max->value()
         << " assumed " << DateFormatter::toString(max->date()) << endl;

    // or the minimum, this time the STL way:

    bool lessthan(const History::Entry& i, const History::Entry& j) {
        return i.value() < j.value();
    }

    History::const_valid_iterator min =
        std::min_element(h.vbegin(),h.vend(),lessthan);
    cout << "Minimum value: " << min->value()
         << " assumed " << DateFormatter::toString(min->date()) << endl;

}
    