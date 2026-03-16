ResourceGroup
=============

The ResourceGroup is the central component of the Resources system.

ResourceGroups represent a collection of Resources.

They can be saved/loaded from a filetype which supersedes resfileindex files.


.. note::

    See :doc:`../../DesignDocuments/resourceGroupFileFormat` file specification for details on Resource Groups.

    See :doc:`../../DesignDocuments/fileFiltering` for further information on filtering.

.. doxygenclass:: CarbonResources::ResourceGroup
    :members:


Input Parameters
----------------

.. doxygenvariable:: DEFAULT_FILE_STREAM_SIZE

.. doxygenvariable:: DEFAULT_STREAM_THRESHOLD_SIZE

.. doxygenstruct:: CarbonResources::ResourceGroupImportFromFileParams
    :members:

.. doxygenstruct:: CarbonResources::ResourceGroupExportToFileParams
    :members:

.. doxygenstruct:: CarbonResources::CreateResourceGroupFromDirectoryParams
    :members:

.. doxygenstruct:: CarbonResources::CreateResourceGroupFromFilterParams
    :members:

.. doxygenstruct:: CarbonResources::PatchCreateParams
    :members:

.. doxygenstruct:: CarbonResources::BundleCreateParams
    :members:

.. doxygenstruct:: CarbonResources::ResourceGroupMergeParams
    :members:

.. doxygenstruct:: CarbonResources::ResourceGroupDiffAgainstGroupParams
    :members:

.. doxygenstruct:: CarbonResources::ResourceGroupRemoveResourcesParams
    :members:

.. doxygenstruct:: CarbonResources::CompressionCalculationSettings
    :members:

.. doxygenstruct:: CarbonResources::ExportResourceSettings
    :members:

.. doxygenstruct:: CarbonResources::FilterSettings
    :members: