/*
 *  catch_runner.hpp
 *  Catch
 *
 *  Created by Phil on 31/10/2010.
 *  Copyright 2010 Two Blue Cubes Ltd. All rights reserved.
 *
 *  Distributed under the Boost Software License, Version 1.0. (See accompanying
 *  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef TWOBLUECUBES_CATCH_RUNNER_HPP_INCLUDED
#define TWOBLUECUBES_CATCH_RUNNER_HPP_INCLUDED

#include "internal/catch_commandline.hpp"
#include "internal/catch_list.hpp"
#include "internal/catch_hub_impl.hpp"
#include "catch_reporter_basic.hpp"
#include "catch_reporter_xml.hpp"
#include "catch_reporter_junit.hpp"

#include <fstream>

namespace Catch
{
    inline int Main( int argc, char * const argv[] )
    {
        Config config;
        ArgParser( argc, argv, config );     
        
        if( !config.m_message.empty() )
        {
            std::cerr << config.m_message << std::endl;
            return std::numeric_limits<int>::max();
        }
        
        // Handle help
        if( config.showHelp() )
        {
            std::string exeName( argv[0] );
            std::string::size_type pos = exeName.find_last_of( "/\\" );
            if( pos != std::string::npos )
            {
                exeName = exeName.substr( pos+1 );
            }
            
            std::cout   << exeName << " is a CATCH host application. Options are as follows:\n\n"
                        << "\t-l, --list <tests | reporters> [xml]\n"
                        << "\t-t, --test <testspec> [<testspec>...]\n"
                        << "\t-r, --reporter <reporter name>\n"
                        << "\t-o, --out <file name>\n"
                        << "\t-s, --success\n"
                        << "\t-b, --break\n\n"
                        << "For more detail usage please see: https://github.com/philsquared/Catch/wiki/Command-line" << std::endl;
            return 0;
        }
        
        // Handle list request
        if( config.listWhat() != Config::List::None )
            return List( config );
        
        // Open output file, if specified
        std::ofstream ofs;
        if( !config.getFilename().empty() )
        {
            ofs.open( config.getFilename().c_str() );
            if( ofs.fail() )
            {
                std::cerr << "Unable to open file: '" << config.getFilename() << "'" << std::endl;
                return std::numeric_limits<int>::max();
            }
            config.setStreamBuf( ofs.rdbuf() );
        }

        Runner runner( config );

        // Run test specs specified on the command line - or default to all
        if( config.m_testSpecs.size() == 0 )
        {
            config.getReporter()->StartGroup( "" );
            runner.runAll();
            config.getReporter()->EndGroup( "", runner.getSuccessCount(), runner.getFailureCount() );
        }
        else
        {
            // !TBD We should get all the testcases upfront, report any missing,
            // then just run them
            std::vector<std::string>::const_iterator it = config.m_testSpecs.begin();
            std::vector<std::string>::const_iterator itEnd = config.m_testSpecs.end();
            for(; it != itEnd; ++it )
            {
                size_t prevSuccess = runner.getSuccessCount();
                size_t prevFail = runner.getFailureCount();
                config.getReporter()->StartGroup( *it );
                if( runner.runMatching( *it ) == 0 )
                {
                    // Use reporter?
//                    std::cerr << "\n[Unable to match any test cases with: " << *it << "]" << std::endl;
                }
                config.getReporter()->EndGroup( *it, runner.getSuccessCount()-prevSuccess, runner.getFailureCount()-prevFail );
            }
        }
        return static_cast<int>(runner.getFailureCount());
    }
    
} // end namespace Catch

#endif // TWOBLUECUBES_CATCH_RUNNER_HPP_INCLUDED