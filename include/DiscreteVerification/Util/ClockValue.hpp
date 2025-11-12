#include <cstdint>

namespace VerifyTAPN::DiscreteVerification::Util {

    typedef uint64_t clockValue;

    double clockToDouble(clockValue value, uint32_t precision = 0);

    clockValue doubleToClock(double value, uint32_t precision = 0);

}
