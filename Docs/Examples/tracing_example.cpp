
#include <ql/quantlib.hpp>

using namespace QuantLib;

namespace Foo {

    int bar(int i) {
        QL_TRACE_ENTER_FUNCTION;
        QL_TRACE_VARIABLE(i);

        if (i == 42) {
            QL_TRACE_LOCATION;
            QL_TRACE("Right answer, but no question");
        } else {
            QL_TRACE_LOCATION;
            QL_TRACE("Wrong answer");
            i *= 2;
        }

        QL_TRACE_VARIABLE(i);
        QL_TRACE_EXIT_FUNCTION;
        return i;
    }

}

int foo(int i) {
    using namespace Foo;
    QL_TRACE_ENTER_FUNCTION;

    int j = bar(i);
    int k = bar(j);

    QL_TRACE_EXIT_FUNCTION;
    return k;
}

int main() {

    QL_TRACE_ENABLE;

    QL_TRACE_ENTER_FUNCTION;

    int i = foo(21);

    QL_TRACE_EXIT_FUNCTION;
    return 0;
}

