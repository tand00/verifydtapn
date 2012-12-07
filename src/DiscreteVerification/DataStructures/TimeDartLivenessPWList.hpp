/*
 * PWList.hpp
 *
 *  Created on: 01/03/2012
 *      Author: MathiasGS
 */

#ifndef TimeDartLivenessPWList_HPP_
#define TimeDartLivenessPWList_HPP_

#include "WaitingList.hpp"
#include <iostream>
#include "google/sparse_hash_map"
#include "NonStrictMarkingBase.hpp"
#include "WaitingList.hpp"
#include "TimeDart.hpp"
#include "PData.h"

namespace VerifyTAPN {
    namespace DiscreteVerification {
        class TimeDartLivenessPWBase;
        class TimeDartLivenessPWHashMap;
        //       class TimeDartPWPData;

        class TimeDartLivenessPWBase {
        public:
            typedef std::vector<TimeDart*> TimeDartList;
            typedef google::sparse_hash_map<size_t, TimeDartList> HashMap;
        
        public:

            TimeDartLivenessPWBase() : discoveredMarkings(0), maxNumTokensInAnyMarking(-1), stored(0) {
            };

            virtual ~TimeDartLivenessPWBase() {
            };


        public: // inspectors
            virtual bool HasWaitingStates() = 0;

            virtual long long Size() const {
                return stored;
            };

        public: // modifiers
            virtual std::pair<TimeDart*, bool> Add(TAPN::TimedArcPetriNet* tapn, NonStrictMarkingBase* base, int youngest, TimeDart* parent, int upper) = 0;
            virtual WaitingDart* GetNextUnexplored() = 0;
            virtual WaitingDart* PopWaiting() = 0;
            virtual void flushBuffer() = 0;

            inline void SetMaxNumTokensIfGreater(int i) {
                if (i > maxNumTokensInAnyMarking) maxNumTokensInAnyMarking = i;
            };

        public:
            int discoveredMarkings;
            int maxNumTokensInAnyMarking;
            long long stored;
        };

        class TimeDartLivenessPWHashMap : public TimeDartLivenessPWBase {
        public:

            TimeDartLivenessPWHashMap(){};
            
            TimeDartLivenessPWHashMap(WaitingList<WaitingDart>* w_l) : TimeDartLivenessPWBase(), markings_storage(256000), waiting_list(w_l) {
            };

            ~TimeDartLivenessPWHashMap() {
            };
            friend std::ostream& operator<<(std::ostream& out, TimeDartLivenessPWHashMap& x);
            virtual std::pair<TimeDart*, bool> Add(TAPN::TimedArcPetriNet* tapn, NonStrictMarkingBase* base, int youngest, TimeDart* parent, int upper);
            virtual WaitingDart* GetNextUnexplored();
            virtual WaitingDart* PopWaiting();

            virtual bool HasWaitingStates() {
                return (waiting_list->Size() > 0);
            };
            virtual void flushBuffer();
        private:
            WaitingList<WaitingDart>* waiting_list;
            HashMap markings_storage;
        };

        class TimeDartLivenessPWPData : public TimeDartLivenessPWBase {
        public:


            
            TimeDartLivenessPWPData(WaitingList<WaitingDart >* w_l, boost::shared_ptr<TAPN::TimedArcPetriNet>& tapn, int knumber, int nplaces, int mage) : TimeDartLivenessPWBase(), waiting_list(w_l), passed(tapn, knumber, nplaces, mage) {
            };

            ~TimeDartLivenessPWPData() {
            };
            friend std::ostream& operator<<(std::ostream& out, TimeDartLivenessPWHashMap& x);
            virtual std::pair<TimeDart*, bool> Add(TAPN::TimedArcPetriNet* tapn, NonStrictMarkingBase* base, int youngest, TimeDart* parent, int upper);
            virtual WaitingDart* GetNextUnexplored();
            virtual WaitingDart* PopWaiting();

            virtual bool HasWaitingStates() {
                return (waiting_list->Size() > 0);
            };
            virtual void flushBuffer();
        private:
            WaitingList<WaitingDart >* waiting_list;
            PData<TimeDart> passed;
        };

    } /* namespace DiscreteVerification */
} /* namespace VerifyTAPN */
#endif /* PWLIST_HPP_ */
