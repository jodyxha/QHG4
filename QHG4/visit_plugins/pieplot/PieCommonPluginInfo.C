// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//  File: PieCommonPluginInfo.C
// ****************************************************************************

#include <PiePluginInfo.h>
#include <PieAttributes.h>

// ****************************************************************************
//  Method: PieCommonPluginInfo::AllocAttributes
//
//  Purpose:
//    Return a pointer to a newly allocated attribute subject.
//
//  Returns:    A pointer to the newly allocated attribute subject.
//
//  Programmer: generated by xml2info
//  Creation:   omitted
//
// ****************************************************************************

AttributeSubject *
PieCommonPluginInfo::AllocAttributes()
{
    return new PieAttributes;
}

// ****************************************************************************
//  Method: PieCommonPluginInfo::CopyAttributes
//
//  Purpose:
//    Copy a Pie attribute subject.
//
//  Arguments:
//    to        The destination attribute subject.
//    from      The source attribute subject.
//
//  Programmer: generated by xml2info
//  Creation:   omitted
//
// ****************************************************************************

void
PieCommonPluginInfo::CopyAttributes(AttributeSubject *to,
    AttributeSubject *from)
{
    *((PieAttributes *) to) = *((PieAttributes *) from);
}

// ****************************************************************************
// Method: PieCommonPluginInfo::GetVariableTypes
//
// Purpose:
//   Returns a flag indicating the types of variables that can be put in the
//   plot's variable list.
//
// Returns:    A flag indicating the types of variables that can be put in
//             the plot's variable list.
//
//  Programmer: generated by xml2info
//  Creation:   omitted
//
// Modifications:
//
// ****************************************************************************

int
PieCommonPluginInfo::GetVariableTypes() const
{
    return VAR_CATEGORY_VECTOR | VAR_CATEGORY_ARRAY;
}

