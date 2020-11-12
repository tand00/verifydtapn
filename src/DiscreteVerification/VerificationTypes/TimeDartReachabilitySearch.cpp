/*
 * NonStrictSearch.cpp
 *
 *  Created on: 26/04/2012
 *      Author: MathiasGS
 */

#include "DiscreteVerification/VerificationTypes/TimeDartReachabilitySearch.hpp"

namespace VerifyTAPN { namespace DiscreteVerification {

    bool TimeDartReachabilitySearch::run() {
        // no need to check if trace is set, just as expensive as setting the generated by
        initialMarking.setGeneratedBy(nullptr);

        if (handleSuccessor(&initialMarking, nullptr, std::numeric_limits<int32_t>::max())) {
            return true;
        }

        //Main loop
        while (pwList->hasWaitingStates()) {
            TimeDartBase &dart = *pwList->getNextUnexplored();
            exploredMarkings++;

            int passed = dart.getPassed();
            dart.setPassed(dart.getWaiting());
            tapn.getTransitions();
            this->tmpdart = nullptr;
            if (options.getTrace() != VerificationOptions::NO_TRACE) {
                this->tmpdart = ((ReachabilityTraceableDart *) &dart)->trace;
            }
            for (auto t : tapn.getTransitions()) {
                auto &transition = *t;
                auto calculatedStart = calculateStart(transition, dart.getBase());
                if (calculatedStart.first == -1) {    // Transition cannot be enabled in marking
                    continue;
                }
                int start = std::max(dart.getWaiting(), calculatedStart.first);
                int end = std::min(passed - 1, calculatedStart.second);

                if (start <= end) {

                    if (transition.hasUntimedPostset()) {
                        NonStrictMarkingBase Mpp(*dart.getBase());
                        Mpp.incrementAge(start);

                        this->tmpupper = start;
                        if (generateAndInsertSuccessors(Mpp, transition)) {
                            return true;
                        }
                    } else {
                        int stop = std::min(std::max(start, calculateStop(transition, dart.getBase())), end);
                        for (int n = start; n <= stop; n++) {
                            NonStrictMarkingBase Mpp(*dart.getBase());
                            Mpp.incrementAge(n);
                            this->tmpupper = n;
                            if (generateAndInsertSuccessors(Mpp, transition)) {
                                return true;
                            }
                        }
                    }
                }
            }
            deleteBase(dart.getBase());
        }

        return false;
    }

    bool
    TimeDartReachabilitySearch::handleSuccessor(NonStrictMarkingBase *marking, WaitingDart *parent, int upper) {
        int start = 0;
        if (options.getTrace() != VerificationOptions::NO_TRACE) {
            start = marking->getYoungest();
        }
        int maxDelay = marking->cut(placeStats);

        unsigned int size = marking->size();

        pwList->setMaxNumTokensIfGreater(size);

        if (size > options.getKBound()) {
            delete marking;
            return false;
        }

        int youngest = marking->makeBase();

        if (pwList->add(marking, youngest, parent, upper, start)) {


            if (maxDelay != std::numeric_limits<int>::max())
                maxDelay += youngest;
            if (maxDelay > tapn.getMaxConstant()) {
                maxDelay = tapn.getMaxConstant() + 1;
            }

            QueryVisitor<NonStrictMarkingBase> checker(*marking, tapn, maxDelay);
            AST::BoolResult context;
            query->accept(checker, context);
            if (context.value) {
                if (options.getTrace()) {
                    lastMarking = pwList->getLast();
                }
                return true;
            } else {
                deleteBase(marking);
                return false;
            }
        }
        deleteBase(marking);
        return false;
    }

    void TimeDartReachabilitySearch::printStats() {
        std::cout << "  discovered markings:\t" << pwList->discoveredMarkings << std::endl;
        std::cout << "  explored markings:\t" << exploredMarkings << std::endl;
        std::cout << "  stored markings:\t" << pwList->size() << std::endl;
    }

    TimeDartReachabilitySearch::~TimeDartReachabilitySearch() = default;

} } /* namespace VerifyTAPN */
