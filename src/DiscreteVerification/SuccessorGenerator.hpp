/*
 * SuccessorGenerator.hpp
 *
 *  Created on: 22/03/2012
 *      Author: MathiasGS
 */

#ifndef SUCCESSORGENERATOR_HPP_
#define SUCCESSORGENERATOR_HPP_

#include "../Core/TAPN/TAPN.hpp"
#include "NonStrictMarking.hpp"

namespace VerifyTAPN {
namespace DiscreteVerification {

using namespace std;
using namespace TAPN;

class SuccessorGenerator {
public:
	SuccessorGenerator(TAPN::TimedArcPetriNet& tapn) : tapn(tapn) {};
	~SuccessorGenerator();
	vector< NonStrictMarking > generateSuccessors(const NonStrictMarking& marking) const;
private:
	const TAPN::TimedArcPetriNet& tapn;
};

} /* namespace DiscreteVerification */
} /* namespace VerifyTAPN */
#endif /* SUCCESSORGENERATOR_HPP_ */
