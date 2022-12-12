﻿#pragma once
#include <cstring>
#include <assert.h>
#include <stdlib.h>
#include <algorithm>
#include <vector>
#include <cmath>
#include <limits>
#include "geometry.h"

//typedef unsigned int IUINT32;
typedef unsigned int UI32;


// in opengl right-hand
/*
	blender obj file is clock-wise
		 v1
		 /\
		/__\
	   /_P__\
	v0/______\ v2
	line1 = v2-v0, line2 = v1-v0
	if line2 cross line1 > 0 then is front
*/

// 基礎繪製
class Device
{
public:
	int width;                  
	int height;                 
	UI32** framebuffer;			// pixelbuffer：framebuffer[y] 代表第 y行
	float** zbuffer;            // 深度緩衝：zbuffer[y][x]
	float zbufferMax;
	UI32** texture;             // 紋理：texture[y][x]
	int tex_width;              // 紋理寬度
	int tex_height;             // 紋理高度
	float max_u;                // 紋理最大寬度：tex_width - 1
	float max_v;                // 紋理最大高度：tex_height - 1
	int render_state;           
	UI32 background;            
	std::vector<std::vector<UI32>> bery_texture;

	std::vector<float> zbuffer2;

	void init(int width, int height, void* fb) {
		int need = sizeof(void*) * (height * 2 + 1024) + width * height * 8;
		char* ptr = (char*)malloc(need + 64);
		char* framebuf, * zbuf;
		int j;
		assert(ptr);
		framebuffer = (UI32**)ptr;
		zbuffer = (float**)(ptr + sizeof(void*) * height);
		zbuffer2.resize(width * height);// index = (height-y)*width + x;

		ptr += sizeof(void*) * height * 2;
		texture = (UI32**)ptr;
		bery_texture.resize(256, std::vector<UI32>(256, 0));
		ptr += sizeof(void*) * 1024;
		framebuf = (char*)ptr;
		zbuf = (char*)ptr + width * height * 4;
		ptr += width * height * 8;
		if (fb != NULL) framebuf = (char*)fb;
		for (j = 0; j < height; j++) {
			framebuffer[j] = (UI32*)(framebuf + width * 4 * j);
			zbuffer[j] = (float*)(zbuf + width * 4 * j);
		}
		zbufferMax = -(std::numeric_limits<float>::max)();
		texture[0] = (UI32*)ptr;
		texture[1] = (UI32*)(ptr + 16);
		memset(texture[0], 0, 64);
		tex_width = 2;
		tex_height = 2;
		max_u = 1.0f;
		max_v = 1.0f;
		this->width = width;
		this->height = height;
		background = 0xFFFFFF;
	}
	void destroy() {
		if (framebuffer)
			free(framebuffer);
		framebuffer = NULL;
		zbuffer = NULL;
		texture = NULL;
	}
	void clear(int mode = 0) {
		int y, x;
		for (y = 0; y < height; y++) {
			UI32* dst = framebuffer[y];
			UI32 cc = (height - 1 - y) * 230 / (height - 1);
			cc = (cc << 16) | (cc << 8) | cc;
			if (mode == 0) cc = background;
			for (x = width; x > 0; dst++, x--) dst[0] = cc;
		}
		for (y = 0; y < height; y++) {
			float* dst = zbuffer[y];
			for (x = width; x > 0; dst++, x--) dst[0] = zbufferMax;
		}

		std::fill(zbuffer2.begin(), zbuffer2.end(), std::numeric_limits<float>::infinity());
	}
	void setPixel(int x, int y, UI32 color) {
		if (((UI32)x) < (UI32)width && ((UI32)y) < (UI32)height) {
			framebuffer[y][x] = color;
		}
	}
	void drawLine(int x1, int y1, int x2, int y2, UI32 c) {
		int x, y, rem = 0;
		if (x1 == x2 && y1 == y2) {
			setPixel(x1, y1, c);
		}
		else if (x1 == x2) {
			int inc = (y1 <= y2) ? 1 : -1;
			for (y = y1; y != y2; y += inc) setPixel(x1, y, c);
			setPixel(x2, y2, c);
		}
		else if (y1 == y2) {
			int inc = (x1 <= x2) ? 1 : -1;
			for (x = x1; x != x2; x += inc) setPixel(x, y1, c);
			setPixel(x2, y2, c);
		}
		else {
			int dx = (x1 < x2) ? x2 - x1 : x1 - x2;
			int dy = (y1 < y2) ? y2 - y1 : y1 - y2;
			if (dx >= dy) {
				if (x2 < x1) x = x1, y = y1, x1 = x2, y1 = y2, x2 = x, y2 = y;
				for (x = x1, y = y1; x <= x2; x++) {
					setPixel(x, y, c);
					rem += dy;
					if (rem >= dx) {
						rem -= dx;
						y += (y2 >= y1) ? 1 : -1;
						setPixel(x, y, c);
					}
				}
				setPixel(x2, y2, c);
			}
			else {
				if (y2 < y1) x = x1, y = y1, x1 = x2, y1 = y2, x2 = x, y2 = y;
				for (x = x1, y = y1; y <= y2; y++) {
					setPixel(x, y, c);
					rem += dx;
					if (rem >= dy) {
						rem -= dy;
						x += (x2 >= x1) ? 1 : -1;
						setPixel(x, y, c);
					}
				}
				setPixel(x2, y2, c);
			}
		}
	}

	void drawTriangle(Vec3f v1, Vec3f v2, Vec3f v3, UI32 c);
	void fillTriangle2(Vec3f v1, Vec3f v2, Vec3f v3, UI32 c);
	void filltriangle_bery(Vec3f* pts, UI32 c);
	void filltriangle_bery_zbuffer(Vec3f* pts, UI32 c);
	void filltriangle_bery_testRGB(Vec3f* pts, Vec3f colors[]);
	void filltriangle_bery_texture(Vec3f* pts, Vec3f* vt);
};