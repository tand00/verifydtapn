/* 
 * File:   SMCRunGenerator.cpp
 * Author: Tanguy Dubois
 * 
 * Created on 11 April 2024, 10.13
 */

#include "DiscreteVerification/Generators/SMCRunGenerator.h"

#include <numeric>
#include <random>
#include <algorithm>

namespace VerifyTAPN {
    namespace DiscreteVerification {

        using Util::interval;

        void SMCRunGenerator::prepare(RealMarking *parent) {
            _origin = new RealMarking(*parent);
            _parent = new RealMarking(*_origin);
            RealPlaceList& places = _origin->getPlaceList();
            std::vector<bool> transitionSeen(_defaultTransitionIntervals.size(), false);
            _originMaxDelay = std::numeric_limits<double>::infinity();
            for(auto &pit : places) {
                if(pit.place->getInvariant().getBound() != std::numeric_limits<int>::max()) {
                    double place_max_delay = pit.availableDelay();
                    if(place_max_delay < _originMaxDelay) {
                        _originMaxDelay = place_max_delay;
                    }
                }
                for(auto arc : pit.place->getInputArcs()) {
                    TimedTransition &transi = arc->getOutputTransition();
                    if(transitionSeen[transi.getIndex()]) continue;
                    std::vector<interval<double>> firingDates = transitionFiringDates(transi);
                    _defaultTransitionIntervals[transi.getIndex()] = firingDates;
                    transitionSeen[transi.getIndex()] = true;
                }
                for(auto arc : pit.place->getTransportArcs()) {
                    TimedTransition &transi = arc->getTransition();
                    if(transitionSeen[transi.getIndex()]) continue;
                    std::vector<interval<double>> firingDates = transitionFiringDates(transi);
                    _defaultTransitionIntervals[transi.getIndex()] = firingDates;
                    transitionSeen[transi.getIndex()] = true;
                }
            }
            for(auto &transi : _tapn.getTransitions()) {
                if(transi->getPresetSize() == 0) {
                    _defaultTransitionIntervals[transi->getIndex()] = { interval<double>(0, std::numeric_limits<double>::infinity()) };
                    transitionSeen[transi->getIndex()] = true;
                }
            }
            bool deadlocked = true;
            std::vector<interval<double>> invInterval = { interval<double>(0, _originMaxDelay) };
            for(auto iter = _defaultTransitionIntervals.begin() ; iter != _defaultTransitionIntervals.end() ; iter++) {
                if(iter->empty()) continue;
                deadlocked = false;
                *iter = Util::setIntersection(*iter, invInterval);
            }
            _origin->setDeadlocked(deadlocked);
            reset();
        }

        void SMCRunGenerator::reset() {
            if(_trace.size() > 0) {
                 for(RealMarking* marking : _trace) {
                    if(marking != nullptr) delete marking;
                }
            }
            if(!recordTrace && _parent != nullptr) delete _parent;
            _parent = new RealMarking(*_origin);
            if(recordTrace) {
                _trace = { new RealMarking(*_origin), _parent };
            }
            _transitionIntervals = _defaultTransitionIntervals;
            _maximal = false;
            _max_delay = _originMaxDelay;
            _totalTime = 0;
            _totalSteps = 0;
            _modifiedPlaces.clear();
            _dates_sampled = std::vector<double>(_transitionIntervals.size(), std::numeric_limits<double>::infinity());
            for(int i = 0 ; i < _dates_sampled.size() ; i++) {
                auto* intervals = &_transitionIntervals[i];
                if(!intervals->empty() && intervals->front().lower() == 0) {
                    const SMCDistribution& distrib = _tapn.getTransitions()[i]->getDistribution();
                    _dates_sampled[i] = distrib.sample(_rng);
                }
            }
        }

        SMCRunGenerator SMCRunGenerator::copy() const
        {
            SMCRunGenerator clone(_tapn);
            clone._origin = new RealMarking(*_origin);
            clone._defaultTransitionIntervals = _defaultTransitionIntervals;
            clone._originMaxDelay = _originMaxDelay;
            clone.reset();
            return clone;
        }

        void SMCRunGenerator::refreshTransitionsIntervals()
        {
            RealPlaceList& places = _parent->getPlaceList();
            std::vector<bool> transitionSeen(_transitionIntervals.size(), false);
            _max_delay = _parent->availableDelay();
            for(auto &modified : _modifiedPlaces) {
                const TimedPlace& place = _tapn.getPlace(modified);
                for(auto arc : place.getInputArcs()) {
                    TimedTransition &transi = arc->getOutputTransition();
                    if(transitionSeen[transi.getIndex()]) continue;
                    std::vector<interval<double>> firingDates = transitionFiringDates(transi);
                    _transitionIntervals[transi.getIndex()] = firingDates;
                    transitionSeen[transi.getIndex()] = true;
                }
                for(auto arc : place.getInhibitorArcs()) {
                    TimedTransition &transi = arc->getOutputTransition();
                    if(transitionSeen[transi.getIndex()]) continue;
                    std::vector<interval<double>> firingDates = transitionFiringDates(transi);
                    _transitionIntervals[transi.getIndex()] = firingDates;
                    transitionSeen[transi.getIndex()] = true;
                }
                for(auto arc : place.getTransportArcs()) {
                    TimedTransition &transi = arc->getTransition();
                    if(transitionSeen[transi.getIndex()]) continue;
                    std::vector<interval<double>> firingDates = transitionFiringDates(transi);
                    _transitionIntervals[transi.getIndex()] = firingDates;
                    transitionSeen[transi.getIndex()] = true;
                }
            }
            std::vector<interval<double>> invInterval = { interval<double>(0, _max_delay) };
            bool deadlocked = true;
            for(int i = 0 ; i < _transitionIntervals.size() ; i++) {
                _transitionIntervals[i] = Util::setIntersection<double>(_transitionIntervals[i], invInterval);
                deadlocked &= _transitionIntervals[i].empty();
                bool enabled = (!_transitionIntervals[i].empty()) && (_transitionIntervals[i].front().lower() == 0);
                bool newlyEnabled = enabled && (_dates_sampled[i] == std::numeric_limits<double>::infinity());
                if(!enabled) {
                    _dates_sampled[i] = std::numeric_limits<double>::infinity();
                /*
                } else if(_transitionIntervals[i].front().upper() == 0 && !newlyEnabled) { // Happens if not fired
                    _transitionIntervals[i].clear();
                    _dates_sampled[i] = std::numeric_limits<double>::infinity();
                */
                } else if(newlyEnabled) {
                    const SMCDistribution& distrib = _tapn.getTransitions()[i]->getDistribution();
                    _dates_sampled[i] = distrib.sample(_rng);
                }
            }
            _parent->setDeadlocked(deadlocked);
        }

        RealMarking* SMCRunGenerator::next() {
            auto [transi, delay] = getWinnerTransitionAndDelay();
            
            if(delay == std::numeric_limits<double>::infinity()) {
                _maximal = true;
                return nullptr;
            }

            _parent->deltaAge(delay);
            _totalTime += delay;
            _modifiedPlaces.clear();

            _parent->setPreviousDelay(delay + _parent->getPreviousDelay());
        
            if(transi != nullptr) {
                _totalSteps++;
                _transitionsStatistics[transi->getIndex()]++;
                _dates_sampled[transi->getIndex()] = std::numeric_limits<double>::infinity();
                RealMarking* child = fire(transi);
                child->setGeneratedBy(transi);
                if(recordTrace) {
                    _trace.push_back(child);
                    _parent = new RealMarking(*child);
                    _trace.push_back(_parent);
                } else {
                    delete _parent;
                    _parent = child;
                }
            }

            // Translate intervals, so we don't have to compute some of them next
            for(int i = 0 ; i < _transitionIntervals.size() ; i++) {
                auto* intervals = &_transitionIntervals[i];
                Util::setDeltaIntoPositive(*intervals, -delay);
                double date = _dates_sampled[i];
                _dates_sampled[i] = (date == std::numeric_limits<double>::infinity()) ?
                    std::numeric_limits<double>::infinity() : date - delay;
            }

            refreshTransitionsIntervals();

            return _parent;
        }

        std::pair<TimedTransition*, double> SMCRunGenerator::getWinnerTransitionAndDelay() {
            std::vector<size_t> winner_indexs;
            double date_min = std::numeric_limits<double>::infinity();
            for(int i = 0 ; i < _transitionIntervals.size() ; i++) {
                auto* intervals = &_transitionIntervals[i];
                if(intervals->empty()) continue;
                interval<double>& first = intervals->front();
                double date = first.lower() > 0 ? first.lower() :
                                first.upper() > 0 ? first.upper() : 
                                std::numeric_limits<double>::infinity();
                if(date < date_min) {
                    date_min = date;
                    winner_indexs.clear();
                }
                date = _dates_sampled[i];
                if(date != std::numeric_limits<double>::infinity() && date <= first.upper()) {
                    if(date < date_min) {
                        date_min = date;
                        winner_indexs.clear();
                    }
                    if(date == date_min) {
                        winner_indexs.push_back(i);
                    }
                }
            }
            TimedTransition *winner;
            if(winner_indexs.empty()) { 
                winner = nullptr;
            } else if(winner_indexs.size() == 1) {
                winner = _tapn.getTransitions()[winner_indexs[0]];
            } else {
                winner = chooseWeightedWinner(winner_indexs);
            }
            return std::make_pair(winner, date_min);
        }

        TimedTransition* SMCRunGenerator::chooseWeightedWinner(const std::vector<size_t>& winner_indexs) {
            double total_weight = 0.0f;
            std::vector<size_t> infty_weights;
            for(auto& candidate : winner_indexs) {
                double priority = _tapn.getTransitions()[candidate]->getWeight();
                if(priority == std::numeric_limits<double>::infinity()) {
                    infty_weights.push_back(candidate);
                } else {
                    total_weight += priority;
                }
            }
            if(!infty_weights.empty()) {
                int winner_index = std::uniform_int_distribution<>(0, infty_weights.size() - 1)(_rng);
                return _tapn.getTransitions()[infty_weights[winner_index]];
            }
            if(total_weight == 0) {
                int winner_index = std::uniform_int_distribution<>(0, winner_indexs.size() - 1)(_rng);
                return _tapn.getTransitions()[winner_indexs[winner_index]];
            }
            double winning_weight = std::uniform_real_distribution<>(0.0, total_weight)(_rng);
            for(auto& candidate : winner_indexs) {
                TimedTransition* transi = _tapn.getTransitions()[candidate];
                winning_weight -= transi->getWeight();
                if(winning_weight <= 0) {
                    return transi;
                }
            }
            return _tapn.getTransitions()[winner_indexs[0]];
        }

        std::vector<interval<double>> SMCRunGenerator::transitionFiringDates(const TimedTransition& transi) {
            RealPlaceList &places = _parent->getPlaceList();
            std::vector<interval<double>> firingInterval = { interval<double>(0, std::numeric_limits<double>::infinity()) };
            std::vector<interval<double>> disabled;
            for(InhibitorArc* inhib : transi.getInhibitorArcs()) {
                if(_parent->numberOfTokensInPlace(inhib->getInputPlace().getIndex()) >= inhib->getWeight()) {
                    return disabled;
                } 
            }
            for(TimedInputArc* arc : transi.getPreset()) {
                auto &place = _parent->getPlaceList()[arc->getInputPlace().getIndex()];
                if(place.isEmpty()) return disabled;
                firingInterval = Util::setIntersection<double>(firingInterval, arcFiringDates(arc->getInterval(), arc->getWeight(), place.tokens));
                if(firingInterval.empty()) return firingInterval;
            }
            for(TransportArc* arc : transi.getTransportArcs()) {
                auto &place = _parent->getPlaceList()[arc->getSource().getIndex()];
                if(place.isEmpty()) return disabled;
                TimeInvariant targetInvariant = arc->getDestination().getInvariant();
                TimeInterval arcInterval = arc->getInterval();
                if(targetInvariant.getBound() < arcInterval.getUpperBound()) {
                    arcInterval.setUpperBound(targetInvariant.getBound(), targetInvariant.isBoundStrict());
                } 
                firingInterval = Util::setIntersection<double>(firingInterval, arcFiringDates(arcInterval, arc->getWeight(), place.tokens));
                if(firingInterval.empty()) return firingInterval;
            }
            return firingInterval;
        }

        std::vector<interval<double>> SMCRunGenerator::arcFiringDates(TimeInterval time_interval, uint32_t weight, RealTokenList& tokens) {
            // We assume tokens is SORTED !
            Util::interval<double> arcInterval(time_interval.getLowerBound(), time_interval.getUpperBound());
            size_t total_tokens = 0;
            for(auto &t : tokens) {
                total_tokens += t.getCount();
            }
            if(total_tokens < weight) return std::vector<interval<double>>();
            std::vector<interval<double>> firingDates;
            size_t firstTokenIndex = 0;
            size_t consumedInFirst = 0;
            while(firstTokenIndex < tokens.size()) {
                RealToken t = tokens[firstTokenIndex];
                interval<double> tokensSetInterval = remainingForToken(arcInterval, t);
                size_t inThisSet = t.getCount() - consumedInFirst;
                consumedInFirst++;
                if(inThisSet <= weight) {
                    size_t remaining = weight - inThisSet;
                    size_t nextSet = firstTokenIndex + 1;
                    while(remaining >= 0 && nextSet < tokens.size()) {
                        RealToken nextToken = tokens[nextSet];
                        tokensSetInterval = Util::intersect<double>(tokensSetInterval, remainingForToken(arcInterval, nextToken));
                        if(nextToken.getCount() >= remaining) break;
                        remaining -= nextToken.getCount();
                        nextSet++;
                    }
                    if(remaining > 0) {
                        return firingDates; // Reached end of tokens...
                    }
                }
                Util::setAdd<double>(firingDates, tokensSetInterval);
                if(consumedInFirst >= t.getCount()) {
                    consumedInFirst = 0;
                    firstTokenIndex++;
                }
            }
            return firingDates;
        }

        Util::interval<double> SMCRunGenerator::remainingForToken(const interval<double>& arcInterval, const RealToken& token) {
            interval<double> tokenInterv = arcInterval;
            tokenInterv.delta(-token.getAge());
            return tokenInterv.positive();
        }

        RealMarking* SMCRunGenerator::fire(TimedTransition* transi) {
            if (transi == nullptr) {
                assert(false);
                return nullptr;
            }
            auto *child = new RealMarking(*_parent);
            RealPlaceList &placelist = child->getPlaceList();

            for (auto &input : transi->getPreset()) {
                int source = input->getInputPlace().getIndex();
                RealPlace& place = placelist[source];
                RealTokenList& tokenlist = place.tokens;
                int remaining = input->getWeight();
                std::uniform_int_distribution<> randomTokenIndex(0, tokenlist.size() - 1);
                size_t tok_index = randomTokenIndex(_rng);
                size_t tested = 0;
                while(remaining > 0 && tested < tokenlist.size()) {
                    if(input->getInterval().contains(tokenlist[tok_index].getAge())) {
                        remaining--;
                        tokenlist[tok_index].remove(1);
                        if(tokenlist[tok_index].getCount() == 0) {
                            tokenlist.erase(tokenlist.begin() + tok_index);
                            randomTokenIndex = std::uniform_int_distribution<>(0, tokenlist.size() - 1);
                        }
                        if(remaining > 0) {
                            tok_index = randomTokenIndex(_rng);
                            tested = 0;
                        }
                    } else {
                        tok_index = (tok_index + 1) % tokenlist.size();
                        tested++;
                    }
                }
                assert(remaining == 0);
                _modifiedPlaces.push_back(source);
            }

            for (auto &transport : transi->getTransportArcs()) {
                int source = transport->getSource().getIndex();
                int dest = transport->getDestination().getIndex();
                int destInv = transport->getDestination().getInvariant().getBound();
                RealPlace& place = placelist[source];
                RealTokenList& tokenlist = place.tokens;
                int remaining = transport->getWeight();
                std::uniform_int_distribution<> randomTokenIndex(0, tokenlist.size() - 1);
                size_t tok_index = randomTokenIndex(_rng);
                size_t tested = 0;
                while(remaining > 0 && tested < tokenlist.size()) {
                    double age = tokenlist[tok_index].getAge();
                    if(transport->getInterval().contains(age) && age <= destInv) {
                        remaining--;
                        tokenlist[tok_index].remove(1);
                        if(tokenlist[tok_index].getCount() == 0) {
                            tokenlist.erase(tokenlist.begin() + tok_index);
                            randomTokenIndex = std::uniform_int_distribution<>(0, tokenlist.size() - 1);
                        }
                        if(remaining > 0) {
                            tok_index = randomTokenIndex(_rng);
                            tested = 0;
                        }
                        child->addTokenInPlace(transport->getDestination(), age);
                    } else {
                        tok_index = (tok_index + 1) % tokenlist.size();
                        tested++;
                    }
                }
                assert(remaining == 0);
                _modifiedPlaces.push_back(source);
                _modifiedPlaces.push_back(dest);
            }
            for (auto* output : transi->getPostset()) {
                TimedPlace &place = output->getOutputPlace();
                RealToken token = RealToken(0.0, output->getWeight());
                child->addTokenInPlace(place, token);
                _modifiedPlaces.push_back(place.getIndex());
            }
            return child;
        }

        bool SMCRunGenerator::reachedEnd() const {
            return _maximal;
        }

        double SMCRunGenerator::getRunDelay() const {
            return _totalTime;
        }

        int SMCRunGenerator::getRunSteps() const {
            return _totalSteps;
        }

        void SMCRunGenerator::printTransitionStatistics(std::ostream &out) const {
            out << std::endl << "TRANSITION STATISTICS";
            for (unsigned int i = 0; i < _transitionsStatistics.size(); i++) {
                if ((i) % 6 == 0) {
                    out << std::endl;
                    out << "<" << _tapn.getTransitions()[i]->getName() << ":" << _transitionsStatistics[i] << ">";
                } else {
                    out << " <" << _tapn.getTransitions()[i]->getName() << ":" << _transitionsStatistics[i] << ">";
                }
            }
            out << std::endl;
            out << std::endl;
        }

        std::stack<RealMarking*> SMCRunGenerator::getTrace() const {
            std::stack<RealMarking*> trace;
            for(int i = 0 ; i < _trace.size() ; i++) {
                RealMarking* marking = _trace[_trace.size() - 1 - i];
                if(marking == nullptr) trace.push(nullptr);
                else trace.push(new RealMarking(*marking));
            }
            return trace;
        }

    }
}