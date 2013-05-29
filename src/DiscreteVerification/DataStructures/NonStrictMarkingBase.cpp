/*
 * NonStrictMarkingBase.cpp
 *
 *  Created on: 29/02/2012
 *      Author: MathiasGS
 */

#include "NonStrictMarkingBase.hpp"

using namespace std;

namespace VerifyTAPN {
namespace DiscreteVerification {

TokenList NonStrictMarkingBase::emptyTokenList = TokenList();
    
NonStrictMarkingBase::NonStrictMarkingBase() : children(0), generatedBy(NULL){
    // empty constructor
}

NonStrictMarkingBase::NonStrictMarkingBase(const TAPN::TimedArcPetriNet& tapn, const std::vector<int>& v) :children(0), generatedBy(NULL){
	int prevPlaceId = -1;
	for(std::vector<int>::const_iterator iter = v.begin(); iter != v.end(); iter++){
		if(*iter == prevPlaceId){
			Place& p = places.back();
			if(p.tokens.size() == 0){
				Token t(0,1);
				p.tokens.push_back(t);
			}else{
				p.tokens.begin()->add(1);
			}
		}else{

			Place p(&tapn.getPlace(*iter));
			Token t(0,1);
			p.tokens.push_back(t);
			places.push_back(p);
		}
		prevPlaceId = *iter;
	}
}

NonStrictMarkingBase::NonStrictMarkingBase(const NonStrictMarkingBase& nsm) : children(0), generatedBy(NULL){

	places = nsm.places;
	parent = nsm.parent;
	generatedBy = nsm.generatedBy;
}

unsigned int NonStrictMarkingBase::size(){
	int count = 0;
	for(PlaceList::const_iterator iter = places.begin(); iter != places.end(); iter++){
		for(TokenList::const_iterator it = iter->tokens.begin(); it != iter->tokens.end(); it++){
			count += it->getCount();
		}
	}
	return count;
}

int NonStrictMarkingBase::numberOfTokensInPlace(int placeId) const{
	int count = 0;
	for(PlaceList::const_iterator iter = places.begin(); iter != places.end(); iter++){
		if(iter->place->getIndex() == placeId){
			for(TokenList::const_iterator it = iter->tokens.begin(); it != iter->tokens.end(); it++){
				count = count + it->getCount();
			}
		}
	}
	return count;
}

const TokenList& NonStrictMarkingBase::getTokenList(int placeId) const{
	for(PlaceList::const_iterator iter = places.begin(); iter != places.end(); iter++){
		if(iter->place->getIndex() == placeId) return iter->tokens;
	}
	return emptyTokenList;
}

bool NonStrictMarkingBase::removeToken(int placeId, int age){
	for(PlaceList::iterator pit = places.begin(); pit != places.end(); pit++){
		if(pit->place->getIndex() == placeId){
			for(TokenList::iterator tit = pit->tokens.begin(); tit != pit->tokens.end(); tit++){
				if(tit->getAge() == age){
					return removeToken(*pit, *tit);
				}
			}
		}
	}
	return false;
}

void NonStrictMarkingBase::removeRangeOfTokens(Place& place, TokenList::iterator begin, TokenList::iterator end){
	place.tokens.erase(begin, end);
}

bool NonStrictMarkingBase::removeToken(Place& place, Token& token){
	if(token.getCount() > 1){
		token.remove(1);
		return true;
	}else{
		for(TokenList::iterator iter = place.tokens.begin(); iter != place.tokens.end(); iter++){
			if(iter->getAge() == token.getAge()){
				place.tokens.erase(iter);
				if(place.tokens.empty()){
					for(PlaceList::iterator it = places.begin(); it != places.end(); it++){
						if(it->place->getIndex() == place.place->getIndex()){
							places.erase(it);
							return true;
						}
					}
				}
				return true;
			}
		}
	}
	return false;
}

void NonStrictMarkingBase::addTokenInPlace(TAPN::TimedPlace& place, int age){
	for(PlaceList::iterator pit = places.begin(); pit != places.end(); pit++){
		if(pit->place->getIndex() == place.getIndex()){
			for(TokenList::iterator tit = pit->tokens.begin(); tit != pit->tokens.end(); tit++){
				if(tit->getAge() == age){
					Token t(age, 1);
					addTokenInPlace(*pit, t);
					return;
				}
			}
			Token t(age,1);
			addTokenInPlace(*pit, t);
			return;
		}
	}
	Token t(age,1);
	Place p(&place);
	addTokenInPlace(p,t);

	// Insert place
	bool inserted = false;
	for(PlaceList::iterator it = places.begin(); it != places.end(); it++){
		if(it->place->getIndex() > place.getIndex()){
			places.insert(it, p);
			inserted = true;
			break;
		}
	}
	if(!inserted){
		places.push_back(p);
	}
}

void NonStrictMarkingBase::addTokenInPlace(const TAPN::TimedPlace& place, Token& token){
	for(PlaceList::iterator pit = places.begin(); pit != places.end(); pit++){
		if(pit->place->getIndex() == place.getIndex()){
			addTokenInPlace(*pit, token);
			return;
		}
	}

	Place p(&place);
	addTokenInPlace(p,token);

	// Insert place
	bool inserted = false;
	for(PlaceList::iterator it = places.begin(); it != places.end(); it++){
		if(it->place->getIndex() > place.getIndex()){
			places.insert(it, p);
			inserted = true;
			break;
		}
	}
	if(!inserted){
		places.push_back(p);
	}
}

void NonStrictMarkingBase::addTokenInPlace(Place& place, Token& token){
	if(token.getCount() == 0) return;
	for(TokenList::iterator iter = place.tokens.begin(); iter != place.tokens.end(); iter++){
		if(iter->getAge() == token.getAge()){
			iter->add(token.getCount());
			return;
		}
	}
	// Insert token
	bool inserted = false;
	for(TokenList::iterator it = place.tokens.begin(); it != place.tokens.end(); it++){
		if(it->getAge() > token.getAge()){
			place.tokens.insert(it, token);
			inserted = true;
			break;
		}
	}
	if(!inserted){
		place.tokens.push_back(token);
	}
}

NonStrictMarkingBase::~NonStrictMarkingBase() { }

bool NonStrictMarkingBase::equals(const NonStrictMarkingBase &m1) const{
	if(m1.places.size() != places.size())	return false;

	PlaceList::const_iterator p_iter = m1.places.begin();
	for(PlaceList::const_iterator iter = places.begin(); iter != places.end(); iter++, p_iter++){
		if(iter->place->getIndex() != p_iter->place->getIndex())	return false;
		if(iter->tokens.size() != p_iter->tokens.size())	return false;
		TokenList::const_iterator pt_iter = p_iter->tokens.begin();
		for(TokenList::const_iterator t_iter = iter->tokens.begin(); t_iter != iter->tokens.end(); t_iter++, pt_iter++){
			if(!t_iter->equals(*pt_iter))	return false;
		}
	}

	return true;
}

std::ostream& operator<<(std::ostream& out, NonStrictMarkingBase& x ) {
	out << "-";
	for(PlaceList::iterator iter = x.places.begin(); iter != x.places.end(); iter++){
		out << "place " << iter->place->getId() << " has tokens (age, count): ";
		for(TokenList::iterator it = iter->tokens.begin(); it != iter->tokens.end(); it++){
			out << "(" << it->getAge() << ", " << it->getCount() << ") ";
		}
		if(iter != x.places.end()-1){
			out << endl;
		}
	}

	return out;
}

void NonStrictMarkingBase::cut(){
#ifdef DEBUG
	std::cout << "Before cut: " << *this << std::endl;
#endif
	for(PlaceList::iterator place_iter = this->places.begin(); place_iter != this->places.end(); place_iter++){
		//set age of too old tokens to max age
		int count = 0;
		for(TokenList::iterator token_iter = place_iter->tokens.begin(); token_iter != place_iter->tokens.end(); token_iter++) {
                        if(token_iter->getAge() > place_iter->place->getMaxConstant()){ // this will also removed dead tokens
				TokenList::iterator beginDelete = token_iter;
				if(place_iter->place->getType() == TAPN::Std){
					for(; token_iter != place_iter->tokens.end(); token_iter++){
						count += token_iter->getCount();
					}
				}
				this->removeRangeOfTokens(*place_iter, beginDelete, place_iter->tokens.end());
				break;
			}
		}
                if(count){
                        Token t(place_iter->place->getMaxConstant()+1,count);
                        this->addTokenInPlace(*place_iter, t);
                }
	}
	this->cleanUp();
	#ifdef DEBUG
		std::cout << "After cut: " << *this << std::endl;
	#endif
}

int NonStrictMarkingBase::getYoungest(){
    	int youngest = INT_MAX;
	for(PlaceList::const_iterator place_iter = getPlaceList().begin(); place_iter != getPlaceList().end(); place_iter++){
		if(youngest > place_iter->tokens.front().getAge() && place_iter->tokens.front().getAge() <= place_iter->place->getMaxConstant()){
			youngest = place_iter->tokens.front().getAge();
		}
	}

	if(youngest == INT_MAX){
		youngest = 0;
	}
        return youngest;
}

int NonStrictMarkingBase::makeBase(TAPN::TimedArcPetriNet* tapn){
	#ifdef DEBUG
		std::cout << "Before makeBase: " << *this << std::endl;
	#endif
	int youngest = getYoungest();

	for(PlaceList::iterator place_iter = places.begin(); place_iter != places.end(); place_iter++){
		for(TokenList::iterator token_iter = place_iter->tokens.begin(); token_iter != place_iter->tokens.end(); token_iter++){
			if(token_iter->getAge() != place_iter->place->getMaxConstant() + 1){
				token_iter->setAge(token_iter->getAge()-youngest);
			}
#ifdef DEBUG
			else if(token_iter->getAge() > place_iter->place->getMaxConstant() + 1){
				assert(false);
			}
#endif
		}
	}

	#ifdef DEBUG
		std::cout << "After makeBase: " << *this << std::endl;
		std::cout << "Youngest: " << youngest << std::endl;
	#endif

	return youngest;
}

} /* namespace DiscreteVerification */
} /* namespace VerifyTAPN */