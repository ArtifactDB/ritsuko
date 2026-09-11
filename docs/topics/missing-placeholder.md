# Missing value placeholder

## Overview

To support missing values, writers should define a placeholder attribute on the dataset.
(The exact name of this placeholder depends on the specification for each object, but is generally something like `missing-value-placeholder` or `missing_placeholder`.)
This attribute should be a scalar that holds a "missing value placeholder",
where all values in the dataset that are equal to this placeholder should be considered as missing.
If no placeholder is present, readers may assume that no values are missing.

Our placeholder approach is a variation of the sentinel method used by R for its 32-bit integers.
Here, the lowest value (-2147483648) is considered to be missing, which works well but has a few pitfalls.
Most obviously, it excludes -2147483648 as a valid integer.
It also prohibits the use of a smaller integer datatype for efficient storage of a dataset with missing values in a HDF5 file.
Both problems can be avoided by allowing writers to customize the placeholder according to the contents of the dataset.

## For integers

This attribute should be a scalar of the same exact datatype as the dataset.
All values in the dataset comparing equal to the placeholder value should be treated as missing.

## For floating-point 

This attribute should be a scalar of the same exact datatype as the dataset.
All values in the dataset comparing equal to the placeholder value should be treated as missing.
Note that the equality comparison has some implications:

- If the dataset is stored in double precision, readers should represent the data in memory with at least as much precision during the placeholder comparison.
  Otherwise, a cast to lower precision may cause non-missing values to be (incorrectly) equal to the placeholder.
- If the placeholder is an NaN value, all NaNs in the dataset should be treated as missing, regardless of the specific type of NaN.
  We do not consider the NaN payload during placeholder comparisons as this may not be handled reliably across machines.

## For strings

This attribute should be a string scalar in either UTF-8 or ASCII encoding.
All values in the dataset with the same sequence of bytes (as defined by, e.g., `strcmp`) as the placeholder value should be treated as missing.

## Further comments

Previous implementations of this concept were more relaxed and only required integer and floating-point placeholders to be of the same datatype class.
We moved a stricter interpretation to avoid problems with casting between types (and potential loss of precision) when comparing the data to the placeholder.

In R, a NaN with a special payload is used to mark missing floating-point values.
This is ingenious but has potential problems with stability and portability, given that preservation of NaN payloads is not mandated by the IEEE754 specification.
In such cases, a writer-defined placeholder can more reliably identify missing values.

For completeness, we note that NumPy uses masking to represent missing values.
While this is the most explicit approach, we chose not to use it as the burden on the reader is too high.
Readers must scan both the actual and masking datasets for full interpretation, which complicates the implementation and increases memory usage and disk I/O.
Writers also need to carefully coordinate the chunk dimensions in both datasets to enable efficient simultaneous iteration.

