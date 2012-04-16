#ifndef INHIBITORARC_HPP_
#define INHIBITORARC_HPP_

#include <vector>
#include "TimeInterval.hpp"
#include "boost/smart_ptr.hpp"

namespace VerifyTAPN {
	namespace TAPN {
		class TimedTransition;
		class TimedPlace;

		class InhibitorArc {
			public: // typedefs
				typedef std::vector< boost::shared_ptr<InhibitorArc> > Vector;
				typedef std::vector< boost::weak_ptr<InhibitorArc> > WeakPtrVector;
			public:
				InhibitorArc(const boost::shared_ptr<TimedPlace>& place, const boost::shared_ptr<TimedTransition>& transition, const int weight) : place(place), transition(transition), weight(weight) { };
				virtual ~InhibitorArc() { /* empty */ }

			public: // modifiers
				inline TimedPlace& InputPlace() { return *place; }
				inline TimedTransition& OutputTransition() { return *transition; }

			public: // Inspectors
				void Print(std::ostream& out) const;
				inline const int GetWeight() const { return weight; }
			private:
				const boost::shared_ptr<TimedPlace> place;
				const boost::shared_ptr<TimedTransition> transition;
				const int weight;
		};

		inline std::ostream& operator<<(std::ostream& out, const InhibitorArc& arc)
		{
			arc.Print(out);
			return out;
		}
	}

}

#endif /* INHIBITORARC_HPP_ */
