/******************************************************************************
 * @file     OpenCLInfo.cpp
 * @author   Vadim Demchik <vadimdi@yahoo.com>
 * @version  2.0
 *
 * @brief    [OpenCLInfo]
 *           Returns detailed OpenCL information about all available
 *           compute devices
 *
 *
 * @section  LICENSE
 *
 * Copyright (c) 2015 Vadim Demchik
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 *    Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 *****************************************************************************/

#include <stdio.h>
#include <iostream>

#ifndef _WIN32
#include <cstring>
#endif

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#include <CL/cl_ext.h>
#endif


#define HGPU_OPENCL_VERSION_STR     "OpenCL C "
#define HGPU_OPENCL_VERSION_1_0     HGPU_OPENCL_VERSION_STR"1.0"
#define HGPU_OPENCL_VERSION_1_1     HGPU_OPENCL_VERSION_STR"1.1"
#define HGPU_OPENCL_VERSION_1_2     HGPU_OPENCL_VERSION_STR"1.2"
#define HGPU_OPENCL_VERSION_2_0     HGPU_OPENCL_VERSION_STR"2.0"

#define HGPU_OPENCL_1_0             1
#define HGPU_OPENCL_1_1             2
#define HGPU_OPENCL_1_2             3
#define HGPU_OPENCL_2_0             4

#define HGPU_DEVICE_VENDOR_AMD      "Advanced Micro Devices"
#define HGPU_DEVICE_VENDOR_NVIDIA   "NVIDIA"
#define HGPU_DEVICE_VENDOR_INTEL    "Intel"

#define HGPU_VENDOR_AMD             1
#define HGPU_VENDOR_NVIDIA          2
#define HGPU_VENDOR_INTEL           3

#define HGPU_OUT_FMT_NSTR           "%-46s: "
#define HGPU_OUT_FMT_END            "\n"
#define HGPU_OUT_FMT_N0STR          "%s:\n"
#define HGPU_OUT_FMT_TN             "\t%s\n"

#define HGPU_OUT_FMT_STR0           HGPU_OUT_FMT_NSTR"%s"
#define HGPU_OUT_FMT_HEX0           HGPU_OUT_FMT_NSTR"%#x"
#define HGPU_OUT_FMT_LONGHEX0       HGPU_OUT_FMT_NSTR"%#llx"
#define HGPU_OUT_FMT_UINT0          HGPU_OUT_FMT_NSTR"%u"
#define HGPU_OUT_FMT_LONG0          HGPU_OUT_FMT_NSTR"%llu"

#define HGPU_OUT_FMT_NSTRN          HGPU_OUT_FMT_NSTR"\n"
#define HGPU_OUT_FMT_STR            HGPU_OUT_FMT_STR0"\n"
#define HGPU_OUT_FMT_HEX            HGPU_OUT_FMT_HEX0"\n"
#define HGPU_OUT_FMT_LONGHEX        HGPU_OUT_FMT_LONGHEX0"\n"
#define HGPU_OUT_FMT_UINT           HGPU_OUT_FMT_UINT0"\n"
#define HGPU_OUT_FMT_LONG           HGPU_OUT_FMT_LONG0"\n"

#define HGPU_OUT_FMT_NONE           0
#define HGPU_OUT_FMT_KB             1
#define HGPU_OUT_FMT_MB             2
#define HGPU_OUT_FMT_GB             3
#define HGPU_OUT_FMT_MHz            4


    void
    HGPU_GPU_error_message( int error_code, const char* error_message )
    {
        if ( error_code != CL_SUCCESS )
        {
            printf( "ERROR %i: (%s)\n", error_code, error_message );
            exit( 1 );
        }
    }

    void
    clGetPlatformInfoStr( cl_platform_id platform, cl_platform_info inf, const char* param_name )
    {
        char infobuf[4096];
        cl_int CLerr = clGetPlatformInfo( platform, inf, sizeof(infobuf), infobuf, NULL );
        if ( CLerr == CL_SUCCESS )
            printf( HGPU_OUT_FMT_STR, param_name, infobuf );                
    }

    void
    clGetDeviceInfoStr( cl_device_id device, cl_platform_info inf, const char* param_name )
    {
        char infobuf[4096];
        cl_int CLerr = clGetDeviceInfo( device, inf, sizeof(infobuf), infobuf, NULL );
        if ( CLerr == CL_SUCCESS )
            printf( HGPU_OUT_FMT_STR, param_name, infobuf );                
    }

    template <typename T>
    void inline
    clPrintAddFmt( T param, int fmt_type = HGPU_OUT_FMT_NONE )
    {
        switch( fmt_type )
        {
            case HGPU_OUT_FMT_KB:  printf( " (%5.3f KB)", ( param / 1024. ) );                 break;
            case HGPU_OUT_FMT_MB:  printf( " (%5.3f MB)", ( param / 1024. / 1024. ) );         break;
            case HGPU_OUT_FMT_GB:  printf( " (%5.3f GB)", ( param / 1024. / 1024. / 1024. ) ); break;
            case HGPU_OUT_FMT_MHz: printf( " MHz" );                                           break;
        }
        printf( HGPU_OUT_FMT_END );
    }

    cl_uint
    clGetDeviceInfoUint( cl_device_id device, cl_device_info inf, const char* param_name, int fmt_type = HGPU_OUT_FMT_NONE )
    {
        cl_uint rslt;
        cl_int CLerr = clGetDeviceInfo( device, inf, sizeof(rslt), &rslt, NULL );
        if ( CLerr == CL_SUCCESS )
        {
            printf( HGPU_OUT_FMT_UINT0, param_name, (unsigned int) rslt );
            clPrintAddFmt( rslt, fmt_type );
        }
        return rslt;
    }

    cl_uint
    clGetDeviceInfoHex( cl_device_id device, cl_device_info inf, const char* param_name )
    {
        cl_uint rslt;
        cl_int CLerr = clGetDeviceInfo( device, inf, sizeof(rslt), &rslt, NULL );
        if ( CLerr == CL_SUCCESS )
            printf( HGPU_OUT_FMT_HEX, param_name, rslt );
        return rslt;
    }

    size_t
    clGetDeviceInfoSizeT( cl_device_id device, cl_device_info inf, const char* param_name, int fmt_type = HGPU_OUT_FMT_NONE )
    {
        size_t rslt;
        cl_int CLerr = clGetDeviceInfo( device, inf, sizeof(rslt), &rslt, NULL );
        if ( CLerr == CL_SUCCESS )
        {
            printf( HGPU_OUT_FMT_UINT0, param_name, (unsigned int) rslt );
            clPrintAddFmt( rslt, fmt_type );
        }
        return rslt;
    }

    cl_ulong
    clGetDeviceInfoUlong( cl_device_id device, cl_device_info inf, const char* param_name, int fmt_type = HGPU_OUT_FMT_NONE )
    {
        cl_ulong rslt;
        cl_int CLerr = clGetDeviceInfo( device, inf, sizeof(rslt), &rslt, NULL );
        if ( CLerr == CL_SUCCESS )
        {
            printf( HGPU_OUT_FMT_LONG0, param_name, (long long unsigned int) rslt );
            clPrintAddFmt( rslt, fmt_type );
        }

        return rslt;
    }

    cl_bool
    clGetDeviceInfoBool( cl_device_id device, cl_device_info inf, const char* param_name )
    {
        cl_bool rslt;
        cl_int CLerr = clGetDeviceInfo( device, inf, sizeof(rslt), &rslt, NULL );
        if ( CLerr == CL_SUCCESS )
            printf( HGPU_OUT_FMT_STR, param_name, ( (rslt) ? "Yes" : "No" ) );
        return rslt;
    }

int main(int argc, char ** argv)
{
    char infobuf[4096];
    cl_int   CLerr  = CL_SUCCESS;
    cl_int   CLerr2 = CL_SUCCESS;
    cl_uint  platform_number;    // number of available platforms
    cl_uint  devices_number;     // number of available devices
    cl_uint  infoitemdims    = 0;
    cl_platform_id platform  = NULL;
    cl_platform_id platform2 = NULL;
    cl_device_local_mem_type    local_mem_type;
    cl_device_exec_capabilities device_exec_cap;
    cl_command_queue_properties comqueue_properties;

    HGPU_GPU_error_message( clGetPlatformIDs( 0, NULL, &platform_number ), "clGetPlatformIDs failed" );
    if( platform_number == 0 )
    {
        printf( "There are no any available OpenCL platforms\n" );
        exit( 0 );
    }

    cl_platform_id* platforms = new cl_platform_id[platform_number];
    HGPU_GPU_error_message( clGetPlatformIDs( platform_number, platforms, NULL ), "clGetPlatformIDs failed" );
    printf( "Platforms available: %u\n", platform_number );
    for( size_t i = 0; i < platform_number; i++ )
    {
        platform = platforms[i];
        printf( "--------------------------------------------------------------------------------------\n" );
        printf( "Info on platform %u\n", (unsigned int) ( i + 1 ) );
        clGetPlatformInfoStr( platforms[i], CL_PLATFORM_NAME,       "CL_PLATFORM_NAME"       );
        clGetPlatformInfoStr( platforms[i], CL_PLATFORM_VENDOR,     "CL_PLATFORM_VENDOR"     );
        clGetPlatformInfoStr( platforms[i], CL_PLATFORM_VERSION,    "CL_PLATFORM_VERSION"    );
        clGetPlatformInfoStr( platforms[i], CL_PLATFORM_PROFILE,    "CL_PLATFORM_PROFILE"    );
#if defined( CL_PLATFORM_ICD_SUFFIX_KHR )
        clGetPlatformInfoStr( platforms[i], CL_PLATFORM_ICD_SUFFIX_KHR, "CL_PLATFORM_ICD_SUFFIX_KHR" );
#endif
        clGetPlatformInfoStr( platforms[i], CL_PLATFORM_EXTENSIONS, "CL_PLATFORM_EXTENSIONS" );

        HGPU_GPU_error_message( clGetDeviceIDs( platform, CL_DEVICE_TYPE_ALL, 0, 0, &devices_number ), "clGetDeviceIDs failed" );
        cl_device_id* devices = new cl_device_id[devices_number];
        HGPU_GPU_error_message( clGetDeviceIDs( platform, CL_DEVICE_TYPE_ALL, devices_number, devices, &devices_number ), "clGetDeviceIDs failed" );
        printf( "Devices available: %u\n", devices_number );
        for( size_t t = 0; t < devices_number; t++ )
        {
            int device_opencl_c_version = 0;
            int device_vendor = 0;
            size_t workitemdims[16];
            cl_device_type device_type;
            cl_device_fp_config fp_config;
            cl_device_mem_cache_type mem_cache_type;

            printf( "--------------------------------------------------------------------------------------\n" );
            printf( "Info on device %u\n", (unsigned int) ( t + 1 ) );

            HGPU_GPU_error_message( clGetDeviceInfo( devices[t], CL_DEVICE_VENDOR, sizeof(infobuf), &infobuf, NULL ), "clGetDeviceInfo failed" );
                if( strstr( infobuf, HGPU_DEVICE_VENDOR_AMD    ) ) device_vendor = HGPU_VENDOR_AMD;
                if( strstr( infobuf, HGPU_DEVICE_VENDOR_NVIDIA ) ) device_vendor = HGPU_VENDOR_NVIDIA;
                if( strstr( infobuf, HGPU_DEVICE_VENDOR_INTEL  ) ) device_vendor = HGPU_VENDOR_INTEL;

            switch( device_vendor )
            {
                case HGPU_VENDOR_AMD:    printf( HGPU_OUT_FMT_STR, "DEVICE VENDOR", "AMD"    ); break;
                case HGPU_VENDOR_NVIDIA: printf( HGPU_OUT_FMT_STR, "DEVICE VENDOR", "NVIDIA" ); break;
                case HGPU_VENDOR_INTEL:  printf( HGPU_OUT_FMT_STR, "DEVICE VENDOR", "INTEL"  ); break;
            }

            clGetDeviceInfoStr( devices[t], CL_DEVICE_NAME,    "CL_DEVICE_NAME"    );
#if defined( CL_DEVICE_BOARD_NAME_AMD )
                clGetDeviceInfoStr( devices[t], CL_DEVICE_BOARD_NAME_AMD, "CL_DEVICE_BOARD_NAME_AMD" );
#endif

            clGetDeviceInfoStr( devices[t], CL_DEVICE_VENDOR,  "CL_DEVICE_VENDOR"  );
            clGetDeviceInfoStr( devices[t], CL_DRIVER_VERSION, "CL_DRIVER_VERSION" );
            clGetDeviceInfoStr( devices[t], CL_DEVICE_PROFILE, "CL_DEVICE_PROFILE" );
            clGetDeviceInfoStr( devices[t], CL_DEVICE_VERSION, "CL_DEVICE_VERSION" );
            HGPU_GPU_error_message( clGetDeviceInfo( devices[t], CL_DEVICE_OPENCL_C_VERSION, sizeof(infobuf), &infobuf, NULL ), "clGetDeviceInfo failed" );
            printf( HGPU_OUT_FMT_STR, "CL_DEVICE_OPENCL_C_VERSION", infobuf );
                if( strstr( infobuf, HGPU_OPENCL_VERSION_1_0 ) ) device_opencl_c_version = HGPU_OPENCL_1_0;
                if( strstr( infobuf, HGPU_OPENCL_VERSION_1_1 ) ) device_opencl_c_version = HGPU_OPENCL_1_1;
                if( strstr( infobuf, HGPU_OPENCL_VERSION_1_2 ) ) device_opencl_c_version = HGPU_OPENCL_1_2;
                if( strstr( infobuf, HGPU_OPENCL_VERSION_2_0 ) ) device_opencl_c_version = HGPU_OPENCL_2_0;
            HGPU_GPU_error_message( clGetDeviceInfo( devices[t], CL_DEVICE_TYPE, sizeof(device_type), &device_type, NULL ), "clGetDeviceInfo failed" );
            printf( HGPU_OUT_FMT_NSTR, "CL_DEVICE_TYPE" );
            if( device_type & CL_DEVICE_TYPE_CPU         ) printf( "CPU "         );
            if( device_type & CL_DEVICE_TYPE_GPU         ) printf( "GPU "         );
            if( device_type & CL_DEVICE_TYPE_ACCELERATOR ) printf( "ACCELERATOR " );
            if( device_type & CL_DEVICE_TYPE_DEFAULT     ) printf( "DEFAULT "     );

#if defined( CL_VERSION_1_2 )
            if( device_opencl_c_version >= HGPU_OPENCL_1_2 )
            {
                if( device_type & CL_DEVICE_TYPE_CUSTOM  ) printf( "CUSTOM "      );
            }
#endif
            printf( "\n" );

#if defined( CL_DEVICE_SPIR_VERSIONS )
            clGetDeviceInfoStr( devices[t], CL_DEVICE_SPIR_VERSIONS, "CL_DEVICE_SPIR_VERSIONS" );
#endif

            clGetDeviceInfoStr(  devices[t], CL_DEVICE_EXTENSIONS,           "CL_DEVICE_EXTENSIONS"        );
            clGetDeviceInfoHex(  devices[t], CL_DEVICE_VENDOR_ID,            "CL_DEVICE_VENDOR_ID"         );
            clGetDeviceInfoUint( devices[t], CL_DEVICE_MAX_COMPUTE_UNITS,    "CL_DEVICE_MAX_COMPUTE_UNITS" );

#if ( defined( CL_DEVICE_GFXIP_MAJOR_AMD ) && defined( CL_DEVICE_GFXIP_MINOR_AMD ) )
            cl_uint GFXIP_major = 0;
            cl_uint GFXIP_minor = 0;
            CLerr  = clGetDeviceInfo( devices[t], CL_DEVICE_GFXIP_MAJOR_AMD, sizeof(GFXIP_major), &GFXIP_major, NULL );
            CLerr2 = clGetDeviceInfo( devices[t], CL_DEVICE_GFXIP_MINOR_AMD, sizeof(GFXIP_minor), &GFXIP_minor, NULL );
            if( ( CLerr | CLerr2 ) == CL_SUCCESS )
            {
                printf( HGPU_OUT_FMT_NSTR, "CL_DEVICE_GFXIP_MAJOR/MINOR_AMD" );
                printf( "%u.%u\n", GFXIP_major, GFXIP_minor );
            }
#endif
#if ( defined( CL_DEVICE_COMPUTE_CAPABILITY_MAJOR_NV ) && defined( CL_DEVICE_COMPUTE_CAPABILITY_MINOR_NV ) )
            cl_uint NVCC_major = 0;
            cl_uint NVCC_minor = 0;
            CLerr  = clGetDeviceInfo( devices[t], CL_DEVICE_COMPUTE_CAPABILITY_MAJOR_NV, sizeof(NVCC_major), &NVCC_major, NULL );
            CLerr2 = clGetDeviceInfo( devices[t], CL_DEVICE_COMPUTE_CAPABILITY_MINOR_NV, sizeof(NVCC_minor), &NVCC_minor, NULL );
            if( ( CLerr | CLerr2 ) == CL_SUCCESS )
            {
                printf( HGPU_OUT_FMT_NSTR, "CL_DEVICE_COMPUTE_CAPABILITY_MAJOR/MINOR_NV" );
                printf( "%u.%u\n", NVCC_major, NVCC_minor );
            }
#endif

            infoitemdims = clGetDeviceInfoUint( devices[t], CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, "CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS" );
            if ( infoitemdims > 0 )
            {
                HGPU_GPU_error_message( clGetDeviceInfo( devices[t], CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(workitemdims), &workitemdims, NULL ), "clGetDeviceInfo failed" );
                printf( HGPU_OUT_FMT_NSTR, "CL_DEVICE_MAX_WORK_ITEM_SIZES" );
                for( unsigned int j = 0; j < infoitemdims; j++ )
                    printf( "%u ", (unsigned int) workitemdims[j] );
                printf( "\n" );
            }
            clGetDeviceInfoSizeT( devices[t], CL_DEVICE_MAX_WORK_GROUP_SIZE,            "CL_DEVICE_MAX_WORK_GROUP_SIZE"           );

#if defined( CL_DEVICE_TOPOLOGY_AMD )
            cl_device_topology_amd topology_amd;
            CLerr = clGetDeviceInfo( devices[t], CL_DEVICE_TOPOLOGY_AMD, sizeof(topology_amd), &topology_amd, NULL );
            if( CLerr == CL_SUCCESS )
            {
                printf( HGPU_OUT_FMT_NSTR, "CL_DEVICE_TOPOLOGY_AMD" );
                if ( topology_amd.raw.type == CL_DEVICE_TOPOLOGY_TYPE_PCIE_AMD )
                {
                    printf( "PCIe, %02x:%02x.%u\n", topology_amd.pcie.bus, topology_amd.pcie.device, topology_amd.pcie.function );
                }
                else
                {
                    printf( "type/raw %04x:%04x\n", (unsigned int) topology_amd.raw.type, (unsigned int) topology_amd.raw.data[4] );
                }
            }
#endif

#if ( defined( CL_DEVICE_PCI_BUS_ID_NV ) && defined( CL_DEVICE_PCI_SLOT_ID_NV ) )
            cl_uint NV_bus  = 0;
            cl_uint NV_slot = 0;
            CLerr  = clGetDeviceInfo( devices[t], CL_DEVICE_PCI_BUS_ID_NV, sizeof(NV_bus), &NV_bus, NULL );
            CLerr2 = clGetDeviceInfo( devices[t], CL_DEVICE_PCI_SLOT_ID_NV, sizeof(NV_slot), &NV_slot, NULL );
            if( ( CLerr | CLerr2 ) == CL_SUCCESS )
            {
                printf( HGPU_OUT_FMT_NSTR, "CL_DEVICE_PCI_BUS/SLOT_ID_NV" );
                printf( "%02x:%02x\n", NV_bus, NV_slot );
            }
#endif
#if defined( CL_DEVICE_REGISTERS_PER_BLOCK_NV )
            clGetDeviceInfoUint(  devices[t], CL_DEVICE_REGISTERS_PER_BLOCK_NV,         "CL_DEVICE_REGISTERS_PER_BLOCK_NV"        );
#endif
#if defined( CL_DEVICE_WARP_SIZE_NV )
            clGetDeviceInfoUint(  devices[t], CL_DEVICE_WARP_SIZE_NV,                   "CL_DEVICE_WARP_SIZE_NV"                  );
#endif
#if defined( CL_DEVICE_GPU_OVERLAP_NV )
            clGetDeviceInfoBool(  devices[t], CL_DEVICE_GPU_OVERLAP_NV,                 "CL_DEVICE_GPU_OVERLAP_NV"                );
#endif
#if defined( CL_DEVICE_KERNEL_EXEC_TIMEOUT_NV )
            clGetDeviceInfoBool(  devices[t], CL_DEVICE_KERNEL_EXEC_TIMEOUT_NV,         "CL_DEVICE_KERNEL_EXEC_TIMEOUT_NV"        );
#endif
#if defined( CL_DEVICE_INTEGRATED_MEMORY_NV )
            clGetDeviceInfoBool(  devices[t], CL_DEVICE_INTEGRATED_MEMORY_NV,           "CL_DEVICE_INTEGRATED_MEMORY_NV"          );
#endif
#if defined( CL_DEVICE_ATTRIBUTE_ASYNC_ENGINE_COUNT_NV )
            clGetDeviceInfoUint(  devices[t], CL_DEVICE_ATTRIBUTE_ASYNC_ENGINE_COUNT_NV, "CL_DEVICE_ATTRIBUTE_ASYNC_ENGINE_COUNT_NV" );
#endif

#if defined( CL_DEVICE_SIMD_PER_COMPUTE_UNIT_AMD )
            clGetDeviceInfoUint(  devices[t], CL_DEVICE_SIMD_PER_COMPUTE_UNIT_AMD,      "CL_DEVICE_SIMD_PER_COMPUTE_UNIT_AMD"     );
#endif
#if defined( CL_DEVICE_SIMD_WIDTH_AMD )
            clGetDeviceInfoUint(  devices[t], CL_DEVICE_SIMD_WIDTH_AMD,                 "CL_DEVICE_SIMD_WIDTH_AMD"                );
#endif
#if defined( CL_DEVICE_SIMD_INSTRUCTION_WIDTH_AMD )
            clGetDeviceInfoUint(  devices[t], CL_DEVICE_SIMD_INSTRUCTION_WIDTH_AMD,     "CL_DEVICE_SIMD_INSTRUCTION_WIDTH_AMD"    );
#endif
#if defined( CL_DEVICE_WAVEFRONT_WIDTH_AMD )
            clGetDeviceInfoUint(  devices[t], CL_DEVICE_WAVEFRONT_WIDTH_AMD,            "CL_DEVICE_WAVEFRONT_WIDTH_AMD"           );
#endif
#if defined( CL_DEVICE_THREAD_TRACE_SUPPORTED_AMD )
            clGetDeviceInfoBool(  devices[t], CL_DEVICE_THREAD_TRACE_SUPPORTED_AMD,     "CL_DEVICE_THREAD_TRACE_SUPPORTED_AMD"    );
#endif

#if defined( CL_DEVICE_MAX_ATOMIC_COUNTERS_EXT )
            clGetDeviceInfoSizeT( devices[t], CL_DEVICE_MAX_ATOMIC_COUNTERS_EXT,        "CL_DEVICE_MAX_ATOMIC_COUNTERS_EXT"       );
#endif

            clGetDeviceInfoUint(  devices[t], CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR,    "CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR"   );
            clGetDeviceInfoUint(  devices[t], CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT,   "CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT"  );
            clGetDeviceInfoUint(  devices[t], CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT,     "CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT"    );
            clGetDeviceInfoUint(  devices[t], CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG,    "CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG"   );
            clGetDeviceInfoUint(  devices[t], CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT,   "CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT"  );
            clGetDeviceInfoUint(  devices[t], CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE,  "CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE" );

#if defined( CL_VERSION_1_1 )
            if( device_opencl_c_version >= HGPU_OPENCL_1_1 )
            {
                clGetDeviceInfoUint( devices[t], CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF, "CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF"   );
                clGetDeviceInfoUint( devices[t], CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR,    "CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR"      );
                clGetDeviceInfoUint( devices[t], CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT,   "CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT"     );
                clGetDeviceInfoUint( devices[t], CL_DEVICE_NATIVE_VECTOR_WIDTH_INT,     "CL_DEVICE_NATIVE_VECTOR_WIDTH_INT"       );
                clGetDeviceInfoUint( devices[t], CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG,    "CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG"      );
                clGetDeviceInfoUint( devices[t], CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT,   "CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT"     );
                clGetDeviceInfoUint( devices[t], CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE,  "CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE"    );
                clGetDeviceInfoUint( devices[t], CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF,    "CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF"      );
            }
#endif
            clGetDeviceInfoUint(  devices[t], CL_DEVICE_MAX_CLOCK_FREQUENCY,  "CL_DEVICE_MAX_CLOCK_FREQUENCY", HGPU_OUT_FMT_MHz   );
            clGetDeviceInfoUint(  devices[t], CL_DEVICE_ADDRESS_BITS,         "CL_DEVICE_ADDRESS_BITS"         );
            clGetDeviceInfoUlong( devices[t], CL_DEVICE_MAX_MEM_ALLOC_SIZE,   "CL_DEVICE_MAX_MEM_ALLOC_SIZE",  HGPU_OUT_FMT_GB    );
            clGetDeviceInfoBool(  devices[t], CL_DEVICE_IMAGE_SUPPORT,        "CL_DEVICE_IMAGE_SUPPORT"        );
            clGetDeviceInfoUint(  devices[t], CL_DEVICE_MAX_READ_IMAGE_ARGS,  "CL_DEVICE_MAX_READ_IMAGE_ARGS"  );
            clGetDeviceInfoUint(  devices[t], CL_DEVICE_MAX_WRITE_IMAGE_ARGS, "CL_DEVICE_MAX_WRITE_IMAGE_ARGS" );
            clGetDeviceInfoSizeT( devices[t], CL_DEVICE_IMAGE2D_MAX_WIDTH,    "CL_DEVICE_IMAGE2D_MAX_WIDTH"    );
            clGetDeviceInfoSizeT( devices[t], CL_DEVICE_IMAGE2D_MAX_HEIGHT,   "CL_DEVICE_IMAGE2D_MAX_HEIGHT"   );
            clGetDeviceInfoSizeT( devices[t], CL_DEVICE_IMAGE3D_MAX_WIDTH,    "CL_DEVICE_IMAGE3D_MAX_WIDTH"    );
            clGetDeviceInfoSizeT( devices[t], CL_DEVICE_IMAGE3D_MAX_HEIGHT,   "CL_DEVICE_IMAGE3D_MAX_HEIGHT"   );
            clGetDeviceInfoSizeT( devices[t], CL_DEVICE_IMAGE3D_MAX_DEPTH,    "CL_DEVICE_IMAGE3D_MAX_DEPTH"    );
            clGetDeviceInfoUint(  devices[t], CL_DEVICE_MAX_SAMPLERS,         "CL_DEVICE_MAX_SAMPLERS"         );

#if defined( CL_VERSION_1_2 )
            if( device_opencl_c_version >= HGPU_OPENCL_1_2 )
            {
                clGetDeviceInfoSizeT( devices[t], CL_DEVICE_IMAGE_MAX_BUFFER_SIZE, "CL_DEVICE_IMAGE_MAX_BUFFER_SIZE",  HGPU_OUT_FMT_MB );
                clGetDeviceInfoSizeT( devices[t], CL_DEVICE_IMAGE_MAX_ARRAY_SIZE,  "CL_DEVICE_IMAGE_MAX_ARRAY_SIZE",   HGPU_OUT_FMT_KB );
            }
#endif

#if defined( CL_VERSION_2_0 )
            // if( device_opencl_c_version >= HGPU_OPENCL_2_0 )
            // {
                clGetDeviceInfoUint( devices[t], CL_DEVICE_IMAGE_PITCH_ALIGNMENT,        "CL_DEVICE_IMAGE_PITCH_ALIGNMENT"        );
                clGetDeviceInfoUint( devices[t], CL_DEVICE_IMAGE_BASE_ADDRESS_ALIGNMENT, "CL_DEVICE_IMAGE_BASE_ADDRESS_ALIGNMENT" );
                clGetDeviceInfoUint( devices[t], CL_DEVICE_MAX_PIPE_ARGS,                "CL_DEVICE_MAX_PIPE_ARGS"                );
                clGetDeviceInfoUint( devices[t], CL_DEVICE_PIPE_MAX_ACTIVE_RESERVATIONS, "CL_DEVICE_PIPE_MAX_ACTIVE_RESERVATIONS" );
                clGetDeviceInfoUint( devices[t], CL_DEVICE_PIPE_MAX_PACKET_SIZE,         "CL_DEVICE_PIPE_MAX_PACKET_SIZE"         );
            // }
#endif

            clGetDeviceInfoSizeT( devices[t], CL_DEVICE_MAX_PARAMETER_SIZE,       "CL_DEVICE_MAX_PARAMETER_SIZE",   HGPU_OUT_FMT_KB );
            clGetDeviceInfoUint(  devices[t], CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE, "CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE" );
            clGetDeviceInfoUint(  devices[t], CL_DEVICE_MEM_BASE_ADDR_ALIGN,      "CL_DEVICE_MEM_BASE_ADDR_ALIGN"      );

            printf( HGPU_OUT_FMT_N0STR, "CL_DEVICE_SINGLE_FP_CONFIG configuration" );
            HGPU_GPU_error_message( clGetDeviceInfo( devices[t], CL_DEVICE_SINGLE_FP_CONFIG, sizeof(fp_config), &fp_config, NULL ), "clGetDeviceInfo failed" );
                printf( "\tCL_FP_DENORM:           %s\n", ( ( fp_config & CL_FP_DENORM )           ? "Yes" : "No" ) );
                printf( "\tCL_FP_INF_NAN:          %s\n", ( ( fp_config & CL_FP_INF_NAN )          ? "Yes" : "No" ) );
                printf( "\tCL_FP_ROUND_TO_NEAREST: %s\n", ( ( fp_config & CL_FP_ROUND_TO_NEAREST ) ? "Yes" : "No" ) );
                printf( "\tCL_FP_ROUND_TO_ZERO:    %s\n", ( ( fp_config & CL_FP_ROUND_TO_ZERO )    ? "Yes" : "No" ) );
                printf( "\tCL_FP_ROUND_TO_INF:     %s\n", ( ( fp_config & CL_FP_ROUND_TO_INF )     ? "Yes" : "No" ) );
                printf( "\tCL_FP_FMA:              %s\n", ( ( fp_config & CL_FP_FMA )              ? "Yes" : "No" ) );
#if defined( CL_VERSION_1_1 )
                if( device_opencl_c_version >= HGPU_OPENCL_1_1 )
                {
                    printf( "\tCL_FP_SOFT_FLOAT:       %s\n", ( ( fp_config & CL_FP_SOFT_FLOAT )   ? "Yes" : "No" ) );
                }
#endif

#if defined( CL_VERSION_1_2 )
            if( device_opencl_c_version >= HGPU_OPENCL_1_2 )
            {
                    printf( "\tCL_FP_CORRECTLY_ROUNDED_DIVIDE_SQRT: %s\n", ( ( fp_config & CL_FP_CORRECTLY_ROUNDED_DIVIDE_SQRT ) ? "Yes" : "No" ) );

                printf( HGPU_OUT_FMT_N0STR, "CL_DEVICE_DOUBLE_FP_CONFIG configuration" );
                HGPU_GPU_error_message( clGetDeviceInfo( devices[t], CL_DEVICE_DOUBLE_FP_CONFIG, sizeof(fp_config), &fp_config, NULL ), "clGetDeviceInfo failed" );
                    printf( "\tCL_FP_DENORM:           %s\n", ( ( fp_config & CL_FP_DENORM )           ? "Yes" : "No" ) );
                    printf( "\tCL_FP_INF_NAN:          %s\n", ( ( fp_config & CL_FP_INF_NAN )          ? "Yes" : "No" ) );
                    printf( "\tCL_FP_ROUND_TO_NEAREST: %s\n", ( ( fp_config & CL_FP_ROUND_TO_NEAREST ) ? "Yes" : "No" ) );
                    printf( "\tCL_FP_ROUND_TO_ZERO:    %s\n", ( ( fp_config & CL_FP_ROUND_TO_ZERO )    ? "Yes" : "No" ) );
                    printf( "\tCL_FP_ROUND_TO_INF:     %s\n", ( ( fp_config & CL_FP_ROUND_TO_INF )     ? "Yes" : "No" ) );
                    printf( "\tCL_FP_FMA:              %s\n", ( ( fp_config & CL_FP_FMA )              ? "Yes" : "No" ) );
                    printf( "\tCL_FP_SOFT_FLOAT:       %s\n", ( ( fp_config & CL_FP_SOFT_FLOAT )       ? "Yes" : "No" ) );
            }
#endif

            HGPU_GPU_error_message( clGetDeviceInfo( devices[t], CL_DEVICE_GLOBAL_MEM_CACHE_TYPE, sizeof(mem_cache_type), &mem_cache_type, NULL ), "clGetDeviceInfo failed" );
            printf( HGPU_OUT_FMT_NSTR, "CL_DEVICE_GLOBAL_MEM_CACHE_TYPE" );
            switch ( mem_cache_type )
            {
                case CL_NONE:             printf( "NONE\n"             ); break;
                case CL_READ_ONLY_CACHE:  printf( "READ_ONLY_CACHE\n"  ); break;
                case CL_READ_WRITE_CACHE: printf( "READ_WRITE_CACHE\n" ); break;
            }

            clGetDeviceInfoUint(  devices[t], CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE, "CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE" );
            clGetDeviceInfoUlong( devices[t], CL_DEVICE_GLOBAL_MEM_CACHE_SIZE,     "CL_DEVICE_GLOBAL_MEM_CACHE_SIZE",       HGPU_OUT_FMT_KB );

#if defined( CL_DEVICE_GLOBAL_MEM_CHANNELS_AMD )
            clGetDeviceInfoUint(  devices[t], CL_DEVICE_GLOBAL_MEM_CHANNELS_AMD,   "CL_DEVICE_GLOBAL_MEM_CHANNELS_AMD"   );
#endif
#if defined( CL_DEVICE_GLOBAL_MEM_CHANNEL_BANKS_AMD )
            clGetDeviceInfoUint(  devices[t], CL_DEVICE_GLOBAL_MEM_CHANNEL_BANKS_AMD, "CL_DEVICE_GLOBAL_MEM_CHANNEL_BANKS_AMD" );
#endif
#if defined( CL_DEVICE_GLOBAL_MEM_CHANNEL_BANK_WIDTH_AMD )
            clGetDeviceInfoUint(  devices[t], CL_DEVICE_GLOBAL_MEM_CHANNEL_BANK_WIDTH_AMD, "CL_DEVICE_GLOBAL_MEM_CHANNEL_BANK_WIDTH_AMD" );
#endif
#if defined( CL_DEVICE_GLOBAL_FREE_MEMORY_AMD )
            size_t amdfreemem[1024];
            size_t reslt_siz = 0;
            CLerr = clGetDeviceInfo( devices[t], CL_DEVICE_GLOBAL_FREE_MEMORY_AMD, sizeof(amdfreemem), &amdfreemem, &reslt_siz );
            printf( HGPU_OUT_FMT_NSTR, "CL_DEVICE_GLOBAL_FREE_MEMORY_AMD" );
            bool flag_free = false;
            for( size_t u = 0; u < reslt_siz; u++ )
            {
                if( amdfreemem[u] )
                {
                    if( flag_free ) printf( ", " );
                    printf( "%u", (unsigned int) amdfreemem[u] );
                    flag_free = true;
                }
            }
            printf( "\n" );
#endif

            clGetDeviceInfoUlong( devices[t], CL_DEVICE_GLOBAL_MEM_SIZE,           "CL_DEVICE_GLOBAL_MEM_SIZE",             HGPU_OUT_FMT_GB );
            clGetDeviceInfoUlong( devices[t], CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE,  "CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE",    HGPU_OUT_FMT_KB );
            clGetDeviceInfoUint(  devices[t], CL_DEVICE_MAX_CONSTANT_ARGS,         "CL_DEVICE_MAX_CONSTANT_ARGS"         );

#if defined( CL_VERSION_2_0 )
            if( device_opencl_c_version >= HGPU_OPENCL_2_0 )
            {
                clGetDeviceInfoSizeT( devices[t], CL_DEVICE_MAX_GLOBAL_VARIABLE_SIZE,             "CL_DEVICE_MAX_GLOBAL_VARIABLE_SIZE"             );
                clGetDeviceInfoSizeT( devices[t], CL_DEVICE_GLOBAL_VARIABLE_PREFERRED_TOTAL_SIZE, "CL_DEVICE_GLOBAL_VARIABLE_PREFERRED_TOTAL_SIZE" );
            }
#endif
            HGPU_GPU_error_message( clGetDeviceInfo( devices[t], CL_DEVICE_LOCAL_MEM_TYPE, sizeof(local_mem_type), &local_mem_type, NULL ), "clGetDeviceInfo failed" );
            printf( HGPU_OUT_FMT_NSTR, "CL_DEVICE_LOCAL_MEM_TYPE" );
            switch ( local_mem_type )
            {
                case CL_LOCAL:  printf( "local\n"  ); break;
                case CL_GLOBAL: printf( "global\n" ); break;
            }
            clGetDeviceInfoUlong( devices[t], CL_DEVICE_LOCAL_MEM_SIZE,             "CL_DEVICE_LOCAL_MEM_SIZE",               HGPU_OUT_FMT_KB );
#if defined( CL_DEVICE_LOCAL_MEM_SIZE_PER_COMPUTE_UNIT_AMD )
            clGetDeviceInfoUint(  devices[t], CL_DEVICE_LOCAL_MEM_SIZE_PER_COMPUTE_UNIT_AMD, "CL_DEVICE_LOCAL_MEM_SIZE_PER_COMPUTE_UNIT_AMD" );
#endif
#if defined( CL_DEVICE_LOCAL_MEM_BANKS_AMD )
            clGetDeviceInfoUint(  devices[t], CL_DEVICE_LOCAL_MEM_BANKS_AMD,        "CL_DEVICE_LOCAL_MEM_BANKS_AMD"        );
#endif
            clGetDeviceInfoBool(  devices[t], CL_DEVICE_ERROR_CORRECTION_SUPPORT,   "CL_DEVICE_ERROR_CORRECTION_SUPPORT"   );

#if defined( CL_VERSION_1_1 )
            if( device_opencl_c_version >= HGPU_OPENCL_1_1 )
            {
                clGetDeviceInfoBool( devices[t], CL_DEVICE_HOST_UNIFIED_MEMORY,     "CL_DEVICE_HOST_UNIFIED_MEMORY"        );
            }
#endif

            clGetDeviceInfoSizeT( devices[t], CL_DEVICE_PROFILING_TIMER_RESOLUTION, "CL_DEVICE_PROFILING_TIMER_RESOLUTION" );
#if defined( CL_DEVICE_PROFILING_TIMER_OFFSET_AMD )
            clGetDeviceInfoUlong( devices[t], CL_DEVICE_PROFILING_TIMER_OFFSET_AMD, "CL_DEVICE_PROFILING_TIMER_OFFSET_AMD" );
#endif

            clGetDeviceInfoBool(  devices[t], CL_DEVICE_ENDIAN_LITTLE,              "CL_DEVICE_ENDIAN_LITTLE"              );
            clGetDeviceInfoBool(  devices[t], CL_DEVICE_AVAILABLE,                  "CL_DEVICE_AVAILABLE"                  );
            clGetDeviceInfoBool(  devices[t], CL_DEVICE_COMPILER_AVAILABLE,         "CL_DEVICE_COMPILER_AVAILABLE"         );

#if defined( CL_VERSION_1_2 )
            if( device_opencl_c_version >= HGPU_OPENCL_1_2 )
            {
                clGetDeviceInfoBool(  devices[t], CL_DEVICE_LINKER_AVAILABLE,            "CL_DEVICE_LINKER_AVAILABLE"            );
                clGetDeviceInfoSizeT( devices[t], CL_DEVICE_PRINTF_BUFFER_SIZE,          "CL_DEVICE_PRINTF_BUFFER_SIZE",         HGPU_OUT_FMT_MB );
                clGetDeviceInfoBool(  devices[t], CL_DEVICE_PREFERRED_INTEROP_USER_SYNC, "CL_DEVICE_PREFERRED_INTEROP_USER_SYNC" );
                clGetDeviceInfoStr(   devices[t], CL_DEVICE_BUILT_IN_KERNELS,            "CL_DEVICE_BUILT_IN_KERNELS"            );

                cl_device_id device_parent = NULL;
                HGPU_GPU_error_message( clGetDeviceInfo( devices[t], CL_DEVICE_PARENT_DEVICE, sizeof(device_parent), device_parent, NULL ), "clGetDeviceInfo failed" );
                printf( HGPU_OUT_FMT_LONG, "CL_DEVICE_PARENT_DEVICE", (long long unsigned int) device_parent );

                clGetDeviceInfoUint( devices[t], CL_DEVICE_PARTITION_MAX_SUB_DEVICES, "CL_DEVICE_PARTITION_MAX_SUB_DEVICES" );

                cl_device_partition_property part_prop[1024];
                size_t part_prop_size = 0;
                HGPU_GPU_error_message( clGetDeviceInfo( devices[t], CL_DEVICE_PARTITION_PROPERTIES, sizeof(part_prop), part_prop, &part_prop_size ), "clGetDeviceInfo failed" );
                printf( HGPU_OUT_FMT_NSTR, "CL_DEVICE_PARTITION_PROPERTIES" );
                for( size_t p = 0; p < part_prop_size; p++ )
                {
                    if( part_prop[p] )
                    {
                        switch( part_prop[p] )
                        {
                            case CL_DEVICE_PARTITION_EQUALLY:            printf( "CL_DEVICE_PARTITION_EQUALLY "            );       break;
                            case CL_DEVICE_PARTITION_BY_COUNTS:          printf( "CL_DEVICE_PARTITION_BY_COUNTS "          );       break;
                            case CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN: printf( "CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN " );       break;
#if defined( CL_DEVICE_PARTITION_BY_NAMES_INTEL )
                            case CL_DEVICE_PARTITION_BY_NAMES_INTEL:     printf( "CL_DEVICE_PARTITION_BY_NAMES_INTEL "     );       break;
#else
#if defined( CL_DEVICE_PARTITION_BY_NAMES_EXT )
                            case CL_DEVICE_PARTITION_BY_NAMES_EXT:       printf( "CL_DEVICE_PARTITION_BY_NAMES_EXT "       );       break;
#endif                                                                                                                  
#endif
                            default:                                     printf( "%#llx ", (long long unsigned int) part_prop[p] ); break;
                        }
                    }
                }
                printf( "\n" );

                cl_device_affinity_domain dev_affin;
                HGPU_GPU_error_message( clGetDeviceInfo( devices[t], CL_DEVICE_PARTITION_TYPE, sizeof(dev_affin), &dev_affin, NULL ), "clGetDeviceInfo failed" );
                if( dev_affin != 0 )
                {
                    printf( "CL_DEVICE_PARTITION_TYPE:\n" );
                        if( dev_affin & CL_DEVICE_AFFINITY_DOMAIN_NUMA               ) printf( HGPU_OUT_FMT_TN, "CL_DEVICE_AFFINITY_DOMAIN_NUMA"               );
                        if( dev_affin & CL_DEVICE_AFFINITY_DOMAIN_L4_CACHE           ) printf( HGPU_OUT_FMT_TN, "CL_DEVICE_AFFINITY_DOMAIN_L4_CACHE"           );
                        if( dev_affin & CL_DEVICE_AFFINITY_DOMAIN_L3_CACHE           ) printf( HGPU_OUT_FMT_TN, "CL_DEVICE_AFFINITY_DOMAIN_L3_CACHE"           );
                        if( dev_affin & CL_DEVICE_AFFINITY_DOMAIN_L2_CACHE           ) printf( HGPU_OUT_FMT_TN, "CL_DEVICE_AFFINITY_DOMAIN_L2_CACHE"           );
                        if( dev_affin & CL_DEVICE_AFFINITY_DOMAIN_L1_CACHE           ) printf( HGPU_OUT_FMT_TN, "CL_DEVICE_AFFINITY_DOMAIN_L1_CACHE"           );
                        if( dev_affin & CL_DEVICE_AFFINITY_DOMAIN_NEXT_PARTITIONABLE ) printf( HGPU_OUT_FMT_TN, "CL_DEVICE_AFFINITY_DOMAIN_NEXT_PARTITIONABLE" );
                }


// CL_PARTITION_BY_NAMES_LIST_END_INTEL
#if defined( CL_DEVICE_PARTITION_PROPERTIES_EXT )
                cl_device_partition_property_ext part_prop_ext[1024];
                size_t part_prop_size = 0;
                HGPU_GPU_error_message( clGetDeviceInfo( devices[t], CL_DEVICE_PARTITION_PROPERTIES, sizeof(part_prop), part_prop, &part_prop_size ), "clGetDeviceInfo failed" );
                printf( HGPU_OUT_FMT_NSTR, "CL_DEVICE_PARTITION_PROPERTIES" );
                for( size_t p = 0; p < part_prop_size; p++ )
                {
                    if( part_prop[p] )
                    {
                        switch( part_prop[p] )
                        {
                            case CL_DEVICE_PARTITION_EQUALLY:            printf( "CL_DEVICE_PARTITION_EQUALLY "            );       break;
                            case CL_DEVICE_PARTITION_BY_COUNTS:          printf( "CL_DEVICE_PARTITION_BY_COUNTS "          );       break;
                            case CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN: printf( "CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN " );       break;
#if defined( CL_DEVICE_PARTITION_BY_NAMES_INTEL )
                            case CL_DEVICE_PARTITION_BY_NAMES_INTEL:     printf( "CL_DEVICE_PARTITION_BY_NAMES_INTEL "     );       break;
#else
#if defined( CL_DEVICE_PARTITION_BY_NAMES_EXT )
                            case CL_DEVICE_PARTITION_BY_NAMES_EXT:       printf( "CL_DEVICE_PARTITION_BY_NAMES_EXT "       );       break;
#endif                                                                                                                  
#endif
                            default:                                     printf( "%#llx ", (long long unsigned int) part_prop[p] ); break;
                        }
                    }
                }
                printf( "\n" );
#endif


//#if defined( CL_DEVICE_AFFINITY_DOMAINS_EXT )
//                cl_device_partition_property_ext dev_affin_ext;
//                HGPU_GPU_error_message( clGetDeviceInfo( devices[t], CL_DEVICE_AFFINITY_DOMAINS_EXT, sizeof(dev_affin_ext), &dev_affin_ext, NULL ), "clGetDeviceInfo failed" );
//                if( dev_affin_ext )
//                {
//                    printf( "CL_DEVICE_AFFINITY_DOMAINS_EXT:\n" );
//    #if defined( CL_AFFINITY_DOMAIN_L1_CACHE_EXT )
//                        if( dev_affin_ext & CL_AFFINITY_DOMAIN_L1_CACHE_EXT           ) printf( " CL_AFFINITY_DOMAIN_L1_CACHE_EXT"           );
//    #endif
//    #if defined( CL_AFFINITY_DOMAIN_L2_CACHE_EXT )
//                        if( dev_affin_ext & CL_AFFINITY_DOMAIN_L2_CACHE_EXT           ) printf( " CL_AFFINITY_DOMAIN_L2_CACHE_EXT"           );
//    #endif
//    #if defined( CL_AFFINITY_DOMAIN_L3_CACHE_EXT )
//                        if( dev_affin_ext & CL_AFFINITY_DOMAIN_L3_CACHE_EXT           ) printf( " CL_AFFINITY_DOMAIN_L3_CACHE_EXT"           );
//    #endif
//    #if defined( CL_AFFINITY_DOMAIN_L4_CACHE_EXT )
//                        if( dev_affin_ext & CL_AFFINITY_DOMAIN_L4_CACHE_EXT           ) printf( " CL_AFFINITY_DOMAIN_L4_CACHE_EXT"           );
//    #endif
//    #if defined( CL_AFFINITY_DOMAIN_NUMA_EXT )
//                        if( dev_affin_ext & CL_AFFINITY_DOMAIN_NUMA_EXT               ) printf( " CL_AFFINITY_DOMAIN_NUMA_EXT"               );
//    #endif
//    #if defined( CL_AFFINITY_DOMAIN_NEXT_FISSIONABLE_EXT )
//                        if( dev_affin_ext & CL_AFFINITY_DOMAIN_NEXT_FISSIONABLE_EXT   ) printf( " CL_AFFINITY_DOMAIN_NEXT_FISSIONABLE_EXT"   );
//    #endif
//                    if( dev_affin_ext ) printf( "\n" );
//                }
//#endif
//
                clGetDeviceInfoUint( devices[t], CL_DEVICE_REFERENCE_COUNT, "CL_DEVICE_REFERENCE_COUNT" );
            }
#endif

#if defined( CL_VERSION_2_0 )
            if( device_opencl_c_version >= HGPU_OPENCL_2_0 )
            {
                cl_device_svm_capabilities svm_cap = 0;
                HGPU_GPU_error_message( clGetDeviceInfo( devices[t], CL_DEVICE_SVM_CAPABILITIES, sizeof(svm_cap), &svm_cap, NULL ), "clGetDeviceInfo failed" );
                printf( "CL_DEVICE_SVM_CAPABILITIES:\n" );
                    if( svm_cap & CL_DEVICE_SVM_COARSE_GRAIN_BUFFER ) printf( HGPU_OUT_FMT_TN, "CL_DEVICE_SVM_COARSE_GRAIN_BUFFER" );
                    if( svm_cap & CL_DEVICE_SVM_FINE_GRAIN_BUFFER   ) printf( HGPU_OUT_FMT_TN, "CL_DEVICE_SVM_FINE_GRAIN_BUFFER"   );
                    if( svm_cap & CL_DEVICE_SVM_FINE_GRAIN_SYSTEM   ) printf( HGPU_OUT_FMT_TN, "CL_DEVICE_SVM_FINE_GRAIN_SYSTEM"   );
                    if( svm_cap & CL_DEVICE_SVM_ATOMICS             ) printf( HGPU_OUT_FMT_TN, "CL_DEVICE_SVM_ATOMICS"             );

                clGetDeviceInfoUint( devices[t], CL_DEVICE_PREFERRED_PLATFORM_ATOMIC_ALIGNMENT, "CL_DEVICE_PREFERRED_PLATFORM_ATOMIC_ALIGNMENT" );
                clGetDeviceInfoUint( devices[t], CL_DEVICE_PREFERRED_GLOBAL_ATOMIC_ALIGNMENT,   "CL_DEVICE_PREFERRED_GLOBAL_ATOMIC_ALIGNMENT"   );
                clGetDeviceInfoUint( devices[t], CL_DEVICE_PREFERRED_LOCAL_ATOMIC_ALIGNMENT,    "CL_DEVICE_PREFERRED_LOCAL_ATOMIC_ALIGNMENT"    );
            }
#endif

            HGPU_GPU_error_message( clGetDeviceInfo( devices[t], CL_DEVICE_EXECUTION_CAPABILITIES, sizeof(device_exec_cap), &device_exec_cap, NULL ), "clGetDeviceInfo failed" );
            printf( HGPU_OUT_FMT_NSTR, "CL_DEVICE_EXECUTION_CAPABILITIES" );
                if( device_exec_cap & CL_EXEC_KERNEL        ) printf( "CL_EXEC_KERNEL "        );
                if( device_exec_cap & CL_EXEC_NATIVE_KERNEL ) printf( "CL_EXEC_NATIVE_KERNEL " );
            printf( "\n" );

            HGPU_GPU_error_message( clGetDeviceInfo( devices[t], CL_DEVICE_QUEUE_PROPERTIES, sizeof(comqueue_properties), &comqueue_properties, NULL ), "clGetDeviceInfo failed" );
            printf( HGPU_OUT_FMT_NSTR, "CL_DEVICE_QUEUE_PROPERTIES" );
                if( comqueue_properties & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE ) printf( "CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE " );
                if( comqueue_properties & CL_QUEUE_PROFILING_ENABLE              ) printf( "CL_QUEUE_PROFILING_ENABLE "              );
            printf( "\n" );

#if defined( CL_VERSION_2_0 )
            if( device_opencl_c_version >= HGPU_OPENCL_2_0 )
            {
                clGetDeviceInfoUint( devices[t], CL_DEVICE_QUEUE_ON_DEVICE_PREFERRED_SIZE, "CL_DEVICE_QUEUE_ON_DEVICE_PREFERRED_SIZE" );
                clGetDeviceInfoUint( devices[t], CL_DEVICE_QUEUE_ON_DEVICE_MAX_SIZE,       "CL_DEVICE_QUEUE_ON_DEVICE_MAX_SIZE"       );
                clGetDeviceInfoUint( devices[t], CL_DEVICE_MAX_ON_DEVICE_QUEUES,           "CL_DEVICE_MAX_ON_DEVICE_QUEUES"           );
                clGetDeviceInfoUint( devices[t], CL_DEVICE_MAX_ON_DEVICE_EVENTS,           "CL_DEVICE_MAX_ON_DEVICE_EVENTS"           );
            }
#endif

            HGPU_GPU_error_message( clGetDeviceInfo( devices[t], CL_DEVICE_PLATFORM, sizeof(platform2), &platform2, NULL), "clGetDeviceInfo failed" );
            printf( HGPU_OUT_FMT_LONGHEX, "CL_DEVICE_PLATFORM", (long long unsigned int) platform2 );
        }
        delete[] devices;
    }
    delete[] platforms;
    printf( "\n\n" );
}


// ------------------------------------------------------------- AMD
// CL_DEVICE_PARTITION_EQUALLY_EXT
// CL_DEVICE_PARTITION_BY_COUNTS_EXT
// CL_DEVICE_PARTITION_BY_NAMES_EXT
// CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN_EXT
// CL_DEVICE_PARENT_DEVICE_EXT

// CL_DEVICE_PARTITION_TYPES_EXT
// 
// CL_DEVICE_REFERENCE_COUNT_EXT
// CL_DEVICE_PARTITION_STYLE_EXT
// CL_DEVICE_SVM_CAPABILITIES_AMD
// CL_DEVICE_SVM_COARSE_GRAIN_BUFFER_AMD
// CL_DEVICE_SVM_FINE_GRAIN_BUFFER_AMD
// CL_DEVICE_SVM_FINE_GRAIN_SYSTEM_AMD
// CL_DEVICE_SVM_ATOMICS_AMD


// ------------------------------------------------------------- NVIDIA


// ------------------------------------------------------------- INTEL


// ------------------------------------------------------------- OTHER
