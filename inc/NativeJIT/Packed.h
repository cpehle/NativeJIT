#pragma once

#include <type_traits>


namespace NativeJIT
{
    //*************************************************************************
    // DESIGN NOTE
    // 
    // The Packed class design makes some accomodations in order to simplify
    // the NativeJIT code. In VS 2012 and VS 2013, the compiler uses a more
    // complex calling convention when invoking a function whose return value
    // is not a POD type as determined by Microsoft's implementation of
    // std::is_pod.
    // 
    // When the return type is not a POD, the first parameter of the function
    // will be a pointer to caller-allocated storage for the return value, and
    // the caller will assume that the function will return this pointer in RAX.
    // 
    // When the return type is POD, its value is returned directly in RAX.
    // 
    // The Microsoft implementation of std::is_pod is more restrictive than
    // the C++11 definition. The Packed class design is impacted in, particular,
    // by Microsoft's restriction on the use of private data members, base
    // classes, and user defined constructors in POD type definitions.
    // 
    // A cleaner design of Packed would make m_field private, and probably
    // move it to a base class common to all of the template specializations.
    // The Create() methods would be replaced by constructors with the same
    // parameter list.
    // 
    // Down the road, we may consider implementing support for non-POD
    // return values in NativeJIT, but even if we do this, we may still want
    // to keep Packed as POD for performance reasons.
    // 
    //*************************************************************************

    // TODO: Use traits to ensure that REST is a Packed type (when not void).
    // TODO: This class and PackedMinMaxNode should probably move from NativeJIT to BitFunnel.Library.

    template <unsigned W = 0, typename REST = void>
    class Packed
    {
    public:
        static const unsigned c_fieldCount = 1 + REST::c_fieldCount;
        static const unsigned __int64 c_fieldSizes = (W << (REST::c_fieldCount * 8)) | REST::c_fieldSizes;

        static Packed Create(unsigned __int64 value)
        {
            Packed result;
            result.m_fields = value;
            return result;
        }

        template <unsigned X>
        Packed<X, Packed> Push(unsigned __int64 value)
        {
            static_assert(X + REST::c_fieldCount <= 8, "Packed supports a maximum of 8 bit fields.");
            unsigned __int64 fields = (m_fields << X) | value;
            return Packed<X, Packed>::Create(fields);
        }

        typename REST Pop() const
        {
            return REST::Create(m_fields >> W);
        }

        unsigned __int64 Back() const
        {
            return m_fields & ((1 << W) - 1);
        }

        unsigned __int64 GetBits() const
        {
            return m_fields;
        }

        operator unsigned __int64()
        {
            return m_fields;
        }

        unsigned __int64 m_fields;
    };


    template <unsigned W>
    class Packed<W, void>
    {
    public:
        static const unsigned c_fieldCount = 1;
        static const unsigned __int64 c_fieldSizes = W;

        static Packed Create(unsigned __int64 value)
        {
            Packed result;
            result.m_fields = value;
            return result;
        }

        template <unsigned X>
        Packed<X, Packed> Push(unsigned __int64 value)
        {
            unsigned __int64 fields = (m_fields << X) | value;
            return Packed<X, Packed>::Create(fields);
        }

        unsigned __int64 Back() const
        {
            return m_fields & ((1 << W) - 1);
        }

        unsigned __int64 GetBits() const
        {
            return m_fields;
        }

        operator unsigned __int64()
        {
            return m_fields;
        }

        unsigned __int64 m_fields;
    };


    template <>
    class Packed<0, void>
    {
    public:
        static const unsigned c_fieldCount = 0;
        static const unsigned __int64 c_fieldSizes = 0;

        template <unsigned X>
        static Packed<X, void> Push(unsigned __int64 value)
        {
            unsigned __int64 fields = value;
            return Packed<X>::Create(fields);
        }
    };
}
