/*
 * PlaceVisitor.hpp
 *
 *  Created on: 21/03/2012
 *      Author: MathiasGS
 */

#ifndef PLACEVISITOR_HPP_
#define PLACEVISITOR_HPP_

#include "../Core/QueryParser/Visitor.hpp"
#include "../Core/QueryParser/AST.hpp"
#include <exception>
#include <vector>

namespace VerifyTAPN {
namespace DiscreteVerification {

using namespace AST;

class PlaceVisitor : public Visitor{
	public:
		PlaceVisitor(){};
		virtual ~PlaceVisitor(){};

	public: // visitor methods
		virtual void visit(const NotExpression& expr, Result& context);
		virtual void visit(const OrExpression& expr, Result& context);
		virtual void visit(const AndExpression& expr, Result& context);
		virtual void visit(const AtomicProposition& expr, Result& context);
		virtual void visit(const BoolExpression& expr, Result& context);
		virtual void visit(const Query& query, Result& context);
		virtual void visit(const DeadlockExpression& expr, Result& context);
                virtual void visit(const NumberExpression& expr, Result& context);
                virtual void visit(const IdentifierExpression& expr, Result& context);
                virtual void visit(const MinusExpression& expr, Result& context);
                virtual void visit(const OperationExpression& expr, Result& context);
                virtual void visit(const MultiplyExpression& expr, Result& context){visit((OperationExpression&)expr, context);};
                virtual void visit(const SubtractExpression& expr, Result& context){visit((OperationExpression&)expr, context);};;
                virtual void visit(const PlusExpression& expr, Result& context){visit((OperationExpression&)expr, context);};;
};

} /* namespace DiscreteVerification */
} /* namespace VerifyTAPN */
#endif /* PLACEVISITOR_HPP_ */
