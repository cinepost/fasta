//
// Copyright 2018 Intel
//
// Licensed under the Apache License, Version 2.0 (the "Apache License")
// with the following modification; you may not use this file except in
// compliance with the Apache License and the following modification to it:
// Section 6. Trademarks. is deleted and replaced with:
//
// 6. Trademarks. This License does not grant permission to use the trade
//    names, trademarks, service marks, or product names of the Licensor
//    and its affiliates, except as required to comply with Section 4(c) of
//    the License and to reproduce the content of the NOTICE file.
//
// You may obtain a copy of the Apache License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the Apache License with the above modification is
// distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied. See the Apache License for the specific
// language governing permissions and limitations under the Apache License.
//
#include "pxr/imaging/hdXenon/config.h"

#include "pxr/base/tf/envSetting.h"
#include "pxr/base/tf/instantiateSingleton.h"

#include <iostream>

PXR_NAMESPACE_OPEN_SCOPE

// Instantiate the config singleton.
TF_INSTANTIATE_SINGLETON(HdXenonConfig);

// Each configuration variable has an associated environment variable.
// The environment variable macro takes the variable name, a default value,
// and a description...
// clang-format off
TF_DEFINE_ENV_SETTING(HDXenon_SAMPLES_PER_FRAME, -1,
        "Raytraced samples per pixel per frame (must be >= 1)");

TF_DEFINE_ENV_SETTING(HDXenon_SAMPLES_TO_CONVERGENCE, 100,
        "Samples per pixel before we stop rendering (must be >= 1)");

TF_DEFINE_ENV_SETTING(HDXenon_AMBIENT_OCCLUSION_SAMPLES, 0,
        "Ambient occlusion samples per camera ray (must be >= 0; a value of 0 disables ambient occlusion)");

TF_DEFINE_ENV_SETTING(HDXenon_CAMERA_LIGHT_INTENSITY, 300,
        "Intensity of the camera light, specified as a percentage of <1,1,1>.");

TF_DEFINE_ENV_SETTING(HDXenon_PRINT_CONFIGURATION, 0,
        "Should HdXenon print configuration on startup? (values > 0 are true)");

TF_DEFINE_ENV_SETTING(HDXenon_USE_PATH_TRACING, 1,
        "Should HdXenon use path tracing");

TF_DEFINE_ENV_SETTING(HDXenon_INIT_ARGS, "",
        "Initialization arguments sent to Xenon");

TF_DEFINE_ENV_SETTING(HDXenon_USE_DENOISER, 0,
        "Xenon uses denoiser");

TF_DEFINE_ENV_SETTING(HDXenon_FORCE_QUADRANGULATE, 0,
        "Xenon force Quadrangulate meshes for debug");

HdXenonConfig::HdXenonConfig()
{
    // Read in values from the environment, clamping them to valid ranges.
    samplesPerFrame = std::max(-1,
            TfGetEnvSetting(HDXenon_SAMPLES_PER_FRAME));
    samplesToConvergence = std::max(1,
            TfGetEnvSetting(HDXenon_SAMPLES_TO_CONVERGENCE));
    ambientOcclusionSamples = std::max(0,
            TfGetEnvSetting(HDXenon_AMBIENT_OCCLUSION_SAMPLES));
    cameraLightIntensity = (std::max(100,
            TfGetEnvSetting(HDXenon_CAMERA_LIGHT_INTENSITY)) / 100.0f);
    usePathTracing =TfGetEnvSetting(HDXenon_USE_PATH_TRACING);
    initArgs =TfGetEnvSetting(HDXenon_INIT_ARGS);
    useDenoiser = TfGetEnvSetting(HDXenon_USE_DENOISER);
    forceQuadrangulate = TfGetEnvSetting(HDXenon_FORCE_QUADRANGULATE);

    if (TfGetEnvSetting(HDXenon_PRINT_CONFIGURATION) > 0) {
        std::cout
            << "HdXenon Configuration: \n"
            << "  samplesPerFrame            = "
            <<    samplesPerFrame         << "\n"
            << "  samplesToConvergence       = "
            <<    samplesToConvergence    << "\n"
            << "  ambientOcclusionSamples    = "
            <<    ambientOcclusionSamples << "\n"
            << "  cameraLightIntensity      = "
            <<    cameraLightIntensity   << "\n"
            << "  initArgs                  = "
            <<    initArgs   << "\n"
            ;
    }
}

/*static*/
const HdXenonConfig&
HdXenonConfig::GetInstance()
{
    return TfSingleton<HdXenonConfig>::GetInstance();
}

PXR_NAMESPACE_CLOSE_SCOPE
