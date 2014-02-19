/*
 *  catch_section.hpp
 *  Catch
 *
 *  Created by Phil on 03/11/2010.
 *  Copyright 2010 Two Blue Cubes Ltd. All rights reserved.
 *
 *  Distributed under the Boost Software License, Version 1.0. (See accompanying
 *  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef TWOBLUECUBES_CATCH_SECTION_HPP_INCLUDED
#define TWOBLUECUBES_CATCH_SECTION_HPP_INCLUDED

#include "catch_capture.hpp"

#include <string>

namespace Catch
{
    class Section
    {
    public:
        Section( const std::string& name, const std::string& description )
        :   m_name( name ),
            m_sectionIncluded( ResultsCapture::getListener().sectionStarted( name, description, m_successes, m_failures ) )
        {
        }

        ~Section()
        {
            ResultsCapture::getListener().sectionEnded( m_name, m_successes, m_failures );
        }

        // This indicates whether the section should be executed or not
        operator bool()
        {
            return m_sectionIncluded;
        }

    private:
        std::string m_name;
        std::size_t m_successes;
        std::size_t m_failures;
        bool m_sectionIncluded;
    };

} // end namespace Catch

#define CATCH_SECTION( name, desc ) if( Catch::Section catch_internal_Section = Catch::Section( name, desc ) )

#endif // TWOBLUECUBES_CATCH_SECTION_HPP_INCLUDED
