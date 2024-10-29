// EPOS List Utility Declarations

#ifndef __list_h
#define __list_h

#include <system/config.h>

__BEGIN_UTIL

// List Element Rank (for ordered lists)
class List_Element_Rank
{
public:
    List_Element_Rank(int r = 0) : _rank(r) {}

    operator int() const { return _rank; }

protected:
    int _rank;
};

// List Elements
namespace List_Elements
{
    typedef List_Element_Rank Rank;

    // Vector Element
    template <typename T>
    class Pointer
    {
    public:
        typedef T Object_Type;
        typedef Pointer Element;

    public:
        Pointer(const T *o) : _object(o) {}

        T *object() const { return const_cast<T *>(_object); }

    private:
        const T *_object;
    };

    // Hash Table Element
    template <typename T, typename R = Rank>
    class Ranked
    {
    public:
        typedef T Object_Type;
        typedef R Rank_Type;
        typedef Ranked Element;

    public:
        Ranked(const T *o, const R &r = 0) : _object(o), _rank(r) {}

        T *object() const { return const_cast<T *>(_object); }

        const R &rank() const { return _rank; }
        const R &key() const { return _rank; }
        void rank(const R &r) { _rank = r; }
        int promote(const R &n = 1)
        {
            _rank -= n;
            return _rank;
        }
        int demote(const R &n = 1)
        {
            _rank += n;
            return _rank;
        }

    private:
        const T *_object;
        R _rank;
    };

    // Simple List Element
    template <typename T>
    class Singly_Linked
    {
    public:
        typedef T Object_Type;
        typedef Singly_Linked Element;

    public:
        Singly_Linked(const T *o) : _object(o), _next(0) {}

        T *object() const { return const_cast<T *>(_object); }

        Element *next() const { return _next; }
        void next(Element *e) { _next = e; }

    private:
        const T *_object;
        Element *_next;
    };

    // Simple Ordered List Element
    // Hash Table's Synonym List Element
    template <typename T, typename R = Rank>
    class Singly_Linked_Ordered
    {
    public:
        typedef T Object_Type;
        typedef R Rank_Type;
        typedef Singly_Linked_Ordered Element;

    public:
        Singly_Linked_Ordered(const T *o, const R &r = 0) : _object(o), _rank(r), _next(0) {}

        T *object() const { return const_cast<T *>(_object); }

        Element *next() const { return _next; }
        void next(Element *e) { _next = e; }

        const R &rank() const { return _rank; }
        const R &key() const { return _rank; }
        void rank(const R &r) { _rank = r; }
        int promote(const R &n = 1)
        {
            _rank -= n;
            return _rank;
        }
        int demote(const R &n = 1)
        {
            _rank += n;
            return _rank;
        }

    private:
        const T *_object;
        R _rank;
        Element *_next;
    };

    // Simple Grouping List Element
    template <typename T>
    class Singly_Linked_Grouping
    {
    public:
        typedef T Object_Type;
        typedef Singly_Linked_Grouping Element;

    public:
        Singly_Linked_Grouping(const T *o, unsigned long s) : _object(o), _size(s), _next(0) {}

        T *object() const { return const_cast<T *>(_object); }

        Element *next() const { return _next; }
        void next(Element *e) { _next = e; }

        unsigned long size() const { return _size; }
        void size(unsigned long l) { _size = l; }
        void shrink(unsigned long n) { _size -= n; }
        void expand(unsigned long n) { _size += n; }

    private:
        const T *_object;
        unsigned long _size;
        Element *_next;
    };

    // List Element
    template <typename T>
    class Doubly_Linked
    {
    public:
        typedef T Object_Type;
        typedef Doubly_Linked Element;

    public:
        Doubly_Linked(const T *o) : _object(o), _prev(0), _next(0) {}

        T *object() const { return const_cast<T *>(_object); }

        Element *prev() const { return _prev; }
        Element *next() const { return _next; }
        void prev(Element *e) { _prev = e; }
        void next(Element *e) { _next = e; }

    private:
        const T *_object;
        Element *_prev;
        Element *_next;
    };

    // Ordered List Element
    template <typename T, typename R = Rank>
    class Doubly_Linked_Ordered
    {
    public:
        typedef T Object_Type;
        typedef R Rank_Type;
        typedef Doubly_Linked_Ordered Element;

    public:
        Doubly_Linked_Ordered(const T *o, const R &r = 0) : _object(o), _rank(r), _prev(0), _next(0) {}

        T *object() const { return const_cast<T *>(_object); }

        Element *prev() const { return _prev; }
        Element *next() const { return _next; }
        void prev(Element *e) { _prev = e; }
        void next(Element *e) { _next = e; }

        const R &rank() const { return _rank; }
        void rank(const R &r) { _rank = r; }
        int promote(const R &n = 1)
        {
            _rank -= n;
            return _rank;
        }
        int demote(const R &n = 1)
        {
            _rank += n;
            return _rank;
        }

    private:
        const T *_object;
        R _rank;
        Element *_prev;
        Element *_next;
    };

    // Ordered List Element
    template <typename T, typename R = Rank>
    class Doubly_Linked_Typed
    {
    public:
        typedef T Object_Type;
        typedef R Rank_Type;
        typedef Doubly_Linked_Typed Element;

    public:
        Doubly_Linked_Typed(const T *o, const R &t = 0) : _object(o), _type(t), _prev(0), _next(0) {}

        T *object() const { return const_cast<T *>(_object); }

        Element *prev() const { return _prev; }
        Element *next() const { return _next; }
        void prev(Element *e) { _prev = e; }
        void next(Element *e) { _next = e; }

        const R &type() const { return _type; }
        void rank(const R &t) { _type = t; }

    private:
        const T *_object;
        R _type;
        Element *_prev;
        Element *_next;
    };

    // Scheduling List Element
    template <typename T, typename R = Rank>
    class Doubly_Linked_Scheduling
    {
    public:
        typedef T Object_Type;
        typedef R Rank_Type;
        typedef Doubly_Linked_Scheduling Element;

    public:
        Doubly_Linked_Scheduling(const T *o, const R &r = 0) : _object(o), _rank(r), _prev(0), _next(0) {}

        T *object() const { return const_cast<T *>(_object); }

        Element *prev() const { return _prev; }
        Element *next() const { return _next; }
        void prev(Element *e) { _prev = e; }
        void next(Element *e) { _next = e; }

        const R &rank() const { return _rank; }
        void rank(const R &r) { _rank = r; }
        int promote(const R &n = 1)
        {
            _rank -= n;
            return _rank;
        }
        int demote(const R &n = 1)
        {
            _rank += n;
            return _rank;
        }

    private:
        const T *_object;
        R _rank;
        Element *_prev;
        Element *_next;
    };

    // Grouping List Element
    template <typename T>
    class Doubly_Linked_Grouping
    {
    public:
        typedef T Object_Type;
        typedef Doubly_Linked_Grouping Element;

    public:
        Doubly_Linked_Grouping(const T *o, unsigned long s) : _object(o), _size(s), _prev(0), _next(0) {}

        T *object() const { return const_cast<T *>(_object); }

        Element *prev() const { return _prev; }
        Element *next() const { return _next; }
        void prev(Element *e) { _prev = e; }
        void next(Element *e) { _next = e; }

        unsigned long size() const { return _size; }
        void size(unsigned long l) { _size = l; }
        void shrink(unsigned long n) { _size -= n; }
        void expand(unsigned long n) { _size += n; }

    private:
        const T *_object;
        unsigned long _size;
        Element *_prev;
        Element *_next;
    };
};

// List Iterators
namespace List_Iterators
{
    // Forward Iterator (for singly linked lists)
    template <typename El>
    class Forward
    {
    private:
        typedef Forward<El> Iterator;

    public:
        typedef El Element;

    public:
        Forward() : _current(0) {}
        Forward(Element *e) : _current(e) {}

        operator Element *() const { return _current; }

        Element &operator*() const { return *_current; }
        Element *operator->() const { return _current; }

        Iterator &operator++()
        {
            _current = _current->next();
            return *this;
        }
        Iterator operator++(int)
        {
            Iterator tmp = *this;
            ++*this;
            return tmp;
        }

        bool operator==(const Iterator &i) const { return _current == i._current; }
        bool operator!=(const Iterator &i) const { return _current != i._current; }

    protected:
        Element *_current;
    };

    // Bidirectional Iterator (for doubly linked lists)
    template <typename El>
    class Bidirecional
    {
    private:
        typedef Bidirecional<El> Iterator;

    public:
        typedef El Element;

    public:
        Bidirecional() : _current(0) {}
        Bidirecional(Element *e) : _current(e) {}

        operator Element *() const { return _current; }

        Element &operator*() const { return *_current; }
        Element *operator->() const { return _current; }

        Iterator &operator++()
        {
            _current = _current->next();
            return *this;
        }
        Iterator operator++(int)
        {
            Iterator tmp = *this;
            ++*this;
            return tmp;
        }

        Iterator &operator--()
        {
            _current = _current->prev();
            return *this;
        }
        Iterator operator--(int)
        {
            Iterator tmp = *this;
            --*this;
            return tmp;
        }

        bool operator==(const Iterator &i) const { return _current == i._current; }
        bool operator!=(const Iterator &i) const { return _current != i._current; }

    private:
        Element *_current;
    };
}

// Singly-Linked List
template <typename T, typename El = List_Elements::Singly_Linked<T>>
class Simple_List
{
public:
    typedef T Object_Type;
    typedef El Element;
    typedef List_Iterators::Forward<El> Iterator;

public:
    Simple_List() : _size(0), _head(0), _tail(0) {}

    bool empty() const { return (_size == 0); }
    unsigned long size() const { return _size; }

    Element *head() { return _head; }
    Element *tail() { return _tail; }

    Iterator begin() { return Iterator(_head); }
    Iterator end() { return Iterator(0); }

    void insert(Element *e) { insert_tail(e); }

    void insert_head(Element *e)
    {
        if (empty())
            insert_first(e);
        else
        {
            e->next(_head);
            _head = e;
            _size++;
        }
    }

    void insert_tail(Element *e)
    {
        if (empty())
            insert_first(e);
        else
        {
            _tail->next(e);
            e->next(0);
            _tail = e;
            _size++;
        }
    }

    Element *remove() { return remove_head(); }

    Element *remove(Element *e)
    {
        if (last())
            remove_last();
        else if (e == _head)
            remove_head();
        else
        {
            Element *p = _head;
            for (; p && p->next() && (p->next() != e); p = p->next())
                ;
            if (p)
                p->next(e->next());
            if (e == _tail)
                _tail = p;
            _size--;
        }
        return e;
    }

    Element *remove_head()
    {
        if (empty())
            return 0;
        if (last())
            return remove_last();
        Element *e = _head;
        _head = _head->next();
        _size--;
        return e;
    }

    Element *remove_tail()
    {
        if (_tail)
            return remove(_tail);
        else
            return 0;
    }

    Element *remove(const Object_Type *obj)
    {
        Element *e = search(obj);
        if (e)
            return remove(e);
        return 0;
    }

    Element *search(const Object_Type *obj)
    {
        Element *e = _head;
        for (; e && (e->object() != obj); e = e->next())
            ;
        return e;
    }

protected:
    bool last() const { return (_size == 1); }

    void insert(Element *e, Element *p, Element *n)
    {
        p->next(e);
        e->next(n);
        _size++;
    }

    void insert_first(Element *e)
    {
        e->next(0);
        _head = e;
        _tail = e;
        _size++;
    }

    Element *remove_last()
    {
        Element *e = _head;
        _head = 0;
        _tail = 0;
        _size--;
        return e;
    }

private:
    unsigned long _size;
    Element *_head;
    Element *_tail;
};

// Singly-Linked, Ordered List
template <typename T,
          typename R = List_Element_Rank,
          typename El = List_Elements::Singly_Linked_Ordered<T, R>,
          bool relative = false>
class Simple_Ordered_List : public Simple_List<T, El>
{
private:
    typedef Simple_List<T, El> Base;

    using Base::insert_first;
    using Base::insert_head;
    using Base::insert_tail;

public:
    typedef T Object_Type;
    typedef R Rank_Type;
    typedef El Element;
    typedef List_Iterators::Forward<El> Iterator;

public:
    using Base::begin;
    using Base::empty;
    using Base::end;
    using Base::head;
    using Base::remove_head;
    using Base::remove_tail;
    using Base::search;
    using Base::size;
    using Base::tail;

    void insert(Element *e)
    {
        if (empty())
            insert_first(e);
        else
        {
            Element *next, *prev;
            for (next = head(), prev = 0;
                 (next->rank() <= e->rank()) && next->next();
                 prev = next, next = next->next())
                if (relative)
                    e->rank(e->rank() - next->rank());
            if (next->rank() <= e->rank())
            {
                if (relative)
                    e->rank(e->rank() - next->rank());
                insert_tail(e);
            }
            else if (!prev)
            {
                if (relative)
                    next->rank(next->rank() - e->rank());
                insert_head(e);
            }
            else
            {
                if (relative)
                    next->rank(next->rank() - e->rank());
                Base::insert(e, prev, next);
            }
        }
    }

    Element *remove()
    {
        Element *e = remove_head();
        if (e && relative && e->next())
            e->next()->rank(e->next()->rank() + e->rank());
        return e;
    }

    Element *remove(Element *e)
    {
        Base::remove(e);
        if (relative && e->next())
            e->next()->rank(e->next()->rank() + e->rank());
        return e;
    }

    Element *remove(const Object_Type *obj)
    {
        Element *e = search(obj);
        if (e)
            return remove(e);
        return 0;
    }

    Element *search_rank(const Rank_Type &rank)
    {
        Element *e = head();
        for (; e && (e->rank() != rank); e = e->next())
            ;
        return e;
    }

    Element *remove_rank(const Rank_Type &rank)
    {
        Element *e = search_rank(rank);
        if (e)
            return remove(e);
        return 0;
    }
};

// Singly-Linked, Relative Ordered List
template <typename T,
          typename R = List_Element_Rank,
          typename El = List_Elements::Singly_Linked_Ordered<T, R>>
class Simple_Relative_List : public Simple_Ordered_List<T, R, El, true>
{
};

// Singly-Linked, Grouping List
template <typename T,
          typename El = List_Elements::Singly_Linked_Grouping<T>>
class Simple_Grouping_List : public Simple_List<T, El>
{
private:
    typedef Simple_List<T, El> Base;

public:
    typedef T Object_Type;
    typedef El Element;
    typedef List_Iterators::Forward<El> Iterator;

public:
    Simple_Grouping_List() : _grouped_size(0) {}

    using Base::begin;
    using Base::empty;
    using Base::end;
    using Base::head;
    using Base::insert_first;
    using Base::insert_head;
    using Base::insert_tail;
    using Base::remove;
    using Base::search;
    using Base::size;
    using Base::tail;

    unsigned long grouped_size() const { return _grouped_size; }

    Element *search_size(unsigned long s)
    {
        Element *e = head();
        if (sizeof(Object_Type) < sizeof(Element))
            for (; e && (e->size() < sizeof(Element) / sizeof(Object_Type) + s) && (e->size() != s); e = e->next())
                ;
        else
            for (; e && (e->size() < s); e = e->next())
                ;
        return e;
    }

    void insert_merging(Element *e, Element **m1, Element **m2)
    {
        _grouped_size += e->size();
        *m1 = *m2 = 0;
        Element *r = search(e->object() + e->size());
        Element *l = search_left(e->object());
        if (r)
        {
            e->size(e->size() + r->size());
            remove(r);
            *m1 = r;
        }
        if (l)
        {
            l->size(l->size() + e->size());
            *m2 = e;
        }
        else
            insert_tail(e);
    }

    Element *search_decrementing(unsigned long s)
    {
        Element *e = search_size(s);
        if (e)
        {
            e->shrink(s);
            _grouped_size -= s;
            if (!e->size())
                remove(e);
        }
        return e;
    }

private:
    Element *search_left(const Object_Type *obj)
    {
        Element *e = head();
        for (; e && (e->object() + e->size() != obj); e = e->next())
            ;
        return e;
    }

private:
    unsigned long _grouped_size;
};

// Doubly-Linked List
template <typename T,
          typename El = List_Elements::Doubly_Linked<T>>
class List
{
public:
    typedef T Object_Type;
    typedef El Element;
    typedef List_Iterators::Bidirecional<El> Iterator;

public:
    List() : _size(0), _head(0), _tail(0) {}

    bool empty() const { return (_size == 0); }
    unsigned long size() const { return _size; }

    Element *head() { return _head; }
    Element *tail() { return _tail; }

    Iterator begin() { return Iterator(_head); }
    Iterator end() { return Iterator(0); }

    void insert(Element *e) { insert_tail(e); }

    void insert_head(Element *e)
    {
        db<Lists>(TRC) << "List::insert_head(e=" << e
                       << ") => {p=" << (e ? e->prev() : (void *)-1)
                       << ",o=" << (e ? e->object() : (void *)-1)
                       << ",n=" << (e ? e->next() : (void *)-1)
                       << "}" << endl;

        print_head();
        print_tail();

        if (empty())
            insert_first(e);
        else
        {
            e->prev(0);
            e->next(_head);
            _head->prev(e);
            _head = e;
            _size++;
        }

        print_head();
        print_tail();
    }

    void insert_tail(Element *e)
    {
        db<Lists>(TRC) << "List::insert_tail(e=" << e
                       << ") => {p=" << (e ? e->prev() : (void *)-1)
                       << ",o=" << (e ? e->object() : (void *)-1)
                       << ",n=" << (e ? e->next() : (void *)-1)
                       << "}" << endl;

        print_head();
        print_tail();

        if (empty())
            insert_first(e);
        else
        {
            _tail->next(e);
            e->prev(_tail);
            e->next(0);
            _tail = e;
            _size++;
        }

        print_head();
        print_tail();
    }

    Element *remove() { return remove_head(); }

    Element *remove(Element *e)
    {
        db<Lists>(TRC) << "List::remove(e=" << e
                       << ") => {p=" << (e ? e->prev() : (void *)-1)
                       << ",o=" << (e ? e->object() : (void *)-1)
                       << ",n=" << (e ? e->next() : (void *)-1)
                       << "}" << endl;

        print_head();
        print_tail();

        if (last())
            remove_last();
        else if (!e->prev())
            remove_head();
        else if (!e->next())
            remove_tail();
        else
        {
            e->prev()->next(e->next());
            e->next()->prev(e->prev());
            _size--;
        }

        print_head();
        print_tail();

        return e;
    }

    Element *remove_head()
    {
        db<Lists>(TRC) << "List::remove_head()" << endl;

        print_head();
        print_tail();

        if (empty())
            return 0;
        if (last())
            return remove_last();
        Element *e = _head;
        _head = _head->next();
        _head->prev(0);
        _size--;

        print_head();
        print_tail();

        return e;
    }

    Element *remove_tail()
    {
        db<Lists>(TRC) << "List::remove_tail()" << endl;

        print_head();
        print_tail();

        if (empty())
            return 0;
        if (last())
            return remove_last();
        Element *e = _tail;
        _tail = _tail->prev();
        _tail->next(0);
        _size--;

        print_head();
        print_tail();

        return e;
    }

    Element *remove(const Object_Type *obj)
    {
        Element *e = search(obj);
        if (e)
            return remove(e);
        return 0;
    }

    Element *search(const Object_Type *obj)
    {
        Element *e = _head;
        for (; e && (e->object() != obj); e = e->next())
            ;
        return e;
    }

protected:
    bool last() const { return (_size == 1); }

    void insert(Element *e, Element *p, Element *n)
    {
        db<Lists>(TRC) << "List::insert(e=" << e << ",p=" << p << ",n=" << n
                       << ") => {p=" << (e ? e->prev() : (void *)-1)
                       << ",o=" << (e ? e->object() : (void *)-1)
                       << ",n=" << (e ? e->next() : (void *)-1)
                       << "},{p=" << (p ? p->prev() : (void *)-1)
                       << ",o=" << (p ? p->object() : (void *)-1)
                       << ",n=" << (p ? p->next() : (void *)-1)
                       << "},{p=" << (n ? n->prev() : (void *)-1)
                       << ",o=" << (n ? n->object() : (void *)-1)
                       << ",n=" << (n ? n->next() : (void *)-1)
                       << "}" << endl;

        print_head();
        print_tail();

        p->next(e);
        n->prev(e);
        e->prev(p);
        e->next(n);
        _size++;

        print_head();
        print_tail();
    }

    void insert_first(Element *e)
    {
        db<Lists>(TRC) << "List::insert_first(e=" << e
                       << ") => {p=" << (e ? e->prev() : (void *)-1)
                       << ",o=" << (e ? e->object() : (void *)-1)
                       << ",n=" << (e ? e->next() : (void *)-1)
                       << "}" << endl;

        print_head();
        print_tail();

        e->prev(0);
        e->next(0);
        _head = e;
        _tail = e;
        _size++;

        print_head();
        print_tail();
    }

    Element *remove_last()
    {
        db<Lists>(TRC) << "List::remove_last()" << endl;

        print_head();
        print_tail();

        Element *e = _head;
        _head = 0;
        _tail = 0;
        _size--;

        print_head();
        print_tail();

        return e;
    }

    void print_head()
    {
        db<Lists>(INF) << "List[" << this << "]::head=" << head()
                       << " => {p=" << (head() ? head()->prev() : (void *)-1)
                       << ",o=" << (head() ? head()->object() : (void *)-1)
                       << ",n=" << (head() ? head()->next() : (void *)-1)
                       << "}" << endl;
    }

    void print_tail()
    {
        db<Lists>(INF) << "List[" << this << "]::tail=" << tail()
                       << " => {p=" << (tail() ? tail()->prev() : (void *)-1)
                       << ",o=" << (tail() ? tail()->object() : (void *)-1)
                       << ",n=" << (tail() ? tail()->next() : (void *)-1)
                       << "}" << endl;
    }

private:
    unsigned long _size;
    Element *_head;
    Element *_tail;
};

// Doubly-Linked, Ordered List
template <typename T,
          typename R = List_Element_Rank,
          typename El = List_Elements::Doubly_Linked_Ordered<T, R>,
          bool relative = false>
class Ordered_List : public List<T, El>
{
private:
    typedef List<T, El> Base;

public:
    typedef T Object_Type;
    typedef R Rank_Type;
    typedef El Element;
    typedef List_Iterators::Bidirecional<El> Iterator;

public:
    using Base::begin;
    using Base::empty;
    using Base::end;
    using Base::head;
    using Base::insert_first;
    using Base::insert_head;
    using Base::insert_tail;
    using Base::search;
    using Base::size;
    using Base::tail;

    void insert(Element *e)
    {
        db<Lists>(TRC) << "Ordered_List::insert(e=" << e
                       << ") => {p=" << (e ? e->prev() : (void *)-1)
                       << ",o=" << (e ? e->object() : (void *)-1)
                       << ",n=" << (e ? e->next() : (void *)-1)
                       << "}" << endl;

        if (empty())
            insert_first(e);
        else
        {
            Element *next;
            for (next = head();
                 (next->rank() <= e->rank()) && next->next();
                 next = next->next())
                if (relative)
                    e->rank(e->rank() - next->rank());
            if (next->rank() <= e->rank())
            {
                if (relative)
                    e->rank(e->rank() - next->rank());
                insert_tail(e);
            }
            else if (!next->prev())
            {
                if (relative)
                    next->rank(next->rank() - e->rank());
                insert_head(e);
            }
            else
            {
                if (relative)
                    next->rank(next->rank() - e->rank());
                Base::insert(e, next->prev(), next);
            }
        }
    }

    Element *remove()
    {
        db<Lists>(TRC) << "Ordered_List::remove()" << endl;
        Element *e = Base::remove_head();
        if (e && relative && e->next())
            e->next()->rank(e->next()->rank() + e->rank());
        return e;
    }

    Element *remove(Element *e)
    {
        db<Lists>(TRC) << "Ordered_List::remove(e=" << e
                       << ") => {p=" << (e ? e->prev() : (void *)-1)
                       << ",o=" << (e ? e->object() : (void *)-1)
                       << ",n=" << (e ? e->next() : (void *)-1)
                       << "}" << endl;

        Base::remove(e);
        if (relative && e->next())
            e->next()->rank(e->next()->rank() + e->rank());

        return e;
    }

    Element *remove(const Object_Type *obj)
    {
        db<Lists>(TRC) << "Ordered_List::remove(o=" << obj << ")" << endl;

        Element *e = search(obj);
        if (e)
            return remove(e);
        else
            return 0;
    }

    Element *search_rank(const Rank_Type &rank)
    {
        Element *e = head();
        for (; e && (e->rank() != rank); e = e->next())
            ;
        return e;
    }

    Element *remove_rank(const Rank_Type &rank)
    {
        db<Lists>(TRC) << "Ordered_List::remove_rank(r=" << rank << ")" << endl;

        Element *e = search_rank(rank);
        if (e)
            return remove(e);
        return 0;
    }
};

// Doubly-Linked, Relative Ordered List
template <typename T,
          typename R = List_Element_Rank,
          typename El = List_Elements::Doubly_Linked_Ordered<T, R>>
class Relative_List : public Ordered_List<T, R, El, true>
{
};

// Doubly-Linked, Typed List
template <typename T = void,
          typename R = List_Element_Rank,
          typename El = List_Elements::Doubly_Linked_Typed<T, R>,
          bool relative = false>
class Typed_List : public List<T, El>
{
};

// Doubly-Linked, Scheduling List
// Objects subject to scheduling must export a type "Criterion" compatible
// with those available at scheduler.h .
// In this implementation, the chosen element is kept outside the list
// referenced by the _chosen attribute.
template <typename T,
          typename R = typename T::Criterion,
          typename El = List_Elements::Doubly_Linked_Scheduling<T, R>>
class Scheduling_List : private Ordered_List<T, R, El>
{
    template <typename FT, typename FR, typename FEl, unsigned int FH>
    friend class Multihead_Scheduling_List; // for chosen() and remove()
    template <typename FT, typename FR, typename FEl, typename FL, unsigned int FQ>
    friend class Scheduling_Multilist; // for chosen() and remove()

private:
    typedef Ordered_List<T, R, El> Base;

public:
    typedef T Object_Type;
    typedef R Rank_Type;
    typedef El Element;
    typedef typename Base::Iterator Iterator;

public:
    Scheduling_List() : _chosen(0) {}

    using Base::begin;
    using Base::empty;
    using Base::end;
    using Base::head;
    using Base::size;
    using Base::tail;

    Element *volatile &chosen() { return _chosen; }

    void insert(Element *e)
    {
        db<Lists>(TRC) << "Scheduling_List::insert(e=" << e
                       << ") => {p=" << (e ? e->prev() : (void *)-1)
                       << ",o=" << (e ? e->object() : (void *)-1)
                       << ",n=" << (e ? e->next() : (void *)-1)
                       << "}" << endl;

        if (_chosen)
            Base::insert(e);
        else
            _chosen = e;
    }

    Element *remove(Element *e)
    {
        db<Lists>(TRC) << "Scheduling_List::remove(e=" << e
                       << ") => {p=" << (e ? e->prev() : (void *)-1)
                       << ",o=" << (e ? e->object() : (void *)-1)
                       << ",n=" << (e ? e->next() : (void *)-1)
                       << "}" << endl;

        if (e == _chosen)
            _chosen = Base::remove_head();
        else
            e = Base::remove(e);

        return e;
    }

    Element *choose()
    {
        db<Lists>(TRC) << "Scheduling_List::choose()" << endl;

        if (!empty())
        {
            Base::insert(_chosen);
            _chosen = Base::remove_head();
        }

        return _chosen;
    }

    Element *choose_another()
    {
        db<Lists>(TRC) << "Scheduling_List::choose_another()" << endl;

        if (!empty() && head()->rank() != R::IDLE)
        {
            Element *tmp = _chosen;
            _chosen = Base::remove_head();
            Base::insert(tmp);
        }

        return _chosen;
    }

    Element *choose(Element *e)
    {
        db<Lists>(TRC) << "Scheduling_List::choose(e=" << e
                       << ") => {p=" << (e ? e->prev() : (void *)-1)
                       << ",o=" << (e ? e->object() : (void *)-1)
                       << ",n=" << (e ? e->next() : (void *)-1)
                       << "}" << endl;

        if (e != _chosen)
        {
            Base::insert(_chosen);
            _chosen = Base::remove(e);
        }

        return _chosen;
    }

private:
    using Base::remove;
    void chosen(Element *e) { _chosen = e; }

private:
    Element *volatile _chosen;
};

// Estrutura de lista similar a Multilist, com único chosen
// Adaptando para nosso algoritmo
template <typename T,
          typename R = typename T::Criterion,
          typename El = List_Elements::Doubly_Linked_Scheduling<T, R>,
          typename L = Ordered_List<T, R, El>,
          unsigned int Q = R::QUEUES>
class Scheduling_Multilist_Single_Chosen
{
public:
    typedef T Object_Type;
    typedef R Rank_Type;
    typedef El Element;
    typedef typename L::Iterator Iterator;

public:
    Scheduling_Multilist_Single_Chosen() { _total_size = 0; }

    bool empty() const { return _list[R::current_queue()].empty(); }
    bool empty(unsigned int queue) { return _list[queue].empty(); }

    unsigned long size() const { return _list[R::current_queue()].size(); }
    unsigned long size(unsigned int queue) const { return _list[queue].size(); }

    unsigned long total_size() const { return _total_size; }

    Element *head() { return _list[R::current_queue()].head(); }
    Element *head(unsigned int i) { return _list[i].head(); }
    Element *tail() { return _list[R::current_queue()].tail(); }
    Element *tail(unsigned int i) { return _list[i].tail(); }

    Iterator begin() { return Iterator(_list[R::current_queue()].head()); }
    Iterator begin(unsigned int queue) { return Iterator(_list[queue].head()); }
    Iterator end() { return Iterator(0); }
    Iterator end(unsigned int queue) { return Iterator(_list[queue].tail()); }

    // Quantidade de filas ocupadas em determinado momento
    const int occupied_queues() { return _occupied_queues; }

    // Se não tem _chosen -> escolhe chosen
    Element *volatile &chosen() {
        if (!_chosen) {
            choose();
        }
        return _chosen;
    }

    void insert(Element *e)
    {
        // Se é primeiro a ser inserido -> chosen vai ser ele mesmo
        if (_total_size == 0 ) {
            _chosen = e;
        }

        if (_list[e->rank().queue()].empty())
        {
            _occupied_queues++;
        }

        // Insere o elemento na sublista especifica 
        _list[e->rank().queue()].insert(e);
        _total_size++;
    }

    Element *remove(Element *e)
    {
        // Pagamos o preco desse if para man
        if (e == _chosen)
        {
            _chosen = 0;
        }

        if (_list[e->rank().queue()].size() == 1)
        {
            _occupied_queues--;
        }

        _total_size--;

        return _list[e->rank().queue()].remove(e);
    }

    Element *choose()
    {
        _chosen = _list[R::current_queue()].head();
        return _chosen;
    }

    // TODO: Improve this method
    Element *choose_another()
    {
        _chosen = _list[R::current_queue()].head()->next();
        return _chosen;
    }

    Element *choose(Element *e)
    {   
        _chosen = e;
        return _chosen;
    }

private:
    L _list[Q];
    unsigned int _total_size;
    unsigned int _occupied_queues; 
    Element *volatile _chosen;
};

// Doubly-Linked, Multihead Scheduling List
// Besides declaring "Criterion", objects subject to scheduling policies that
// use the Multihead list must export the HEADS constant to indicate the
// number of heads in the list and the current_head() class method to designate
// the head to which the current operation applies.
template <typename T,
          typename R = typename T::Criterion,
          typename El = List_Elements::Doubly_Linked_Scheduling<T, R>,
          unsigned int H = R::HEADS>
class Multihead_Scheduling_List : private Ordered_List<T, R, El>
{
    template <typename FT, typename FR, typename FEl, typename FL, unsigned int FQ>
    friend class Scheduling_Multilist; // for chosen() and remove()

private:
    typedef Ordered_List<T, R, El> Base;

public:
    typedef T Object_Type;
    typedef R Rank_Type;
    typedef El Element;
    typedef typename Base::Iterator Iterator;

public:
    Multihead_Scheduling_List()
    {
        for (unsigned int i = 0; i < H; i++)
            _chosen[i] = 0;
    }

    using Base::begin;
    using Base::empty;
    using Base::end;
    using Base::head;
    using Base::size;
    using Base::tail;

    Element *volatile &chosen() { return _chosen[R::current_head()]; }
    Element *volatile &chosen(unsigned int head) { return _chosen[head]; }

    void insert(Element *e)
    {
        db<Lists>(TRC) << "Scheduling_List::insert(e=" << e
                       << ") => {p=" << (e ? e->prev() : (void *)-1)
                       << ",o=" << (e ? e->object() : (void *)-1)
                       << ",n=" << (e ? e->next() : (void *)-1)
                       << "}" << endl;

        if (_chosen[R::current_head()])
            Base::insert(e);
        else
            _chosen[R::current_head()] = e;
    }

    Element *remove(Element *e)
    {
        db<Lists>(WRN) << "Scheduling_List::remove(e=" << e
                       << ") => {p=" << (e ? e->prev() : (void *)-1)
                       << ",o=" << (e ? e->object() : (void *)-1)
                       << ",n=" << (e ? e->next() : (void *)-1)
                       << "}" << endl;

        if (e == _chosen[R::current_head()])
            _chosen[R::current_head()] = Base::remove_head();
        else
            e = Base::remove(e);

        return e;
    }

    Element *choose()
    {
        db<Lists>(TRC) << "Scheduling_List::choose()" << endl;

        if (!empty())
        {
            Base::insert(_chosen[R::current_head()]);
            _chosen[R::current_head()] = Base::remove_head();
        }
        //db<Lists>(WRN) << "CHOOSE MULTIHEAD: " << _chosen[R::current_head()] << endl;
        return _chosen[R::current_head()];
    }

    Element *choose_another()
    {
        db<Lists>(TRC) << "Scheduling_List::choose_another()" << endl;

        if (!empty() && head()->rank() != R::IDLE)
        {
            Element *tmp = _chosen[R::current_head()];
            _chosen[R::current_head()] = Base::remove_head();
            Base::insert(tmp);
        }

        return _chosen[R::current_head()];
    }

    Element *choose(Element *e)
    {
        db<Lists>(TRC) << "Scheduling_List::choose(e=" << e
                       << ") => {p=" << (e ? e->prev() : (void *)-1)
                       << ",o=" << (e ? e->object() : (void *)-1)
                       << ",n=" << (e ? e->next() : (void *)-1)
                       << "}" << endl;

        if (e != _chosen[R::current_head()])
        {
            Base::insert(_chosen[R::current_head()]);
            _chosen[R::current_head()] = Base::remove(e);
        }

        return _chosen[R::current_head()];
    }

private:
    using Base::remove;
    void chosen(Element *e) { _chosen[R::current_head()] = e; }

private:
    Element *volatile _chosen[H];
};

// Doubly-Linked, Scheduling Multilist
// Besides declaring "Criterion", objects subject to scheduling policies that
// use the Multilist must export the QUEUES constant to indicate the number of
// sublists in the list, the current_queue() class method to designate the
// queue to which the current operation applies, and the queue() method to
// return the queue in which the object currently resides.
template <typename T,
          typename R = typename T::Criterion,
          typename El = List_Elements::Doubly_Linked_Scheduling<T, R>,
          typename L = Scheduling_List<T, R, El>,
          unsigned int Q = R::QUEUES>
class Scheduling_Multilist
{
public:
    typedef T Object_Type;
    typedef R Rank_Type;
    typedef El Element;
    typedef typename L::Iterator Iterator;

public:
    Scheduling_Multilist() {}

    bool empty() const { return _list[R::current_queue()].empty(); }
    bool empty(unsigned int queue) { return _list[queue].empty(); }

    unsigned long size() const { return _list[R::current_queue()].size(); }
    unsigned long size(unsigned int queue) const { return _list[queue].size(); }

    unsigned long total_size() const
    {
        unsigned long s = 0;
        for (unsigned int i = 0; i < Q; i++)
            s += _list[i].size();
        return s;
    }

    Element *head() { return _list[R::current_queue()].head(); }
    Element *head(unsigned int i) { return _list[i].head(); }
    Element *tail() { return _list[R::current_queue()].tail(); }
    Element *tail(unsigned int i) { return _list[i].tail(); }

    Iterator begin() { return Iterator(_list[R::current_queue()].head()); }
    Iterator begin(unsigned int queue) { return Iterator(_list[queue].head()); }
    Iterator end() { return Iterator(0); }
    Iterator end(unsigned int queue) { return Iterator(_list[queue].tail()); }

    // Quantidade de filas ocupadas em determinado momento baseado em seu próprio chosen / fila com threads
    // Usado em round_profile, não liga para threads escolhidas por outros cores
    const int occupied_queues() { 
        int count = 0;
        for (unsigned int i = 0; i < Q; i++) {
            if (_list[i].chosen() || !_list[i].empty()) {
                count++;
            }
        }
        return count;
    }

    Element *volatile &chosen()
    {
        return _list[R::current_queue()].chosen();
    }
    
    Element *volatile &chosen(unsigned int queue)
    {
        return _list[queue].chosen();
    }

    // workaround para escolher o chosen sem passar
    Element *volatile &chosen_now(unsigned int queue)
    {
        return _list[queue].chosen();
    }

    void insert(Element *e)
    {
        // if (_list[e->rank().queue()].empty() && e->rank() != -1) {
        //     _list[e->rank().queue()].insert_bruh(e);
        // }
        _list[e->rank().queue()].insert(e);
    }

    Element *remove(Element *e)
    {
        return _list[e->rank().queue()].remove(e);
    }

    Element *choose()
    {
        if (_list[R::current_queue()].chosen()->rank().queue() != R::current_queue())
        {
            insert(_list[R::current_queue()].chosen());
            _list[R::current_queue()].chosen(_list[R::current_queue()].remove());
        }
        //db<Lists>(WRN) << "CHOOSE MULTILIST - current queue: " << R::current_queue() << endl;

        return _list[R::current_queue()].choose();
    }

    Element *choose_another()
    {
        if (_list[R::current_queue()].chosen()->rank().queue() != R::current_queue())
        {
            insert(_list[R::current_queue()].chosen());
            _list[R::current_queue()].chosen(_list[R::current_queue()].remove());
        }

        return _list[R::current_queue()].choose_another();
    }

    Element *choose(Element *e)
    {
        if (_list[R::current_queue()].chosen()->rank().queue() != R::current_queue())
        {
            insert(_list[R::current_queue()].chosen());
            _list[R::current_queue()].chosen(_list[R::current_queue()].remove());
        }

        return _list[e->rank().queue()].choose(e);
    }

private:
    L _list[Q];
};

// Doubly-Linked, Multihead Scheduling Multilist
// Besides declaring "Criterion", objects subject to scheduling policies that
// use the Multilist must export the QUEUES constant to indicate the number of
// sublists in the list, the HEADS constant to indicate the number of heads in
// each of the sublists, the current_queue() class method to designate the
// queue to which the current operation applies, the current_head() class
// method to designate the head to which the current operation applies, and
// the queue() method to return the queue in which the object currently resides.
template <typename T,
          typename R = typename T::Criterion,
          typename El = List_Elements::Doubly_Linked_Scheduling<T, R>,
          unsigned int Q = R::QUEUES,
          unsigned int H = R::HEADS>
class Multihead_Scheduling_Multilist : public Scheduling_Multilist<T, R, El, Multihead_Scheduling_List<T, R, El, H>, Q>
{
};

// Doubly-Linked, Grouping List
template <typename T,
          typename El = List_Elements::Doubly_Linked_Grouping<T>>
class Grouping_List : public List<T, El>
{
private:
    typedef List<T, El> Base;

public:
    typedef T Object_Type;
    typedef El Element;
    typedef List_Iterators::Bidirecional<El> Iterator;

public:
    Grouping_List() : _grouped_size(0) {}

    using Base::begin;
    using Base::empty;
    using Base::end;
    using Base::head;
    using Base::insert_tail;
    using Base::print_head;
    using Base::print_tail;
    using Base::remove;
    using Base::search;
    using Base::size;
    using Base::tail;

    unsigned long grouped_size() const { return _grouped_size; }

    Element *search_size(unsigned long s)
    {
        Element *e = head();
        if (sizeof(Object_Type) < sizeof(Element))
            for (; e && (e->size() < sizeof(Element) / sizeof(Object_Type) + s) && (e->size() != s); e = e->next())
                ;
        else
            for (; e && (e->size() < s); e = e->next())
                ;
        return e;
    }

    void insert_merging(Element *e, Element **m1, Element **m2)
    {
        db<Lists>(TRC) << "Grouping_List::insert_merging(e=" << e << ")" << endl;

        _grouped_size += e->size();
        *m1 = *m2 = 0;
        Element *r = search(e->object() + e->size());
        Element *l = search_left(e->object());
        if (!l)
        {
            insert_tail(e);
        }
        if (r)
        {
            e->size(e->size() + r->size());
            remove(r);
            *m1 = r;
        }
        if (l)
        {
            l->size(l->size() + e->size());
            *m2 = e;
        }
    }

    Element *search_decrementing(unsigned long s)
    {
        db<Lists>(TRC) << "Grouping_List::search_decrementing(s=" << s << ")" << endl;
        print_head();
        print_tail();

        Element *e = search_size(s);
        if (e)
        {
            e->shrink(s);
            _grouped_size -= s;
            if (!e->size())
                remove(e);
        }

        return e;
    }

private:
    Element *search_left(const Object_Type *obj)
    {
        Element *e = head();
        for (; e && (e->object() + e->size() != obj); e = e->next())
            ;
        return e;
    }

private:
    unsigned long _grouped_size;
};

__END_UTIL

#endif


// Thread(entry=0x80000038,state=0,priority=-1,queue=3,stack={b=0xffc3be0c,s=16384}
// ,context={b=0xffc3fddc,{flags=0x200,ax=0,bx=0,cx=0,dx=0,si=0,di=0,bp=0x00000000,
// sp=0xffb03c58,ip=0x80000038,cs=8,ccs=8,cds=10,ces=10,cfs=10,cgs=10,css=10,cr3=0x
// 3fffc000}}) => 0xffc3fee8@3
// CPU 0 Fila 0: CPU 0 CHOSEN: 0x00000000
// CPU 0 Fila 1: CPU 0 CHOSEN: 0x00000000
// CPU 0 Fila 2: CPU 0 CHOSEN: 0x00000000
// CPU 0 Fila 3: CPU 0 CHOSEN: 0xffc3fee8

// Thread(entry=0x8000512c,state=1,priority=2147483647,queue=3,stack={b=0xffc37c48,
// s=16384},context={b=0xffc3bc18,{flags=0x200,ax=0,bx=0,cx=0,dx=0,si=0,di=0,bp=0x0
// 0000000,sp=0xffb03c58,ip=0x8000512c,cs=8,ccs=8,cds=10,ces=10,cfs=10,cgs=10,css=1
// 0,cr3=0x3fffc000}}) => 0xffc3bd24@3
// CPU 0 Fila 0: CPU 0 CHOSEN: 0x00000000
// CPU 0 Fila 1: CPU 0 CHOSEN: 0x00000000
// CPU 0 Fila 2: CPU 0 CHOSEN: 0x00000000
// CPU 0 Fila 3: 0xffc3bd24 CPU 0 CHOSEN: 0xffc3fee8
// MAIN: Hello world!

// Thread(entry=0x800000ab,state=1,priority=1073741823,queue=3,stack={b=0xffc33c28,
// s=16384},context={b=0xffc37bf4,{flags=0x200,ax=0,bx=0,cx=0,dx=0,si=0,di=0,bp=0x0
// 0000000,sp=0xffc3f6e4,ip=0x800000ab,cs=8,ccs=8,cds=10,ces=10,cfs=10,cgs=10,css=1
// 0,cr3=0x3fffc000}}) => 0x80800f38@3
// CPU 0 Fila 0: CPU 0 CHOSEN: 0x00000000
// CPU 0 Fila 1: CPU 0 CHOSEN: 0x00000000
// CPU 0 Fila 2: CPU 0 CHOSEN: 0x00000000
// CPU 0 Fila 3: 0xffc3bd24 0x80800f38 CPU 0 CHOSEN: 0xffc3fee8
// Thread ready!

// Thread(entry=0x800000ab,state=1,priority=536870911,queue=2,stack={b=0xffc2fc08,s
// =16384},context={b=0xffc33bd4,{flags=0x200,ax=0,bx=0,cx=0,dx=0,si=0,di=0,bp=0x00
// 000000,sp=0xffc3f6e4,ip=0x800000ab,cs=8,ccs=8,cds=10,ces=10,cfs=10,cgs=10,css=10
// ,cr3=0x3fffc000}}) => 0x80800d94@2
// CPU 0 Fila 0: CPU 0 CHOSEN: 0x00000000
// CPU 0 Fila 1: CPU 0 CHOSEN: 0x00000000
// CPU 0 Fila 2: CPU 0 CHOSEN: 0x80800d94
// CPU 0 Fila 3: 0xffc3bd24 0x80800f38 CPU 0 CHOSEN: 0xffc3fee8
// Thread ready!
// bruh - 1
// Scheduling_List::remove(e=0x80800d94) => {p=0x00000000,o=0x80800cc0,n=0x00000000
// }

// Thread(entry=0x800000ab,state=1,priority=0,queue=2,stack={b=0xffc2bbe8,s=16384},
// context={b=0xffc2fbb4,{flags=0x200,ax=0,bx=0,cx=0,dx=0,si=0,di=0,bp=0x00000000,s
// p=0xffc3f6e4,ip=0x800000ab,cs=8,ccs=8,cds=10,ces=10,cfs=10,cgs=10,css=10,cr3=0x3
// fffc000}}) => 0x80800bf0@2
// CPU 0 Fila 0: CPU 0 CHOSEN: 0x00000000
// CPU 0 Fila 1: CPU 0 CHOSEN: 0x00000000
// CPU 0 Fila 2: CPU 0 CHOSEN: 0x80800bf0
// CPU 0 Fila 3: 0xffc3bd24 0x80800f38 CPU 0 CHOSEN: 0xffc3fee8
// Thread ready!
// bruh - 2
// Scheduling_List::remove(e=0x80800bf0) => {p=0x00000000,o=0x80800b1c,n=0x00000000
// }

// Thread(entry=0x800000ab,state=1,priority=1073741823,queue=3,stack={b=0xffc27bc8,
// s=16384},context={b=0xffc2bb94,{flags=0x200,ax=0,bx=0,cx=0,dx=0,si=0,di=0,bp=0x0
// 0000000,sp=0xffc3f6e4,ip=0x800000ab,cs=8,ccs=8,cds=10,ces=10,cfs=10,cgs=10,css=1
// 0,cr3=0x3fffc000}}) => 0x80800a4c@3
// CPU 0 Fila 0: CPU 0 CHOSEN: 0x00000000
// CPU 0 Fila 1: CPU 0 CHOSEN: 0x00000000
// CPU 0 Fila 2: CPU 0 CHOSEN: 0x00000000
// CPU 0 Fila 3: 0xffc3bd24 0x80800a4c 0x80800f38 CPU 0 CHOSEN: 0xffc3fee8
// Thread ready!

// Thread(entry=0x800000ab,state=1,priority=536870911,queue=2,stack={b=0xffc23ba8,s
// =16384},context={b=0xffc27b74,{flags=0x200,ax=0,bx=0,cx=0,dx=0,si=0,di=0,bp=0x00
// 000000,sp=0xffc3f6e4,ip=0x800000ab,cs=8,ccs=8,cds=10,ces=10,cfs=10,cgs=10,css=10
// ,cr3=0x3fffc000}}) => 0x808008a8@2
// CPU 0 Fila 0: CPU 0 CHOSEN: 0x00000000
// CPU 0 Fila 1: CPU 0 CHOSEN: 0x00000000
// CPU 0 Fila 2: CPU 0 CHOSEN: 0x808008a8
// CPU 0 Fila 3: 0xffc3bd24 0x80800a4c 0x80800f38 CPU 0 CHOSEN: 0xffc3fee8
// Thread ready!
// bruh - 4
// Scheduling_List::remove(e=0x808008a8) => {p=0x00000000,o=0x808007d4,n=0x00000000
// }

// Thread(entry=0x800000ab,state=1,priority=0,queue=2,stack={b=0xffc1fb88,s=16384},
// context={b=0xffc23b54,{flags=0x200,ax=0,bx=0,cx=0,dx=0,si=0,di=0,bp=0x00000000,s
// p=0xffc3f6e4,ip=0x800000ab,cs=8,ccs=8,cds=10,ces=10,cfs=10,cgs=10,css=10,cr3=0x3
// fffc000}}) => 0x80800704@2
// CPU 0 Fila 0: CPU 0 CHOSEN: 0x00000000
// CPU 0 Fila 1: CPU 0 CHOSEN: 0x00000000
// CPU 0 Fila 2: CPU 0 CHOSEN: 0x80800704
// CPU 0 Fila 3: 0xffc3bd24 0x80800a4c 0x80800f38 CPU 0 CHOSEN: 0xffc3fee8
// Thread ready!
// bruh - 5
// Scheduling_List::remove(e=0x80800704) => {p=0x00000000,o=0x80800630,n=0x00000000
// }

// Thread(entry=0x800001aa,state=2,priority=0,queue=3,stack={b=0xffc1bb68,s=16384},
// context={b=0xffc1fb34,{flags=0x200,ax=0,bx=0,cx=0,dx=0,si=0,di=0,bp=0x00000000,s
// p=0xffc3f6e4,ip=0x800001aa,cs=8,ccs=8,cds=10,ces=10,cfs=10,cgs=10,css=10,cr3=0x3
// fffc000}}) => 0x80800524@3
// Thread not ready!
// Scheduling_List::remove(e=0x80800524) => {p=0x00000000,o=0x80800450,n=0x80800f38
// }
// CPU 0 Fila 0: CPU 0 CHOSEN: 0x00000000
// CPU 0 Fila 1: CPU 0 CHOSEN: 0x00000000
// CPU 0 Fila 2: CPU 0 CHOSEN: 0x00000000
// CPU 0 Fila 3: 0xffc3bd24 0x80800a4c 0x80800f38 CPU 0 CHOSEN: 0xffc3fee8

// Thread(entry=0x800001aa,state=2,priority=-1,queue=32,stack={b=0xffc17b48,s=16384
// },context={b=0xffc1bb14,{flags=0x200,ax=0,bx=0,cx=0,dx=0,si=0,di=0,bp=0x00000000
// ,sp=0xffc3f6e4,ip=0x800001aa,cs=8,ccs=8,cds=10,ces=10,cfs=10,cgs=10,css=10,cr3=0
// x3fffc000}}) => 0x80800344@32
// Thread not ready!
// Scheduling_List::remove(e=0x80800344) => {p=0x00000000,o=0x80800270,n=0x00000000
// }
// CPU 0 Fila 0: CPU 0 CHOSEN: 0x00000000
// CPU 0 Fila 1: CPU 0 CHOSEN: 0x00000000
// CPU 0 Fila 2: CPU 0 CHOSEN: 0x00000000
// CPU 0 Fila 3: 0xffc3bd24 0x80800a4c 0x80800f38 0x80800524 CPU 0 CHOSEN: 0xffc3fe
// e8

// Thread(entry=0x800001aa,state=2,priority=-1,queue=0,stack={b=0xffc13b28,s=16384}
// ,context={b=0xffc17af4,{flags=0x200,ax=0,bx=0,cx=0,dx=0,si=0,di=0,bp=0x00000000,
// sp=0xffc3f6e4,ip=0x800001aa,cs=8,ccs=8,cds=10,ces=10,cfs=10,cgs=10,css=10,cr3=0x
// 3fffc000}}) => 0x80800164@0
// Thread not ready!
// Scheduling_List::remove(e=0x80800164) => {p=0x00000000,o=0x80800090,n=0x00000000
// }
// CPU 0 Fila 0: CPU 0 CHOSEN: 0x00000000
// CPU 0 Fila 1: CPU 0 CHOSEN: 0x00000000
// CPU 0 Fila 2: CPU 0 CHOSEN: 0x00000000
// CPU 0 Fila 3: 0xffc3bd24 0x80800a4c 0x80800f38 0x80800524 CPU 0 CHOSEN: 0xffc3fe
// e8
// poggers - 8
// Scheduling_List::remove(e=0x80800164) => {p=0x00000000,o=0x80800090,n=0x00000000
// }
// Thread::join(this=0x80800e64,state=1)
// Scheduling_List::remove(e=0xffc3fee8) => {p=0x00000000,o=0xffc3fe14,n=0x80800524
// }
// PROXIMO THREAD: 0x80800524
// poggers - 6
// Scheduling_List::remove(e=0x80800524) => {p=0x00000000,o=0x80800450,n=0x80800f38
// }
// bruh - 0
// Scheduling_List::remove(e=0x80800f38) => {p=0x00000000,o=0x80800e64,n=0x80800a4c
// }
// Thread::join(this=0x80800cc0,state=4)
// Thread::join(this=0x80800b1c,state=4)
// Thread::join(this=0x80800978,state=1)
// Scheduling_List::remove(e=0xffc3fee8) => {p=0x00000000,o=0xffc3fe14,n=0x80800a4c
// }
// PROXIMO THREAD: 0x80800f38
// bruh - 3
// Scheduling_List::remove(e=0x80800a4c) => {p=0x00000000,o=0x80800978,n=0x80800f38
// }
// Thread::join(this=0x808007d4,state=4)
// Thread::join(this=0x80800630,state=4)
// Thread::join(this=0x80800450,state=3)
// Scheduling_List::remove(e=0xffc3fee8) => {p=0x00000000,o=0xffc3fe14,n=0x00000000
// }
// PROXIMO THREAD: 0x80800f38
// poggers - 6
// Scheduling_List::remove(e=0x80800524) => {p=0x00000000,o=0x80800450,n=0x80800f38
// }
// poggers - 8
// Scheduling_List::remove(e=0x80800164) => {p=0x00000000,o=0x80800090,n=0x00000000
// }
// Scheduling_List::remove(e=0x80800524) => {p=0x00000000,o=0x80800450,n=0x80800f38
// }
// Thread::join(this=0x80800270,state=1)
// Scheduling_List::remove(e=0xffc3fee8) => {p=0x00000000,o=0xffc3fe14,n=0x00000000
// }
// PROXIMO THREAD: 0xffc3bd24
// Scheduling_List::remove(e=0x80800164) => {p=0x00000000,o=0x80800090,n=0x00000000
// }
