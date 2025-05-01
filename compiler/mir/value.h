#ifndef MEDDLE_VALUE_H
#define MEDDLE_VALUE_H

#include <algorithm>
#include <cassert>
#include <fstream>
#include <string>
#include <vector>

using String = std::string;

namespace mir {

class Function;
class Inst;
class Segment;
class Type;

class Value {
protected:
    String m_Name;
    Type *m_Type;
    std::vector<Inst *> m_Uses = {};

public:
    Value(String N, Type *T) : m_Name(N), m_Type(T) {}
    virtual ~Value() = default;

    virtual bool is_constant() const { return false; }

    bool is_named() const { return !m_Name.empty(); }

    String get_name() const { return m_Name; }

    Type *get_type() const { return m_Type; }

    bool is_used() const { return m_Uses.size() != 0; }

    bool is_used_by(Inst *user) const 
    { return std::find(m_Uses.begin(), m_Uses.end(), user) != m_Uses.end(); }

    void add_use(Inst *user) { m_Uses.push_back(user); }

    void del_use(Inst *user) { 
        m_Uses.erase(
            std::remove(m_Uses.begin(), m_Uses.end(), user), 
            m_Uses.end()
        ); 
    }

    virtual void print(std::ostream &OS) const = 0;
};

class Data final : public Value {
    friend class Builder;

public:
    enum class Linkage {
        Internal,
        External,
    };

private:
    Linkage m_Linkage;
    Segment *m_Parent;
    Value *m_Value;
    unsigned m_Align;
    bool m_ReadOnly;

public:
    Data(String N, Type *T, Linkage L, Segment *P, Value *V, unsigned A, 
         bool R);

    Value *get_value() const { return m_Value; }

    unsigned get_align() const { return m_Align; }

    bool is_read_only() const { return m_ReadOnly; }

    /// Detach this data from its parent segment and delete it.
    void detach();

    void print(std::ostream &OS) const override;
};

class Slot final : public Value {
    friend class Builder;

    Function *m_Parent;
    Type *m_Alloc;
    unsigned m_Align;

    Slot(String N, Type *T, Function *P, Type *A, unsigned AL);

public:
    ~Slot() override = default;

    Type *get_alloc_type() const { return m_Alloc; }

    unsigned get_align() const { return m_Align; }

    void print(std::ostream &OS) const override;
};

class Constant : public Value {
public:
    Constant(Type *T) : Value("", T) {}
    virtual ~Constant() = default;

    bool is_constant() const override { return true; }
};

class ConstantInt final : public Constant {
    friend class Segment;

    long m_Value;

    ConstantInt(Type *T, long V) : Constant(T), m_Value(V) {}

public:
    static ConstantInt *get(Segment *S, Type *T, long V);

    long get_value() const { return m_Value; }

    void print(std::ostream &OS) const override;
};

class ConstantFP final : public Constant {
    friend class Segment;

    double m_Value;

    ConstantFP(Type *T, double V) : Constant(T), m_Value(V) {}

public:
    static ConstantFP *get(Segment *S, Type *T, double V);

    double get_value() const { return m_Value; }

    void print(std::ostream &OS) const override;
};

class ConstantNil final : public Constant {
    friend class Segment;
    
    ConstantNil(Type *T) : Constant(T) {}

public:
    static ConstantNil *get(Segment *S, Type *T);

    void print(std::ostream &OS) const override;
};

class ConstantString final : public Constant {
    String m_Value;

public:
    ConstantString(Type *T, String V) : Constant(T), m_Value(V) {}

    String get_value() const { return m_Value; }

    void print(std::ostream &OS) const override;
};

class ConstantAggregate final : public Constant {
    std::vector<Value *> m_Values;

public:
    ConstantAggregate(Type *T, std::vector<Value *> V) 
        : Constant(T), m_Values(V) {}

    const std::vector<Value *> &get_values() const { return m_Values; }

    void print(std::ostream &OS) const override;
};

} // namespace mir

#endif // MEDDLE_VALUE_H
