#include <iostream>
#include "boost/smart_ptr.hpp"
#include "Core/TAPNParser/TAPNXmlParser.hpp"
#include "Core/VerificationOptions.hpp"
#include "Core/ArgsParser.hpp"
#include "Core/QueryParser/TAPNQueryParser.hpp"
#include "Core/TAPN/TimedPlace.hpp"
#include "DiscreteVerification/DiscreteVerification.hpp"
#include "DiscreteVerification/DeadlockVisitor.hpp"

using namespace std;
using namespace VerifyTAPN;
using namespace VerifyTAPN::TAPN;
using namespace boost;

int main(int argc, char* argv[])
{
	srand ( time(NULL) );

	ArgsParser parser;
	VerificationOptions options = parser.parse(argc, argv);

	TAPNXmlParser modelParser;
	boost::shared_ptr<TAPN::TimedArcPetriNet> tapn;

	try{
		tapn = modelParser.parse(options.getInputFile());
	}catch(const std::string& e){
		std::cout << "There was an error parsing the model file: " << e << std::endl;
		return 1;
	}

	tapn->initialize(options.getGlobalMaxConstantsEnabled());

	std::vector<int> initialPlacement(modelParser.parseMarking(options.getInputFile(), *tapn));

	AST::Query* query;
	try{
		TAPNQueryParser queryParser(*tapn);
		queryParser.parse(options.getQueryFile());
		query = queryParser.getAST();
	}catch(...){
		std::cout << "There was an error parsing the query file." << std::endl;
		return 1;
	}

	if(query->getQuantifier() == AST::EF || query->getQuantifier() == AST::AG){
		bool anyRemoved = tapn->removeOrphantedTransitions();
                DiscreteVerification::DeadlockVisitor deadlockVisitor = DiscreteVerification::DeadlockVisitor();
                boost::any c;
                deadlockVisitor.visit(*query, c);
                bool queryContainsDeadlock = boost::any_cast<bool>(c);
                if(queryContainsDeadlock && anyRemoved){
                        std::cout << "The model contains orphan transitions and the query a deadlock proposition. This combination is not supported." << std::endl; 
                        return 1;
                }
	}

	tapn->updatePlaceTypes(query, options);

	return DiscreteVerification::DiscreteVerification::run(tapn, initialPlacement, query, options);
}


