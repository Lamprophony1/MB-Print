/*
 * appparameters.h
 *
 *  Created on: Apr 9, 2013
 *      Author: paul
 *
 *  Description:  Contains header definition for the AppParameters class.
 */

#ifndef APP_PARAMETERS_HH
#define APP_PARAMETERS_HH

#include <cstddef>
#include <map>
#include <string>

namespace MakerBot {

/*
 * This class is a light wrapper for the parameters provided to an application's main() entry point.
 *
 * Note:  This class may be extended at some point to provide parsed, dictionary-based lookup of
 *        command line arguments.
 */
class AppParameters {
public:

	typedef std::map<std::string, std::string> ParameterMap;

	/*
	 * Default Constructor.
	 */
	AppParameters() :
		m_argc(0),
		m_argv(NULL){

	}

	/*
	 * Alternate constructor.  Initializes all members.
	 */
	AppParameters(int argc, char* argv[]) :
		m_argc(argc),
		m_argv(argv) {
            parse();
       }
	
	/*
	 * Destructor.
	 */
	virtual ~AppParameters(){;}

        /*
         * Returns a map containing all parameters parsed form the command line with their associated value(s).
         */
	ParameterMap& parameters() { return m_parsedArgs; }
	
	// Members.  Left public (for now) for ease of use.
	int m_argc;
	char** m_argv;

private:

	void parse() {
                
            // Last arg - The last "--" argument detected          
            std::string lastArg;
            
            // Iterate over the list of provided arguments.
            for (int i = 0; i < m_argc; ++i) {

                // Current argument being evaluated      
		std::string curVal = m_argv[i];
                
                // If the current value starts with "--", it is an argument that may or may not have an associated value.  Remember it, strip the "--" off of it and add it
                // to the argument map.
                if (curVal.find("--") == 0) {
                    curVal = curVal.erase(0,2);
                    lastArg = curVal;
		    m_parsedArgs[curVal] = "";                        
                }
                else {
                    // The current value is a value for a parsed argument.  If the last thing seen was an argument, add the value to the dictionary.  Otherwise the value is lost.
                    if (!lastArg.empty()) {
                        m_parsedArgs[lastArg] = curVal;
                    }

                    lastArg.clear();
                }
	    }
        }


	ParameterMap m_parsedArgs;
};


}

#endif // APP_PARAMETERS_H
