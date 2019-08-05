#include <iostream>
#include <list>
#include <typeinfo>
#include <cstdlib>
#include "gc_details.h"
#include "gc_iterator.h"
/*
    Pointer implements a pointer type that uses
    garbage collection to release unused memory.
    A Pointer must only be used to point to memory
    that was dynamically allocated using new.
    When used to refer to an allocated array,
    specify the array size.
*/
template <class T, int size = 0>
class Pointer{
private:
    // refContainer maintains the garbage collection list.
    static std::list<PtrDetails<T> > refContainer;
    // addr points to the allocated memory to which
    // this Pointer pointer currently points.
    T *addr;
    /*  isArray is true if this Pointer points
        to an allocated array. It is false
        otherwise. 
    */
    bool isArray; 
    // true if pointing to array
    // If this Pointer is pointing to an allocated
    // array, then arraySize contains its size.
    unsigned arraySize; // size of the array
    static bool first; // true when first Pointer is created
    // Return an iterator to pointer details in refContainer.
    typename std::list<PtrDetails<T> >::iterator findPtrInfo(T *ptr);
public:
    // Define an iterator type for Pointer<T>.
    typedef Iter<T> GCiterator;
    // Empty constructor
    // NOTE: templates aren't able to have prototypes with default arguments
    // this is why constructor is designed like this:
    Pointer(){
        Pointer(NULL);
    }
    Pointer(T*);

    // Copy constructor.
    Pointer(const Pointer &);

    // Destructor for Pointer.
    ~Pointer();
    // Collect garbage. Returns true if at least
    // one object was freed.

    static bool collect();
    // Overload assignment of pointer to Pointer.
    T *operator=(T *t);

    // Overload assignment of Pointer to Pointer.
    Pointer &operator=(Pointer &rv);
    // Return a reference to the object pointed
    // to by this Pointer.
    T &operator*(){
        return *addr;
    }

    // Return the address being pointed to.
    T *operator->() { return addr; }
    // Return a reference to the object at the
    // index specified by i.

    T &operator[](int i){ return addr[i];}
    // Conversion function to T *.
    operator T *() { return addr; }

    // Return an Iter to the start of the allocated memory.
    Iter<T> begin(){
        int _size;
        if (isArray)
            _size = arraySize;
        else
            _size = 1;
        return Iter<T>(addr, addr, addr + _size);
    }
    // Return an Iter to one past the end of an allocated array.
    Iter<T> end(){
        int _size;
        if (isArray)
            _size = arraySize;
        else
            _size = 1;
        return Iter<T>(addr + _size, addr, addr + _size);
    }
    // Return the size of refContainer for this type of Pointer.
    static int refContainerSize() { return refContainer.size(); }
    // A utility function that displays refContainer.
    static void showlist();
    // Clear refContainer when program exits.
    static void shutdown();
};

// STATIC INITIALIZATION
// Creates storage for the static variables
template <class T, int size>
std::list<PtrDetails<T> > Pointer<T, size>::refContainer;

template <class T, int size>
bool Pointer<T, size>::first = true;

// Constructor for both initialized and uninitialized objects. -> see class interface
template<class T,int size>
Pointer<T,size>::Pointer(T *t){
    // Register shutdown() as an exit function.
    if (first)
        atexit(shutdown);
    first = false;

    // MARK: Implement Pointer constructor
    // Lab: Smart Pointer Project Lab
    // Check to see if ptr address already exists in ref container
    // ?? Does the iterator return a single instance or a list?
    // Assuming an instance based on given code.
    typename std::list<PtrDetails<T> >::iterator p = findPtrInfo(t);

    // Check for null
    if (p == refContainer.end()) {
        // If null, create new instance
        PtrDetails<T> new_p(t, size);
        // If not already in ref container add
        refContainer.push_back(new_p);
    } else {
        
    }

    // increase ref count by one
    p->refcount++;

    // set address attribute
    addr = t;

    // Check for array and set attributes
    arraySize = size;
    isArray = ((arraySize > 0) ? true : false);

}
// Copy constructor.
template< class T, int size>
Pointer<T,size>::Pointer(const Pointer &ob){

    // MARK: Pointer Constructor Implementation
    typename std::list<PtrDetails<T> >::iterator p = findPtrInfo(ob.addr);

    // Increment ref count
    p->refcount++;

    // Save memory address for later use
    // should this be saved to the new instance?
    addr = ob.addr;

    // set array size
    arraySize = ob.arraySize;

    // Check for array
    isArray = ((ob.arraySize > 0) ? true : false);

}

// Destructor for Pointer.
template <class T, int size>
Pointer<T, size>::~Pointer(){

    // MARK: Pointer Destructor Implementation
    typename std::list<PtrDetails<T>>::iterator p;
    p = findPtrInfo(addr);
    if (p->refcount) {
        p->refcount--;
    }

    collect();
}

// Collect garbage. Returns true if at least
// one object was freed.
template <class T, int size>
bool Pointer<T, size>::collect(){

    // MARK: Collection Implementation
    bool memfreed = false;
    typename std::list<PtrDetails<T>>::iterator p;

    do {
        for (p = refContainer.begin(); p != refContainer.end(); p++) {
            if (p->refcount > 0) {
                continue;
            }

            memfreed = true;
            refContainer.remove(*p);

            if(p->memPtr) {
                if (p->isArray) {
                    delete[] p->memPtr;
                } else {
                    delete p->memPtr;
                }
            }
            break;
        }
    }

    while (p != refContainer.end());
    return memfreed;
}

// Overload assignment of pointer to Pointer.
template <class T, int size>
T *Pointer<T, size>::operator=(T *t) {

    // MARK: Implement operator==
    typename std::list<PtrDetails<T> >::iterator p = findPtrInfo(addr);

    // decrement current ref if found
    if (p != refContainer.end()) { p->refcount--; }

    // Check if new pointer address is already in container
    p = findPtrInfo(t);
    if (p == refContainer.end()) {
        // If null, create new instance
        PtrDetails<T> new_p(t, size);
        // If not already in ref container add
        refContainer.push_back(new_p);
    }

    // set various parameters
    addr = t;
    arraySize = size;
    isArray = (arraySize > 0) ? true : false;
    p->refcount++;

}

// Overload assignment of Pointer to Pointer.
template <class T, int size>
Pointer<T, size> &Pointer<T, size>::operator=(Pointer &rv){

    // MARK: Implement operator==
    typename std::list<PtrDetails<T> >::iterator p;

    // decrement current
    p = findPtrInfo(addr);
    p->refcount--;
    
    // new address
    p = findPtrInfo(rv.addr);
    addr = rv.addr;
    arraySize = rv.arraySize;
    isArray = (arraySize > 0) ? true : false;
    p->refcount++;

}

// A utility function that displays refContainer.
template <class T, int size>
void Pointer<T, size>::showlist(){
    typename std::list<PtrDetails<T> >::iterator p;
    std::cout << "refContainer<" << typeid(T).name() << ", " << size << ">:\n";
    std::cout << "memPtr refcount value\n ";
    if (refContainer.begin() == refContainer.end())
    {
        std::cout << " Container is empty!\n\n ";
    }
    for (p = refContainer.begin(); p != refContainer.end(); p++)
    {
        std::cout << "[" << (void *)p->memPtr << "]"
             << " " << p->refcount << " ";
        if (p->memPtr)
            std::cout << " " << *p->memPtr;
        else
            std::cout << "---";
        std::cout << std::endl;
    }
    std::cout << std::endl;
}
// Find a pointer in refContainer.
template <class T, int size>
typename std::list<PtrDetails<T> >::iterator
Pointer<T, size>::findPtrInfo(T *ptr){
    typename std::list<PtrDetails<T> >::iterator p;
    // Find ptr in refContainer.
    for (p = refContainer.begin(); p != refContainer.end(); p++)
        if (p->memPtr == ptr)
            return p;
    return p;
}
// Clear refContainer when program exits.
template <class T, int size>
void Pointer<T, size>::shutdown(){
    if (refContainerSize() == 0)
        return; // list is empty
    typename std::list<PtrDetails<T> >::iterator p;
    for (p = refContainer.begin(); p != refContainer.end(); p++)
    {
        // Set all reference counts to zero
        p->refcount = 0;
    }
    collect();
}