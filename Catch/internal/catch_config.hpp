/*
 *  catch_config.hpp
 *  Catch
 *
 *  Created by Phil on 08/11/2010.
 *  Copyright 2010 Two Blue Cubes Ltd. All rights reserved.
 *
 *  Distributed under the Boost Software License, Version 1.0. (See accompanying
 *  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef TWOBLUECUBES_CATCH_RUNNERCONFIG_HPP_INCLUDED
#define TWOBLUECUBES_CATCH_RUNNERCONFIG_HPP_INCLUDED

#include "internal/catch_hub.h"
#include "catch_interfaces_reporter.h"

#include <memory>
#include <vector>
#include <string>
#include <iostream>

namespace Catch
{
    class Config : public IReporterConfig
    {
    private:
        Config( const Config& other );
        Config& operator = ( const Config& other );
    public:

        struct Include { enum What
        {
            FailedOnly,
            SuccessfulResults
        }; };

        struct List{ enum What
        {
            None = 0,

            Reports = 1,
            Tests = 2,
            All = 3,

            WhatMask = 0xf,

            AsText = 0x10,
            AsXml = 0x11,

            AsMask = 0xf0
        }; };


        Config()
        :   m_reporter( NULL ),
            m_listSpec( List::None ),
            m_shouldDebugBreak( false ),
            m_showHelp( false ),
            m_os( std::cout.rdbuf() ),
            m_includeWhat( Include::FailedOnly )
        {}

        void setReporterInfo( const std::string& reporterName )
        {
            if( m_reporter.get() )
                return setError( "Only one reporter may be specified" );
            setReporter( Hub::getReporterRegistry().create( reporterName, *this ) );
        }

        void addTestSpec( const std::string& testSpec )
        {
            m_testSpecs.push_back( testSpec );
        }
        void setListSpec( List::What listSpec )
        {
            m_listSpec = listSpec;
        }

        void setFilename( const std::string& filename )
        {
            m_filename = filename;
        }

        std::string getFilename()
        {
            return m_filename;
        }

        void setError( const std::string& errorMessage )
        {
            m_message = errorMessage + "\n\n" + "Usage: ...";
        }

        void setReporter( IReporter* reporter )
        {
            m_reporter = std::auto_ptr<IReporter>( reporter );
        }

        IReporter* getReporter()
        {
            if( !m_reporter.get() )
                setReporter( Hub::getReporterRegistry().create( "basic", *this ) );
            return m_reporter.get();
        }

        IReporter* getReporter() const
        {
            return const_cast<Config*>( this )->getReporter();
        }

        List::What listWhat() const
        {
            return (List::What)( m_listSpec & List::WhatMask );
        }

        List::What listAs() const
        {
            return (List::What)( m_listSpec & List::AsMask );
        }

        void setIncludeWhat( Include::What includeWhat )
        {
            m_includeWhat = includeWhat;
        }
        void setShouldDebugBreak( bool shouldDebugBreak )
        {
            m_shouldDebugBreak = shouldDebugBreak;
        }
        bool shouldDebugBreak() const
        {
            return m_shouldDebugBreak;
        }
        void setShowHelp( bool showHelp )
        {
            m_showHelp = showHelp;
        }
        bool showHelp() const
        {
            return m_showHelp;
        }

        ///////////////////////////////////////////////////////////////////////////
        virtual std::ostream& stream() const
        {
            return m_os;
        }

        ///////////////////////////////////////////////////////////////////////////
        void setStreamBuf( std::streambuf* buf )
        {
            m_os.rdbuf( buf );
        }

        ///////////////////////////////////////////////////////////////////////////
        virtual bool includeSuccessfulResults() const
        {
            return m_includeWhat == Include::SuccessfulResults;
        }



        std::auto_ptr<IReporter> m_reporter;
        std::string m_filename;
        std::string m_message;
        List::What m_listSpec;
        std::vector<std::string> m_testSpecs;
        bool m_shouldDebugBreak;
        bool m_showHelp;
        mutable std::ostream m_os;
        Include::What m_includeWhat;
    };

} // end namespace Catch

#endif // TWOBLUECUBES_CATCH_RUNNERCONFIG_HPP_INCLUDED
