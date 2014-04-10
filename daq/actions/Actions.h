#include <iostream>

namespace Actions 
{
    void Error (std::string message ) {
        std::cerr << "ERRMSG " << message << std::endl;
    }  

    void Log (std::string message ) {
        std::cerr << "LOGMSG " << message << std::endl;
    }  

    void Warning (std::string message ) {
        std::cerr << "WRNMSG " << message << std::endl;
    }  

    void TCLCommand (std::string message ) {
        std::cerr << "TCLCMD " << message << std::endl;
    }  

    void Output (std::string message ) {
        std::cerr << "OUTPUT " << message << std::endl;
    }  

    void Debug (std::string message ) {
        std::cerr << "DBGMSG " << message << std::endl;
    }  

    void PauseRun () {
        std::cerr << "TCLCMD pause" << std::endl;
    }

    void StopRun () {
        std::cerr << "TCLCMD end" << std::endl;
    }
}
