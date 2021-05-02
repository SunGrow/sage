#ifndef RESOURCE_BINDINGS
#define RESOURCE_BINDINGS 1
#include "sage_rend.h"

SgResourceBinding materialChaletResourceBindings[] = {
    {
        .type = SG_RESOURCE_TYPE_MESH,
        .stage = SG_SHADER_STAGE_VERTEX_BIT,
        .setBinding = 0,
        .binding = 0,
    },
    {
        .type = SG_RESOURCE_TYPE_TEXTURE_2D,
        .stage = SG_SHADER_STAGE_FRAGMENT_BIT,
        .setBinding = 0,
        .binding = 1,
    },
    {
        .type = SG_RESOURCE_TYPE_MESH,
        .stage = SG_SHADER_STAGE_VERTEX_BIT,
        .setBinding = 0,
        .binding = 2,
    },
    {
        .type = SG_RESOURCE_TYPE_UNIFORM,
        .stage = SG_SHADER_STAGE_VERTEX_BIT,
        .setBinding = 0,
        .binding = 3,
    },
    {
        .type = SG_RESOURCE_TYPE_UNIFORM,
        .stage = SG_SHADER_STAGE_VERTEX_BIT,
        .setBinding = 1,
        .binding = 0,
    },
};

SgResourceBinding materialMyriamSetResourceBinding[] = {
    {
        .type = SG_RESOURCE_TYPE_MESH,
        .stage = SG_SHADER_STAGE_VERTEX_BIT,
        .setBinding = 0,
        .binding = 0,
    },
    {
        .type = SG_RESOURCE_TYPE_TEXTURE_2D,
        .stage = SG_SHADER_STAGE_FRAGMENT_BIT,
        .setBinding = 0,
        .binding = 1,
    },
    {
        .type = SG_RESOURCE_TYPE_TEXTURE_2D,
        .stage = SG_SHADER_STAGE_FRAGMENT_BIT,
        .setBinding = 0,
        .binding = 2,
    },
    {
        .type = SG_RESOURCE_TYPE_MESH,
        .stage = SG_SHADER_STAGE_VERTEX_BIT,
        .setBinding = 0,
        .binding = 3,
    },
    {
        .type = SG_RESOURCE_TYPE_UNIFORM,
        .stage = SG_SHADER_STAGE_VERTEX_BIT,
        .setBinding = 0,
        .binding = 4,
    },
    {
        .type = SG_RESOURCE_TYPE_UNIFORM,
        .stage = SG_SHADER_STAGE_VERTEX_BIT,
        .setBinding = 1,
        .binding = 0,
    },
};

SgResourceBinding materialLightingSetResourceBinding[] = {
    {
        .type = SG_RESOURCE_TYPE_MESH,
        .stage = SG_SHADER_STAGE_VERTEX_BIT,
        .setBinding = 0,
        .binding = 0,
    },
    {
        .type = SG_RESOURCE_TYPE_MESH,
        .stage = SG_SHADER_STAGE_VERTEX_BIT,
        .setBinding = 0,
        .binding = 1,
    },
    {
        .type = SG_RESOURCE_TYPE_UNIFORM,
        .stage = SG_SHADER_STAGE_VERTEX_BIT,
        .setBinding = 0,
        .binding = 2,
    },
    {
        .type = SG_RESOURCE_TYPE_UNIFORM,
        .stage = SG_SHADER_STAGE_VERTEX_BIT,
        .setBinding = 1,
        .binding = 0,
    },
};
#endif
