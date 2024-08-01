template<typename T>
struct io_is_atom
{
    static bool constexpr value = false;
};

template<> struct io_is_atom<u8> { static bool constexpr value = true; };
template<> struct io_is_atom<u16> { static bool constexpr value = true; };
template<> struct io_is_atom<u32> { static bool constexpr value = true; };
template<> struct io_is_atom<u64> { static bool constexpr value = true; };
template<> struct io_is_atom<i8> { static bool constexpr value = true; };
template<> struct io_is_atom<i16> { static bool constexpr value = true; };
template<> struct io_is_atom<i32> { static bool constexpr value = true; };
template<> struct io_is_atom<i64> { static bool constexpr value = true; };

template<typename Io, typename T>
function void
io_visit(Io* io, T* t)
{
    // If you are visiting a non-atom, you need to define a template specialization of io_visit
    StaticAssert(io_is_atom<T>::value);
}
