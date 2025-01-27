// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//  File: PiePluginInfo.h
// ************************************************************************* //

#ifndef PIE_PLUGIN_INFO_H
#define PIE_PLUGIN_INFO_H
#include <PlotPluginInfo.h>
#include <plot_plugin_exports.h>

class PieAttributes;

// ****************************************************************************
//  Class: PiePluginInfo
//
//  Purpose:
//    Five classes that provide all the information about a Pie
//    plot plugin.  The information is broken up into five classes since
//    portions of it are only relevant to particular components within
//    visit.  There is the general information which all the components
//    are interested in, the gui information which the gui is interested in,
//    the viewer information which the viewer is interested in, the
//    engine information which the engine is interested in, and finally a.
//    scripting portion that enables the Python VisIt extension to use the
//    plugin.
//
//  Programmer: generated by xml2info
//  Creation:   omitted
//
// ****************************************************************************

class PieGeneralPluginInfo: public virtual GeneralPlotPluginInfo
{
  public:
    virtual const char *GetName() const;
    virtual const char *GetVersion() const;
    virtual const char *GetID() const;
    virtual bool  EnabledByDefault() const;
};

class PieCommonPluginInfo : public virtual CommonPlotPluginInfo, public virtual PieGeneralPluginInfo
{
  public:
    virtual AttributeSubject *AllocAttributes();
    virtual void CopyAttributes(AttributeSubject *to, AttributeSubject *from);
    virtual int GetVariableTypes() const;
};

class PieGUIPluginInfo : public virtual GUIPlotPluginInfo, public virtual PieCommonPluginInfo
{
  public:
    virtual QString *GetMenuName() const;
    virtual QvisPostableWindowObserver *CreatePluginWindow(int type,
        AttributeSubject *attr, const QString &caption, const QString &shortName,
        QvisNotepadArea *notepad);
    virtual const char **XPMIconData() const;
};

class PieViewerEnginePluginInfo : public virtual ViewerEnginePlotPluginInfo, public virtual PieCommonPluginInfo
{
  public:
    virtual AttributeSubject *GetClientAtts();
    virtual AttributeSubject *GetDefaultAtts();
    virtual void SetClientAtts(AttributeSubject *atts);
    virtual void GetClientAtts(AttributeSubject *atts);

    virtual avtPlot *AllocAvtPlot();

    virtual void InitializePlotAtts(AttributeSubject *atts, const avtPlotMetaData &);
    virtual void ReInitializePlotAtts(AttributeSubject *atts, const avtPlotMetaData &);
    virtual void ResetPlotAtts(AttributeSubject *atts, const avtPlotMetaData &);
    virtual const char *GetMenuName() const;
    static void InitializeGlobalObjects();
  private:
    static PieAttributes *defaultAtts;
    static PieAttributes *clientAtts;
    // User-defined functions
  private:
    void PrivateSetPlotAtts(AttributeSubject *atts, const avtPlotMetaData &plot);
};

class PieViewerPluginInfo : public virtual ViewerPlotPluginInfo, public virtual PieViewerEnginePluginInfo
{
  public:
    virtual const char **XPMIconData() const;

};

class PieEnginePluginInfo : public virtual EnginePlotPluginInfo, public virtual PieViewerEnginePluginInfo
{
  public:
};

class PieScriptingPluginInfo : public virtual ScriptingPlotPluginInfo, public virtual PieCommonPluginInfo
{
  public:
    virtual void InitializePlugin(AttributeSubject *subj, void *data);
    virtual void *GetMethodTable(int *nMethods);
    virtual bool TypesMatch(void *pyobject);
    virtual char *GetLogString();
    virtual void SetDefaults(const AttributeSubject *atts);
};

#endif
