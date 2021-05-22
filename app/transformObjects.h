#ifndef TRANSFORM_OBJECTS
#define TRANSFORM_OBJECTS 1
#include <math.h>

#include "sage_math.h"

struct SgObjectInfo {
	m4 transform;
	v4 position;
};

struct SgObjectInfo myriamObjects[] = {
    {
        .transform =
            {
                {0.01, 0.0, 0.0, 0.0},
                {0.0, 0.01, 0.0, 0.0},
                {0.0, 0.0, 0.01, 0.0},
                {0.0, 0.0, 0.0, 1},
            },
        .position = {-10.0, 1.0, 100.0, 1.0},
    },
    {
        .transform =
            {
                {0.01, 0.0, 0.0, 0.0},
                {0.0, 0.01, 0.0, 0.0},
                {0.0, 0.0, 0.01, 0.0},
                {0.0, 0.0, 0.0, 1},
            },
        .position = {2.0, 0.0, 0.0, 1.0},
    },
};

struct SgObjectInfo chaletObjects[] = {
    {
        .transform = {{1.0, 0.0, 0.0, 0.0},
                      {0.0, 1.0, 0.0, 0.0},
                      {0.0, 0.0, 1.0, 0.0},
                      {0.0, 0.0, 0.0, 1.0}},
        .position  = {1.0, 1.0, 0.0, 1.0},
    },
    {
        .transform = {{1.0, 0.0, 0.0, 0.0},
                      {0.0, 1.0, 0.0, 0.0},
                      {0.0, 0.0, 1.0, 0.0},
                      {0.0, 0.0, 0.0, 1.0}},
        .position  = {2.0, 0.0, 0.0, 1.0},
    },
};

struct SgObjectInfo kittenObjects[] = {
    {
        .transform = {{1.0, 0.0, 0.0, 0.0},
                      {0.0, 1.0, 0.0, 0.0},
                      {0.0, 0.0, 1.0, 0.0},
                      {0.0, 0.0, 0.0, 1.0}},
        .position  = {1.0, 2.0, 0.0, 1.0},
    },
    {
        .transform = {{1.0, 0.0, 0.0, 0.0},
                      {0.0, 1.0, 0.0, 0.0},
                      {0.0, 0.0, 1.0, 0.0},
                      {0.0, 0.0, 0.0, 1.0}},
        .position  = {-2.0, 1.0, 1.0, 1.0},
    },
};

struct SgObjectInfo lightingObjects[] = {
    {
        .transform = {{1.0, 0.0, 0.0, 0.0},
                      {0.0, 1.0, 0.0, 0.0},
                      {0.0, 0.0, 1.0, 0.0},
                      {0.0, 0.0, 0.0, 15.0}},
        .position  = {0, 0, 0.0, 1.0},
    },
};

#endif
