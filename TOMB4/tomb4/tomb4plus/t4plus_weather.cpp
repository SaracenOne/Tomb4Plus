#include "../../tomb4/pch.h"

#include "../../global/types.h"
#include "../../game/control.h"
#include "../../game/lara.h"
#include "../../game/camera.h"
#include "../../specific/file.h"
#include "../../game/objects.h"
#include "../../game/effect2.h"
#include "../../game/tomb4fx.h"
#include "../../specific/function_table.h"
#include "../../specific/function_stubs.h"
#include "../../specific/gamemain.h"
#include "../../specific/3dmath.h"
#include "../../specific/output.h"

#include "t4plus_weather.h"
#include "t4plus_objects.h"

// T4Plus - weather effects

T4POverrideFogMode t4_override_fog_mode = T4P_FOG_DEFAULT;

T4PWeatherType t4p_rain_type = T4P_WEATHER_DISABLED;
T4PWeatherType t4p_snow_type = T4P_WEATHER_DISABLED;

int32_t rain_outside = 0;
int32_t snow_outside = 0;

#define PERCIPITATION_ARRAY_SIZE 1024

static RAINDROPS Rain[PERCIPITATION_ARRAY_SIZE];
static SNOWFLAKE Snow[PERCIPITATION_ARRAY_SIZE];
static int16_t rain_count = 0;
static int16_t snow_count = 0;
static int16_t max_rain = 0;
static int16_t max_snow = 0;

void InitWeatherFX() {
	t4p_rain_type = T4P_WEATHER_DISABLED;
	t4p_snow_type = T4P_WEATHER_DISABLED;

	rain_outside = 0;
	snow_outside = 0;

	snow_count = PERCIPITATION_ARRAY_SIZE;
	rain_count = PERCIPITATION_ARRAY_SIZE;
	max_snow = 64;
	max_rain = 64;
}

void ClearWeatherFX() {
	for (int32_t i = 0; i < PERCIPITATION_ARRAY_SIZE; i++) {
		Rain[i].x = 0;
		Snow[i].x = 0;
	}
}

void DoRain() {
	RAINDROPS* rptr;
	ROOM_INFO* r;
	FLOOR_INFO* floor;
	GFXTLVERTEX v[2];
	TEXTURESTRUCT tex;
	int16_t* clip;
	float ctop, cbottom, cright, cleft, zv, fx, fy, fz, mx, my, mz;
	int32_t num_alive, rad, angle, rnd, x, z, x_size, y_size, c;
	int16_t room_number, clipFlag;

	num_alive = 0;

	for (int32_t i = 0; i < rain_count; i++) {
		rptr = &Rain[i];

		if (rain_outside && !rptr->x && num_alive < max_rain) {
			num_alive++;
			rad = GetRandomDraw() & DEGREES_TO_ROTATION(45) + 1;
			angle = GetRandomDraw() & DEGREES_TO_ROTATION(45);
			rptr->x = camera.pos.x + (rad * rcossin_tbl[angle] >> (W2V_SHIFT - 2));
			rptr->y = camera.pos.y + -BLOCK_SIZE - (GetRandomDraw() & 0x7FF);
			rptr->z = camera.pos.z + (rad * rcossin_tbl[angle + 1] >> (W2V_SHIFT - 2));

			if (t4p_rain_type == T4P_WEATHER_DISABLED) {
				rptr->x = 0;
				continue;
			}

			if (IsRoomOutside(rptr->x, rptr->y, rptr->z) < 0) {
				rptr->x = 0;
				continue;
			}

			if (room[IsRoomOutsideNo].flags & ROOM_UNDERWATER) {
				rptr->x = 0;
				continue;
			}

			if (!(room[IsRoomOutsideNo].flags & ROOM_RAIN) && t4p_rain_type == T4P_WEATHER_ENABLED_IN_SPECIFIC_ROOMS) {
				rptr->x = 0;
				continue;
			}

			rptr->xv = (GetRandomDraw() & 7) - 4;
			rptr->yv = uint8_t((GetRandomDraw() & 3) + GetFixedScale(8));
			rptr->zv = (GetRandomDraw() & 7) - 4;
			rptr->room_number = IsRoomOutsideNo;
			rptr->life = 64 - rptr->yv;
		}

		if (rptr->x) {
			if (rptr->life > 240 || abs(CamPos.x - rptr->x) > 6000 || abs(CamPos.z - rptr->z) > 6000) {
				rptr->x = 0;
				continue;
			}

			rptr->x += rptr->xv + 4 * SmokeWindX;
			rptr->y += rptr->yv << 3;
			rptr->z += rptr->zv + 4 * SmokeWindZ;
			r = &room[rptr->room_number];
			x = r->x + BLOCK_SIZE;
			z = r->z + BLOCK_SIZE;
			x_size = r->x_size - 1;
			y_size = r->y_size - 1;

			if (rptr->y <= r->maxceiling || rptr->y >= r->minfloor || rptr->z <= z ||
			        rptr->z >= r->z + (x_size << 10) || rptr->x <= x || rptr->x >= r->x + (y_size << 10)) {
				room_number = rptr->room_number;
				floor = GetFloor(rptr->x, rptr->y, rptr->z, &room_number);

				if (room_number == rptr->room_number || room[room_number].flags & ROOM_UNDERWATER) {
					if (room[room_number].flags & ROOM_UNDERWATER) {
						// T4Plus - fixes splashes appearing underwater and increased ripple frequency
						SetupRipple(rptr->x, GetWaterHeight(rptr->x, rptr->y, rptr->z, room_number), rptr->z, 3, 0);
					} else {
						TriggerSmallSplash(rptr->x, GetHeight(floor, rptr->x, rptr->y, rptr->z), rptr->z, 1);
					}

					rptr->x = 0;
					continue;
				} else {
					rptr->room_number = room_number;
				}
			}

			rnd = GetRandomDraw();

			if ((rnd & 3) != 3) {
				rptr->xv += (rnd & 3) - 1;

				if (rptr->xv < -4)
					rptr->xv = -4;
				else if (rptr->xv > 4)
					rptr->xv = 4;
			}

			rnd = (rnd >> 2) & 3;

			if (rnd != 3) {
				rptr->zv += (int8_t)(rnd - 1);

				if (rptr->zv < -4)
					rptr->zv = -4;
				else if (rptr->zv > 4)
					rptr->zv = 4;
			}

			rptr->life -= 2;

			if (rptr->life > 240)
				rptr->x = 0;
		}
	}

	tex.drawtype = 2;
	tex.tpage = 0;
	tex.flag = 0;
	ctop = f_top;
	cleft = f_left + 4.0F;
	cbottom = f_bottom;
	cright = f_right - 4.0F;
	phd_PushMatrix();
	phd_TranslateAbs(lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos);

	for (int32_t i = 0; i < rain_count; i++) {
		rptr = &Rain[i];

		if (rptr->x) {
			clipFlag = 0;
			clip = clipflags;
			fx = float(rptr->x - lara_item->pos.x_pos - (SmokeWindX << 2));
			fy = float(rptr->y - (rptr->yv << 3) - lara_item->pos.y_pos);
			fz = float(rptr->z - lara_item->pos.z_pos - (SmokeWindZ << 2));
			mx = mMXPtr[M00] * fx + mMXPtr[M01] * fy + mMXPtr[M02] * fz + mMXPtr[M03];
			my = mMXPtr[M10] * fx + mMXPtr[M11] * fy + mMXPtr[M12] * fz + mMXPtr[M13];
			mz = mMXPtr[M20] * fx + mMXPtr[M21] * fy + mMXPtr[M22] * fz + mMXPtr[M23];

			c = int32_t((1.0F - (f_mzfar - mz) * (1.0F / f_mzfar)) * 8.0F + 8.0F);
			v[0].specular = 0xFF000000;
			v[0].color = RGBA(c, c, c, 128);
			v[0].tu = mx;
			v[0].tv = my;

			if (mz < f_mznear)
				clipFlag = -128;
			else {
				if (mz > f_mzfar) {
					mz = f_zfar;
					clipFlag = 16;
				}

				zv = f_mpersp / mz;
				v[0].sx = zv * mx + f_centerx;
				v[0].sy = zv * my + f_centery;
				v[0].rhw = f_moneopersp * zv;

				if (v[0].sx < cleft)
					clipFlag++;
				else if (v[0].sx > cright)
					clipFlag += 2;

				if (v[0].sy < ctop)
					clipFlag += 4;
				else if (v[0].sy > cbottom)
					clipFlag += 8;
			}

			v[0].sz = mz;
			*clip++ = clipFlag;
			clipFlag = 0;

			fx = float(rptr->x - lara_item->pos.x_pos);
			fy = float(rptr->y - lara_item->pos.y_pos);
			fz = float(rptr->z - lara_item->pos.z_pos);
			mx = mMXPtr[M00] * fx + mMXPtr[M01] * fy + mMXPtr[M02] * fz + mMXPtr[M03];
			my = mMXPtr[M10] * fx + mMXPtr[M11] * fy + mMXPtr[M12] * fz + mMXPtr[M13];
			mz = mMXPtr[M20] * fx + mMXPtr[M21] * fy + mMXPtr[M22] * fz + mMXPtr[M23];

			c = int32_t((1.0F - (f_mzfar - mz) * (1.0F / f_mzfar)) * 16.0F + 16.0F);
			c <<= 1;
			v[1].specular = 0xFF000000;
			v[1].color = RGBA(c, c, c, 0xFF);
			v[1].tu = mx;
			v[1].tv = my;

			if (mz < f_mznear)
				clipFlag = -128;
			else {
				if (mz > f_mzfar) {
					mz = f_zfar;
					clipFlag = 16;
				}

				zv = f_mpersp / mz;
				v[1].sx = zv * mx + f_centerx;
				v[1].sy = zv * my + f_centery;
				v[1].rhw = f_moneopersp * zv;

				if (v[1].sx < cleft)
					clipFlag++;
				else if (v[1].sx > cright)
					clipFlag += 2;

				if (v[1].sy < ctop)
					clipFlag += 4;
				else if (v[1].sy > cbottom)
					clipFlag += 8;
			}

			v[1].sz = mz;
			*clip-- = clipFlag;

			if (!clip[0] && !clip[1])
				AddLineSorted(v, &v[1], 6);
		}
	}

	phd_PopMatrix();
}

void DoSnow() {
	SNOWFLAKE* snow;
	ROOM_INFO* r;
	SPRITESTRUCT* sprite;
	GFXTLVERTEX v[4];
	TEXTURESTRUCT tex;
	float* pSize;
	float x, y, z, xv, yv, zv, vx, vy, xSize, ySize;
	int32_t num_alive, rad, angle, ox, oy, oz, col;
	int16_t room_number, clipFlag;

	num_alive = 0;

	for (int32_t i = 0; i < snow_count; i++) {
		snow = &Snow[i];

		if (!snow->x) {
			if (!snow_outside || num_alive >= max_snow)
				continue;

			num_alive++;
			rad = GetRandomDraw() & 0x1FFF;
			angle = (GetRandomDraw() & 0xFFF) << 1;
			snow->x = camera.pos.x + (rad * rcossin_tbl[angle] >> (W2V_SHIFT - 2));
			snow->y = camera.pos.y - BLOCK_SIZE - (GetRandomDraw() & 0x7FF);
			snow->z = camera.pos.z + (rad * rcossin_tbl[angle + 1] >> (W2V_SHIFT - 2));

			if (t4p_snow_type == T4P_WEATHER_DISABLED) {
				snow->x = 0;
				continue;
			}

			if (IsRoomOutside(snow->x, snow->y, snow->z) < 0) {
				snow->x = 0;
				continue;
			}

			if (room[IsRoomOutsideNo].flags & ROOM_UNDERWATER) {
				snow->x = 0;
				continue;
			}

			if (!(room[IsRoomOutsideNo].flags & ROOM_SNOW) && t4p_snow_type == T4P_WEATHER_ENABLED_IN_SPECIFIC_ROOMS) {
				snow->x = 0;
				continue;
			}

			snow->stopped = 0;
			snow->xv = (GetRandomDraw() & 7) - 4;
			snow->yv = ((GetRandomDraw() & 0xF) + 8) << 3;
			snow->zv = (GetRandomDraw() & 7) - 4;
			snow->room_number = IsRoomOutsideNo;
			snow->life = 112 - (snow->yv >> 2);
		}

		ox = snow->x;
		oy = snow->y;
		oz = snow->z;

		if (!snow->stopped) {
			snow->x += snow->xv;
			snow->y += (snow->yv >> 1) & 0xFC;
			snow->z += snow->zv;
			r = &room[snow->room_number];

			if (snow->y <= r->maxceiling || snow->y >= r->minfloor ||
			        snow->z <= r->z + BLOCK_SIZE || snow->z >= (r->x_size << 10) + r->z - BLOCK_SIZE ||
			        snow->x <= r->x + BLOCK_SIZE || snow->x >= (r->y_size << 10) + r->x - BLOCK_SIZE) {
				room_number = snow->room_number;
				GetFloor(snow->x, snow->y, snow->z, &room_number);

				if (room_number == snow->room_number) {
					snow->x = 0;
					continue;
				}

				if (room[room_number].flags & ROOM_UNDERWATER) {
					snow->stopped = 1;
					snow->x = ox;
					snow->y = oy;
					snow->z = oz;

					if (snow->life > 16)
						snow->life = 16;
				} else
					snow->room_number = room_number;
			}
		}

		if (!snow->life) {
			snow->x = 0;
			continue;
		}

		if ((abs(CamPos.x - snow->x) > 6000 || abs(CamPos.z - snow->z) > 6000) && snow->life > 16)
			snow->life = 16;

		if (snow->xv < SmokeWindX << 2)
			snow->xv += 2;
		else if (snow->xv > SmokeWindX << 2)
			snow->xv -= 2;

		if (snow->zv < SmokeWindZ << 2)
			snow->zv += 2;
		else if (snow->zv > SmokeWindZ << 2)
			snow->zv -= 2;

		snow->life -= 2;

		if ((snow->yv & 7) != 7)
			snow->yv++;
	}

	sprite = &spriteinfo[objects[T4PlusGetDefaultSpritesSlotID()].mesh_index + 10];
	tex.tpage = sprite->tpage;
	tex.drawtype = 2;
	tex.flag = 0;
	tex.u1 = sprite->x2;
	tex.v1 = sprite->y1;
	tex.u2 = sprite->x2;
	tex.v2 = sprite->y2;
	tex.v3 = sprite->y2;
	tex.u3 = sprite->x1;
	tex.u4 = sprite->x1;
	tex.v4 = sprite->y2;

	phd_PushMatrix();
	phd_TranslateAbs(camera.pos.x, camera.pos.y, camera.pos.z);
	//aSetViewMatrix();

	clipflags[0] = 0;
	clipflags[1] = 0;
	clipflags[2] = 0;
	clipflags[3] = 0;

	for (int32_t i = 0; i < snow_count; i++) {
		snow = &Snow[i];

		if (!snow->x)
			continue;

		x = float(snow->x - camera.pos.x);
		y = float(snow->y - camera.pos.y);
		z = float(snow->z - camera.pos.z);
		zv = mMXPtr[M20] * x + mMXPtr[M21] * y + mMXPtr[M22] * z + mMXPtr[M23];

		if (zv < f_mznear)
			continue;

		col = 0;

		if ((snow->yv & 7) != 7)
			col = (snow->yv & 7) << 4;
		else if (snow->life > 32)
			col = 130;
		else
			col = snow->life << 3;

		col = RGBA(col, col, col, 0xFF);
		pSize = &SnowSizes[snow->yv & 24];
		xv = mMXPtr[M00] * x + mMXPtr[M01] * y + mMXPtr[M02] * z + mMXPtr[M03];
		yv = mMXPtr[M10] * x + mMXPtr[M11] * y + mMXPtr[M12] * z + mMXPtr[M13];

		zv = f_mpersp / zv;

		for (int32_t j = 0; j < 4; j++) {
			xSize = pSize[0] * zv;
			ySize = pSize[1] * zv;
			pSize += 2;

			vx = xv * zv + xSize + f_centerx;
			vy = yv * zv + ySize + f_centery;
			clipFlag = 0;

			if (vx < f_left)
				clipFlag++;
			else if (vx > f_right)
				clipFlag += 2;

			if (vy < f_top)
				clipFlag += 4;
			else if (vy > f_bottom)
				clipFlag += 8;

			clipflags[j] = clipFlag;
			v[j].sx = vx;
			v[j].sy = vy;
			v[j].rhw = zv * f_moneopersp;
			v[j].tu = 0;
			v[j].tv = 0;
			v[j].color = col;
			v[j].specular = 0xFF000000;
		}

		AddTriSorted(v, 2, 0, 1, &tex, 1);
	}

	phd_PopMatrix();
}

void DoWeather() {
	if (rain_outside)
		DoRain();

	if (snow_outside)
		DoSnow();
}