#include "function.h"
#include "segment.h"
#include "type.h"
#include "value.h"

#include <map>

using namespace mir;

static std::map<String, unsigned> g_Dict = {};

Data::Data(String N, Type *T, Linkage L, Segment *P, Value *V, unsigned A, 
           bool R)
    : Value(N, T), m_Linkage(L), m_Parent(P), m_Value(V), m_Align(A), 
      m_ReadOnly(R) 
{
    if (g_Dict[N] == 0) {
        m_Name = N;
        g_Dict[N] = 1;
    } else {
        m_Name = N + std::to_string(g_Dict[N]);
        g_Dict[N]++;
    }

    if (m_Parent)
        m_Parent->add_data(this);
}

void Data::detach() {
    assert(m_Parent && "Data has no parent.");
    m_Parent->remove_data(this);
}

Slot::Slot(String N, Type *T, Function *P, Type *A, unsigned AL) 
  : Value(N, T), m_Parent(P), m_Alloc(A), m_Align(AL) {
    m_Parent->add_slot(this);
}

ConstantInt *ConstantInt::get(Segment *S, Type *T, long V) {
    assert(T->is_integer_ty() && "Type must be integer.");

    auto *IT = dynamic_cast<IntegerType *>(T);
    switch (IT->get_kind()) {
        case IntegerType::Kind::Int1:
            if (V == 0)
                return S->m_I1Zero;
            else if (V == 1)
                return S->m_I1One;
            else
                assert(false && "Invalid value for 'i1' type.");
        case IntegerType::Kind::Int8:
        {
            auto it = S->m_I8Pool.find(V);
            if (it != S->m_I8Pool.end())
                return it->second;
            else
                return S->m_I8Pool[V] = new ConstantInt(T, V);
        }
        case IntegerType::Kind::Int16:
        {
            auto it = S->m_I16Pool.find(V);
            if (it != S->m_I16Pool.end())
                return it->second;
            else
                return S->m_I16Pool[V] = new ConstantInt(T, V);
        }
        case IntegerType::Kind::Int32:
        {
            auto it = S->m_I32Pool.find(V);
            if (it != S->m_I32Pool.end())
                return it->second;
            else
                return S->m_I32Pool[V] = new ConstantInt(T, V);
        }
        case IntegerType::Kind::Int64:
        {
            auto it = S->m_I64Pool.find(V);
            if (it != S->m_I64Pool.end())
                return it->second;
            else
                return S->m_I64Pool[V] = new ConstantInt(T, V);
        }
    }
}

ConstantFP *ConstantFP::get(Segment *S, Type *T, double V) {
    assert(T->is_float_ty() && "Type must be float.");

    auto *FT = dynamic_cast<FloatType *>(T);
    switch (FT->get_kind()) {
        case FloatType::Kind::Float32:
        {
            auto it = S->m_F32Pool.find(V);
            if (it != S->m_F32Pool.end())
                return it->second;
            else
                return S->m_F32Pool[V] = new ConstantFP(T, V);
        }
        case FloatType::Kind::Float64:
        {
            auto it = S->m_F64Pool.find(V);
            if (it != S->m_F64Pool.end())
                return it->second;
            else
                return S->m_F64Pool[V] = new ConstantFP(T, V);
        }
    }
}

ConstantNil *ConstantNil::get(Segment *S, Type *T) {
    assert(T->is_pointer_ty() && "Type must be pointer.");

    auto it = S->m_NilPool.find(T);
    if (it != S->m_NilPool.end())
        return it->second;
    else
        return S->m_NilPool[T] = new ConstantNil(T);
}
