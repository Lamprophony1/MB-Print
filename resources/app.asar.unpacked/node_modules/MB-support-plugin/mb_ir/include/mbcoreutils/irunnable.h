/*
 * IRunnable.hh
 *
 *  Created on: Apr 16, 2013
 *      Author: paul
 *
 *  Contains the full definition of the IRunnable class.
 */

#ifndef IRUNNABLE_HH_
#define IRUNNABLE_HH_

#include "commontypes.h"

namespace MakerBot {

	/*
	 * This class defines an interface that can be used to make an object runnable by a thread via the MakerBot::Thread class.
	 * Derived classes need only implement the run() method, which becomes the entry point of the thread.
	 */
	class IRunnable {
	public:

		/*
		 * Default constructor.  Initializes m_isRunning to false.
		 */
		IRunnable()
			: m_isRunning(false) {;}

		/*
		 * Destructor.
		 */
		virtual ~IRunnable(){;}

		/*
		 * Main entry point of a thread invoking a derived instance of this class.  Needs to be overridden.
		 */
		virtual void run() { m_isRunning = true; }

		/*
		 * Stops the run() function.
		 */
		void terminate() { m_isRunning = false;}

		/*
		 * Returns true if running, false otherwise.
		 */
		bool isRunning() { return m_isRunning; }

		/*
		 * Allows a derived instance of this class to be invoked as a function.
		 */
		void operator()() {
			this->run();
		}

	private:

		// Maintains running state of a derived instance of this class.
		volatile bool m_isRunning;
	};

	typedef shared_ptr<MakerBot::IRunnable>::type RunnablePtr;

}


#endif /* IRUNNABLE_HH_ */
