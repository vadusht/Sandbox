#include <OAIdl.h>
#include <stdio.h>

void Sort(SAFEARRAY* safearray)
{
    int HUGEP* data;
    
    HRESULT result = SafeArrayAccessData(safearray, (void HUGEP**)&data);
    
    if(SUCCEEDED(result))
    {
        for(int index = 0; index < safearray->rgsabound[0].cElements - 1; index++)
        {
            printf("%d %d\n", index, data[index]);
        }
    }
}

int main(int argc, char* argv[])
{
    {
        SAFEARRAY* safearray;
        unsigned int dimensions_number = 2;
        HRESULT result = SafeArrayAllocDescriptor(dimensions_number, &safearray);
        if(SUCCEEDED(result))
        {
            safearray->rgsabound[0].lLbound = 0;
            safearray->rgsabound[0].cElements = 5;
            safearray->rgsabound[1].lLbound = 1;
            safearray->rgsabound[1].cElements = 4;
            result = SafeArrayAllocData(safearray);
            if(FAILED(result))
            {
                SafeArrayDestroyDescriptor(safearray);
            }
        }
    }
    
    {
        SAFEARRAY* safearray;
        unsigned int dimensions_number = 2;
        HRESULT result = SafeArrayAllocDescriptorEx(VT_I4, dimensions_number, &safearray);
        if(SUCCEEDED(result))
        {
            safearray->rgsabound[0].lLbound = 0;
            safearray->rgsabound[0].cElements = 5;
            safearray->rgsabound[1].lLbound = 1;
            safearray->rgsabound[1].cElements = 4;
            result = SafeArrayAllocData(safearray);
            if(FAILED(result))
            {
                SafeArrayDestroyDescriptor(safearray);
            }
        }
    }
    
    
    {
        UINT safearray_size = 10;
        SAFEARRAY* safearray = SafeArrayCreateVector(VT_I4, 0, safearray_size);
        HRESULT result = SafeArrayLock(safearray);
        if(SUCCEEDED(result))
        {
            for(int index = 0; index < safearray_size; index++)
            {
                ((int*)(safearray->pvData))[index] = index;
            }
        }
        result = SafeArrayUnlock(safearray);
        
        SAFEARRAY* safearray_copy;
        result = SafeArrayCopy(safearray, &safearray_copy);
        if(FAILED(result))
        {
            SafeArrayDestroy(safearray);
        }
    }
    
    
    {
        UINT safearray_size = 10;
        SAFEARRAY* safearray = SafeArrayCreateVector(VT_I4, 0, safearray_size);
        HRESULT result = SafeArrayLock(safearray);
        if(SUCCEEDED(result))
        {
            for(int index = 0; index < safearray_size; index++)
            {
                ((int*)(safearray->pvData))[index] = index;
            }
        }
        result = SafeArrayUnlock(safearray);
        
        SAFEARRAY* safearray_copy = SafeArrayCreateVector(VT_I4, 0, safearray_size);
        UINT safearray_copy_dimension = SafeArrayGetDim(safearray_copy);
        result = SafeArrayCopyData(safearray, safearray_copy);
        
        if(SUCCEEDED(result))
        {
            int element;
            for(LONG index = 0; index < safearray_size; index++)
            {
                result = SafeArrayGetElement(safearray_copy, &index, &element);
            }
            
            UINT element_size = SafeArrayGetElemsize(safearray_copy);
            
            LONG lower_bound;
            result = SafeArrayGetLBound(safearray_copy, 1, &lower_bound);
            
            LONG upper_bound;
            result = SafeArrayGetUBound(safearray_copy, 1, &upper_bound);
            
            VARTYPE vartype;
            result = SafeArrayGetVartype(safearray_copy, &vartype); 
            
            int b = 30;
        }
    }
}