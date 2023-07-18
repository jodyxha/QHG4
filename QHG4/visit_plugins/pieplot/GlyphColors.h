// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//  File: GlyphColors.h
// ****************************************************************************

#ifndef __GLYPHCOLORS_H__
#define __GLYPHCOLORS_H__

// ********************************************************
// 
// Purpose:
//   color constants
//
// Programmer: jodyxha
// Creation: August 29, 2021
//
// Modifications:
//
// ********************************************************

const double black_color[4] = {    0.0,     0.0,    0.0, 255.0};  // BLACK
const double white_color[4] = {  255.0,   255.0,  255.0, 255.0};  // WHITE

const double basic_colors[][4] = {
    {255.0,   0.0,   0.0, 255.0},  // RED
    {  0.0, 255.0,   0.0, 255.0},  // GREEN
    {  0.0,   0.0, 255.0, 255.0},  // BLUE
    {  0.0, 255.0, 255.0, 255.0},  // CYAN
    {255.0,   0.0, 255.0, 255.0},  // MAGENTA
    {255.0, 255.0,   0.0, 255.0},  // YELLOW
    {127.0, 127.0, 255.0, 255.0},  // LILA
    {127.0, 127.0,   0.0, 255.0},  // GOLD
    {127.0, 127.0, 127.0, 255.0},  // GRAY
    {127.0, 255.0, 127.0, 255.0},  // AVOCADO
    {127.0,   0.0, 127.0, 255.0},  // PURPLE
    {255.0, 127.0, 127.0, 255.0},  // PEACH
    {  0.0, 127.0, 127.0, 255.0},  // TURQ
    {255.0, 255.0, 127.0, 255.0},  // LYELLOW
    {127.0, 255.0, 255.0, 255.0},  // LBLUE
    {255.0, 127.0, 255.0, 255.0},  // PINK
    {255.0, 255.0, 255.0, 255.0},  // WHITE
};

const double rbow_colors[][4] = {
    {255.0,   0.0,   0.0, 255.0},  // RED
    {255.0, 255.0,   0.0, 255.0},  // YELLOW
    {  0.0, 255.0,   0.0, 255.0},  // GREEN
    {  0.0, 255.0, 255.0, 255.0},  // CYAN
    {  0.0,   0.0, 255.0, 255.0},  // BLUE
    {255.0,   0.0, 255.0, 255.0},  // MAGENTA
    {127.0, 127.0, 255.0, 255.0},  // LILA
    {127.0, 127.0,   0.0, 255.0},  // GOLD
    {127.0, 127.0, 127.0, 255.0},  // GRAY
    {127.0, 255.0, 127.0, 255.0},  // AVOCADO
    {127.0,   0.0, 127.0, 255.0},  // PURPLE
    {255.0, 127.0, 127.0, 255.0},  // PEACH
    {  0.0, 127.0, 127.0, 255.0},  // TURQ
    {255.0, 255.0, 127.0, 255.0},  // LYELLOW
    {127.0, 255.0, 255.0, 255.0},  // LBLUE
    {255.0, 127.0, 255.0, 255.0},  // PINK
    {255.0, 255.0, 255.0, 255.0},  // WHITE
};

const double reds[][4] = {
    {255.0,   0.0,   0.0, 255.0},  // RED
    {255.0,  64.0,   0.0, 255.0},  // 
    {255.0, 128.0,   0.0, 255.0},  // 
    {255.0, 192.0,   0.0, 255.0},  // 
    {255.0, 255.0,   0.0, 255.0},  // YELLOW
    {192.0, 255.0,   0.0, 255.0},  // 
    {128.0, 255.0,   0.0, 255.0},  // 
    { 64.0, 255.0,   0.0, 255.0},  // 
    {  0.0, 255.0,   0.0, 255.0},  // GREEN
    {  0.0, 255.0,  64.0, 255.0},  // 
    {  0.0, 255.0, 128.0, 255.0},  // 
    {  0.0, 255.0, 192.0, 255.0},  // 
    {  0.0, 255.0, 255.0, 255.0},  // CYAN
    {  0.0, 255.0, 255.0, 255.0},  // 
    {  0.0, 192.0, 255.0, 255.0},  // 
    {  0.0, 127.0, 255.0, 255.0},  //
    {  0.0,  64.0, 255.0, 255.0},  //
    {  0.0,   0.0, 255.0, 255.0},  // BLUE
    {  0.0,   0.0, 255.0, 255.0},  // 
    { 64.0,   0.0, 255.0, 255.0},  // 
    {128.0,   0.0, 255.0, 255.0},  // 
    {192.0,   0.0, 255.0, 255.0},  // 
    {255.0,   0.0, 255.0, 255.0},  // MAGENTA
    {128.0, 255.0, 255.0, 255.0},  // WHITE
};


#endif