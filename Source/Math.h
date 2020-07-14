#ifndef _MATH_H
#define _MATH_H

struct vec2f
{
    f32 x, y;
};

internal vec2f
operator+(vec2f left, vec2f right)
{
    vec2f result;
    
    result.x = left.x + right.x;
    result.y = left.y + right.y;
    
    return result;
}


union vec3f
{
    struct 
    {
        f32 x, y, z;
    };
    struct
    {
        f32 r, g, b;
    };
};


internal vec3f
operator+(vec3f left, vec3f right)
{
    vec3f result;
    
    result.x = left.x + right.x;
    result.y = left.y + right.y;
    result.z = left.z + right.z;
    
    return result;
}


internal s32
RoundF32ToS32(f32 value)
{
    s32 result = (s32)(value + 0.5f);
    return result;
}

internal u32
RoundF32ToU32(f32 value)
{
    u32 result = (u32)(value + 0.5f);
    return result;
}

#endif //_MATH_H
