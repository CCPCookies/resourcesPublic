File Filtering
##############

Resource file filter rules can be passed while creating resources groups. 

Filter files can be passed during Resource Group creation via

:doc:`CarbonResources::ResourceGroup::CreateFromFilter <../../Tools/Api/ResourceGroup>`

or through the CLI via the ``--create-group-from-filter`` operation.

.. contents:: Table of Contents
    :depth: 3

.. _file-format:

File Format
***********

Filtering is achieved by providing a filter file that follows the following structure:

.. code-block:: ini

    [DEFAULT]                                           # [REQUIRED] Default section for settings
        prefixmap = prefix1:Path1                       # [REQUIRED] Base search paths
        filter = [ include ] ![ exclude ]               # [OPTIONAL] Global include and exclude patterns

    [exampleSection]                                    # [OPTIONAL] Filter section
        filter = [ include ] ![ exclude ]               # [OPTIONAL] Section local include and exclude patterns
        respaths = prefix1:/*                           # [OPTIONAL] Path pattern
        respaths = prefix1:/* [ include ] ![ exclude ]  # [OPTIONAL] Path pattern with extra include and exclude patterns

Prefixmap
=========

The prefix map must be defined as part of the ``[DEFAULT]`` section.

It contains a list of search paths linked to named tags.

A prefix can be given any name and can contain many search paths.

.. code-block:: ini

    prefixmap = prefix1:Path1,Path2 prefix2:.

- Here two prefixes are created, ``prefix1`` and ``prefix2``.
- ``prefix1`` has 2 search paths ``Path1`` and ``Path2``.
- ``prefix2`` has 1 search path ``.``

.. _include-exclude-pattern:

Global include and exclude patterns
===================================

Optionally global include and exclude patterns can be specified in the ``[DEFAULT]`` section.

Both include and exclude rules are not always required, just specifying include rules or exclude rules alone is valid.

Many rules can be used, separated by commas.

- Include rules are encased by square brackets.
- Exclude rules are encased by square brakets with a preceeding ``!``

.. code-block:: ini
    
    filter = [ include1, include2 ] ![ exclude ]

- Here both include and exclude filters are specified.
- Include patterns are ``include1`` and ``include2``.
- A single exclude pattern ``exclude`` is set.

Filter Sections
===============

Many filter sections can be specified. At least one is required for any filtering to take effect.

.. code-block:: ini

    [section1]                              # [OPTIONAL] Filter section
        filter = [ include ] ![ exclude ]   # [OPTIONAL] Section local include and exclude patterns
        respaths = prefix1:/*               # [OPTIONAL] Path pattern

    [section2]                              # [OPTIONAL] Filter section
        respaths = prefix2:/*               # [OPTIONAL] Path pattern

- Two sections are specified ``section1`` and ``section2``

Section include and exclude patterns
====================================

Each section can specify local include and exclude patterns.

These follow the same syntax as the global patterns. See :ref:`include-exclude-pattern`

Section respaths
================

Respaths are the part that actually specifies the paths to perform filtering on.

There are 3 pattern variations that are supported which can be seen in the following:

.. code-block:: ini

    [section1]                              # Filter section
        respaths = prefix1:/Path/*          # 1. Path with asterisk path wildcard
        respaths = prefix2:/Path/...        # 2. Path with elipsis recursive path wildcard
        respaths = prefix3:/Path/File.txt   # 3. Exact path specification

Respaths can also include inlined include and exclude patterns:

.. code-block:: ini

    [section1]                           
        respaths = prefix1:/Path/* [ include ] ![ exclude ]    

See filtering logic to understand expected behaviour from this.

See :ref:`include-exclude-pattern` for details on include and exclude patterns

.. _filtering-logic:

Filtering Logic
***************

It is important to understand the logic behind resource filtering to be sure your resources are included as you expect.


The logic behind the filtering was inherited from a legacy system, therefore some of the logic may be suprising.

Path construction
=================

The filtering system operates on a single based directory, the prefix base map.

This directory acts as the base for all the search directories that are specified inside the filter files.

.. code-block:: ini

    prefixmap = prefix1:A/Path,B/Path prefix2:../Another/Path

With this above example specifies 2 prefixes ``prefix1`` and ``prefix2`` and between them 3 search paths are specified

1. ``A/Path``
2. ``B/Path``
3. ``../Another/Path``

The full path is resolved by adding these paths to the base prefix map passed in as part of the operation.

.. code-block:: ini

    [section1]
        respaths = prefix1:/*

Each sections' respaths use the prefixes and adds the remaining path.

The prefix name, ``prefix1`` in this case needs to be present in the prefix map.

Each prefix can have multiple paths, therefore following the example the following path patterns will be checked

1. ``[Prefix base path]/A/Path/*``
2. ``[Prefix base path]/B/Path/*``

When a resource is found that matches one of the patterns it will be added to the Resource Group.

The resources' relative path will be based on the prefix base path. e.g:

``[Prefix base path]/A/Path/Found.txt`` produces a resource with relative path ``Found.txt``

Order matters! If duplicate relative paths are encountered, the first resource will be included in the Resource Group.

Include & Exclude patterns
==========================

Patterns can be provided at three different levels.

1. Globally in the ``[DEFAULT]`` section using ``filter =`` .
2. Section locally in sections using ``filter =`` .
3. Semi locally to respath adding include/exclude rules to each path ``respaths = prefix1:/* [ include ] ![ exclude ]``

If no include pattern matches are specified then all files will be included. Only if an include is provided will files be excluded if they don't match the include patterns.

The pattern matching is performed against the whole path of a file that is being tested from the patterns.

.. code-block:: ini
    
    filter = [ include ]

This example ensures each path contains the pattern ``include``. Paths that don't will not be included in the Resource Group.

.. warning::

    The include exclude patterns apply across the entire absolute path to the resource that is being checked.

    This could lead to some unexpected behaviour for example if a pattern of ``c`` was provided it would match every single file tested if source files lived on the ``c:/`` drive. It is understandable that the user may have wanted to include only files that contained the letter ``c`` but this is not going to be the result.

    This behaviour was inherited from a previous system, it may change in the future.

Excludes work oposite to includes. Any path that matches the pattern will be excluded from Resource Groups.

.. code-block:: ini
    
    filter = ![ exclude ]

Here any path which contains ``exclude`` will not be added to the Resource Group.

Excludes take precedence over includes, if a path matches an exclude pattern and an include pattern, it will be excluded.

.. code-block:: ini
    
    filter = [ match ] ![ match ]

Paths which match the pattern ``match`` would hit include and exclude rules, exclude takes preceedence so it will be excluded.

How include/exclude pattern are combined
----------------------------------------

There are 3 filter patterns and understanding how these combine to include or exclude a path is important.

1. Global filters affect all path matching. Anything added there will be applied to all paths that are tested for inclusion.

2. Section local filters are combined with any filters specified in global filters.

3. respaths filters combine with both global and section filters and importantly these add for all subsequent paths. This is explained more in the following examples.

This is easier to see by example:

Assuming there are 3 files in the prefix base path

1. ``include1.txt``
2. ``include2.txt``
3. ``include3.txt``
4. ``Path/include3.txt``

Creating a Resource Group with the following filter file:

.. code-block:: ini

    [DEFAULT]
        prefixmap = prefix1:. prefix2:Path/
        filter = [ include1 ]                   # 1. Global include

    [exampleSection]
        filter = [ include2 ]                   # 2. Section local include
        respaths = prefix1:/*                   # 3. respath1
        respaths = prefix1:/* [ include3 ]      # 4. respath2 with respath include
        respaths = prefix2:/*                   # 5. respath3 referencing another prefix

Two paths will be tested for inclusion: 

``#3`` will use ``respaths = prefix1:/*`` and combine global and section local patterns ``include1`` and ``include2``. This will match the following from the source files:
1. ``include1.txt``
2. ``include2.txt``

``#4`` will use ``respaths = prefix1:/* [ include3 ]`` which will extend the section local patterns to include ``include3``. This will match the following source files:
1. ``include1.txt``
2. ``include2.txt``
3. ``include3.txt``

``#5`` will use ``respaths = prefix2:/*`` and doesn't sepecify any include rules. It will apply the include rules that have been constructed for the section at this point ``include1``, ``include2`` and ``include3``. This may be suprising. So this will match the following source files:
1. ``Path/include3.txt``

So in this example all files were matched.

Order is important. Swapping the last two paths will give result in ``Path/include3.txt`` not being included.

.. warning::

    When include and exclude patterns are specified as part of a ``respath`` they are added to the whole set of include and exclude patterns for the section and apply to subsequent entries. Order is important.