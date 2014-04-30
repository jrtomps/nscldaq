
#ifndef ACTIONS_H
#define ACTIONS_H

#include <iostream>
#include <string>

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

    void ResumRun () {
        TCLCommand ( "resume" );
    }

    void EndRun () {
//        std::string tclcmd;
//        tclcmd =  "set sm [RunstateMachineSingleton %AUTO]; ";
//        tclcmd += "set state [$sm getState]; ";
//        tclcmd += "if {$state in [list Paused Active]} {";
//        tclcmd += "  $sm transition Halted ";
//        tclcmd += "} else { ";
//        tclcmd += "  ERROR: end run clicked when state is $state";
//        tclcmd += "}; $sm destroy";
        TCLCommand ( "end" );
    }
}

#endif
