bool TestDynArray()
{
    Memory_Region memory;
    Defer(mem_region_end(memory));
    {
        uint cBytes = Max(Kilobytes(1), CONST::cBytesMinimumRegion);
        u8 * buffer = new u8[cBytes];
        memory = BeginRootMemory_Region(buffer, cBytes);
        DoTest(memory);
    }
    
    DynArray<int> array = {};
    array.memory = memory;
    
    EnsureCapacity(&array, 5);
    DoTest(array.capacity >= 5);
    
    for (int i = 0; i < 12; i++)
    {
        Append(&array, i);
    }
    DoTest(array.count == 12);
    DoTest(array.capacity >= array.count);

    int iIter = 0;
    for (int n : array)
    {
        DoTest(n == iIter);
        iIter++;
    }

    Remove(&array, 2);          // 00 01 03 04 05 06 07 08 09 10 11
    Remove(&array, 4);          // 00 01 03 04 06 07 08 09 10 11
    Remove(&array, 6);          // 00 01 03 04 06 07 09 10 11
    RemoveUnordered(&array, 1); // 00 11 03 04 06 07 09 10

    DoTest(array.count == 8);
    DoTest(array[0] == 0);
    DoTest(array[1] == 11);
    DoTest(array[2] == 3);
    DoTest(array[3] == 4);
    DoTest(array[4] == 6);
    DoTest(array[5] == 7);
    DoTest(array[6] == 9);
    DoTest(array[7] == 10);

    for (int* n : ByPtr(array))
    {
        *n = 3;
    }

    for (int n : array)
    {
        DoTest(n == 3);
    }

    AllTestsPass();
}
