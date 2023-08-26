 bool
TestMemory()
{
    uint cBytes = Max(Kilobytes(16), CONST::cBytesMinimumRegion);
    u8 * buffer = new u8[cBytes];
    
    Memory_Region programMemory = BeginRootMemory_Region(buffer, cBytes);
    DoTest(programMemory);
    
    u8 * memory0 = (u8 *)allocate(programMemory, 135);
    u8 * memory1 = (u8 *)allocate(programMemory, 206);
    u8 * memory2 = (u8 *)allocate(programMemory, 112);

    // NOTE - These tests will change once alignment is implemented!
    
    DoTest(memory0 == (u8 *)programMemory + sizeof(Memory_Region_Header));
    DoTest(memory1 - memory0 == 135);
    DoTest(memory2 - memory1 == 206);
    
    DoTest(g_cntNew == 1);
    
    u8 * memoryOverflow = (u8 *)allocate(programMemory, (uint)Kilobytes(32));
    DoTest(g_cntNew == 2);
    
    DoTest(mem_region_end(programMemory));
    DoTestAuditLeaks();
    AllTestsPass();
}
