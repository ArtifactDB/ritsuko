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
 * This provides an RAII interface for HDF5's variable length strings.
 * The idea is to create an instance of this class immediately after the `H5::DataSet::read()` call.
 * The allocated memory for each string is then reclaimed once the instance goes out of scope.
 */
class ReclaimVlsMemory  {
public:
    /**
     * @param tid ID for the HDF5 datatype for the in-memory strings.
     * The lifetime of this datatype should exceed that of this `ReclaimVlsMemory` instance.
     * @param sid ID for the HDF5 dataspace for the in-memory strings.
     * The lifetime of this dataspace should exceed that of this `ReclaimVlsMemory` instance.
     * @param pid ID for the memory transfer property list, typically `H5P_DEFAULT`. 
     * The lifetime of this property list should exceed that of this `ReclaimVlsMemory` instance.
     * @param buffer Array of C-style strings allocated by `H5::DataSet::read()` with the specified datatype, dataspace and property list.
     */ 
    ReclaimVlsMemory(hid_t tid, hid_t sid, hid_t pid, char** buffer) : my_tid(tid), my_sid(sid), my_pid(pid), my_buffer(buffer) {}

    /**
     * @cond
     */
    ~ReclaimVlsMemory() {
        H5Dvlen_reclaim(my_tid, my_sid, my_pid, my_buffer);
    }
    /**
     * @endcond
     */

private:
    hid_t my_tid, my_sid, my_pid;
    char** my_buffer; 
};

}

}

#endif
