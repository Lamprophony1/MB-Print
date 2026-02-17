/*
 * ilifecyclecontrol.hh
 *
 *  Created on: Apr 9, 2013
 *      Author: paul
 *
 *  Description:  Contains header definition for the ILifecycleControl class.
 */

#ifndef ILIFECYCLE_CONTROL_HH
#define ILIFECYCLE_CONTROL_HH

#include "appparameters.h"

namespace MakerBot {

/*
 * Provides a common interface for classes that implement staged execution:
 *
 * 	Initialization - Init()
 * 	Start	       - Start()
 * 	Stop           - Stop
 */
class ILifecycleControl {

public:

	/*
	 * Default constructor.
	 */
	ILifecycleControl() {;}

	/*
	 * Destructor.
	 */
	virtual ~ILifecycleControl(){;}
	
	/*
	 * Implemented by derived classes for pre-startup initialization logic
	 */
	virtual void init(AppParameters &params) = 0;

	/*
	 * Implemented by derived classes for startup logic.
	 */
	virtual void start(AppParameters &params) = 0;

	/*
	 * Implemented by derived classes for stop / shutdown logic.
	 */
	virtual void stop() = 0;

};

}

#endif // ILIFECYCLE_CONTROL_HH
