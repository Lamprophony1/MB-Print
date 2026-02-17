/*
 * macapplication.h
 *
 *  Created on: Apr 9, 2013
 *      Author: paul
 *
 *  Description:  Contains header definition for the MacApplication class.
 */

#ifndef MAC_APPLICATION_HH
#define MAC_APPLICATION_HH

#include <unistd.h>
#include "appparameters.h"

namespace MakerBot {

/*
 *      Templatized class that provides a light framework for implementing an application.
 * 
 * 	This class instantiates an app object as a member, which means that an instance of this class really should be
 * 	declared statically.  An application's main() entry point should be the only point at which this class is used.
 *
 * 	Template Parameters:
 *
 * 		AppClass - Type of the application argument that will be instantiated.
 */
template<class AppClass>
class MacApplication {

public:

	/*
	 * Default constructor.
	 */
	MacApplication() {;}
	
	/*
	 * Destructor. 
	 */
	virtual ~MacApplication(){;}

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
            try {

                m_theApp.start(m_params);
				
                // TODO: Do something useful here
		while(1) {
                    sleep(1);
                }
		
            } catch(...) {
                // If an exception occurs, stop the app and attempt to restart according to m_autoRestart
                m_theApp.stop();
            }
	}
	
	/*
	 * Stops the application.
	 */
	inline void stop()
	{ 
		m_theApp.stop();
	}
	
private:
	
	AppClass m_theApp;
	AppParameters m_params;

};

}

#endif // MAC_APPLICATION_HH
