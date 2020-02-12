//
// Copyright 2018 Pixar
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
#ifndef PXR_IMAGING_PLUGIN_HD_FASTA_RENDERER_H
#define PXR_IMAGING_PLUGIN_HD_FASTA_RENDERER_H

#include <random>
#include <atomic>

#include "pxr/pxr.h"

#include "pxr/imaging/hd/renderThread.h"
#include "pxr/imaging/hd/renderPassState.h"
#include "pxr/imaging/hd/tokens.h"

#include "pxr/base/gf/matrix4d.h"

#include "fasta_lib/renderer.h"


PXR_NAMESPACE_OPEN_SCOPE

/// \class HdFastaRenderer
///
/// HdFastaRenderer implements a renderer on top of Fasta's raycasting
/// abilities.  This is currently a very simple renderer.  It breaks the
/// framebuffer into tiles for multithreading; sends out jittered camera
/// rays; and implements the following shading:
///  - Colors via the "color" primvar.
///  - Lighting via N dot Camera-ray, simulating a point light at the camera
///    origin.
///  - Ambient occlusion.
///
class HdFastaRenderer final: public FstRenderer {
public:
    /// Renderer constructor.
    HdFastaRenderer();

    /// Renderer destructor.
    ~HdFastaRenderer();

    /// Specify a new viewport size for the sample/color buffer.
    ///   \param width The new viewport width.
    ///   \param height The new viewport height.
    void SetViewport(unsigned int width, unsigned int height);

    /// Set the camera to use for rendering.
    ///   \param viewMatrix The camera's world-to-view matrix.
    ///   \param projMatrix The camera's view-to-NDC projection matrix.
    void SetCamera(const GfMatrix4d& viewMatrix, const GfMatrix4d& projMatrix);

    /// Set the aov bindings to use for rendering.
    ///   \param aovBindings A list of aov bindings.
    void SetAovBindings(HdRenderPassAovBindingVector const &aovBindings);

    /// Get the aov bindings being used for rendering.
    ///   \return the current aov bindings.
    HdRenderPassAovBindingVector const& GetAovBindings() const {
        return _aovBindings;
    }

    /// Set how many samples to render before considering an image converged.
    ///   \param samplesToConvergence How many samples are needed, per-pixel,
    ///                               before the image is considered finished.
    void SetSamplesToConvergence(int samplesToConvergence);

    /// Set how many samples to use for ambient occlusion.
    ///   \param ambientOcclusionSamples How many samples are needed for
    ///                                  ambient occlusion? 0 = disable.
    void SetAmbientOcclusionSamples(int ambientOcclusionSamples);

    /// Sets whether to use scene colors while rendering.
    ///   \param enableSceneColors Whether drawing should sample color, or draw
    ///                            everything as white.
    void SetEnableSceneColors(bool enableSceneColors);

    /// Rendering entrypoint: add one sample per pixel to the whole sample
    /// buffer, and then loop until the image is converged.  After each pass,
    /// the image will be resolved into a color buffer.
    ///   \param renderThread A handle to the render thread, used for checking
    ///                       for cancellation and locking the color buffer.
    void Render(HdRenderThread *renderThread);

    /// Clear the bound aov buffers (typically before rendering).
    void Clear();

    /// Mark the aov buffers as unconverged.
    void MarkAovBuffersUnconverged();

    /// Get the number of samples completed so far.
    int GetCompletedSamples() const;

private:
    // The bound aovs for this renderer.
    HdRenderPassAovBindingVector _aovBindings;
    // Parsed AOV name tokens.
    HdParsedAovTokenVector _aovNames;

    // Do the aov bindings need to be re-validated?
    bool _aovBindingsNeedValidation;
    // Are the aov bindings valid?
    bool _aovBindingsValid;

    // The width of the viewport we're rendering into.
    unsigned int _width;
    // The height of the viewport we're rendering into.
    unsigned int _height;

    // View matrix: world space to camera space.
    GfMatrix4d _viewMatrix;
    // Projection matrix: camera space to NDC space.
    GfMatrix4d _projMatrix;
    // The inverse view matrix: camera space to world space.
    GfMatrix4d _inverseViewMatrix;
    // The inverse projection matrix: NDC space to camera space.
    GfMatrix4d _inverseProjMatrix;

    // How many samples should we render to convergence?
    int _samplesToConvergence;
    // How many samples should we use for ambient occlusion?
    int _ambientOcclusionSamples;
    // Should we enable scene colors?
    bool _enableSceneColors;
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // PXR_IMAGING_PLUGIN_HD_FASTA_RENDERER_H
