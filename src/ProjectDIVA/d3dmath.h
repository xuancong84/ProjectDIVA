#pragma once

#include <d3dx9math.h>

FLOAT DistanceLine2Point(const D3DXVECTOR3 &eye, const D3DXVECTOR3 &dir, const D3DXVECTOR3 &point);

FLOAT AngleBetween2Vectors(const D3DXVECTOR3 &v1, const D3DXVECTOR3 &v2);

bool LineIntersectSphere(const D3DXVECTOR3 &eye, const D3DXVECTOR3 &dir, const D3DXVECTOR3 &point, FLOAT radius);

bool LineIntersectTriangle(const D3DXVECTOR3 &eye, const D3DXVECTOR3 &dir,
	const D3DXVECTOR3 &p1, const D3DXVECTOR3 &p2, const D3DXVECTOR3 &p3);

D3DXVECTOR3 LineIntersectTriangleWhere(const D3DXVECTOR3 &eye, const D3DXVECTOR3 &dir,
	const D3DXVECTOR3 &p1, const D3DXVECTOR3 &p2, const D3DXVECTOR3 &p3);

void ScreenXY2Ray(	D3DXVECTOR3 &outDirv, const D3DXVECTOR3 &eyev,
	const D3DXVECTOR3 &centerv, const D3DXVECTOR3 &upv,
	int mouse_hit_x, int mouse_hit_y, int Width, int Height, float Yangle);

D3DXVECTOR3 findLongestEdgeCenter(D3DXVECTOR3 &v0, D3DXVECTOR3 &v1, D3DXVECTOR3 &v2);

