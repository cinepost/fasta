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
#ifndef PXR_IMAGING_PLUGIN_HD_Fasta_RENDER_PARAM_H
#define PXR_IMAGING_PLUGIN_HD_Fasta_RENDER_PARAM_H

#include "pxr/pxr.h"
#include "pxr/imaging/hd/renderDelegate.h"
#include "pxr/imaging/hd/renderThread.h"

#include "fasta_lib/renderer.h"

PXR_NAMESPACE_OPEN_SCOPE

///
/// \class HdFastaRenderParam
///
/// The render delegate can create an object of type HdRenderParam, to pass
/// to each prim during Sync(). HdFasta uses this class to pass top-level
/// Fasta state around.
/// 
class HdFastaRenderParam final : public HdRenderParam {
public:
    HdFastaRenderParam(FstRenderer *renderer,
                        HdRenderThread *renderThread,
                        std::atomic<int> *sceneVersion)
        : _renderer(renderer)
        , _renderThread(renderThread)
        ,_sceneVersion(sceneVersion)
        {}
    virtual ~HdFastaRenderParam() = default;

    /// Accessor for the top-level Fasta scene.
    //RTCScene AcquireSceneForEdit() {
    //    _renderThread->StopRender();
    //    (*_sceneVersion)++;
    //    return _scene;
    //}
    /// Accessor for the top-level Fasta renderer.
    XN_Renderer GetFastaRenderer() { return _renderer; }

private:
    /// A handle to the top-level Fasta renderer.
    FstRenderer *_renderer;
    /// A handle to the global render thread.
    HdRenderThread *_renderThread;
    /// A version counter for edits to _scene.
    std::atomic<int> *_sceneVersion;
    /// version of xn model.  Used for tracking image changes
    std::atomic<int> _modelVersion { 1 };
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // PXR_IMAGING_PLUGIN_HD_Fasta_RENDER_PARAM_H
