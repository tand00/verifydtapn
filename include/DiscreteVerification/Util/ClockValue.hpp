#include <cstdint>

#include "Core/TAPN/TimedArcPetriNet.hpp"

namespace VerifyTAPN::DiscreteVerification::Util {

    typedef uint64_t clockValue;

    double clockToDouble(clockValue value, const uint32_t precision = 0);

    clockValue toClock(double value, const uint32_t precision = 0);

    clockValue toClock(int value, const uint32_t precision = 0);

    clockValue toClock(uint64_t value, const uint32_t precision = 0);

    TAPN::TimedArcPetriNet adaptNetToPrecision(
        const TAPN::TimedArcPetriNet& net, 
        const uint32_t precision = 0
    );

}
