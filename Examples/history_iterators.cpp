
#include "history.h"
#include "statistics.h"
#include "qlerrors.h"
#include <iostream>
#include <algorithm>

using QuantLib::History;
using QuantLib::Math::Statistics;
using QuantLib::DateFormatter;
using namespace std;

void printHistoricalAnalysis(const History& h) {

    // first of all, we check that the history makes some sense:
    
    QL_REQUIRE(h.size() >= 2, "History must contain at least two values");

    /* now, the history could contain null data: we will use 
       const_valid_iterator and const_valid_data_iterator to skip them. */
    
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
    
    double lessthan(const History::Entry& i, const History::Entry& j) {
        return i.value() < j.value();
    }
    
    History::const_valid_iterator min = 
        std::min_element(h.vbegin(),h.vend(),lessthan);
    cout << "Minimum value: " << min->value() 
         << " assumed " << DateFormatter::toString(min->date()) << endl;
        
}
    