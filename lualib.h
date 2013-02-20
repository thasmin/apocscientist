#ifndef LUALIB_H
#define LUALIB_H

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
extern lua_State *L;

#include "building.h"
#include "robot.h"
#include "point.h"

const char *LUA_MT_ROBOT;
const char *LUA_MT_POINT;
const char *LUA_MT_BUILDING;
const char *LUA_MT_BUILDING_MODEL;

void lualib_init(lua_State *L);

building* lh_push_building(lua_State *L, building *src);
building_model* lh_push_building_model(lua_State *L, building_model *src);
void lh_push_robot(lua_State *L, robot *src);
point* lh_push_point(lua_State *L, point *src);

#endif
