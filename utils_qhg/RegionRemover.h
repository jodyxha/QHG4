#ifndef __REGIONREMOVER_H__
#define __REGIONREMOVER_H__

#include <cstdio>

#include <hdf5.h>

#include <vector>
#include <map>
#include <string>

#include "QDFUtils.h"

typedef std::vector<double>   dvec;
typedef std::pair<dvec, dvec> coords;
typedef std::map<std::string, coords> namedcoords;

static dvec vamericas[2] = {{-168.4, -268.4, -29.2, -29.2, -90.8},
                       {87.877, -60.923, -60.923, 48.677, 86.01}};

static dvec vafrica[2] = {{-20.8, -20.8, 20.667, 60, 56.533, 44, 32.533, 12.667, 11.2, 9.733, -5.733},
                     {35.477, 2.677,-39.99,-38.39,14.677,11.877,37.743,34.810, 38.943, 37.743,35.743}};

static dvec vaustralia[2] = {{156.667, 133.467, 109.733, 114.4, 148.0, 157.733},
                        {-14.523, -8.390, -20.79, -36.123, -45.857, -23.323}};

static dvec vaustralia2[2] = {{131.733, 108.167, 111.433, 156.700, 156.233},
                     {  8.319, -16.832, -48.103, -48.803, 12.097}}; 

static dvec veurasia[2] = {{ -21.0,  6.267, 15.60, 32.80, 32.67, 43.733, 57.867, 121.467, 136.933, 191.067, 180.533, 48.0, 0.0, -21.33},
                    { 34.143, 38.143, 36.010, 32.81, 29.077, 12.543, 14.277, -13.457, 30.277, 65.477, 81.477, 89.610, 81.543, 53.343}}; 


static dvec vantarctica[2] = {{-180.0, -180.0, 180.0, 180.0},
                       {-90.0, -60.0, -60.0, -90.0}}; 



static dvec vafrica2[2] = {{-20.8, -20.8, 20.667, 60, 56.533, 44, 42.6, 41.8, 34.4, 32.8, 31.6, 14.4, 10.2, -0.2, -5.6},
                           {35.477, 2.677, -39.99, -38.39, 14.677, 11.877, 14.1, 15.5, 27.5, 29.3, 32.9, 35.1, 38.5, 36.7, 36.1}};    
  

static dvec veurasafrica[2] = {{  20.00,  38.40,  63.00,  76.80,  84.60,  89.40, 90.00,  116.80,  132.40,  156.20,  215.0,  219.0,  73.60,  67.00,  -9.40, -20.40, -16.60,  13.00},
                               { -39.97, -26.17,  22.83,   4.43,   5.23,  18.23,  2.63,  -14.97,   -5.37,  -15.37,   56.0,   77.0,  87.43,  78.43,  66.03,   2.43,   0.00, -34.97}};
static namedcoords mvRegions{
    std::make_pair("americas",    coords(vamericas[0],    vamericas[1])),
    std::make_pair("africa",      coords(vafrica[0],      vafrica[1])),
    std::make_pair("africa2",     coords(vafrica2[0],     vafrica2[1])),
    std::make_pair("australia",   coords(vaustralia[0],   vaustralia[1])),
    std::make_pair("australia2",  coords(vaustralia2[0],  vaustralia2[1])),
    std::make_pair("eurasia",     coords(veurasia[0],     veurasia[1])),
    std::make_pair("eurasafrica", coords(veurasafrica[0], veurasafrica[1])),
    std::make_pair("antarctica",  coords(vantarctica[0],  vantarctica[1])),
};


typedef std::vector<std::string> stringvec;
typedef std::map<std::string, double*> namedboxes;

class RegionRemover {
public:
    static RegionRemover *createInstance(const std::string sQDF, stringvec &vsRegions);
    static void getRegionNames(stringvec &vs);
    static bool isPointInPoly(double dX, double dY, coords &c);

    virtual ~RegionRemover();

    int removeRegions();
    int removeRegionsInvert();

    void displayPolys();
    void displayBoxes();
protected:
    RegionRemover();
    int init(const std::string sQDF, stringvec &vsRegions);
    int checkQDF(const std::string sQDF);

    int createPolys(stringvec &vsRegions);
    hid_t m_hFile;
    hid_t m_hGeoGroup;
    int m_iNumCells;

    double *m_adLon;
    double *m_adLat;
    double *m_adAlt;

    namedcoords m_vPoly;
    namedboxes  m_vBoxes;
};


#endif
