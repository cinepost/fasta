//
// Copyright 2017 Pixar
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
#include "pxr/imaging/glf/glew.h"

//#include "hdFasta/config.h"
//#include "hdFasta/instancer.h"
//#include "hdFasta/renderParam.h"
//#include "hdFasta/renderPass.h"

#include "pxr/imaging/hd/extComputation.h"
#include "pxr/imaging/hd/resourceRegistry.h"
#include "pxr/imaging/hd/tokens.h"

//#include "xenon_usd/hdFasta/mesh.h"
//XXX: Add other Rprim types later
#include "pxr/imaging/hd/camera.h"
//XXX: Add other Sprim types later
#include "pxr/imaging/hd/bprim.h"
//XXX: Add bprim types

#include "fasta_usd/hdFasta/renderDelegate.h"


PXR_NAMESPACE_OPEN_SCOPE

TF_DEFINE_PUBLIC_TOKENS(HdFastaRenderSettingsTokens, HDFasta_RENDER_SETTINGS_TOKENS);

const TfTokenVector HdFastaRenderDelegate::SUPPORTED_RPRIM_TYPES =
{
    HdPrimTypeTokens->mesh,
};

const TfTokenVector HdFastaRenderDelegate::SUPPORTED_SPRIM_TYPES =
{
    HdPrimTypeTokens->camera,
    HdPrimTypeTokens->extComputation,
};

const TfTokenVector HdFastaRenderDelegate::SUPPORTED_BPRIM_TYPES =
{
    HdPrimTypeTokens->renderBuffer,
};

std::mutex HdFastaRenderDelegate::_mutexResourceRegistry;
std::atomic_int HdFastaRenderDelegate::_counterResourceRegistry;
HdResourceRegistrySharedPtr HdFastaRenderDelegate::_resourceRegistry;

/* static */
void HdFastaRenderDelegate::HandleFastaError(const int code, const char* msg) {
    // Forward RTC error messages through to hydra logging.
    /*
    switch (code) {
        case RTC_UNKNOWN_ERROR:
            TF_CODING_ERROR("Fasta unknown error: %s", msg);
            break;
        case RTC_INVALID_ARGUMENT:
            TF_CODING_ERROR("Fasta invalid argument: %s", msg);
            break;
        case RTC_INVALID_OPERATION:
            TF_CODING_ERROR("Fasta invalid operation: %s", msg);
            break;
        case RTC_OUT_OF_MEMORY:
            TF_CODING_ERROR("Fasta out of memory: %s", msg);
            break;
        case RTC_UNSUPPORTED_CPU:
            TF_CODING_ERROR("Fasta unsupported CPU: %s", msg);
            break;
        case RTC_CANCELLED:
            TF_CODING_ERROR("Fasta cancelled: %s", msg);
            break;
        default:
            TF_CODING_ERROR("Fasta invalid error code: %s", msg);
            break;
    }
    */
}

static void _RenderCallback(HdFastaRenderer *renderer, HdRenderThread *renderThread) {
    renderer->Clear();
    renderer->Render(renderThread);
}

HdFastaRenderDelegate::HdFastaRenderDelegate() : HdRenderDelegate() {
    _Initialize();
}

HdFastaRenderDelegate::HdFastaRenderDelegate( HdRenderSettingsMap const& settingsMap )
    : HdRenderDelegate(settingsMap) {
    _Initialize();
}

void HdFastaRenderDelegate::_Initialize() {
    // Initialize the settings and settings descriptors.
    _settingDescriptors.resize(4);
    _settingDescriptors[0] = { "Enable Scene Colors",
        HdFastaRenderSettingsTokens->enableSceneColors,
        VtValue(HdFastaConfig::GetInstance().useFaceColors) };
    _settingDescriptors[1] = { "Enable Ambient Occlusion",
        HdFastaRenderSettingsTokens->enableAmbientOcclusion,
        VtValue(HdFastaConfig::GetInstance().ambientOcclusionSamples > 0) };
    _settingDescriptors[2] = { "Ambient Occlusion Samples",
        HdFastaRenderSettingsTokens->ambientOcclusionSamples,
        VtValue(int(HdFastaConfig::GetInstance().ambientOcclusionSamples)) };
    _settingDescriptors[3] = { "Samples To Convergence",
        HdRenderSettingsTokens->convergedSamplesPerPixel,
        VtValue(int(HdFastaConfig::GetInstance().samplesToConvergence)) };
    _PopulateDefaultSettings(_settingDescriptors);

    // Initialize the Fasta library handle (_rtcDevice).
    _renderer.init();

    // Register our error message callback.
    //_renderer.registerErrorCallback(_rtcDevice, HandleFastaError);

    // Fasta has an internal cache for subdivision surface computations.
    // HdFasta exposes the size as an environment variable.
    //unsigned int subdivisionCache =
    //    HdFastaConfig::GetInstance().subdivisionCache;

    // Create the top-level scene.
    //
    // RTC_SCENE_DYNAMIC indicates we'll be updating the scene between draw
    // calls. RTC_INTERSECT1 indicates we'll be casting single rays, and
    // RTC_INTERPOLATE indicates we'll be storing primvars in Fasta objects
    // and querying them with rtcInterpolate.
    //
    // XXX: Investigate ray packets.
    //_rtcScene = rtcDeviceNewScene(_rtcDevice, RTC_SCENE_DYNAMIC,
    //    RTC_INTERSECT1 | RTC_INTERPOLATE);

    // Store top-level Fasta objects inside a render param that can be
    // passed to prims during Sync(). Also pass a handle to the render thread.
    _renderParam = std::make_shared<HdFastaRenderParam>(
        &_renderer, &_renderThread, &_sceneVersion);

    // Pass the scene handle to the renderer.
    //_renderer.SetScene(_rtcScene);

    // Set the background render thread's rendering entrypoint to
    // HdFastaRenderer::Render.
    _renderThread.SetRenderCallback(
        std::bind(_RenderCallback, &_renderer, &_renderThread));
    // Start the background render thread.
    _renderThread.StartThread();

    // Initialize one resource registry for all Fasta plugins
    std::lock_guard<std::mutex> guard(_mutexResourceRegistry);

    if (_counterResourceRegistry.fetch_add(1) == 0) {
        _resourceRegistry.reset( new HdResourceRegistry() );
    }
}

HdFastaRenderDelegate::~HdFastaRenderDelegate() {
    // Clean the resource registry only when it is the last Fasta delegate
    {
        std::lock_guard<std::mutex> guard(_mutexResourceRegistry);
        if (_counterResourceRegistry.fetch_sub(1) == 1) {
            _resourceRegistry.reset();
        }
    }

    _renderThread.StopThread();

    // Destroy Fasta library and scene state.
    _renderParam.reset();
}

HdRenderSettingDescriptorList
HdFastaRenderDelegate::GetRenderSettingDescriptors() const {
    return _settingDescriptors;
}

HdRenderParam*
HdFastaRenderDelegate::GetRenderParam() const {
    return _renderParam.get();
}

void HdFastaRenderDelegate::CommitResources(HdChangeTracker *tracker) {}

TfTokenVector const&
HdFastaRenderDelegate::GetSupportedRprimTypes() const {
    return SUPPORTED_RPRIM_TYPES;
}

TfTokenVector const&
HdFastaRenderDelegate::GetSupportedSprimTypes() const {
    return SUPPORTED_SPRIM_TYPES;
}

TfTokenVector const&
HdFastaRenderDelegate::GetSupportedBprimTypes() const {
    return SUPPORTED_BPRIM_TYPES;
}

HdResourceRegistrySharedPtr
HdFastaRenderDelegate::GetResourceRegistry() const {
    return _resourceRegistry;
}

HdAovDescriptor
HdFastaRenderDelegate::GetDefaultAovDescriptor(TfToken const& name) const {
    if (name == HdAovTokens->color) {
        return HdAovDescriptor(HdFormatUNorm8Vec4, true,
                               VtValue(GfVec4f(0.0f)));
    } else if (name == HdAovTokens->normal || name == HdAovTokens->Neye) {
        return HdAovDescriptor(HdFormatFloat32Vec3, false,
                               VtValue(GfVec3f(-1.0f)));
    } else if (name == HdAovTokens->depth) {
        return HdAovDescriptor(HdFormatFloat32, false, VtValue(1.0f));
    } else if (name == HdAovTokens->cameraDepth) {
        return HdAovDescriptor(HdFormatFloat32, false, VtValue(0.0f));
    } else if (name == HdAovTokens->primId ||
               name == HdAovTokens->instanceId ||
               name == HdAovTokens->elementId) {
        return HdAovDescriptor(HdFormatInt32, false, VtValue(-1));
    } else {
        HdParsedAovToken aovId(name);
        if (aovId.isPrimvar) {
            return HdAovDescriptor(HdFormatFloat32Vec3, false,
                                   VtValue(GfVec3f(0.0f)));
        }
    }

    return HdAovDescriptor();
}

VtDictionary 
HdFastaRenderDelegate::GetRenderStats() const {
    VtDictionary stats;
    stats[HdPerfTokens->numCompletedSamples.GetString()] = 
        _renderer.getCompletedSamples();
    return stats;
}

bool HdFastaRenderDelegate::IsPauseSupported() const {
    return true;
}

bool HdFastaRenderDelegate::Pause() {
    _renderThread.PauseRender();
    return true;
}

bool HdFastaRenderDelegate::Resume() {
    _renderThread.ResumeRender();
    return true;
}

HdRenderPassSharedPtr
HdFastaRenderDelegate::CreateRenderPass(HdRenderIndex *index,
                            HdRprimCollection const& collection) {
    //return HdRenderPassSharedPtr(
    //    new HdFastaRenderPass(index, collection, &_renderThread, &_renderer, &_sceneVersion));
    return nullptr;
}

HdInstancer *
HdFastaRenderDelegate::CreateInstancer(HdSceneDelegate *delegate,
                                        SdfPath const& id,
                                        SdfPath const& instancerId) {
    //return new HdFastaInstancer(delegate, id, instancerId);
    return nullptr;
}

void HdFastaRenderDelegate::DestroyInstancer(HdInstancer *instancer) {
    delete instancer;
}

HdRprim *HdFastaRenderDelegate::CreateRprim(TfToken const& typeId,
                                    SdfPath const& rprimId,
                                    SdfPath const& instancerId) {
    if (typeId == HdPrimTypeTokens->mesh) {
        //return new HdFastaMesh(rprimId, instancerId);
    } else {
        TF_CODING_ERROR("Unknown Rprim Type %s", typeId.GetText());
    }

    return nullptr;
}

void HdFastaRenderDelegate::DestroyRprim(HdRprim *rPrim) {
    delete rPrim;
}

HdSprim *HdFastaRenderDelegate::CreateSprim(TfToken const& typeId,
                                    SdfPath const& sprimId) {
    if (typeId == HdPrimTypeTokens->camera) {
        return new HdCamera(sprimId);
    } else if (typeId == HdPrimTypeTokens->extComputation) {
        return new HdExtComputation(sprimId);
    } else {
        TF_CODING_ERROR("Unknown Sprim Type %s", typeId.GetText());
    }

    return nullptr;
}

HdSprim *HdFastaRenderDelegate::CreateFallbackSprim(TfToken const& typeId) {
    // For fallback sprims, create objects with an empty scene path.
    // They'll use default values and won't be updated by a scene delegate.
    if (typeId == HdPrimTypeTokens->camera) {
        return new HdCamera(SdfPath::EmptyPath());
    } else if (typeId == HdPrimTypeTokens->extComputation) {
        return new HdExtComputation(SdfPath::EmptyPath());
    } else {
        TF_CODING_ERROR("Unknown Sprim Type %s", typeId.GetText());
    }

    return nullptr;
}

void HdFastaRenderDelegate::DestroySprim(HdSprim *sPrim) {
    delete sPrim;
}

HdBprim *HdFastaRenderDelegate::CreateBprim(TfToken const& typeId,
                                    SdfPath const& bprimId) {
    if (typeId == HdPrimTypeTokens->renderBuffer) {
        //return new HdFastaRenderBuffer(bprimId);
    } else {
        TF_CODING_ERROR("Unknown Bprim Type %s", typeId.GetText());
    }
    return nullptr;
}

HdBprim *HdFastaRenderDelegate::CreateFallbackBprim(TfToken const& typeId){
    if (typeId == HdPrimTypeTokens->renderBuffer) {
        //return new HdFastaRenderBuffer(SdfPath::EmptyPath());
    } else {
        TF_CODING_ERROR("Unknown Bprim Type %s", typeId.GetText());
    }
    return nullptr;
}

void HdFastaRenderDelegate::DestroyBprim(HdBprim *bPrim) {
    delete bPrim;
}

PXR_NAMESPACE_CLOSE_SCOPE
