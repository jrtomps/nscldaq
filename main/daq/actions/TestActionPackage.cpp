/**

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file   TestActionPackage.cpp
# @brief  Produces action messages that test the throttling.
# @author <fox@nscl.msu.edu>
*/

#include <unistd.h>
#include <iostream>
#include "Actions.h"
 
static void slowMessages() {
    std::cerr << "--------------------------- Slow messages ----------------\n";
    std::cerr << "Two messages will be output 2 secs apart for each severity\n";
    std::cerr << "Both messages should be present although they are identical\n";
    
    Actions::Error("sample Error Message");
    Actions::Log("Sample Log Message");
    Actions::Warning("Sample warning message");
    Actions::Output("Sample Output message");
    Actions::Debug("Sample Debug message");
    
    sleep(2);                 // Long enough to allow dups:

    Actions::Error("sample Error Message");
    Actions::Log("Sample Log Message");
    Actions::Warning("Sample warning message");
    Actions::Output("Sample Output message");
    Actions::Debug("Sample Debug message");

}

static void fastMessages() {
    std::cerr << "---------------- Fast Messages ------------------\n";
    std::cerr << "Several messages will be output quickly. Then a different\n";
    std::cerr << "Message will be output to force them all out.\n";
    std::cerr << "Should see the first message,then a message with 99 repeat count\n";
    std::cerr << "followed by the different message\n";
    
    for (int i = 0; i < 100; i++) {
        Actions::Error("This message is repeated rapidly");
    }
    Actions::Error("This message forces out the repeats");
}

static void alternateMessages()
{
    std::cerr << "------------------- Alternate Messages -------------------\n";
    std::cerr << "Several messages that will be different will be output\n";
    std::cerr << "No throttling can be done\n";
    
    
    for (int i =0; i < 10; i++) {
        Actions::Error("first message");
        Actions::Error("Second Message");
    }
}

int main (int argc, char** argv)
{
    slowMessages();
    fastMessages();
    alternateMessages();
}