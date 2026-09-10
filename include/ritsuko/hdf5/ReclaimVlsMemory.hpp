#ifndef RITSUKO_HDF5_RECLAIM_VLS_MEMORY_HPP
#define RITSUKO_HDF5_RECLAIM_VLS_MEMORY_HPP

#include "H5Cpp.h"

/**
 * @file ReclaimVlsMemory.hpp
 * @brief Reclaim memory allocated to HDF5's variable length strings.
 */

namespace ritsuko {

namespace hdf5 {

/**
 * @brief Reclaim memory for HDF5's variable length strings.
 *
 * @tparam DataTypePointer_ Class of a pointer to a `H5::DataType`.
 * This can be raw or smart depending on the caller's management of its lifetime.
 * @tparam DataSpacePointer_ Class of a pointer to a `H5::DataSpace`.
 * This can be raw or smart depending on the caller's management of its lifetime.
 * @tparam DSetMemXferPropListPointer_ Class of a pointer to a `H5::DSetMemXferPropList`.
 * This can be raw or smart depending on the caller's management of its lifetime.
 *
 * This provides an RAII interface for HDF5's variable length strings.
 * The idea is to create an instance of this class immediately after the `H5::DataSet::read()` call.
 * The allocated memory for each string is then reclaimed once the instance goes out of scope.
 */
template<
    class DataTypePointer_ = H5::DataType*,
    class DataSpacePointer_ = H5::DataSpace*,
    class DSetMemXferPropListPointer_ = H5::DSetMemXferPropList*
>
class ReclaimVlsMemory  {
public:
    /**
     * @param type_ptr Pointer to the HDF5 datatype used to read the strings.
     * If `type_ptr` is a raw pointer, it should not be deleted before this `ReclaimVlsMemory` instance is destroyed.
     * @param space_ptr Pointer to the HDF5 dataspace used to read the strings.
     * If `space_ptr` is a raw pointer, it should not be deleted before this `ReclaimVlsMemory` instance is destroyed.
     * @param plist_ptr Pointer to the memory transfer property list used to read the strings, typically a reference to `H5::DSetMemXferPropList::H5P_DEFAULT`.
     * If `plist_ptr` is a raw pointer, it should not be deleted before this `ReclaimVlsMemory` instance is destroyed.
     * @param buffer Array of C-style strings allocated by `H5::DataSet::read()` with the specified datatype, dataspace and property list.
     */ 
    ReclaimVlsMemory(
        DataTypePointer_ type_ptr,
        DataSpacePointer_ space_ptr,
        DSetMemXferPropListPointer_ plist_ptr,
        char** buffer
    ) : 
        my_type_ptr(std::move(type_ptr)),
        my_space_ptr(std::move(space_ptr)),
        my_plist_ptr(std::move(plist_ptr)),
        my_buffer(buffer)
    {}

    /**
     * @cond
     */
    ~ReclaimVlsMemory() {
        H5Dvlen_reclaim(my_type_ptr->getId(), my_space_ptr->getId(), my_plist_ptr->getId(), my_buffer);
    }
    /**
     * @endcond
     */

private:
    DataTypePointer_ my_type_ptr;
    DataSpacePointer_ my_space_ptr;
    DSetMemXferPropListPointer_ my_plist_ptr;
    char** my_buffer; 
};

}

}

#endif
