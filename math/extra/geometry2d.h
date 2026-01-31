function bool
triangle_contains_ccw(Vec2 p0, Vec2 p1, Vec2 p2, Vec2 point)
{
    bool result = true;
    f32 cross;

    // NOTE - using >= considers a point *on* the triangle to be contained
    cross = vec_cross_xy(p1 - p0, point - p0);
    result &= cross >= 0.0f;
    cross = vec_cross_xy(p2 - p1, point - p1);
    result &= cross >= 0.0f;
    cross = vec_cross_xy(p0 - p2, point - p2);
    result &= cross >= 0.0f;
    return result;
}

template <typename T>
function T
area_compute_signed_ccw(Slice<Vec<T, 2>> ccw_points)
{
    // --- Shoelace formula
    if (ccw_points.count < 3)
        return T(0.0f);

    T result = T(0);
    for (int i = 0; i < ccw_points.count; i++)
    {
        int j = (i + 1) % ccw_points.count;
        result += vec_cross_xy(ccw_points[i], ccw_points[j]);
    }
    result /= T(2);
    return result;
}

function Slice<i32>
triangulate(Slice<Vec2> positions, Memory_Region memory)
{
    // --- Ear clipping algorithm

    if (!VERIFY(positions.count > 2))
        return Slice<i32>{};

    Memory_Region temp_memory = mem_region_begin(memory, KILOBYTES(1));

    int indices_to_clip_count = positions.count;
    i32* indices_to_clip = (i32*)allocate(temp_memory, sizeof(i32) * indices_to_clip_count);

    // Ensure ccw winding
    if (area_compute_signed_ccw(positions) >= 0)
    {
        for (int i = 0; i < indices_to_clip_count; i++)
        {
            indices_to_clip[i] = i;
        }
    }
    else
    {
        for (int i = 0; i < indices_to_clip_count; i++)
        {
            indices_to_clip[i] = (indices_to_clip_count - 1) - i;
        }
    }


    int result_capacity = 3 * (positions.count - 2);
    int result_count = 0;
    i32* result = (i32*)allocate(memory, sizeof(i32) * result_capacity);

    int prev = 0;
    int iterations = 0;

    while (indices_to_clip_count >= 3)
    {
        if (iterations > 2 * indices_to_clip_count)
        {
            ASSERT_FALSE_WARN;
            result_count = 0;
            break;
        }
        iterations++;

        int current = (prev + 1) % indices_to_clip_count;
        int next = (current + 1) % indices_to_clip_count;

        i32 a = indices_to_clip[prev];
        i32 b = indices_to_clip[current];
        i32 c = indices_to_clip[next];
        prev = current;

        bool can_clip = false;
        {
            Vec2 ab = positions[b] - positions[a];
            Vec2 ac = positions[c] - positions[a];

            // NOTE - using >= allows 3 collinear vertices to be clipped, which we are allowing.
            // It might make sense to make this configurable by the caller.
            bool is_ccw = vec_cross_xy(ab, ac) >= 0.0f;
            if (is_ccw)
            {
                can_clip = true;
                for (i32 i = 0; i < indices_to_clip_count; i++)
                {
                    if (i == a || i == b || i == c)
                        continue;

                    Vec2 candidate = positions[i];
                    if (triangle_contains_ccw(positions[a], positions[b], positions[c], candidate))
                    {
                        can_clip = false;
                        break;
                    }
                }
            }
        }

        if (can_clip)
        {
            mem_move(indices_to_clip + current, indices_to_clip + current + 1, sizeof(i32)* (indices_to_clip_count - 1 - current));

            result[result_count++] = a;
            result[result_count++] = b;
            result[result_count++] = c;
            ASSERT(result_count <= result_capacity);

            indices_to_clip_count--;

            // Step prev back by 1, since we just shifted something new into the current index.
            // Not strictly necessary, but easier for me to think about.
            prev = (prev - 1 + indices_to_clip_count) % indices_to_clip_count;

            iterations = 0;
        }
    }

    mem_region_end(temp_memory);
    return slice_create(result, result_count);
}

template <class T>
function Vec<T, 2>
point_to_segment_closest(Vec<T, 2> point, Vec<T, 2> segment_start, Vec<T, 2> segment_end)
{
    // Reference: https://paulbourke.net/geometry/pointlineplane/
    if (segment_start == segment_end)
        return segment_start;

    Vec<T, 2> start_to_point = point - segment_start;
    Vec<T, 2> start_to_end =  segment_end - segment_start;
    auto u = vec_dot(start_to_point, start_to_end) / vec_length_sq(start_to_end);

    Vec<T, 2> result;
    if (u <= (decltype(u))(0))
    {
        result = segment_start;
    }
    else if (u >= (decltype(u))(1))
    {
        result = segment_end;
    }
    else
    {
        // Closest point is along the edge
        result = segment_start + u * start_to_end;
    }

    return result;
}

template <class T>
function auto
point_to_segment_dist_sq(Vec<T, 2> point, Vec<T, 2> segment_start, Vec<T, 2> segment_end)
{
    Vec<T, 2> closest = point_to_segment_closest(point, segment_start, segment_end);
    auto result = vec_length_sq(closest - point);
    return result;
}
