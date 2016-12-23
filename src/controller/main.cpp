//******************************************************************************
//* File:   oat control main.cpp
//* Author: Jon Newman <jpnewman snail mit dot edu>
//*
//* Copyright (c) Jon Newman (jpnewman snail mit dot edu)
//* All right reserved.
//* This file is part of the Oat project.
//* This is free software: you can redistribute it and/or modify
//* it under the terms of the GNU General Public License as published by
//* the Free Software Foundation, either version 3 of the License, or
//* (at your option) any later version.
//* This software is distributed in the hope that it will be useful,
//* but WITHOUT ANY WARRANTY; without even the implied warranty of
//* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//* GNU General Public License for more details.
//* You should have received a copy of the GNU General Public License
//* along with this source code.  If not, see <http://www.gnu.org/licenses/>.
//****************************************************************************

#include "OatConfig.h" // Generated by CMake

#include <iostream>
#include <csignal>
#include <boost/program_options.hpp>

#include "Controller.h"

#include "../../lib/utility/IOFormat.h"

namespace po = boost::program_options;

void printUsage(po::options_description options) {
    std::cout << "Usage: control [INFO]\n"
              << "   or: control ENDPOINT [INFO]\n"
              << "   or: control ENDPOINT ID COMMAND\n"
              << "   or: control\n"
              << "Control running oat components.\n\n"
              << options << "\n";
}

int main(int argc, char *argv[]) {

    std::string comp_name = "control";
    po::options_description visible_options("OPTIONS");

    try {

        po::options_description options("INFO");
        options.add_options()
            ("help", "Produce help message.")
            ("version,v", "Print version information.")
            ("list,l", "Print a list of controllable components, along with IDs " 
             "and valid commands, for the specified endpoint.")
            ;

        po::options_description hidden("HIDDEN OPTIONS");
        hidden.add_options()
            ("endpoint", po::value<std::string>(),
             "ZMQ style endpoint specifier: "
             "'<transport>://<host>:<port>'. For instance, 'tcp://*:5555' to "
             "specify TCP communication on port 5555. Or, for interprocess "
             "communication: '<transport>://<user-named-pipe>. For instance "
             "'ipc:///tmp/test.pipe'.  Internally, this is used to construct a "
             "ZMQ ROUTER socket that routes COMMANDs to running components "
             "using their ID.")
            ("id", po::value<std::string>(),
            "The ID of the running component to send a command to.")
            ("command", po::value<std::string>(),
            "The COMMAND to send.")
            ;

        po::positional_options_description positional_options;
        positional_options.add("endpoint", 1);
        positional_options.add("id", 1);
        positional_options.add("command", 1);

        po::options_description all_options("ALL");
        all_options.add(options).add(hidden);

        visible_options.add(options);

        po::variables_map variable_map;
        po::store(po::command_line_parser(argc, argv)
                .options(all_options)
                .positional(positional_options)
                .run(),
                variable_map);
        po::notify(variable_map);

        // Use the parsed options
        if (variable_map.count("help")) {
            printUsage(options);
            return 0;
        }

        if (variable_map.count("version")) {
            std::cout << "Oat Control version "
                      << Oat_VERSION_MAJOR
                      << "."
                      << Oat_VERSION_MINOR
                      << "\n";
            std::cout << "Written by Jonathan P. Newman in the MWL@MIT.\n";
            std::cout << "Licensed under the GPL3.0.\n";
            return 0;
        }

        if (variable_map.empty()) {
            // Start stdin intepreter
            std::cout << "Interpreter not implemented." << std::endl;
            return 0;

        } else if ((variable_map.count("endpoint") 
                  && !variable_map.count("id")
                  && !variable_map.count("command"))) {

            auto endpoint = variable_map["endpoint"].as<std::string>();

            oat::Controller ctrl(endpoint.c_str());
            ctrl.scan();
            std::cout << ctrl.list();

        } else if ((variable_map.count("endpoint") 
                  && variable_map.count("id")
                  && variable_map.count("command"))) {

            auto endpoint = variable_map["endpoint"].as<std::string>();
            auto id = variable_map["id"].as<std::string>();
            auto command = variable_map["command"].as<std::string>();
            
            oat::Controller ctrl(endpoint.c_str());
            ctrl.scan();

            if (id[0] == 'O')
                ctrl.send(command, id);
            else
                ctrl.send(command, std::stoi(id));

        } else {

            printUsage(visible_options);
            std::cout << "Error: Invalid program option specification.\n";
            return -1;
        }

    } catch (const po::error &ex) {
        printUsage(visible_options);
        std::cerr << oat::whoError(comp_name, ex.what()) << std::endl;
    } catch (const zmq::error_t &ex) {
        std::cerr << oat::whoError(comp_name + "(ZMQ) " , ex.what()) << std::endl;
    } catch (const std::runtime_error &ex) {
        std::cerr << oat::whoError(comp_name, ex.what()) << std::endl;
    } catch (...) {
        std::cerr << oat::whoError(comp_name, "Unknown exception.")
                  << std::endl;
    }

    // Exit
    return 0;
}