// Copyright (c) 2024 M. Walky
// Licensed under the MIT license (https://opensource.org/license/mit/)

////////////////////////////////
//~ mwalky: [h]

#pragma comment(lib, "user32")
#pragma comment(lib, "dxgi")
#pragma comment(lib, "d3d12")

#include "stdio.h"

#include "windows.h"
#include "initguid.h"

#include "dxgi1_6.h"
#include "third_party/d3d12/d3d12.h"

extern "C" __declspec(dllexport) extern UINT D3D12SDKVersion = 613;
extern "C" __declspec(dllexport) extern PSZ D3D12SDKPath = u8".\\third_party\\d3d12";

#define D3D12MA_D3D12_HEADERS_ALREADY_INCLUDED
#include "d3d12ma/d3d12ma.h"

////////////////////////////////
//~ mwalky: [cpp]

#include "d3d12ma/d3d12ma.cpp"

////////////////////////////////
//~ mwalky: Globals

IDXGIAdapter1 *dxgi_adapter;
IDXGIFactory6 *dxgi_factory;
ID3D12Debug1 *debug;
ID3D12Device *device;
D3D12MA::Allocator *allocator;

int
main()
{
 HRESULT error = 0;
 
 //- mwalky: init debug layer
#if BUILD_DEBUG
 error = D3D12GetDebugInterface(__uuidof(ID3D12Debug1), (void **)(&debug));
 debug->EnableDebugLayer();
 debug->SetEnableGPUBasedValidation(1);
 debug->SetEnableSynchronizedCommandQueueValidation(1);
 
 if(FAILED(error))
 {
  printf("Oh, `D3D12GetDebugInterface` (or something related) failed!\n");
  return -1;
 }
#endif
 
 //- mwalky: init factory
 UINT creation_flags = 0;
#if BUILD_DEBUG
 creation_flags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
 error = CreateDXGIFactory2(creation_flags, __uuidof(IDXGIFactory6), (void **)(&dxgi_factory));
 if(FAILED(error))
 {
  printf("Oh, `CreateDXGIFactory2` failed!\n");
  return -1;
 }
 
 //- mwalky: enum adapters
 {
  for(UINT64 adapter_idx = 0; dxgi_adapter == 0; adapter_idx += 1)
  {
   dxgi_factory->EnumAdapterByGpuPreference(adapter_idx, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, __uuidof(IDXGIAdapter1), (void **)(&dxgi_adapter));
   if(FAILED(error))
   {
    printf("Oh, `EnumAdapterByGpuPreference` failed!\n");
    return -1;
   }
   
   error = D3D12CreateDevice(dxgi_adapter, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), 0);
  }
  
  // mwalky: do not care about WARP driver, simply fail on error
  if(FAILED(error))
  {
   printf("Oh, `D3D12CreateDevice` failed!\n");
   return -1;
  }
 }
 
 //- mwalky: create device
 error = D3D12CreateDevice(dxgi_adapter, D3D_FEATURE_LEVEL_12_1, __uuidof(ID3D12Device), (void **)(&device));
 if(FAILED(error))
 {
  printf("Oh, `D3D12CreateDevice` failed!\n");
  return -1;
 }
 
 //- mwalky: allocator
 {
  D3D12MA::ALLOCATOR_DESC desc = {};
  {
   desc.Flags = D3D12MA::ALLOCATOR_FLAG_SINGLETHREADED;
   desc.pDevice = device;
   desc.pAdapter = dxgi_adapter;
  }
  error = D3D12MA::CreateAllocator(&desc, &allocator);
 }
 if(FAILED(error))
 {
  printf("Oh, `D3D12MA::CreateAllocator` failed!\n");
  return -1;
 }
 
 //- mwalky: query for a d3d12 options 13
 {
  D3D12_FEATURE_DATA_D3D12_OPTIONS13 feature_data = {0};
  device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS13, &feature_data, sizeof(D3D12_FEATURE_DATA_D3D12_OPTIONS13));
  printf("UnrestrictedBufferTextureCopyPitchSupported                                -> %s\n", feature_data.UnrestrictedBufferTextureCopyPitchSupported  ? "TRUE" : "FALSE");
  printf("UnrestrictedVertexElementAlignmentSupported                                -> %s\n", feature_data.UnrestrictedVertexElementAlignmentSupported   ? "TRUE" : "FALSE");
  printf("InvertedViewportHeightFlipsYSupported                                      -> %s\n", feature_data.InvertedViewportHeightFlipsYSupported   ? "TRUE" : "FALSE");
  printf("InvertedViewportDepthFlipsZSupported                                       -> %s\n", feature_data.InvertedViewportDepthFlipsZSupported  ? "TRUE" : "FALSE");
  printf("TextureCopyBetweenDimensionsSupported                                      -> %s\n", feature_data.TextureCopyBetweenDimensionsSupported   ? "TRUE" : "FALSE");
  printf("AlphaBlendFactorSupported                                                  -> %s\n\n", feature_data.AlphaBlendFactorSupported  ? "TRUE" : "FALSE");
 }
 
 //- mwalky: query d3d12 options from allocator
 {
  const D3D12_FEATURE_DATA_D3D12_OPTIONS &options = allocator->GetD3D12Options();
  printf("DoublePrecisionFloatShaderOps                                              -> %s\n", options.DoublePrecisionFloatShaderOps ? "TRUE" : "FALSE");
  printf("OutputMergerLogicOp                                                        -> %s\n", options.OutputMergerLogicOp ? "TRUE" : "FALSE");
  printf("MinPrecisionSupport                                                        -> %d\n", options.MinPrecisionSupport);
  printf("TiledResourcesTier                                                         -> %d\n", options.TiledResourcesTier);
  printf("ResourceBindingTier                                                        -> %d\n", options.ResourceBindingTier);
  printf("PSSpecifiedStencilRefSupported                                             -> %s\n", options.PSSpecifiedStencilRefSupported ? "TRUE" : "FALSE");
  printf("TypedUAVLoadAdditionalFormats                                              -> %s\n", options.TypedUAVLoadAdditionalFormats ? "TRUE" : "FALSE");
  printf("ROVsSupported                                                              -> %s\n", options.ROVsSupported ? "TRUE" : "FALSE");
  printf("ConservativeRasterizationTier                                              -> %d\n", options.ConservativeRasterizationTier);
  printf("MaxGPUVirtualAddressBitsPerResource                                        -> %u\n", options.MaxGPUVirtualAddressBitsPerResource);
  printf("StandardSwizzle64KBSupported                                               -> %s\n", options.StandardSwizzle64KBSupported ? "TRUE" : "FALSE");
  printf("CrossNodeSharingTier                                                       -> %d\n", options.CrossNodeSharingTier);
  printf("CrossAdapterRowMajorTextureSupported                                       -> %s\n", options.CrossAdapterRowMajorTextureSupported ? "TRUE" : "FALSE");
  printf("VPAndRTArrayIndexFromAnyShaderFeedingRasterizerSupportedWithoutGSEmulation -> %s\n", options.VPAndRTArrayIndexFromAnyShaderFeedingRasterizerSupportedWithoutGSEmulation ? "TRUE" : "FALSE");
  printf("ResourceHeapTier                                                           -> %d\n", options.ResourceHeapTier);
 }
 
 return 0;
}
