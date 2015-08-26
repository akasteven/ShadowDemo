#pragma once

#include <D3DX11.h>
#include <xnamath.h>
#include "Light.h"

struct CBNeverChanges
{
	DirectionalLight dirLight;
	float shadowMapSize;
	XMFLOAT3 pad3;
};

struct CBOnResize
{
	XMMATRIX mProjection;
};

struct CBPerFrame
{
	XMFLOAT3 eyePos;
	float pad;
};

struct CBPerObject
{
	XMMATRIX matWorld;
	XMMATRIX matWVP;
	Material material;
	XMMATRIX matWorldInvTranspose;
	XMMATRIX matLightWVPT;
	int isInstancing;
	XMFLOAT3 padding;
};

struct CBPerObjectShadow
{
	XMMATRIX lightWVP;
	int isInstancing;
	XMFLOAT3 padding;
};

struct CBPerFrameScreenQuad
{
	XMMATRIX wvp;
};