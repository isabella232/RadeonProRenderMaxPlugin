/**********************************************************************
Copyright 2020 Advanced Micro Devices, Inc
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
********************************************************************/

#pragma once

#include "Common.h"
#include "parser/SceneParser.h"
#include "parser/RenderParameters.h"

class IFireRenderLight
{
public:
	virtual void CreateSceneLight(
		TimeValue t, const FireRender::ParsedNode& node, frw::Scope scope,
		FireRender::SceneAttachCallback* sceneAttachCallback=NULL ) = 0;
	virtual bool DisplayLight(TimeValue t, INode* inode, ViewExp *vpt, int flags) = 0;
	virtual bool CalculateBBox(void) = 0;
};
