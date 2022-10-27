/**
 * server/src/options.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019-2020 Reinder Feenstra
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef TRAINTASTIC_SERVER_OPTIONS_HPP
#define TRAINTASTIC_SERVER_OPTIONS_HPP

#include <boost/program_options.hpp>
#include <version.hpp>

struct Options
{
#ifdef WIN32
  bool tray;
#endif
#ifdef __unix__
  bool daemonize;
  std::string user;
  std::string group;
  std::string pidFile;
#endif
  std::string dataDir;
  std::string world;
  bool simulate;
  bool online;
  bool power;
  bool run;

  Options(int argc , char* argv[])
  {
    boost::program_options::options_description desc{"Options for traintastic-server"};
    desc.add_options()
      ("help,h", "display this help text and exit")
      ("version,v", "output version information and exit")
#ifdef WIN32
      ("tray", "run as system tray application")
#endif
#ifdef __unix__
      ("daemonize,d", "daemonize")
      ("user,u", boost::program_options::value<std::string>(&user)->value_name("USERNAME"), "run as user")
      ("group,g", boost::program_options::value<std::string>(&group)->value_name("GROUPNAME"), "run as group")
      ("pidfile,P", boost::program_options::value<std::string>(&pidFile)->value_name("FILENAME")->default_value("")->implicit_value("/run/traintastic-server.pid"), "write pid file")
#endif
      ("datadir,D", boost::program_options::value<std::string>(&dataDir)->value_name("PATH"), "data directory")
      ("world,W", boost::program_options::value<std::string>(&world)->value_name("UUID"), "world UUID to load")
      ("simulate", "enable simulation after loading world")
      ("online", "enable communication after loading world")
      ("power", "enable power after loading world")
      ("run", "start after loading world")
      ;

    boost::program_options::variables_map vm;

    try
    {
      boost::program_options::store(parse_command_line(argc, argv, desc), vm);

      if(vm.count("help"))
      {
        std::cout
          << desc << std::endl
          << "NOTES:"<< std::endl
          << "1. --simulate, --online, --power and --run options only apply to the world loaded at startup." << std::endl
          << "2. --run option requires --power, --power option must be set for --run to work."
          << std::endl
          ;
        exit(EXIT_SUCCESS);
      }

      if(vm.count("version"))
      {
        std::cout << TRAINTASTIC_VERSION_FULL << std::endl;
        exit(EXIT_SUCCESS);
      }

#ifdef WIN32
      tray = vm.count("tray");
#endif

#ifdef __unix__
      daemonize = vm.count("daemonize");
#endif

      simulate = vm.count("simulate");
      online = vm.count("online");
      power = vm.count("power");
      run = vm.count("run");

      if(!power && run)
      {
        std::cerr << "Error: --run option requires --power option to be set too." << std::endl;
        exit(EXIT_FAILURE);
      }

      boost::program_options::notify(vm);
    }
    catch(const boost::program_options::required_option& e)
    {
      std::cerr << e.what() << std::endl << std::endl << desc << std::endl;
      exit(EXIT_FAILURE);
    }
    catch(const boost::program_options::error& e)
    {
      std::cerr << e.what() << std::endl;
      exit(EXIT_FAILURE);
    }
  }
};

#endif
