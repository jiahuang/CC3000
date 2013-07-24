//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifndef __DRIVER_PATCH_H__
#define __DRIVER_PATCH_H__

#ifdef  __cplusplus
extern "C" {
#endif

extern void driver_patch(void);

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef  __cplusplus
}
#endif // __cplusplus

//#endif
#endif // __DRIVER_PATCH_H__
