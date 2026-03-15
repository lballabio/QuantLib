import sys

QL_EPSILON = sys.float_info.epsilon


def close(x: float, y: float, n: int = 42) -> bool:
    """Knuth-style floating-point closeness (strict).

    Returns True if |x-y| <= n*eps*|x| AND |x-y| <= n*eps*|y|.
    """
    if x == y:
        return True

    diff = abs(x - y)
    tolerance = n * QL_EPSILON

    if x == 0.0 or y == 0.0:
        return diff < tolerance * tolerance

    return diff <= tolerance * abs(x) and diff <= tolerance * abs(y)


def close_enough(x: float, y: float, n: int = 42) -> bool:
    """Knuth-style floating-point closeness (lenient).

    Returns True if |x-y| <= n*eps*|x| OR |x-y| <= n*eps*|y|.
    """
    if x == y:
        return True

    diff = abs(x - y)
    tolerance = n * QL_EPSILON

    if x == 0.0 or y == 0.0:
        return diff < tolerance * tolerance

    return diff <= tolerance * abs(x) or diff <= tolerance * abs(y)
