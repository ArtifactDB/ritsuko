# HDF5 internal organization

## Attributes

HDF5-based specifications of objects should aim to be self-contained, i.e., all parameters required for interpretation of the file contents should be stored in the same file.
This is usually achieved by attaching relevant attributes on the groups or datasets, depending on the object being described.
The aim is to minimize the amount of information that needs to be drawn from external files or metadata.

Our recommendation here is based on experience with older versions of the ArtifactDB formats where the data was stored in HDF5 but described in a separate JSON file.
This design was more complex to read, write and understand, as developers needed to constantly hop back and forth between the JSON and HDF5 files.

## Groups

When defining an object specification, developers are encouraged to use HDF5 groups to organize the file contents.
The overhead introduced by each new HDF5 group is [reasonably low](https://forum.hdfgroup.org/t/group-overhead-file-size-problem-for-hierarchical-data/9640/2)
so there is no performance reason to avoid using them.
Each group also provides a location to attach attributes to describe the group contents.
