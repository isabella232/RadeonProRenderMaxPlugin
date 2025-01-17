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

#include "FireRenderMtlBase.h"

FIRERENDER_NAMESPACE_BEGIN;

enum FRWardMtl_TexmapId {
	FRWardMtl_TEXMAP_COLOR = 0,
	FRWardMtl_TEXMAP_ROUGHNESSX = 1,
	FRWardMtl_TEXMAP_ROUGHNESSY = 2,
	FRWardMtl_TEXMAP_NORMAL = 3,
	FRWardMtl_TEXMAP_ROTATION = 4,
};

enum FRWardMtl_ParamID : ParamID {
	FRWardMtl_COLOR = 1000,
	FRWardMtl_ROTATION = 1001,
	FRWardMtl_ROUGHNESSX = 1002,
	FRWardMtl_ROUGHNESSY = 1003,
	FRWardMtl_COLOR_TEXMAP = 1004,
	FRWardMtl_ROUGHNESSX_TEXMAP = 1005,
	FRWardMtl_ROUGHNESSY_TEXMAP = 1006,
	FRWardMtl_NORMALMAP = 1007,
	FRWardMtl_ROTATION_TEXMAP = 1008,
};

BEGIN_DECLARE_FRMTLCLASSDESC(WardMtl, L"RPR Ward Material", FIRERENDER_WARDMTL_CID)
END_DECLARE_FRMTLCLASSDESC()

BEGIN_DECLARE_FRMTL(WardMtl)

virtual Color GetDiffuse(int mtlNum, BOOL backFace) override {
	return GetFromPb<Color>(pblock, FRWardMtl_COLOR);
}

END_DECLARE_FRMTL(WardMtl)

FIRERENDER_NAMESPACE_END;
