# Datatype constraints

## Overview

An object specification involving HDF5 files needs to define the allowed HDF5 datatypes.
The general philosophy is to declare the largest datatype that can be used for a particular dataset/attribute.
Implementations can then safely use the declared type to read the dataset/attribute without any overflow or other loss of precision.
However, writers are still free to choose a smaller type to reduce the size of the dataset/attribute on disk.

## Integers 

If a HDF5 dataset/attribute contains integers, the specification should declare the largest integer datatype that can be used.
This includes both the bit width and the signedness of the integer type.
We assume a two's complement representation for signed integers.

For example, most objects containing integers will specify that the corresponding HDF5 dataset/attribute's datatype is no larger than a 32-bit signed integer. 
Readers can thus safely assume that an `int32_t` is sufficient to represent all possible values.
Writers may create HDF5 datasets with any integer datatype that can be fully represented by an `int32_t`, e.g., `H5T_NATIVE_INT8`, `H5T_NATIVE_UINT16`.

## Floats

If a HDF5 dataset/attribute contains floating-point numbers, the specification should declare the largest float datatype that can be used.
We assume that all representations are IEEE754-compliant, to ensure the faithful propagation of special values like infinity and NaN.

For example, most objects containing floats will specify that the corresponding HDF5 dataset/attribute's datatype is no larger than a 64-bit IEEE754-compliant float.
Readers can thus safely assume that an `double` is sufficient to represent all values in memory.
Writers may create HDF5 datasets with any floating-point or integer datatype that can be exactly represented by a `double`, e.g., `H5T_NATIVE_FLOAT`, `H5T_NATIVE_UINT32`.

## Strings

Representations of strings should specify the encoding(s) that can be used in the HDF5 dataset.
Most objects containing strings will specify that the HDF5 dataset should contain values that can be exactly represented by UTF-8 encoded strings.
This supports both ASCII and UTF-8 encodings as the former is a subset of the latter anyway.

From a specification perspective, there is no difference between variable and fixed length string datatypes.
For the fixed-length string datatype, the length of each string is defined as the number of bytes to the first null character, or as the fixed length if no null character is present.
For the variable-length string datatype, each entry of the dataset/attribute should refer to a null-terminated string, i.e., readers should never encounter a `NULL` pointer.

Similarly, specifications do not differentiate between null termination and null padding in HDF5 string datatypes.
Both are considered to yield appropriate C-style strings when the length is unknown.
By comparison, Fortran-style blank-padded strings should not be used as it is unclear whether the trailing blanks are meant to be discarded.
