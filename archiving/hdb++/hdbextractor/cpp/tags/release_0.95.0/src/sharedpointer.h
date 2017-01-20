#ifndef SHAREDPOINTER_H
#define SHAREDPOINTER_H

// #include <stdio.h>

class RC
{
    private:
    int count; // Reference count

    public:
    void AddRef()
    {
        // Increment the reference count
        count++;
    }

    int Release()
    {
        // Decrement the reference count and
        // return the reference count.
        return --count;
    }

    int Count() const
    {
        return count;
    }
};

template < typename T > class SharedPointer
{
private:
    T*    pData;       // pointer
    RC* reference; // Reference count

public:
    SharedPointer() : reference(0)
    {
        pData = 0;
        // Create a new reference
        reference = new RC();
        // Increment the reference count
        reference->AddRef();
      //  printf("\e[1;32m+ SharedPointer() pData NULL %p this %p referencd %p\e[0m\n", pData, this, reference);
    }

    SharedPointer(T* pValue) : pData(pValue), reference(0)
    {
        // Create a new reference
        reference = new RC();
        // Increment the reference count
        reference->AddRef();
      //  printf("\e[1;32m+ SharedPointer(T* )  %pthis %p referencd %p\e[0m\n", pData, this, reference);
    }

    SharedPointer(const SharedPointer<T>& sp) : pData(sp.pData), reference(sp.reference)
    {
        // Copy constructor
        // Copy the data and reference pointer
        // and increment the reference count
        reference->AddRef();
     //   printf("\e[1;32m+ SharedPointer(COPY COINSTRUTCTROR)  %p this %p referencd %p\e[0m\n", pData, this, reference);
    }

    ~SharedPointer()
    {
 //       printf("\e[1;31m~ deleting 0x%p  pData %p\e[0m\n", this, pData);
        // Destructor
        // Decrement the reference count
        // if reference become zero delete the data
        if(reference->Release() == 0)
        {
 //           printf("\e[1;31m~ releasing %s\e[0m\n", (char *) pData);
            delete pData;
            delete reference;
        }
    }

    T& operator* ()
    {
        return *pData;
    }

    T* operator-> ()
    {
        return pData;
    }

    SharedPointer<T>& operator = (const SharedPointer<T>& sp)
    {
        // Assignment operator
        if (this != &sp) // Avoid self assignment
        {
            // Decrement the old reference count
            // if reference become zero delete the old data
            if(reference->Release() == 0)
            {
       //         printf("operator = releasing \"%s\", %p ref %p\n", (char *)pData,  pData, reference);
     //           if(pData == NULL)
     //               printf("\e[1;35mDELETING NULL pData!\e[0m\n");
                delete pData;
                delete reference;
            }

            // Copy the data and reference pointer
            // and increment the reference count
            pData = sp.pData;
            reference = sp.reference;
            reference->AddRef();
      //      printf("operator = new ref %s count %d\n", (char *) pData, reference->Count());
        }
        return *this;
    }
};

#endif // SHAREDPOINTER_H
