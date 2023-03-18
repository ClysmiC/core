// Vec2::Vec2(Vec2x v)
// {
//     this->x = (f32)v.x;
//     this->y = (f32)v.y;
// }

Vec2::Vec2(Vec2i v)
{
    this->x = (f32)v.x;
    this->y = (f32)v.y;
}

// Vec3::Vec3(Vec3x v)
// {
//     this->x = (f32)v.x;
//     this->y = (f32)v.y;
//     this->z = (f32)v.z;
// }

// Vec3::Vec3(Vec2x xy, fix64 z)
// {
//     this->x = (f32)xy.x;
//     this->y = (f32)xy.y;
//     this->z = (f32)z;
// }

Vec3::Vec3(Vec3i v)
{
    this->x = (f32)v.x;
    this->y = (f32)v.y;
    this->z = (f32)v.z;
}

// Vec4::Vec4(Vec4x v)
// {
//     this->x = (f32)v.x;
//     this->y = (f32)v.y;
//     this->z = (f32)v.z;
//     this->w = (f32)v.w;
// }

Vec4::Vec4(Vec4i v)
{
    this->x = (f32)v.x;
    this->y = (f32)v.y;
    this->z = (f32)v.z;
    this->w = (f32)v.w;
}

// Rect2::Rect2(Rect2x r)
// {
//     this->min = Vec2(r.min);
//     this->max = Vec2(r.max);
// }

Rect2::Rect2(Rect2i r)
{
    this->min = Vec2(r.min);
    this->max = Vec2(r.max);
}

// Rect3::Rect3(Rect3x r)
// {
//     this->min = Vec3(r.min);
//     this->max = Vec3(r.max);
// }

