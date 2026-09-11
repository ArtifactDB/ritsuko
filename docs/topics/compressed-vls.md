# Compressed VLS

## Overview

One weakness of HDF5 is its inability to efficiently handle variable length string (VLS) arrays.
Storing them as fixed-length strings requires padding all strings to the longest string,
causing an inflation in disk usage that cannot be completely negated by compression.
On the other hand, HDF5's own VLS datatype does not compress the strings themselves, only the pointers to those strings.

To patch over this weakness in current versions of HDF5, we introduce our own concept of a "compressed VLS array".
This is defined by two HDF5 datasets - one storing a compressed heap of VLSs, and another storing pointers into that heap.

- The VLS heap is a 1-dimensional dataset containing the concatenation of bytes from all variable length strings in the VLS array.
  This is typically compressed to save disk space, hence "compressed VLS".
- The pointer dataset is a scalar or N-dimensional dataset of a compound datatype.
  Each entry is a pointer containing the starting offset and length of a single VLS on the heap.

The idea is to read the pointer dataset into memory and then use the offset and length of each pointer to extract a slice of characters from the heap.

## Pointers

The pointer dataset should have a compound datatype with exactly two members:

- The first member is named `offset` and is of an unsigned integer datatype. 
- The second member is named `length` and is of an unsigned integer datatype. 

Object specifications should define the maximum sizes of the datatypes.
These need not be the same between `offset` and `length`.

In each entry of a pointer dataset, `offset` and `offset + length` should be no greater than the extent of the sole dimension of the corresponding heap dataset.
The `length` for each entry does not need to include the null terminator.
However, if the slice `[offset, offset + length)` on the heap includes a null terminator, the extracted VLS should be terminated at the first occurrence.
This allows the slice to be easily reused for shorter strings when modifying a VLS inside an existing heap.

The pointer dataset is typically chunked and compressed, though this is left to the implementation.

There are no restrictions on the ordering of entries in the pointer dataset.
Consecutive entries do not have to define ordered or contiguous slices in the heap.
This allows one or more entries in the dataset to be modified without invalidating other entries.
Different entries can even refer to the same or even overlapping slices, which provides some opportunities to compress repeated strings.
That said, we'd recommend that entries in the same chunk of the pointer dataset refer to neighboring parts of the heap, to allow readers to efficiently extract the VLSs.

## Heap

This should be a 1-dimensional dataset of unsigned 8-bit integers, representing the concatenation of bytes from all the variable length strings in the VLS array.
Ideally, all bytes are referenced by at least one entry in the associated pointer dataset, though this is not required, e.g., if a VLS is replaced with a shorter string.

We use an integer datatype rather than HDF5's own string datatypes to avoid the risk of a naive incorrect interpretation of the heap as an array of fixed-width strings.
We note that HDF5 also has a `NATIVE_CHAR` type, but this should be avoided as the signedness of `char` may not be consistent across platforms.

The heap dataset is typically chunked and compressed, otherwise there would be no advantage over HDF5's own VLS type.

Both ASCII and UTF-8 define a character in terms of its binary representation, e.g., `01000001` for `A`.
Thus, implementations should ensure that the binary representation of each character is preserved when reading or writing to the heap. 
This requires some care to avoid inadvertent casts to/from a `char` of platform-specific signedness to the heap's unsigned integer type.
We recommend following these procedures:

- For reading, we first read the bytes into an `uint8_t` array via `H5::DataSet::read()` with `NATIVE_UINT8`.
  We then access those bytes via an aliased `char*` to extract each VLS.
- For writing, we create an aliased `char*` pointer to a `uint8_t` array and copy each VLS into this array via the aliased pointer.
  We then use `H5::DataSet::write()` with `NATIVE_UINT8` to write the `uint8_t` array. 

This approach ensures that each byte is faithfully transferred between the HDF5 file and memory as a `uint8_t` with a well-defined and portable binary representation.
The aliasing pointer then interprets the byte as a `char` with the same sequence of bits, regardless of the platform's `char` signedness.

## Further comments

Typically, the pointer and heap datasets for a compressed VLS array will be stored in a separate group.
