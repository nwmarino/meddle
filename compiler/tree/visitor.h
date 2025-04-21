#ifndef MEDDLE_VISITOR_H
#define MEDDLE_VISITOR_H

namespace meddle {

template<typename Derived>
class Visitor {
    template<typename nT>
    void visit(nT *N) {
        static_cast<Derived *>(this)->visit(N);
    }
};

} // namespace meddle

#endif // MEDDLE_VISITOR_H
