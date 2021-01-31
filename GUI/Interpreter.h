/// -----------------------------------------------------------------------------
// This file is part of vAmiga Bare Metal
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include <string>
#include <list>

class Controller;

enum class Token
{
    none,
    
    // Components
    agnus, amiga, cia, cpu, denise, dfn, diskcontroller, paula, rtc,

    // Commands
    about, easteregg, eject, help, insert, dump, list, on, off, pause, reset,
    run, set,
    
    // Keys
    events, revision
};

typedef std::list<std::string> Arguments;

struct CmdDescriptor {
    
    string name;
    string arg1, arg2;
    string info;
    std::vector<CmdDescriptor> args;
    void (Controller::*func)(Arguments&, long) = nullptr;
    isize numArgs = 0;
    long param = 0;
    
    // Returns a matching descriptor from the args vector
    CmdDescriptor *seek(const string& token);
    
    // Appends a new descriptor to the args vector
    CmdDescriptor *add(const std::string &token,
                       const std::string &a1, const std::string &a2,
                       const std::string &help,
                       void (Controller::*func)(Arguments&, long) = nullptr,
                       isize numArgs = 0, long param = 0);
    
    CmdDescriptor *add(const std::string &t1, const std::string &t2,
                       const std::string &a1, const std::string &a2,
                       const std::string &help,
                       void (Controller::*func)(Arguments&, long) = nullptr,
                       isize numArgs = 0, long param = 0);
    
    CmdDescriptor *add(const std::string &t1, const std::string &t2, const std::string &t3,
                       const std::string &a1, const std::string &a2,
                       const std::string &help,
                       void (Controller::*func)(Arguments&, long) = nullptr,
                       isize numArgs = 0, long param = 0);
};

struct ParseError : public std::exception
{
    std::string description;
    ParseError() : description("") { }
    ParseError(const std::string &s) : description(s) { }
    const char *what() const throw() override { return description.c_str(); }
};

struct UnknownComponentError : public ParseError {
    UnknownComponentError(const std::string &s) : ParseError(s) { }
};

struct UnknownCommandError : public ParseError {
    UnknownCommandError(const std::string &s) : ParseError(s) { }
};

struct TooFewArgumentsError : public ParseError {
    TooFewArgumentsError() : ParseError() { }
};

struct TooManyArgumentsError : public ParseError {
    TooManyArgumentsError() : ParseError() { }
};

class Interpreter
{
    // Reference to other components
    class Application &app;
    class Controller &controller;
    
    // The registered instruction set
    CmdDescriptor root;
    
    
    //
    // Initializing
    //

public:
    
    Interpreter(Application &ref);

private:
    
    // Registers the instruction set
    void registerInstructions();
    
    
    //
    // Parsing input
    //
    
private:
    
    string lowercased(const std::string& s);
    
    
    //
    // Executing commands
    //
    
public:
    
    void exec(const std::string& userInput);

private:

    // Executes a command that consists of a single word
    bool execSingle(Arguments &argv);

    // Executes a command that is composed out of multiple words
    bool execMultiple(Arguments &argv);
};
