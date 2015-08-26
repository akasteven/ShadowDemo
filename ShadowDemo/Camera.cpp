#include "Camera.h"


Camera::Camera() 
:mPos(0.0f, 0.0f, 0.0f),
mRight(1.0f, 0.0f, 0.0f),
mUp(0.0f, 1.0f, 0.0f),
mLook(0.0f, 0.0f, 1.0f)
{
}

Camera::~Camera()
{
}

void Camera::MoveForward( float x )
{
	XMVECTOR s = XMVectorReplicate(x);
	XMVECTOR t = XMLoadFloat3(&mLook);
	XMVECTOR p = XMLoadFloat3(&mPos);
	XMStoreFloat3(&mPos, XMVectorMultiplyAdd(s, t, p));
}

void Camera::MoveRight(float x)
{
	XMVECTOR s = XMVectorReplicate(x);
	XMVECTOR r = XMLoadFloat3(&mRight);
	XMVECTOR p = XMLoadFloat3(&mPos);
	XMStoreFloat3(&mPos, XMVectorMultiplyAdd(s, r, p));
}

void Camera::Elevate(float x)
{
	XMVECTOR s = XMVectorReplicate(x);
	XMVECTOR u = XMLoadFloat3(&mUp);
	XMVECTOR p = XMLoadFloat3(&mPos);
	XMStoreFloat3(&mPos, XMVectorMultiplyAdd(s, u, p));
}

void Camera::Pitch(float x)
{
	XMMATRIX rotate = XMMatrixRotationAxis(XMLoadFloat3(&mRight), x);
	XMStoreFloat3(&mLook, XMVector3TransformNormal(XMLoadFloat3(&mLook), rotate));
	XMStoreFloat3(&mUp, XMVector3TransformNormal(XMLoadFloat3(&mUp), rotate));
}


void Camera::Roll(float x)
{
	XMMATRIX rotate = XMMatrixRotationAxis(XMLoadFloat3(&mLook), x);
	XMStoreFloat3(&mRight, XMVector3TransformNormal(XMLoadFloat3(&mRight), rotate));
	XMStoreFloat3(&mUp, XMVector3TransformNormal(XMLoadFloat3(&mUp), rotate));
}

void Camera::Yaw(float x)
{
	XMMATRIX rotate = XMMatrixRotationAxis(XMLoadFloat3(&mUp), x);
	XMStoreFloat3(&mRight, XMVector3TransformNormal(XMLoadFloat3(&mRight), rotate));
	XMStoreFloat3(&mLook, XMVector3TransformNormal(XMLoadFloat3(&mLook), rotate));
}

XMMATRIX Camera::GetViewMatrix() const
{
	return mView;
}


XMMATRIX Camera::GetProjMatrix() const
{
	return mProj;
}

XMMATRIX Camera::GetViewProjMatrix() const
{
	return XMMatrixMultiply(mView, mProj);
}

void Camera::Setup(float fv, float asp, float nz, float fz)
{
	fov = fv;
	aspectRatio = asp;
	nearZ = nz;
	farZ = fz;
	mProj = XMMatrixPerspectiveFovLH( fov, asp, nz, fz );
}

void Camera::Update()
{
	XMVECTOR p = XMLoadFloat3(&mPos);
	XMVECTOR r = XMLoadFloat3(&mRight);
	XMVECTOR u = XMLoadFloat3(&mUp);
	XMVECTOR t = XMLoadFloat3(&mLook);

	t = XMVector3Normalize(t);
	u = XMVector3Normalize(XMVector3Cross(t, r));
	r = XMVector3Normalize(XMVector3Cross(u, t));

	float x = -XMVectorGetX(XMVector3Dot(p, r));
	float y = -XMVectorGetX(XMVector3Dot(p, u));
	float z = -XMVectorGetX(XMVector3Dot(p, t));

	XMStoreFloat3(&mRight, r);
	XMStoreFloat3(&mUp, u);
	XMStoreFloat3(&mLook, t);

	mView = XMMATRIX(
		mRight.x, mUp.x, mLook.x, 0.0f,
		mRight.y, mUp.y, mLook.y, 0.0f,
		mRight.z, mUp.z, mLook.z, 0.0f,
		x, y, z, 1.0f );

}