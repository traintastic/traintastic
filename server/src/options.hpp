/**
 * server/src/options.hpp
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2019 Reinder Feenstra
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
#include <boost/preprocessor/stringize.hpp>

struct Options
{
#ifdef __unix__
  bool daemonize;
  std::string user;
  std::string group;
  std::string pidFile;
#endif
  std::string dataDir;
  std::string world;

  Options(int argc , char* argv[])
  {
#ifdef __unix__
    const char* home = getenv("HOME");
    if(home)
    {
      dataDir += home;
      dataDir += "/.config/traintastic-server";
    }
#endif

    boost::program_options::options_description desc{"Options for traintastic-server"};
    desc.add_options()
      ("help,h", "display this help text and exit")
      ("version,v", "output version information and exit")
#ifdef __unix__
      ("daemonize,d", "daemonize")
      ("user,u", boost::program_options::value<std::string>(&user), "run as user")
      ("group,g", boost::program_options::value<std::string>(&group), "run as group")
      ("pidfile,P", boost::program_options::value<std::string>(&pidFile), "pid file")
#endif
      ("datadir,D", boost::program_options::value<std::string>(&dataDir), "data directory")
      ;

    boost::program_options::variables_map vm;

    try
    {
      boost::program_options::store(parse_command_line(argc, argv, desc), vm);

      if(vm.count("help"))
      {
        std::cout << desc << std::endl;
        exit(EXIT_SUCCESS);
      }

      if(vm.count("version"))
      {
        std::cout << BOOST_PP_STRINGIZE(VERSION) << std::endl;
        exit(EXIT_SUCCESS);
      }

#ifdef __unix__
      daemonize = vm.count("daemonize");
#endif

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
