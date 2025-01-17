// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/painting/scene/scene_shader.h"

#include <memory>
#include <utility>

#include "flutter/display_list/display_list_color_source.h"
#include "flutter/display_list/display_list_tile_mode.h"
#include "flutter/lib/ui/dart_wrapper.h"
#include "flutter/lib/ui/painting/scene/scene_node.h"
#include "flutter/lib/ui/ui_dart_state.h"
#include "impeller/geometry/scalar.h"
#include "impeller/geometry/size.h"

#include "third_party/tonic/converter/dart_converter.h"
#include "third_party/tonic/dart_args.h"
#include "third_party/tonic/dart_binding_macros.h"
#include "third_party/tonic/dart_library_natives.h"
#include "third_party/tonic/typed_data/typed_list.h"

namespace flutter {

IMPLEMENT_WRAPPERTYPEINFO(ui, SceneShader);

SceneShader::SceneShader(fml::RefPtr<SceneNode> scene_node)
    : scene_node_(std::move(scene_node)) {}

void SceneShader::Create(Dart_Handle wrapper, Dart_Handle scene_node_handle) {
  auto* scene_node =
      tonic::DartConverter<SceneNode*>::FromDart(scene_node_handle);
  if (!scene_node) {
    return;
  }

  auto res = fml::MakeRefCounted<SceneShader>(fml::Ref(scene_node));
  res->AssociateWithDartWrapper(wrapper);
}

void SceneShader::SetCameraTransform(const tonic::Float64List& matrix4) {
  camera_transform_ =
      impeller::Matrix(matrix4[0], matrix4[1], matrix4[2], matrix4[3],    //
                       matrix4[4], matrix4[5], matrix4[6], matrix4[7],    //
                       matrix4[8], matrix4[9], matrix4[10], matrix4[11],  //
                       matrix4[12], matrix4[13], matrix4[14], matrix4[15]);
}

static impeller::Matrix DefaultCameraTransform() {
  // TODO(bdero): There's no way to know what the draw area will be yet, so
  //              make the DlSceneColorSource camera transform optional and
  //              defer this default (or parameterize the camera instead).
  return impeller::Matrix::MakePerspective(
             impeller::Degrees(45), impeller::ISize{800, 600}, 0.1, 1000) *
         impeller::Matrix::MakeLookAt({0, 0, -5}, {0, 0, 0}, {0, 1, 0});
}

std::shared_ptr<DlColorSource> SceneShader::shader(DlImageSampling sampling) {
  FML_CHECK(scene_node_);

  if (!scene_node_->node_) {
    return nullptr;
  }

  // TODO(bdero): Gather the mutation log and include it in the scene color
  // source.

  return std::make_shared<DlSceneColorSource>(scene_node_->node_,
                                              camera_transform_.IsIdentity()
                                                  ? DefaultCameraTransform()
                                                  : camera_transform_);
}

void SceneShader::Dispose() {
  scene_node_ = nullptr;
  ClearDartWrapper();
}

SceneShader::~SceneShader() = default;

}  // namespace flutter
