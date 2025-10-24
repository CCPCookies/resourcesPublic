Measure-Command {
.\resources.exe unpack-bundle Bundle/BundleResourceGroup.yaml --chunk-source-base-path Bundle/Chunks --chunk-source-type LOCAL_CDN --verbosity-level 3 | Out-Default
.\resources.exe apply-patch UnpackBundleOut/ResourceGroup.yaml --patch-binaries-base-path UnpackBundleOut/ --resources-to-patch-base-path BuildPrevious --next-resources-base-path BuildNext --verbosity-level 3 | Out-Default
}