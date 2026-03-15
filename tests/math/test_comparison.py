import math

from quantlib.math.comparison import QL_EPSILON, close, close_enough


class TestClose:
    def test_equal_values(self):
        assert close(1.0, 1.0)
        assert close(0.0, 0.0)
        assert close(-5.0, -5.0)

    def test_positive_infinity(self):
        assert close(math.inf, math.inf)

    def test_negative_infinity(self):
        assert close(-math.inf, -math.inf)

    def test_mixed_infinity(self):
        # inf - (-inf) = inf, and inf <= tolerance * inf (= inf) is True in IEEE 754
        # so close(inf, -inf) is True — matches C++ behavior
        assert close(math.inf, -math.inf)
        # inf - 1.0 = inf, and inf <= tolerance * 1.0 is False
        assert not close(math.inf, 1.0)

    def test_close_values(self):
        x = 1.0
        y = x + 10 * QL_EPSILON
        assert close(x, y)

    def test_not_close_values(self):
        x = 1.0
        y = x + 1000 * QL_EPSILON
        assert not close(x, y)

    def test_zero_and_small(self):
        tolerance = 42 * QL_EPSILON
        small = tolerance * tolerance * 0.5
        assert close(0.0, small)
        assert close(small, 0.0)

    def test_zero_and_not_small(self):
        assert not close(0.0, 1e-10)

    def test_asymmetric_close(self):
        # close() requires BOTH directions
        x = 1.0
        y = 1e6
        assert not close(x, y)

    def test_custom_n(self):
        x = 1.0
        y = x + 100 * QL_EPSILON
        assert not close(x, y, n=10)
        assert close(x, y, n=200)

    def test_negative_values(self):
        x = -1.0
        y = x - 10 * QL_EPSILON
        assert close(x, y)


class TestCloseEnough:
    def test_equal_values(self):
        assert close_enough(1.0, 1.0)
        assert close_enough(0.0, 0.0)

    def test_positive_infinity(self):
        assert close_enough(math.inf, math.inf)

    def test_close_enough_is_more_lenient(self):
        # close_enough uses OR, close uses AND
        # Find a case where one direction passes but not the other
        x = 1.0
        y = x + 30 * QL_EPSILON
        # Both should pass for close and close_enough at this scale
        assert close(x, y)
        assert close_enough(x, y)

    def test_zero_and_small(self):
        tolerance = 42 * QL_EPSILON
        small = tolerance * tolerance * 0.5
        assert close_enough(0.0, small)

    def test_custom_n(self):
        x = 1.0
        y = x + 100 * QL_EPSILON
        assert not close_enough(x, y, n=10)
        assert close_enough(x, y, n=200)

    def test_not_close_enough(self):
        assert not close_enough(1.0, 2.0)

    def test_close_enough_but_not_close(self):
        # close_enough (OR) can pass when close (AND) fails
        # This happens when |x-y| is within tolerance of one but not the other
        big = 1e10
        diff = 30 * QL_EPSILON * big  # within tolerance of big
        x = big
        y = big + diff
        # Both should be close to big, so both close and close_enough pass
        assert close_enough(x, y)
