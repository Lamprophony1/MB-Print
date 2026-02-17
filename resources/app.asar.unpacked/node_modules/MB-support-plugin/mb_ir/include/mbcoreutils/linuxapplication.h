/*
 * linuxapplication.h
 *
 *  Created on: Apr 9, 2013
 *      Author: paul
 *
 *  Description:  Contains header definition for the LinuxApplication class.
 */

#ifndef LINUX_APPLICATION_HH
#define LINUX_APPLICATION_HH

#include <unistd.h>
#include "appparameters.h"

namespace MakerBot {

/*
 * Templatized class that provides a light framework for implementing an application in one of the following ways:
 *
 * 	1) As a Linux daemon
 * 	2) As a linux application that terminates on an exception
 * 	3) As a linux application that automatically recovers via restart on any exception
 *
 * 	This class instantiates an app object as a member, which means that an instance of this class really should be
 * 	declared statically.  An application's main() entry point should be the only point at which this class is used.
 *
 * 	Template Parameters:
 *
 * 		AppClass - Type of the application argument that will be instantiated.
 */
template<class AppClass>
class LinuxApplication {

public:

	// Used to specify the mode that this class will run in.
	enum AppMode {
		APPMODE_DEFAULT = 0, // App will behave like an ordinary app.  If it is to stay running, it is up to the app object to do so.
		APPMODE_DAEMON  = 1 // App will become a Linux daemon and will enter a daemon loop.
	};

	/*
	 * Default constructor.
	 */
	LinuxApplication() {;}

	/*
	 * Alternate constructor.
	 *
	 * Arguments:
	 *
	 * 		autoRestart - Indicates if the application should be restarted in the event of an exception
	 * 		runMode     - AppMode value for an instance of this class.
	 */
	LinuxApplication(bool autoRestart, AppMode runMode = APPMODE_DEFAULT) : 
		m_autoRestart(autoRestart),
		m_runMode(runMode)
	{;}
	
	/*
	 * Destructor.
	 */
	virtual ~LinuxApplication(){;}

	/*
	 * Provides pre-startup initialziation logic for the application.  Called by main().
	 *
	 */
	inline void init(int argc, char* argv[])
	{ 
		m_params = AppParameters(argc, argv);
		m_theApp.init(m_params);
	}
	
	/*
	 * Main execution loop for the application.
	 */
	inline void start()
	{
		// Main application loop.  Runs until m_autoRestart is false.
		do
		{
			try {

				m_theApp.start(m_params);
				
				switch(m_runMode) {
					
					// Enter the app daemon loop
					case APPMODE_DAEMON:
						daemonLoop();
						break;
					default:
						// If not a daemon and Start() returns, terminate the application.
						m_theApp.stop();
						m_autoRestart = false;
						break;
				}
			} catch(...) {
				// If an exception occurs, stop the app and attempt to restart according to m_autoRestart
				m_theApp.stop();
			}

		} while (m_autoRestart);
	}
	
	/*
	 * Stops the application.
	 */
	inline void stop()
	{ 
		m_theApp.stop();
	}
	
private:

	/*
	 * Daemon loop for the application.
	 */
	void daemonLoop()
	{
		// Do daemon work here
		while(1) {
			//m_theApp.start(m_params);
			sleep(1);
		}
		
		// Upon exiting the deamon loop, stop the app.  This needs to be revisited to properly handle OS events.
		m_theApp.stop();
	}
	
	AppClass m_theApp;
	bool m_autoRestart;
	AppMode m_runMode;
	AppParameters m_params;

};

}

#endif // LINUX_DAEMON_HH
