
#ifndef ACTIONS_H
#define ACTIONS_H

#include <iostream>
#include <string>

/**! The C++ functions for the actions package */
namespace Actions 
{
    void Error (std::string message ) {
        std::cerr << "ERRMSG " << message.size() << " " 
                  << message << "\n" << std::flush;
    }  

    void Log (std::string message ) {
        std::cerr << "LOGMSG " << message.size() << " " 
                  << message << "\n" << std::flush;
    }  

    void Warning (std::string message ) {
        std::cerr << "WRNMSG " << message.size() << " " 
                  << message << "\n" << std::flush;
    }  

    void TCLCommand (std::string message ) {
        std::cerr << "TCLCMD " << message.size() << " " 
                  << message << "\n" << std::flush;
    }  

    void Output (std::string message ) {
        std::cerr << "OUTPUT " << message.size() << " " 
                  << message << "\n" << std::flush;
    }  

    void Debug (std::string message ) {
        std::cerr << "DBGMSG " << message.size() << " "
                  << message << "\n" << std::flush;
    }  

    void BeginRun () {
        TCLCommand ( "begin" );
    }

    void PauseRun () {
        TCLCommand ( "pause" );
    }

    void ResumeRun () {
        TCLCommand ( "resume" );
    }

    void EndRun (bool propagate=true) {
        if (propagate) {
          TCLCommand ( "end" );
        } else {
          TCLCommand ( "local_end" );
        }
    }
}

#endif
