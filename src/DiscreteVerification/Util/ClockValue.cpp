#include "DiscreteVerification/Util/ClockValue.hpp"

#include <math.h>

#define MAX_PRECISION 10

namespace VerifyTAPN::DiscreteVerification::Util {

using VerifyTAPN::TAPN::TimedArcPetriNet;

double clockToDouble(clockValue value, const uint32_t precision)
{
    double time = static_cast<double>(value);
    if(precision > 0) {
        time /= pow(10.0, precision);
    } else {
        time /= pow(10.0, MAX_PRECISION);
    }
    return 0.0;
}

clockValue toClock(double value, const uint32_t precision)
{
    if(value == std::numeric_limits<double>::infinity()) {
        return std::numeric_limits<clockValue>::max();
    }
    if(precision > 0) {
        value *= pow(10.0, precision);
    } else {
        value *= pow(10.0, MAX_PRECISION);
    }
    return static_cast<clockValue>(value);
}

}