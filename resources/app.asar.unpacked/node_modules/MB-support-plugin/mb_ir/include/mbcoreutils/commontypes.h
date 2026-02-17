/*
 * printerpaneltypes.hh
 *
 *  Created on: Apr 9, 2013
 *      Author: paul
 *
 *  Description:  Contains header definition for cross-component, common application-level typedefs.
 *                NO COMPONENT-SPECIFIC ITEMS ARE TO BE PLACED IN THIS FILE!!  Instead, please place
 *                types of this nature in the types header for each respective component.
 */

#ifndef PRINTERPANEL_TYPES_HH
#define PRINTERPANEL_TYPES_HH

//#include <boost/shared_ptr.hpp>
#include <memory>

namespace MakerBot {

	// Make it easy to transition to C++ 11 shared_ptr
	template<class T>
	class shared_ptr {
	public:
		typedef std::shared_ptr<T> type;
	};
}

#endif // PRINTERPANEL_TYPES_HH
