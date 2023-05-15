.. _pie_plot_head:

Pie Plot
~~~~~~~~

This plot, shown in :numref:`Figure %s <pieplot_pureplus>` (left), displays a number of glyphs based on a vector variable.
See :ref:`Vector Variable Format <format_def>` for a detailed description of the requirements.
Examples of possible use cases for this plot are the visualisation of any location dependent distribution (pies and boxes), or the visualisation of ordered datasets like time series, or age distributions (bars and boxes). 
The number of displayed variables is the same in every glyph of a pie plot.

Usually the **Pie** plot is combined with other plots in order to display multiple values associated with particular positions see :numref:`Figure %s <pieplot_pureplus>` (right).

.. _pieplot_pureplus:

.. figure:: ../images/pieplot_pureplus.png

   Example of a pie plot.  Left: Pie plot only.  Right: Pie plot combined with a pseudocolor plot (Haplotype distribution at various positions) 

Currently, there are three glyph types available to represent the data: pie plots, bar graphs, and boxes  (:numref:`Figure %s <pieplot_barbox>` ).

.. _pieplot_barbox:

.. figure:: ../images/pieplot_barbox.png
    
   Example for a bar plot (left: Maximum Temperatures) and box plot (right: Bimonthly rainfall averages as percentage of total annual precipitation)


.. _pieplot_attribute_dialogs:

.. figure:: ../images/pieplot_attribute_dialogs.png
    
   Attribute dialog for the different glyph types.


Setting the glyph style
"""""""""""""""""""""""

The **Glyph Style** radio buttons allow the choice of the glyphs to be used (see :numref:`Figure %s <pieplot_glyphs>`).

- **STYLE_PIE**: the data is represented by pie diagrams

- **STYLE_BAR**: the data is reprexented by bar diagrams

- **STYLE_BOX**: the data is represented by stripes 

.. _pieplot_glyphs:

.. figure:: ../images/pieplot_allglyphs.png
    
   The three glyph types.

Scaling
"""""""

The pie glyphs have a radius of 1.0 by default, the bar and box glyphs have height and width equal to 1.0.
For pies a single scale may be set in the text field **Pie Radius**, for the other glyphs both an x-scaling factor and a y-scaling factor can be specified (text fields **Scale X** and **Scale Y**, respectively).
For all glyphs the width of the border can be set in text field **Border Width**.
The scalings discussed here are applied to all glyphs of that particular plot.

Setting Glyph Colors
""""""""""""""""""""
By default the colors representing the values are taken from a fixed color table.
To change the color for a particular value, click on that value's **Color button** and select a new color from the Popup color menu. 
To change the opacity for a value, move its opacity slider to the left to make the level more transparent or move the slider to the right to make the level more opaque.

.. _format_def:

Vector Variable Format
""""""""""""""""""""""

This section describes the requirements for a database plugin to create the data structures needed by **Pie** plots.

For a **Pie** plot displaying N\ :sub:`glyphs` glyphs with M\ :sub:`values` values per glyph the vector variable must be structured in a particular way.
The ``vtkDataArray`` returned by the method ``avtXXXFileFormat::GetVectorVar()``  of the database should be a ``vtkDoubleArray`` consisting of N\ :sub:`glyphs` tuples with 6 + M\ :sub:`values` components. 
The descriptions and names of the components of a tuple corresponding to a glyph are shown in the following table:
 
 +-----------------------------+--------------------------+-----------------------------------+
 | **component index**         |  **component name**      |  **description**                  |
 +-----------------------------+--------------------------+-----------------------------------+
 |     0                       |   "pos_x"                |  x-coordinate of glyph            |
 +-----------------------------+--------------------------+-----------------------------------+
 |     1                       |   "pos_y"                |  y-coordinate of glyph            |
 +-----------------------------+--------------------------+-----------------------------------+
 |     2                       |   "pos_z"                |  z-coordinate of glyph            |
 +-----------------------------+--------------------------+-----------------------------------+
 |     3                       |   "norm_x"               |  x-component of normal of glyph   |
 +-----------------------------+--------------------------+-----------------------------------+
 |     4                       |   "norm_y"               |  y-component of normal of glyph   |
 +-----------------------------+--------------------------+-----------------------------------+
 |     5                       |   "norm_z"               |  z-component of normal of glyph   |
 +-----------------------------+--------------------------+-----------------------------------+
 |     6                       |  <name of first value>   |  first value for glyph            |
 +-----------------------------+--------------------------+-----------------------------------+
 |     ...                     |      ...                 |          ...                      |
 +-----------------------------+--------------------------+-----------------------------------+
 |    6 + M\ :sub:`values` - 1 |  <name of last value>    |  last value for glyph             |
 +-----------------------------+--------------------------+-----------------------------------+
  

Each glyph will be rotated in such a way that its normal will be parallel to the normal vector provided in the data. 
Bar and box glyphs are additionally rotated such that their local y-coordinate best matches the global z-direction (0,0,1).

The component names are used for the labelling of the **Pie** plot's legend.

Currently, there must also be a uniquely named instance of  ``avtLabelMetaData`` in the database meta data whose ``originalName`` should be set to a string consisting of the value names for the plot, joined by a semicolon (';').

This should be done in ``avtXXXFileFormat::PopulateDatabaseMetaData()`` and look like this: ::

  // create a avtLabelMetaData instance
  avtLabelMetaData *lmd = new avtLabelMetaData;
  // the label name must be unique
  lmd->name = std::string{"pieInfo_"} +  m_sVarName;
  // sComponentNames holds the ';'-separated value names, e.g. "item1;item2;item3"
  lmd->originalName = sComponentNames;
  // Keep this info from being used elsewhere by VisIt
  lmd->validVariable = false;
  // do not show it in the GUI
  lmd->hideFromGUI = true;
  // add it to the meta data
  md->Add(lmd);

The string set to the ``originalName`` of the ``avtLabelMetaData`` is used to initialize the **Pie** attributes with the value names and number of colors to use when the attribute dialog is created.
If the value names are not provided by the data file, the data base plugin should create the appropriate number of 'fake' names (e,g, "item1;item2;item3").

