#ifndef XMLSubSys_h
#define XMLSubSys_h
/***************************************************************************
                          XMLSubSys.h
			  Utility for initialisation and closing of the XML-subsystem
                             -------------------
    project              : SUMO
    begin                : Mon, 1 Jul 2002
    copyright            : (C) 2001 by DLR/IVF http://ivf.dlr.de/
    author               : Daniel Krajzewicz
    email                : Daniel.Krajzewicz@dlr.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
// $Log$
// Revision 1.4  2005/04/28 09:02:50  dkrajzew
// level3 warnings removed
//
// Revision 1.3  2004/01/26 07:15:08  dkrajzew
// some style changes only
//
// Revision 1.2  2003/02/07 10:53:52  dkrajzew
// updated
//
// Revision 1.1  2002/10/16 14:54:04  dkrajzew
// initial commit for xml-related utility functions
//
// Revision 1.1  2002/07/31 17:29:16  roessel
// Initial sourceforge commit.
//
// Revision 1.1  2002/07/02 08:29:48  dkrajzew
// A class to encapsulate the XML-Subsystem initialisation added
//
/* =========================================================================
 * compiler pragmas
 * ======================================================================= */
#pragma warning(disable: 4786)


/* =========================================================================
 * included modules
 * ======================================================================= */
#ifdef HAVE_CONFIG_H
#ifdef WIN32
#include <windows_config.h>
#else
#include <config.h>
#endif
#endif // HAVE_CONFIG_H


/* =========================================================================
 * class definitions
 * ======================================================================= */
/**
 * @class XMLSubSys
 * some static methods for initialisation and closing of the xml-subsystem.
 * The Xerces-Parses needs such an initialisation and should also be closed.
 * As we use this system for both the input files and the configuration we
 * would have to check whether the system was initialised before. Instead,
 * we call XMLSubSys::init() once at the beginning of our application and
 * XMLSubSys::close() at the end.
 */
class XMLSubSys {
public:
    /// initialises the xml-subsystem
    static bool init();

    /// closes the xml-subsystem
    static void close();

};

/**************** DO NOT DECLARE ANYTHING AFTER THE INCLUDE ****************/

#endif

// Local Variables:
// mode:C++
// End:
