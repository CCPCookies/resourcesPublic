# Performance Tests
These are some simple tests that can be used to get some quick performance results.

Note: These tests are added for convenience, a formal performance test pipeline is not yet implemented.

## Usage
1. The scripts require the CLI resources.exe located in the same folder

2. Create two directories containing previous and next builds to create patches from named 'BuildPrevious' and 'BuildNext' respectively.

3. Run 'CreateBundledPatch.ps1' to measure performance for creation of bundled patches for the provided test data.

4. Run 'ApplyBundledPatch.ps1' to measure performance of patch application.